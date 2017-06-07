#include "include/wl_def.h"

#include <vitasdk.h>
//#include <pthread.h>
//#include <sys/ioctl.h>
//#include <sys/soundcard.h>
#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <unistd.h>

#include "include/fmopl.h"

#define PACKED __attribute__((packed))

typedef	struct {
	longword length;
	word priority;
} PACKED SoundCommon;

typedef	struct {
	SoundCommon common;
	byte data[1];
} PACKED PCSound;

typedef	struct {
	byte mChar, cChar, mScale, cScale, mAttack, cAttack, mSus, cSus,
		mWave, cWave, nConn, voice, mode, unused[3];
} PACKED Instrument;

typedef	struct {
	SoundCommon common;
	Instrument inst;
	byte block, data[1];
} PACKED AdLibSound;

typedef	struct {
	word length, values[1];
} PACKED MusicGroup;

boolean AdLibPresent=true, SoundBlasterPresent=true;

SDMode SoundMode;
SMMode MusicMode;
SDSMode DigiMode;

static int Volume;
static int leftchannel, rightchannel;
static int L, R;

static volatile boolean sqActive;

static word *DigiList;

static boolean SD_Started;

#define NUM_SFX 8

static volatile boolean SoundPositioned[NUM_SFX];
static volatile int SoundPlaying[NUM_SFX];
static volatile int SoundPlayPos[NUM_SFX];
static volatile int SoundPlayLen[NUM_SFX];
static volatile int SoundPage[NUM_SFX];
static volatile int SoundLen[NUM_SFX];
static volatile byte *SoundData[NUM_SFX];
static volatile fixed SoundGX[NUM_SFX];
static volatile fixed SoundGY[NUM_SFX];
static volatile int CurDigi[NUM_SFX];

static FM_OPL *OPL;
static MusicGroup *Music;
static volatile int NewMusic;
static volatile int NewAdlib;
static volatile int AdlibPlaying;
static volatile int CurAdlib;

static boolean SPHack;

#define NUM_SAMPS 512

static short int vitabuf[NUM_SAMPS * 2];
static short int musbuf[NUM_SAMPS];
static int bufpos;

static int MusicLength;
static int MusicCount;
static word *MusicData;
static byte AdlibBlock;
static byte *AdlibData;
static int AdlibLength;


static void SetSoundLoc(fixed gx, fixed gy);
static boolean SD_PlayDirSound(soundnames sound, fixed gx, fixed gy);


void SD_SetVolume(int vol)
{
	Volume = vol;
}

int SD_GetVolume()
{
	return Volume;
}

void InitSoundBuff(void)
{
	MusicLength = 0;
	MusicCount = 0;
	MusicData = NULL;
	AdlibBlock = 0;
	AdlibData = NULL;
	AdlibLength = -1;

	OPLWrite(OPL, 0x01, 0x20); /* Set WSE=1 */
	OPLWrite(OPL, 0x08, 0x00); /* Set CSM=0 & SEL=0 */
}

void FillSoundBuff(void)
{
	int i, j, snd;
	short int samp;
	word dat;

	AdLibSound *AdlibSnd;
	Instrument *inst;

	// start by computing next buffer worth of music
	if ((MusicMode!=smm_Off) || (SoundMode!=sdm_Off)) {
		if (NewAdlib != -1) {
			AdlibPlaying = NewAdlib;
			AdlibSnd = (AdLibSound *)audiosegs[STARTADLIBSOUNDS+AdlibPlaying];
			inst = (Instrument *)&AdlibSnd->inst;
#define alChar		0x20
#define alScale		0x40
#define alAttack	0x60
#define alSus		0x80
#define alFeedCon	0xC0
#define alWave		0xE0

			OPLWrite(OPL, 0 + alChar, 0);
			OPLWrite(OPL, 0 + alScale, 0);
			OPLWrite(OPL, 0 + alAttack, 0);
			OPLWrite(OPL, 0 + alSus, 0);
			OPLWrite(OPL, 0 + alWave, 0);
			OPLWrite(OPL, 3 + alChar, 0);
			OPLWrite(OPL, 3 + alScale, 0);
			OPLWrite(OPL, 3 + alAttack, 0);
			OPLWrite(OPL, 3 + alSus, 0);
			OPLWrite(OPL, 3 + alWave, 0);
			OPLWrite(OPL, 0xA0, 0);
			OPLWrite(OPL, 0xB0, 0);

			OPLWrite(OPL, 0 + alChar, inst->mChar);
			OPLWrite(OPL, 0 + alScale, inst->mScale);
			OPLWrite(OPL, 0 + alAttack, inst->mAttack);
			OPLWrite(OPL, 0 + alSus, inst->mSus);
			OPLWrite(OPL, 0 + alWave, inst->mWave);
			OPLWrite(OPL, 3 + alChar, inst->cChar);
			OPLWrite(OPL, 3 + alScale, inst->cScale);
			OPLWrite(OPL, 3 + alAttack, inst->cAttack);
			OPLWrite(OPL, 3 + alSus, inst->cSus);
			OPLWrite(OPL, 3 + alWave, inst->cWave);

			//OPLWrite(OPL, alFeedCon, inst->nConn);
			OPLWrite(OPL, alFeedCon, 0);

			AdlibBlock = ((AdlibSnd->block & 7) << 2) | 0x20;
			AdlibData = (byte *)&AdlibSnd->data;
			AdlibLength = AdlibSnd->common.length*5;
			//OPLWrite(OPL, 0xB0, AdlibBlock);
			NewAdlib = -1;
		}

		if (NewMusic != -1) {
			NewMusic = -1;
			MusicLength = Music->length;
			MusicData = Music->values;
			MusicCount = 0;
		}
		for (i = 0; i < 4; i++) {
			if (sqActive) {
				while (MusicCount <= 0) {
					dat = *MusicData++;
					MusicCount = *MusicData++;
					MusicLength -= 4;
					OPLWrite(OPL, dat & 0xFF, dat >> 8);
				}
				if (MusicLength <= 0) {
					NewMusic = 1;
				}
				MusicCount-=2;
			}

			if (AdlibPlaying != -1) {
				if (AdlibLength == 0) {
					//OPLWrite(OPL, 0xB0, AdlibBlock);
				} else if (AdlibLength == -1) {
					OPLWrite(OPL, 0xA0, 00);
					OPLWrite(OPL, 0xB0, AdlibBlock);
					AdlibPlaying = -1;
				} else if ((AdlibLength % 5) == 0) {
					OPLWrite(OPL, 0xA0, *AdlibData);
					OPLWrite(OPL, 0xB0, AdlibBlock & ~2);
					AdlibData++;
				}
				AdlibLength--;
				if (AdlibLength == 0) {
					//OPLWrite(OPL, 0xB0, AdlibBlock);
				} else if (AdlibLength == -1) {
					OPLWrite(OPL, 0xA0, 00);
					OPLWrite(OPL, 0xB0, AdlibBlock);
					AdlibPlaying = -1;
				} else if ((AdlibLength % 5) == 0) {
					OPLWrite(OPL, 0xA0, *AdlibData);
					OPLWrite(OPL, 0xB0, AdlibBlock & ~2);
					AdlibData++;
				}
				AdlibLength--;
			}

			YM3812UpdateOne(OPL, &musbuf[i*NUM_SAMPS/4], NUM_SAMPS/4);
		}
	}

	//prefill audio buffer with music
	for (i = 0; i < NUM_SAMPS*2; i++)
		vitabuf[i] = musbuf[i/2];

	// now add in sound effects
	for (j=0; j<NUM_SFX; j++) {
		if (SoundPlaying[j] == -1)
			continue;

		// compute L/R scale based on player position and stored sound position
		if (SoundPositioned[j]) {
			SetSoundLoc(SoundGX[j], SoundGY[j]);
			L = leftchannel;
			R = rightchannel;
		}

		// now add sound effect data to buffer
		for (i = 0; i < NUM_SAMPS*2; i += 2) {
			samp = (SoundData[j][(SoundPlayPos[j] >> 16)] << 8)^0x8000;
			if (SoundPositioned[j]) {
				snd = samp*(16-L)>>5;
				vitabuf[i+0] += snd;
				snd = samp*(16-R)>>5;
				vitabuf[i+1] += snd;
			} else {
				snd = samp>>2;
				vitabuf[i+0] += snd;
				vitabuf[i+1] += snd;
			}
			SoundPlayPos[j] += 10402; // 7000 / 44100 * 65536
			if ((SoundPlayPos[j] >> 16) >= SoundPlayLen[j]) {
				SoundPlayPos[j] -= (SoundPlayLen[j] << 16);
				SoundLen[j] -= 4096;
				SoundPlayLen[j] = (SoundLen[j] < 4096) ? SoundLen[j] : 4096;
				if (SoundLen[j] <= 0) {
					SoundPlaying[j] = -1;
					CurDigi[j] = -1;
					i = NUM_SAMPS*2;
				} else {
					SoundPage[j]++;
					SoundData[j] = PM_GetSoundPage(SoundPage[j]);
				}
			}
		}
	}
}

// Audio thread code
static int audioThread(unsigned int args, void* arg){
	
	// Initializing audio port
	int ch = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, NUM_SAMPS, 44100, SCE_AUDIO_OUT_MODE_STEREO);
	sceAudioOutSetConfig(ch, -1, -1, -1);
	int vol_stereo[] = {32767, 32767};
	sceAudioOutSetVolume(ch, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, vol_stereo);
	
		
	// Audio playback loop
	for (;;){
		
		FillSoundBuff();
		sceAudioOutOutput(ch, vitabuf);			
			
	}
	
}

void Blah()
{
    memptr  list;
    word    *p, pg;
    int     i;

	if (DigiList!=NULL)
		free(DigiList);

    MM_GetPtr(&list,PMPageSize);
    p = PM_GetPage(ChunksInFile - 1);
    memcpy((void *)list,(void *)p,PMPageSize);

    pg = PMSoundStart;
    for (i = 0;i < PMPageSize / (sizeof(word) * 2);i++,p += 2)
    {
	    if (pg >= ChunksInFile - 1)
        	break;
        pg += (p[1] + (PMPageSize - 1)) / PMPageSize;
    }
    MM_GetPtr((memptr *)&DigiList, i * sizeof(word) * 2);
    memcpy((void *)DigiList, (void *)list, i * sizeof(word) * 2);
    MM_FreePtr(&list);
}

SceUID AudioThread;

void SD_Startup()
{
	int i;

	if (SD_Started)
		return;

	Blah();
	InitDigiMap();
	OPL = OPLCreate(OPL_TYPE_YM3812, 3579545, 44100);

	for (i=0; i<NUM_SFX; i++) {
		SoundPlaying[i] = -1;
		CurDigi[i] = -1;
	}

	CurAdlib = -1;
	NewAdlib = -1;
	NewMusic = -1;
	AdlibPlaying = -1;
	sqActive = false;
	
	InitSoundBuff();
	
	// Starting audio thread
	AudioThread = sceKernelCreateThread("Audio Thread", &audioThread, 0x10000100, 0x10000, 0, 0, NULL);
	int res = sceKernelStartThread(AudioThread, sizeof(AudioThread), &AudioThread);
	if (res != 0) {
		printf("failed to init audio\n");
		return;
    }

	SD_Started = true;
}

void SD_Shutdown()
{
	if (!SD_Started)
		return;

	SD_MusicOff();
	SD_StopSound();

	SD_Started = false;
	OPLDestroy(OPL);
	sceKernelDeleteThread(AudioThread);
	
	
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_PlaySound() - plays the specified sound on the appropriate hardware
//
///////////////////////////////////////////////////////////////////////////
boolean SD_PlaySound(soundnames sound)
{
	SPHack = false;
	return SD_PlayDirSound(sound, 0, 0);
}

static boolean SD_PlayDirSound(soundnames sound, fixed gx, fixed gy)
{
	SoundCommon *s;

	s = (SoundCommon *)audiosegs[STARTADLIBSOUNDS + sound];

	if (DigiMode != sds_Off) {
	if (DigiMap[sound] != -1) {
		int i;

		for (i=0; i<NUM_SFX; i++)
			if (SoundPlaying[i] == -1)
				break;
		if (i == NUM_SFX)
			for (i=0; i<NUM_SFX; i++)
				if (s->priority >= ((SoundCommon *)audiosegs[STARTADLIBSOUNDS+CurDigi[i]])->priority)
					break;
		if (i == NUM_SFX)
			return false; // no free channels, and priority not high enough to preempt a playing sound

		SoundPage[i] = DigiList[(DigiMap[sound] * 2) + 0];
		SoundData[i] = PM_GetSoundPage(SoundPage[i]);
		SoundLen[i] = DigiList[(DigiMap[sound] * 2) + 1];
		SoundPlayLen[i] = (SoundLen[i] < 4096) ? SoundLen[i] : 4096;
		SoundPlayPos[i] = 0;
		if (SPHack) {
			SPHack = false;
			SoundGX[i] = gx;
			SoundGY[i] = gy;
			SoundPositioned[i] = true;
		} else {
			SoundPositioned[i] = false;
		}
		CurDigi[i] = sound;
		SoundPlaying[i] = DigiMap[sound];
		return true;
	}
	}

	if (SoundMode != sdm_Off) {
	if ((AdlibPlaying == -1) || (CurAdlib == -1) ||
	(s->priority >= ((SoundCommon *)audiosegs[STARTADLIBSOUNDS+CurAdlib])->priority) ) {
		CurAdlib = sound;
		NewAdlib = sound;
		return true;
	}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SoundPlaying() - returns the sound number of first playing sound,
//		or 0 if nothing playing
//
///////////////////////////////////////////////////////////////////////////
word SD_SoundPlaying()
{
	int i;

	for (i=0; i<NUM_SFX; i++)
		if (SoundPlaying[i] != -1)
			return CurDigi[i];

	if (AdlibPlaying != -1)
		return CurAdlib;

	return 0;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_StopSound() - if a sound is playing, stops it
//
///////////////////////////////////////////////////////////////////////////
void SD_StopSound()
{
	int i;

	for (i=0; i<NUM_SFX; i++) {
		SoundPlaying[i] = -1;
		CurDigi[i] = -1;
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_WaitSoundDone() - waits until the current sound is done playing
//
///////////////////////////////////////////////////////////////////////////
void SD_WaitSoundDone()
{
	while (SD_SoundPlaying())
		sceKernelDelayThread(1000);
}

/*
==========================
=
= SetSoundLoc - Given the location of an object, munges the values
=	for an approximate distance from the left and right ear, and puts
=	those values into leftchannel and rightchannel.
=
= JAB
=
==========================
*/

#define ATABLEMAX 15
static const byte righttable[ATABLEMAX][ATABLEMAX * 2] = {
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 6, 0, 0, 0, 0, 0, 1, 3, 5, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 6, 4, 0, 0, 0, 0, 0, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 4, 1, 0, 0, 0, 1, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 5, 4, 2, 1, 0, 1, 2, 3, 5, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 5, 4, 3, 2, 2, 3, 3, 5, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 4, 4, 4, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 5, 5, 6, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};
static const byte lefttable[ATABLEMAX][ATABLEMAX * 2] = {
{ 8, 8, 8, 8, 8, 8, 8, 8, 5, 3, 1, 0, 0, 0, 0, 0, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 0, 0, 0, 0, 0, 4, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 1, 0, 0, 0, 1, 4, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 5, 3, 2, 1, 0, 1, 2, 4, 5, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 5, 3, 3, 2, 2, 3, 4, 5, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 5, 4, 4, 4, 4, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 6, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};

static void SetSoundLoc(fixed gx, fixed gy)
{
	fixed xt, yt;
	int x, y;

// translate point to view centered coordinates
//
	gx -= viewx;
	gy -= viewy;

//
// calculate newx
//
	xt = FixedByFrac(gx,viewcos);
	yt = FixedByFrac(gy,viewsin);
	x = (xt - yt) >> TILESHIFT;

//
// calculate newy
//
	xt = FixedByFrac(gx,viewsin);
	yt = FixedByFrac(gy,viewcos);
	y = (yt + xt) >> TILESHIFT;

	if (y >= ATABLEMAX)
		y = ATABLEMAX - 1;
	else if (y <= -ATABLEMAX)
		y = -ATABLEMAX;
	if (x < 0)
		x = -x;
	if (x >= ATABLEMAX)
		x = ATABLEMAX - 1;

	leftchannel  =  lefttable[x][y + ATABLEMAX];
	rightchannel = righttable[x][y + ATABLEMAX];
}

/*
==========================
=
= SetSoundLocGlobal - Sets up globalsoundx & globalsoundy and then calls
=	UpdateSoundLoc() to transform that into relative channel volumes. Those
=	values are then passed to the Sound Manager so that they'll be used for
=	the next sound played (if possible).
=
==========================
*/

void PlaySoundLocGlobal(word s, int id, fixed gx, fixed gy)
{
	SPHack = true;
	SD_PlayDirSound(s, gx, gy);
}

// tell sound driver the current player moved/turned
void UpdateSoundLoc(fixed x, fixed y, int angle)
{
	// SetSoundLoc uses viewx, viewy, viewcos, viewsin in FillBuffer
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicOn() - turns on the sequencer
//
///////////////////////////////////////////////////////////////////////////
void SD_MusicOn()
{
	sqActive = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicOff() - turns off the sequencer and any playing notes
//
///////////////////////////////////////////////////////////////////////////
void SD_MusicOff()
{
	int j;

	sqActive = false;
	OPLResetChip(OPL);

	for (j=0; j<NUM_SAMPS; j++)
		musbuf[j] = 0;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_StartMusic() - starts playing the music pointed to
//
///////////////////////////////////////////////////////////////////////////
void SD_StartMusic(int music)
{
	music += STARTMUSIC;

	CA_CacheAudioChunk(music);

	if (MusicMode!=sdm_Off) {
		SD_MusicOff();
		SD_MusicOn();
	}

	Music = (MusicGroup *)audiosegs[music];
	NewMusic = 1;
}

void SD_SetDigiDevice(SDSMode mode)
{
	if ((mode==sds_Off) || (mode==sds_SoundBlaster)) {
		DigiMode=mode;
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetSoundMode() - Sets which sound hardware to use for sound effects
//
///////////////////////////////////////////////////////////////////////////
boolean SD_SetSoundMode(SDMode mode)
{
	if ((mode==sdm_Off) || (mode==sdm_AdLib)) {
		SoundMode=mode;
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetMusicMode() - sets the device to use for background music
//
///////////////////////////////////////////////////////////////////////////
boolean SD_SetMusicMode(SMMode mode)
{
	if (mode==smm_Off) {
		MusicMode=mode;
		SD_MusicOff();
		return true;
	}
	if (mode==smm_AdLib) {
		MusicMode=mode;
		//SD_MusicOn();
		return true;
	}
	return false;
}

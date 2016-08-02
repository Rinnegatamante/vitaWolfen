#include "include/wl_def.h"

/*
=============================================================================

						   WOLFENSTEIN 3-D

					  An Id Software production

						   by John Carmack

=============================================================================
*/

#define FOCALLENGTH     0x5800		/* in global coordinates */

signed char str[80], str2[20];

int viewwidth, viewheight;
int viewwidthwin, viewheightwin; /* for borders */
int xoffset, yoffset;
int vwidth, vheight, vstride; /* size of screen */
int viewsize;

int centerx;
int shootdelta;			/* pixels away from centerx a target can be */

boolean startgame,loadedgame;
int mouseadjustment;

long frameon;
long lasttimecount;
fixed viewsin, viewcos;
fixed viewx, viewy;		/* the focal point */
int pixelangle[MAXVIEWWIDTH];
long finetangent[FINEANGLES/4];
int horizwall[MAXWALLTILES], vertwall[MAXWALLTILES];

signed char configname[13] = "config.";

fixed sintable[ANGLES+ANGLES/4+1], *costable = sintable+(ANGLES/4);

int _argc;
signed char **_argv;

/*
========================
=
= FixedByFrac (FixedMul)
=
= multiply two 16/16 bit, 2's complement fixed point numbers
=
========================
*/

fixed FixedByFrac(fixed a, fixed b)
{
	int64_t ra = a;
	int64_t rb = b;
	int64_t r;

	r = ra * rb;
	r >>= TILESHIFT;
	return (fixed)r;
}

/*
=====================
=
= CalcTics
=
=====================
*/

void CalcTics()
{
	int newtime;
	int ticcount;

	if (demoplayback || demorecord)
		ticcount = DEMOTICS - 1; /* [70/4] 17.5 Hz */
	else
		ticcount = 0 + 1; /* 35 Hz */

	do {
		newtime = get_TimeCount();
		tics = newtime - lasttimecount;
	} while (tics <= ticcount);

	lasttimecount = newtime;

	if (demoplayback || demorecord)
		tics = DEMOTICS;
	else if (tics > MAXTICS)
		tics = MAXTICS;
}

/* ======================================================================== */

static void DiskFlopAnim(int x, int y)
{
	static signed char which = 0;

	if (!x && !y)
		return;

	VWB_DrawPic(x, y, C_DISKLOADING1PIC+which);
	VW_UpdateScreen();

	which ^= 1;
}

static int32_t CalcFileChecksum(FILE* fd, int len)
{
	int32_t cs;
	int i;
	int8_t c1, c2;

	c1 = ReadInt8(fd);
	cs = 0;
	for (i = 0; i < len - 1; i++) {
		c2 = ReadInt8(fd);
		cs += c1 ^ c2;
		c1 = c2;
	}
	return cs;
}

int WriteConfig()
{
	int i;
	FILE* fd;
	int32_t cs;

	fd = OpenWrite(configname);

	if (fd != NULL) {
		WriteBytes(fd, (byte *)GAMEHDR, 8);	/* WOLF3D, 0, 0 */
		WriteBytes(fd, (byte *)CFGTYPE, 4);	/* CFG, 0 */
	/**/	WriteInt32(fd, 0xFFFFFFFF);		/* Version (integer) */
		WriteBytes(fd, (byte *)GAMETYPE, 4);	/* XXX, 0 */
		WriteInt32(fd, time(NULL));		/* Time */
		WriteInt32(fd, 0x00000000);		/* Padding */
		WriteInt32(fd, 0x00000000);		/* Checksum (placeholder) */

		for (i = 0; i < 7; i++) { /* MaxScores = 7 */
			WriteBytes(fd, (byte *)Scores[i].name, 58);
			WriteInt32(fd, Scores[i].score);
			WriteInt32(fd, Scores[i].completed);
			WriteInt32(fd, Scores[i].episode);
		}

		WriteInt32(fd, viewsize);


WriteInt32(fd, viewsize);

		/* sound config, etc. (to be done) */
		WriteInt32(fd, SoundMode); /* padding */
		WriteInt32(fd, MusicMode); /* padding */
		WriteInt32(fd, DigiMode); /* padding */
		WriteInt32(fd, 0); /* padding */
		WriteInt32(fd, 0); /* padding */
		WriteInt32(fd, 0); /* padding */
		WriteInt32(fd, 0); /* padding */
		WriteInt32(fd, 0); /* padding */

		fd = OpenRead(configname);
		ReadSeek(fd, 32, SEEK_SET);
		cs = CalcFileChecksum(fd, ReadLength(fd) - 32);
		CloseRead(fd);

		fd = OpenWriteAppend(configname);
		WriteSeek(fd, 28, SEEK_SET);
		WriteInt32(fd, cs);

		CloseWrite(fd);
	}

	return 0;
}

static void SetDefaults()
{
	viewsize = 50;
	SD_SetSoundMode(sdm_AdLib);
	SD_SetDigiDevice(sds_SoundBlaster);
	SD_SetMusicMode(smm_AdLib);
	SD_SetVolume(10);
}

int ReadConfig()
{
	FILE* fd;
	int configokay;
	signed char buf[8];
	int32_t version, v;
	int i;

	configokay = 0;

	fd = OpenRead(configname);

	if (fd != NULL) {
		SetDefaults();

		ReadBytes(fd, (byte *)buf, 8);
		if (strncmp(buf, GAMEHDR, 8))
			goto configend;

		ReadBytes(fd, (byte *)buf, 4);
		if (strncmp(buf, CFGTYPE, 4))
			goto configend;

		version = ReadInt32(fd);
		if (version != 0xFFFFFFFF && version != 0x00000000)
			goto configend;

		ReadBytes(fd, (byte *)buf, 4);
		if (strncmp(buf, GAMETYPE, 4))
			goto configend;

		ReadInt32(fd);	/* skip over time */
		ReadInt32(fd);	/* skip over padding */

		v = ReadInt32(fd);	/* get checksum */
		if (v != CalcFileChecksum(fd, ReadLength(fd) - 32))
			goto configend;

		ReadSeek(fd, 32, SEEK_SET);

		for (i = 0; i < 7; i++) { /* MaxScores = 7 */
			ReadBytes(fd, (byte *)Scores[i].name, 58);
			Scores[i].score = ReadInt32(fd);
			Scores[i].completed = ReadInt32(fd);
			Scores[i].episode = ReadInt32(fd);
		}

		viewsize = ReadInt32(fd);

		/* load the new data */
		if (version == 0x00000000) {
			/* sound config, etc. */
			SD_SetSoundMode(ReadInt32(fd)); /* padding */
			SD_SetMusicMode(ReadInt32(fd)); /* padding */
			SD_SetDigiDevice(ReadInt32(fd)); /* padding */
			SD_SetVolume(ReadInt32(fd)); /* padding */
			ReadInt32(fd); /* padding */
			ReadInt32(fd); /* padding */
			ReadInt32(fd); /* padding */
			ReadInt32(fd); /* padding */

			/* direction keys */
			for (i = 0; i < 4; i++) {
				dirscan[i] = ReadInt32(fd);
			}

			/* other game keys */
			for (i = 0; i < 8; i++) { /* NUMBUTTONS = 8 */
				buttonscan[i] = ReadInt32(fd);
			}

			/* mouse enabled */
			mouseenabled = ReadInt8(fd);

			/* mouse buttons */
			for (i = 0; i < 4; i++) {
				buttonmouse[i] = ReadInt32(fd);
			}

			/* mouse adjustment */
			mouseadjustment = ReadInt32(fd);

			/* unimplemented joystick */
			v = ReadInt32(fd);
			if (v != 0xFFFFFFFF) {
			}
		}

#ifdef UPLOAD
		MainMenu[readthis].active = 1;
		MainItems.curpos = 0;
#endif

		configokay = 1;
	}

configend:

	if (fd != NULL)
		CloseRead(fd);

	if (!configokay) {
		printf("Config: Setting defaults..");
		SetDefaults();
	}

	mouseenabled = true;

	joystickenabled = false;
	joypadenabled = false;
	joystickport = 0;

	mouseadjustment = 5;

	return 0;
}

/* ======================================================================== */

long DoChecksum(byte *source, int size, long checksum)
{
	int i;

	for (i=0; i<size-1; i++)
		checksum += source[i] ^ source[i+1];

	return checksum;
}

#define WriteByte(a) WriteInt8(fd, a)
#define WriteWord(a) WriteInt16(fd, a)
#define WriteLong(a) WriteInt32(fd, a)

int SaveTheGame(signed char *fn, signed char *tag, int x, int y)
{
	FILE* fd;
	int i;
	objtype *ob, nullobj;
	long checksum;

	fd = OpenWrite(fn);

	if (fd == NULL) {
		Message(STR_NOSPACE1"\n"
			STR_NOSPACE2);

		IN_ClearKeysDown();
		IN_Ack();

		return -1;
	}

	// write savegame header
	WriteBytes(fd, (byte *)GAMEHDR, 8);
	WriteBytes(fd, (byte *)SAVTYPE, 4);
	WriteInt32(fd, 0xFFFFFFFF); /* write version */
	WriteBytes(fd, (byte *)GAMETYPE, 4);
	WriteInt32(fd, time(NULL));
	WriteInt32(fd, 0x00000000);
	WriteInt32(fd, 0x00000000);
	WriteBytes(fd, (byte *)tag, 32); /* write savegame name */

	checksum = 0;

	DiskFlopAnim(x,y);

	WriteBytes(fd, (byte *)&gamestate, sizeof(gamestate));
	checksum = DoChecksum((byte*)&gamestate, sizeof(gamestate), checksum);

	DiskFlopAnim(x,y);

#ifdef SPEAR
	for (i = 0; i < 20; i++)
#else
	for (i = 0; i < 8; i++)
#endif
	{
		WriteBytes(fd, (byte *)&LevelRatios[i], sizeof(LRstruct));
		checksum = DoChecksum((byte*)&LevelRatios[i], sizeof(LRstruct), checksum);
	}

	DiskFlopAnim(x,y);

	WriteBytes(fd, (byte *)tilemap, sizeof(tilemap));
	checksum = DoChecksum((byte*)tilemap, sizeof(tilemap), checksum);

	DiskFlopAnim(x,y);

	WriteBytes(fd, (byte *)actorat, sizeof(actorat));
	checksum = DoChecksum((byte*)actorat, sizeof(actorat), checksum);

	WriteBytes(fd, (byte *)areaconnect, sizeof(areaconnect));
	WriteBytes(fd, (byte *)areabyplayer, sizeof(areabyplayer));

	for (ob = player; ob; ob = ob->next)
	{
		DiskFlopAnim(x,y);

		WriteBytes(fd, (byte *)ob, sizeof(*ob));
	}
	nullobj.active = ac_badobject;          // end of file marker

	DiskFlopAnim(x,y);

	WriteBytes(fd, (byte *)&nullobj, sizeof(nullobj));

	DiskFlopAnim(x,y);

	i = laststatobj  - statobjlist;
	WriteBytes(fd, (byte *)&i, sizeof(i));
	checksum = DoChecksum((byte*)&i, sizeof(i), checksum);

	DiskFlopAnim(x,y);

	WriteBytes(fd, (byte *)statobjlist, sizeof(statobjlist));
	checksum = DoChecksum((byte*)statobjlist, sizeof(statobjlist),checksum);

	DiskFlopAnim(x,y);

	WriteBytes(fd, (byte *)doorposition, sizeof(doorposition));
	checksum = DoChecksum((byte*)doorposition, sizeof(doorposition),checksum);

	DiskFlopAnim(x,y);

	WriteBytes(fd, (byte *)doorobjlist, sizeof(doorobjlist));
	checksum = DoChecksum((byte*)doorobjlist, sizeof(doorobjlist),checksum);

	DiskFlopAnim(x,y);

	WriteBytes(fd, (byte *)&pwallstate, sizeof(pwallstate));
	checksum = DoChecksum((byte*)&pwallstate, sizeof(pwallstate),checksum);

	WriteBytes(fd, (byte *)&pwallx, sizeof(pwallx));
	checksum = DoChecksum((byte*)&pwallx, sizeof(pwallx),checksum);

	WriteBytes(fd, (byte *)&pwally, sizeof(pwally));
	checksum = DoChecksum((byte*)&pwally, sizeof(pwally),checksum);

	WriteBytes(fd, (byte *)&pwalldir, sizeof(pwalldir));
	checksum = DoChecksum((byte*)&pwalldir, sizeof(pwalldir),checksum);

	WriteBytes(fd, (byte *)&pwallpos, sizeof(pwallpos));
	checksum = DoChecksum((byte*)&pwallpos, sizeof(pwallpos),checksum);

	DiskFlopAnim(x,y);

	WriteBytes(fd, (byte *)&Auto_Map, sizeof(automap_t));
	checksum = DoChecksum((byte*)&Auto_Map, sizeof(automap_t), checksum);

	// WRITE OUT CHECKSUM

	WriteBytes(fd, (byte *)&checksum, sizeof(checksum));

	CloseWrite(fd);

	return 0;
}

int ReadSaveTag(signed char *fn, signed char *tag)
{
	signed char buf[8];
	FILE* fd;
	int32_t v;

	fd = OpenRead(fn);
	if (fd == NULL)
		goto rstfail;

	ReadBytes(fd, (byte *)buf, 8);
	if (strncmp(buf, GAMEHDR, 8))
		goto rstfail;

	ReadBytes(fd, (byte *)buf, 4);
	if (strncmp(buf, SAVTYPE, 4))
		goto rstfail;

	v = ReadInt32(fd);
	if (v != 0xFFFFFFFF)
		goto rstfail;

	ReadBytes(fd, (byte *)buf, 4);
	if (strncmp(buf, GAMETYPE, 4))
		goto rstfail;

	ReadSeek(fd, 32, SEEK_SET);
	ReadBytes(fd, (byte *)tag, 32);

	CloseRead(fd);

	return 0;

rstfail:
	if (fd == NULL)
		CloseRead(fd);

	return -1;
}

#define ReadByte(a) a = ReadInt8(fd)
#define ReadWord(a) a = ReadInt16(fd)
#define ReadLong(a) a = ReadInt32(fd)

int LoadTheGame(signed char *fn, int x, int y)
{
	FILE* fd;
	int i;
	long checksum, oldchecksum;
	objtype nullobj;

	fd = OpenRead(fn);
	if (fd == NULL)
		goto openfail;

	ReadSeek(fd, 64, SEEK_SET);

	checksum = 0;

	DiskFlopAnim(x,y);

	ReadBytes(fd, (byte *)&gamestate, sizeof(gamestate));
	checksum = DoChecksum((byte*)&gamestate,sizeof(gamestate),checksum);

	DiskFlopAnim(x,y);

#ifdef SPEAR
	for (i = 0; i < 20; i++)
#else
	for (i = 0; i < 8; i++)
#endif
	{
		ReadBytes(fd, (byte *)&LevelRatios[i], sizeof(LRstruct));
		checksum = DoChecksum((byte*)&LevelRatios[i], sizeof(LRstruct), checksum);
	}

	DiskFlopAnim(x,y);

	SetupGameLevel();

	DiskFlopAnim(x,y);

	ReadBytes(fd, (byte *)tilemap, sizeof(tilemap));
	checksum = DoChecksum((byte*)tilemap,sizeof(tilemap), checksum);

	DiskFlopAnim(x,y);

	ReadBytes(fd, (byte *)actorat, sizeof(actorat));
	checksum = DoChecksum((byte*)actorat, sizeof(actorat), checksum);

	ReadBytes(fd, (byte *)areaconnect, sizeof(areaconnect));
	ReadBytes(fd, (byte *)areabyplayer, sizeof(areabyplayer));

	InitActorList();

	DiskFlopAnim(x,y);

	ReadBytes(fd, (byte *)player, sizeof(*player));

	while (1)
	{
		DiskFlopAnim(x,y);
		ReadBytes(fd, (byte *)&nullobj, sizeof(nullobj));
		if (nullobj.active == ac_badobject)
			break;

		GetNewActor(); // don't copy over the links

		memcpy((void*)new, (void*)&nullobj, sizeof(nullobj));
	}

	DiskFlopAnim(x,y);

	ReadBytes(fd, (byte *)&i, sizeof(i));
	checksum = DoChecksum((byte*)&i, sizeof(i), checksum);
	laststatobj = statobjlist + i;

	DiskFlopAnim(x,y);

	ReadBytes(fd, (byte *)statobjlist, sizeof(statobjlist));
	checksum = DoChecksum((byte*)statobjlist, sizeof(statobjlist),checksum);

	DiskFlopAnim(x,y);

	ReadBytes(fd, (byte *)doorposition, sizeof(doorposition));
	checksum = DoChecksum((byte*)doorposition, sizeof(doorposition),checksum);

	DiskFlopAnim(x,y);

	ReadBytes(fd, (byte *)doorobjlist, sizeof(doorobjlist));
	checksum = DoChecksum((byte*)doorobjlist, sizeof(doorobjlist),checksum);

	DiskFlopAnim(x,y);

	ReadBytes(fd, (byte *)&pwallstate, sizeof(pwallstate));
	checksum = DoChecksum((byte*)&pwallstate, sizeof(pwallstate),checksum);

	ReadBytes(fd, (byte *)&pwallx, sizeof(pwallx));
	checksum = DoChecksum((byte*)&pwallx, sizeof(pwallx),checksum);

	ReadBytes(fd, (byte *)&pwally, sizeof(pwally));
	checksum = DoChecksum((byte*)&pwally, sizeof(pwally),checksum);

	ReadBytes(fd, (byte *)&pwalldir, sizeof(pwalldir));
	checksum = DoChecksum((byte*)&pwalldir,sizeof(pwalldir),checksum);

	ReadBytes(fd, (byte *)&pwallpos, sizeof(pwallpos));
	checksum = DoChecksum((byte*)&pwallpos, sizeof(pwallpos),checksum);

	DiskFlopAnim(x,y);

	ReadBytes(fd, (byte *)&Auto_Map, sizeof(automap_t));
	checksum = DoChecksum((byte*)&Auto_Map, sizeof(automap_t),checksum);

	ReadBytes(fd, (byte *)&oldchecksum, sizeof(oldchecksum));
	if (oldchecksum != checksum)
	{
		Message(STR_SAVECHT1"\n"
			 STR_SAVECHT2"\n"
			 STR_SAVECHT3"\n"
			 STR_SAVECHT4);

		IN_ClearKeysDown();
		IN_Ack();

		memset((void *)&Auto_Map, 0, sizeof(automap_t));

		gamestate.score = 0;
		gamestate.lives = 1;
		gamestate.weapon =
		gamestate.chosenweapon =
		gamestate.bestweapon = wp_pistol;
		gamestate.ammo = 8;
	}

	CloseRead(fd);

	return 0;

openfail:
	Message(STR_SAVECHT1"\n"
		STR_SAVECHT2"\n"
		STR_SAVECHT3"\n"
		STR_SAVECHT4);

	IN_ClearKeysDown();
	IN_Ack();

	NewGame(1, 0);

	return -1;
}

/* ======================================================================== */

/*
=================
=
= MS_CheckParm
=
=================
*/

int MS_CheckParm(signed char *check)
{
	int i;
	signed char *parm;

	for (i = 1; i < _argc; i++) {
		parm = _argv[i];

		while (!isalpha(*parm))		// skip - / \ etc.. in front of parm
			if (!*parm++)
				break;		// hit end of string without an alphanum

		if (!stricmp(check, parm))
			return i;
	}
	return 0;
}

/* ======================================================================== */

/*
==================
=
= BuildTables
=
==================
*/

static const double radtoint = (double)FINEANGLES/2.0/PI;

void BuildTables()
{
	int i;
	double tang, angle, anglestep;
	fixed value;

/* calculate fine tangents */

	finetangent[0] = 0;
	for (i = 1; i < FINEANGLES/8; i++) {
		tang = tan((double)i/radtoint);
		finetangent[i] = tang*TILEGLOBAL;
		finetangent[FINEANGLES/4-1-i] = TILEGLOBAL/tang;
	}

	/* fight off asymptotic behaviour at 90 degrees */
	finetangent[FINEANGLES/4-1] = finetangent[FINEANGLES/4-2]+1;

//
// costable overlays sintable with a quarter phase shift
// ANGLES is assumed to be divisable by four
//

	angle = 0.0;
	anglestep = PI/2.0/ANGLEQUAD;
	for (i = 0; i <= ANGLEQUAD; i++) {
		value = GLOBAL1*sin(angle);

		sintable[i] =
		sintable[i+ANGLES] =
		sintable[ANGLES/2-i] = value;

		sintable[ANGLES-i] =
		sintable[ANGLES/2+i] = -value;

		angle += anglestep;
	}
}

/*
===================
=
= SetupWalls
=
= Map tile values to scaled pics
=
===================
*/

void SetupWalls()
{
	int i;

	for (i=1;i<MAXWALLTILES;i++)
	{
		horizwall[i]=(i-1)*2;
		vertwall[i]=(i-1)*2+1;
	}
}

void ShowViewSize(int width)
{
	int oldwidth,oldheight;

	oldwidth = viewwidthwin;
	oldheight = viewheightwin;

	viewwidthwin = width*16;
	viewheightwin = width*16*HEIGHTRATIO;
	DrawPlayBorder();

	viewheightwin = oldheight;
	viewwidthwin = oldwidth;
}

void NewViewSize(int width)
{
	if (width > 40)
		width = 40;
	if (width < 4)
		width = 4;

	width *= vwidth / 320;

	if ((width*16) > vwidth)
		width = vwidth / 16;

	if ((width*16*HEIGHTRATIO) > (vheight - 40*vheight/200))
		width = (vheight - 40*vheight/200)/8;

	viewwidthwin = width*16*320/vwidth;
	viewheightwin = width*16*HEIGHTRATIO*320/vwidth;
	viewsize = width*320/vwidth;

	viewwidth = width*16;
	viewheight = width*16*HEIGHTRATIO;

	centerx = viewwidth/2-1;
	shootdelta = viewwidth/10;

	yoffset = (vheight-STATUSLINES*vheight/200-viewheight)/2;
	xoffset = (vwidth-viewwidth)/2;

//
// calculate trace angles and projection constants
//
	CalcProjection(FOCALLENGTH);

}

//===========================================================================

#ifndef SPEARDEMO

#ifndef SPEAR
CP_iteminfo MusicItems={8,CTL_Y,6,0,32};
CP_itemtype MusicMenu[]=
{
	{1,"Get Them!",0},
	{1,"Searching",0},
	{1,"P.O.W.",0},
	{1,"Suspense",0},
	{1,"War March",0},
	{1,"Around The Corner!",0},

	{1,"Nazi Anthem",0},
	{1,"Lurking...",0},
	{1,"Going After Hitler",0},
	{1,"Pounding Headache",0},
	{1,"Into the Dungeons",0},
	{1,"Ultimate Conquest",0},

	{1,"Kill the S.O.B.",0},
	{1,"The Nazi Rap",0},
	{1,"Twelfth Hour",0},
	{1,"Zero Hour",0},
	{1,"Ultimate Conquest",0},
	{1,"Wolfpack",0}
};
#else
CP_iteminfo MusicItems={8,CTL_Y-20,9,0,32};
CP_itemtype MusicMenu[]=
{
	{1,"Funky Colonel Bill",0},
	{1,"Death To The Nazis",0},
	{1,"Tiptoeing Around",0},
	{1,"Is This THE END?",0},
	{1,"Evil Incarnate",0},
	{1,"Jazzin' Them Nazis",0},
	{1,"Puttin' It To The Enemy",0},
	{1,"The SS Gonna Get You",0},
	{1,"Towering Above",0}
};
#endif

static const int songs[] =
{
#ifndef SPEAR
	GETTHEM_MUS,
	SEARCHN_MUS,
	POW_MUS,
	SUSPENSE_MUS,
	WARMARCH_MUS,
	CORNER_MUS,

	NAZI_OMI_MUS,
	PREGNANT_MUS,
	GOINGAFT_MUS,
	HEADACHE_MUS,
	DUNGEON_MUS,
	ULTIMATE_MUS,

	INTROCW3_MUS,
	NAZI_RAP_MUS,
	TWELFTH_MUS,
	ZEROHOUR_MUS,
	ULTIMATE_MUS,
	PACMAN_MUS
#else
	XFUNKIE_MUS,             // 0
	XDEATH_MUS,              // 2
	XTIPTOE_MUS,             // 4
	XTHEEND_MUS,             // 7
	XEVIL_MUS,               // 17
	XJAZNAZI_MUS,            // 18
	XPUTIT_MUS,              // 21
	XGETYOU_MUS,             // 22
	XTOWER2_MUS              // 23
#endif
};

void DoJukebox()
{
	int which,lastsong=-1;
	unsigned start;

	IN_ClearKeysDown();
//	if (!AdLibPresent && !SoundBlasterPresent)
//		return;

	MenuFadeOut();

#if !defined(SPEAR) || !defined(UPLOAD)
	start = (US_RndT() % 3) * 6;
#else
	start = 0;
#endif

	CA_CacheGrChunk(STARTFONT+1);
#ifdef SPEAR
	CacheLump(BACKDROP_LUMP_START, BACKDROP_LUMP_END);
#else
	CacheLump(CONTROLS_LUMP_START, CONTROLS_LUMP_END);
#endif
	CA_LoadAllSounds();

	fontnumber=1;
	ClearMScreen();
	VWB_DrawPic(112,184,C_MOUSELBACKPIC);
	DrawStripes(10);
	SETFONTCOLOR (TEXTCOLOR,BKGDCOLOR);

#ifndef SPEAR
	DrawWindow (CTL_X-2,CTL_Y-6,280,13*7,BKGDCOLOR);
#else
	DrawWindow (CTL_X-2,CTL_Y-26,280,13*10,BKGDCOLOR);
#endif

	DrawMenu (&MusicItems,&MusicMenu[start]);

	SETFONTCOLOR (READHCOLOR,BKGDCOLOR);
	PrintY = 15;
	WindowX = 0;
	WindowY = 320;
	US_CPrint("Robert's Jukebox");

	SETFONTCOLOR (TEXTCOLOR,BKGDCOLOR);
	VW_UpdateScreen();
	MenuFadeIn();

	do
	{
		which = HandleMenu(&MusicItems,&MusicMenu[start],NULL);
		if (which>=0)
		{
			if (lastsong >= 0)
				MusicMenu[start+lastsong].active = 1;

			StartCPMusic(songs[start + which]);
			MusicMenu[start+which].active = 2;
			DrawMenu (&MusicItems,&MusicMenu[start]);
			VW_UpdateScreen();
			lastsong = which;
		}
	} while(which>=0);

	MenuFadeOut();
	IN_ClearKeysDown();
#ifdef SPEAR
	UnCacheLump(BACKDROP_LUMP_START, BACKDROP_LUMP_END);
#else
	UnCacheLump(CONTROLS_LUMP_START, CONTROLS_LUMP_END);
#endif
}
#endif

/* ======================================================================== */

/*
==========================
=
= ShutdownId
=
= Shuts down all ID_?? managers
=
==========================
*/

void ShutdownId()
{
	US_Shutdown();
	SD_Shutdown();
	IN_Shutdown();
	VW_Shutdown();
	CA_Shutdown();
	PM_Shutdown();
	MM_Shutdown();
}

/*
=====================
=
= NewGame
=
= Set up new game to start from the beginning
=
=====================
*/

void NewGame(int difficulty, int episode)
{
	memset(&gamestate, 0, sizeof(gamestate));

	gamestate.difficulty = difficulty;
	gamestate.weapon = gamestate.bestweapon
		= gamestate.chosenweapon = wp_pistol;
	gamestate.health = 100;
	gamestate.ammo = STARTAMMO;
	gamestate.lives = 3;
	gamestate.nextextra = EXTRAPOINTS;
	gamestate.episode = episode;

	startgame = true;
}

/*
==========================
=
= InitGame
=
= Load a few things right away
=
==========================
*/

void InitGame()
{
	int i;

	MM_Startup();
	PM_Startup();
	CA_Startup();
	VW_Startup();
	IN_Startup();
	SD_Startup();
	US_Startup();

//
// build some tables
//

	for (i = 0;i < MAPSIZE; i++)
	{
		farmapylookup[i] = i*64;
	}

	ReadConfig();
	printf("Done!\n");
	
/* load in and lock down some basic chunks */
	printf("CacheGrChunk (font) ...");	
	CA_CacheGrChunk(STARTFONT);
	printf("CacheGrChunk (tile) ...");	
	CA_CacheGrChunk(STARTTILE8);
	for (i = LATCHPICS_LUMP_START; i <= LATCHPICS_LUMP_END; i++){
		printf("CacheGrChunk (lump%i) ...", i);	
		CA_CacheGrChunk(i);
	}
	printf("Done!\nBuilding tables...");
	BuildTables();
	printf("Done!\nBuilding walls...");
	SetupWalls();
	printf("Done!\nSet View Size...");
	NewViewSize(viewsize);
	printf("Done!\nInit variables...");

//
// initialize variables
//
	InitRedShifts();

	IN_CheckAck();
	printf("Done!\n");
//
// HOLDING DOWN 'M' KEY?
//
#ifndef SPEARDEMO
	if (IN_KeyDown(sc_M))
		DoJukebox();
#endif
}

/*
=====================
=
= DemoLoop
=
=====================
*/

void DemoLoop()
{
	static int LastDemo;

	int i;
//
// main game cycle
//

	LastDemo = 0;
	
	printf("\nStarting Music...\n");
	StartCPMusic(INTROSONG);

	printf("Skipping demo...\n");
	NoWait = true;
	
	if (!NoWait)
		PG13();

	i = MS_CheckParm("playdemo");
	if (i && ((i+1) < _argc)) {
		i++;
		for (; i < _argc; i++) {
			if (_argv[i][0] == '-')
				break;
			IN_ClearKeysDown();
			if (PlayDemoFromFile(_argv[i]))
				IN_UserInput(3 * 70);
		}
		VW_FadeOut();
	}

	if (MS_CheckParm("demotest")) {
	#ifndef SPEARDEMO
		for (i = 0; i < 4; i++)
			PlayDemo(i);
	#else
		PlayDemo(0);
	#endif
	}

	for (;;)
	{
		while (!NoWait)
		{
//
// title page
//
			MM_SortMem ();
#ifdef SPEAR
			CA_CacheGrChunk (TITLEPALETTE);

			CA_CacheGrChunk (TITLE1PIC);
			VWB_DrawPic (0,0,TITLE1PIC);
			CA_UnCacheGrChunk (TITLE1PIC);

			CA_CacheGrChunk (TITLE2PIC);
			VWB_DrawPic (0,80,TITLE2PIC);
			CA_UnCacheGrChunk(TITLE2PIC);
			VW_UpdateScreen();
			VL_FadeIn(0,255,grsegs[TITLEPALETTE],30);

			CA_UnCacheGrChunk (TITLEPALETTE);
#else
			VL_CacheScreen(TITLEPIC);
			VW_UpdateScreen ();
			VW_FadeIn();
#endif
			if (IN_UserInput(TickBase*15))
				break;
			VW_FadeOut();
//
// credits page
//
			VL_CacheScreen(CREDITSPIC);
			VW_UpdateScreen();
			VW_FadeIn ();
			if (IN_UserInput(TickBase*10))
				break;
			VW_FadeOut ();
//
// high scores
//
			DrawHighScores();
			VW_UpdateScreen();
			VW_FadeIn();

			if (IN_UserInput(TickBase*10))
				break;
//
// demo
//
			#ifndef SPEARDEMO
			PlayDemo(LastDemo++%4);
			#else
			PlayDemo(0);
			#endif

			if (playstate == ex_abort)
				break;
			StartCPMusic(INTROSONG);
		}

		VW_FadeOut();

		if (IN_KeyDown(sc_Tab) && MS_CheckParm("debugmode"))
			RecordDemo();
		else
			US_ControlPanel(0);

		if (startgame || loadedgame)
		{
			GameLoop();
			VW_FadeOut();
			StartCPMusic(INTROSONG);
		}
	}
}


/* ======================================================================== */


/*
==========================
=
= WolfMain
=
==========================
*/

int WolfMain(int argc, signed char *argv[])
{
	_argc = argc;
	_argv = argv;

	if (MS_CheckParm("version")) {
		printf("Game: %s\n", GAMENAME);
		Quit(NULL);
	}

	printf("Now Loading %s\n", GAMENAME);

	CheckForEpisodes();

	InitGame();

	//hidScanInput();
	//pad = hidKeysHeld();
	
	printf("Starting demo...");
	DemoLoop();
	
	Quit("Demo loop exited???");

	return 0;
}

#include <sys/time.h>
#include <stdio.h>
#include "include/wl_def.h"

static struct timeval t0;
static long tc0;

void set_TimeCount(unsigned long t)
{
	tc0 = t;
	gettimeofday(&t0, NULL);
}

unsigned long get_TimeCount()
{
	struct timeval t1;
	long secs, usecs;
	long tc;
	
	gettimeofday(&t1, NULL);
	secs = t1.tv_sec - t0.tv_sec;
	usecs = t1.tv_usec - t0.tv_usec;
	if (usecs < 0) {
		usecs += 1000000;
		secs--;
	}

	tc = tc0 + secs * 70 + usecs * 70 / 1000000;
		
	return tc;
}

long filelength(FILE* handle)
{
	FILE* fd = handle;
	int rst = ftell(fd);
	fseek(fd, 0, SEEK_END);
	int res = ftell(fd);
	fseek(fd, rst, SEEK_SET);
	
	return res;
}

char *strlwr(char *s)
{
	signed char *p = s;
	
	while (*p) {
		*p = tolower(*p);
		p++;
	}
	
	return s;
}
	
char *itoa(int value, char *string, int radix)
{
	/* wolf3d only uses radix 10 */
	sprintf(string, "%d", value);
	return string;
}

char *ltoa(long value, char *string, int radix)
{
	sprintf(string, "%ld", value);
	return string;
}

char *ultoa(unsigned long value, char *string, int radix)
{
	sprintf(string, "%lu", value);
	return string;
}

/* from Dan Olson */
static void put_dos2ansi(byte attrib)
{
	byte fore,back,blink=0,intens=0;
	
	fore = attrib&15;	// bits 0-3
	back = attrib&112; // bits 4-6
       	blink = attrib&128; // bit 7
	
	// Fix background, blink is either on or off.
	back = back>>4;

	// Fix foreground
	if (fore > 7) {
		intens = 1;
		fore-=8;
	}

	// Convert fore/back
	switch (fore) {
		case 0: // BLACK
			fore=30;
			break;
		case 1: // BLUE
			fore=34;
			break;
		case 2: // GREEN
			fore=32;
			break;
		case 3: // CYAN
			fore=36;
			break;
		case 4: // RED
			fore=31;
			break;
		case 5: // Magenta
			fore=35;
			break;
		case 6: // BROWN(yellow)
			fore=33;
			break;
		case 7: //GRAy
			fore=37;
			break;
	}
			
	switch (back) {
		case 0: // BLACK
			back=40;
			break;
		case 1: // BLUE
			back=44;
			break;
		case 2: // GREEN
			back=42;
			break;
		case 3: // CYAN
			back=46;
			break;
		case 4: // RED
			back=41;
			break;
		case 5: // Magenta
			back=45;
			break;
		case 6: // BROWN(yellow)
			back=43;
			break;
		case 7: //GRAy
			back=47;
			break;
	}
	if (blink)
		printf ("%c[%d;5;%dm%c[%dm", 27, intens, fore, 27, back);
	else
		printf ("%c[%d;25;%dm%c[%dm", 27, intens, fore, 27, back);
}

void DisplayTextSplash(byte *text, int l)
{
	int i, x;
	
	//printf("%02X %02X %02X %02X\n", text[0], text[1], text[2], text[3]);
	text += 4;
	//printf("%02X %02X %02X %02X\n", text[0], text[1], text[2], text[3]);
	text += 2;
	
	for (x = 0; x < l; x++) {
		for (i = 0; i < 160; i += 2) {
			put_dos2ansi(text[160*x+i+2]);
			if (text[160*x+i+1] && text[160*x+i+1] != 160)
				printf("%c", text[160*x+i+1]);
			else
				printf(" ");
		}
		printf("%c[m", 27);
		printf("\n");
	}
}

/* ** */

uint16_t SwapInt16L(uint16_t i)
{
#if BYTE_ORDER == BIG_ENDIAN
	return ((uint16_t)i >> 8) | ((uint16_t)i << 8);
#else
	return i;
#endif
}

uint32_t SwapInt32L(uint32_t i)
{
#if BYTE_ORDER == BIG_ENDIAN
	return	((uint32_t)(i & 0xFF000000) >> 24) | 
		((uint32_t)(i & 0x00FF0000) >>  8) |
		((uint32_t)(i & 0x0000FF00) <<  8) | 
		((uint32_t)(i & 0x000000FF) << 24);
#else
	return i;
#endif
}

/* ** */

FILE* OpenWrite(signed char *fn)
{
	char path[256];
	sprintf(path,"ux0:/data/Wolfenstein 3D/%s", fn);
	return fopen(path, "wb");
}

FILE* OpenWriteAppend(signed char *fn)
{
	char path[256];
	sprintf(path,"ux0:/data/Wolfenstein 3D/%s", fn);
	return fopen(path, "ab+");
}

void CloseWrite(FILE* fp)
{
	fclose(fp);
}

int WriteSeek(FILE* fp, int offset, int whence)
{
	fseek(fp, offset, whence);
	return ftell(fp);
}

int WritePos(FILE* fp)
{
	return ftell(fp);
}

int WriteInt8(FILE* fp, int8_t d)
{
	return fputc(d, fp);
}

int WriteInt16(FILE* fp, int16_t d)
{
	int16_t b = SwapInt16L(d);
	
	return fwrite(&b, 1, 2, fp) >> 1;
}

int WriteInt32(FILE* fp, int32_t d)
{
	int32_t b = SwapInt32L(d);
	
	return fwrite(&b, 1, 4, fp) >> 2;
}

int WriteBytes(FILE* fp, byte *d, int len)
{
	return fwrite(d, 1, len, fp);
}


FILE* OpenRead(signed char *fn)
{
	FILE* fp;
	
	char path[256];
	sprintf(path,"ux0:/data/Wolfenstein 3D/%s", fn);
	fp = fopen(path, "rb");
	
	return fp;
}

void CloseRead(FILE* fp)
{
	fclose(fp);
}

int ReadSeek(FILE* fp, int offset, int whence)
{
	fseek(fp, offset, whence);
	return ftell(fp);
}

int ReadLength(FILE* fp)
{
	return filelength(fp);
}

int8_t ReadInt8(FILE* fp)
{
	byte d[1];
	
	d[0] = fgetc(fp);
	
	return d[0];
}

int16_t ReadInt16(FILE* fp)
{
	byte d[2];
	
	fread(d, 1, 2, fp);
	
	return (d[0]) | (d[1] << 8);
}

int32_t ReadInt32(FILE* fp)
{
	byte d[4];
	
	fread(d, 1, 4, fp);
	
	return (d[0]) | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
}

int ReadBytes(FILE* fp, byte *d, int len)
{
	return fread(d, 1, len, fp);
}

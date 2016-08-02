#ifndef __MISC_H__
#define __MISC_H__

extern int _argc;
extern signed char **_argv;

void SavePCX256ToFile(unsigned char *buf, int width, int height, unsigned char *pal, signed char *name);
void SavePCXRGBToFile(unsigned char *buf, int width, int height, signed char *name);

void set_TimeCount(unsigned long t);
unsigned long get_TimeCount();

long filelength(FILE* handle);

#define stricmp strcasecmp
#define strnicmp strncasecmp
char *strlwr(char *s);

char *itoa(int value, char *string, int radix);
char *ltoa(long value, char *string, int radix);
char *ultoa(unsigned long value, char *string, int radix);

uint16_t SwapInt16L(uint16_t i);
uint32_t SwapInt32L(uint32_t i);

extern FILE* OpenWrite(signed char *fn);
extern FILE* OpenWriteAppend(signed char *fn);
extern void CloseWrite(FILE* fp);

extern int WriteSeek(FILE* fp, int offset, int whence);
extern int WritePos(FILE* fp);

extern int WriteInt8(FILE* fp, int8_t d);
extern int WriteInt16(FILE* fp, int16_t d);
extern int WriteInt32(FILE* fp, int32_t d);
extern int WriteBytes(FILE* fp, byte *d, int len);

extern FILE* OpenRead(signed char *fn);
extern void CloseRead(FILE* fp);

extern int ReadSeek(FILE* fp, int offset, int whence);
extern int ReadLength(FILE* fp);

extern int8_t ReadInt8(FILE* fp);
extern int16_t ReadInt16(FILE* fp);
extern int32_t ReadInt32(FILE* fp);
extern int ReadBytes(FILE* fp, byte *d, int len);


static __inline__ uint16_t SwapInt16(uint16_t i)
{
	return ((uint16_t)i >> 8) | ((uint16_t)i << 8);
}

static __inline__ uint32_t SwapInt32(uint32_t i)
{
	return	((uint32_t)(i & 0xFF000000) >> 24) |
		((uint32_t)(i & 0x00FF0000) >>  8) |
		((uint32_t)(i & 0x0000FF00) <<  8) |
		((uint32_t)(i & 0x000000FF) << 24);
}

#endif

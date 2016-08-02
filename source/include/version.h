#ifndef __VERSION_H__
#define __VERSION_H__


/* Change WMODE to point to the executable you would like to build: */
// Mode 0 = Wolf3D Shareware
// Mode 1 = Wolf3d Full
// Mode 2 = SOD Shareware
// Mode 3 = SOD Full / SOD Episode 1 & 2

#ifndef WMODE
#define WMODE 0
#endif

/* --- End of User-Modifiable Variables --- */

#if WMODE == 0
/* #define SPEAR */
/* #define SPEARDEMO */
#define UPLOAD
#define GAMENAME	"Wolfenstein 3D Shareware"
#define GAMEEXT		"wl1"
#define GAMETYPE	"WL1\0"

#elif WMODE == 1
/* #define SPEAR */
/* #define SPEARDEMO */
/* #define UPLOAD */
#define GAMENAME	"Wolfenstein 3D"
#define GAMEEXT		"wl6"
#define GAMETYPE	"WL6\0"

#elif WMODE == 2
#define SPEAR 
#define SPEARDEMO 
/* #define UPLOAD */
#define GAMENAME	"Spear of Destiny Demo"
#define GAMEEXT		"sdm"
#define GAMETYPE	"SDM\0"

#elif WMODE == 3
#define SPEAR
/* #define SPEARDEMO */
/* #define UPLOAD */
#define GAMENAME	"Spear of Destiny"
#define GAMEEXT		"sod"
#define GAMETYPE	"SOD\0"

#else
#error "please edit version.h and fix WMODE"
#endif

#define GAMEHDR		"WOLF3D\0\0"

#define	SAVTYPE		"SAV\0"
#define CFGTYPE		"CFG\0"

#endif

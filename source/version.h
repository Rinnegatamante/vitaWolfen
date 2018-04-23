#ifndef _VERSION_H_
#define _VERSION_H_

#ifndef DATADIR
#define DATADIR "ux0:data/Wolfenstein 3D/"
#endif

/* Defines used for different versions */
#define CARMACIZED

#if WMODE == 0
# define UPLOAD
#elif WMODE == 1
# define GOODTIMES
#elif WMODE == 2
# define SPEAR
# define SPEARDEMO
#elif WMODE == 3
# define SPEAR
#endif

//#define USE_FEATUREFLAGS    // Enables the level feature flags (see bottom of wl_def.h)
//#define USE_SHADING         // Enables shading support (see wl_shade.cpp)
//#define USE_DIR3DSPR        // Enables directional 3d sprites (see wl_dir3dspr.cpp)
#define USE_FLOORCEILINGTEX // Enables floor and ceiling textures stored in the third mapplane (see wl_floorceiling.cpp)
//#define USE_HIRES           // Enables high resolution textures/sprites (128x128)
//#define USE_PARALLAX 16     // Enables parallax sky with 16 textures per sky (see wl_parallax.cpp)
//#define USE_CLOUDSKY        // Enables cloud sky support (see wl_cloudsky.cpp)
//#define USE_STARSKY         // Enables star sky support (see wl_atmos.cpp)
//#define USE_RAIN            // Enables rain support (see wl_atmos.cpp)
//#define USE_SNOW            // Enables snow support (see wl_atmos.cpp)
//#define FIXRAINSNOWLEAKS    // Enables leaking ceilings fix (by Adam Biser, only needed if maps with rain/snow and ceilings exist)

#define DEBUGKEYS             // Comment this out to compile without the Tab debug keys
#define ARTSEXTERN
#define DEMOSEXTERN
#define PLAYDEMOLIKEORIGINAL  // When playing or recording demos, several bug fixes do not take
                              // effect to let the original demos work as in the original Wolf3D v1.4
                              // (actually better, as the second demo rarely worked)

#endif

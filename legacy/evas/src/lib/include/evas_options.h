#ifndef EVAS_OPTIONS_H
#define EVAS_OPTIONS_H 1

/* these may vary per OS */

/* what to build *//* small dither table is MUCH faster on ipaq */
/*#define BUILD_SMALL_DITHER_MASK*/

/*#define HARD_CODED_P3*/
/*#define HARD_CODED_P2*/

/*#define BUILD_CONVERT_1_GRY_1*/
/*#define BUILD_CONVERT_4_GRY_1*/
/*#define BUILD_CONVERT_4_GRY_4*/
/*#define BUILD_CONVERT_8_GRY_1*/
/*#define BUILD_CONVERT_8_GRY_4*/
/*#define BUILD_CONVERT_8_GRY_16*/
/*#define BUILD_CONVERT_8_GRY_64*/
/*#define BUILD_CONVERT_8_GRY_256*/
/*#define BUILD_CONVERT_8_RGB_332*/
/*#define BUILD_CONVERT_8_RGB_666*/
/*#define BUILD_CONVERT_8_RGB_232*/
/*#define BUILD_CONVERT_8_RGB_222*/
/*#define BUILD_CONVERT_8_RGB_221*/
/*#define BUILD_CONVERT_8_RGB_121*/
/*#define BUILD_CONVERT_8_RGB_111*/
/*#define BUILD_CONVERT_16_RGB_565*/
/*#define BUILD_CONVERT_16_RGB_555*/
/*#define BUILD_CONVERT_16_RGB_444*/
/*#define BUILD_CONVERT_16_RGB_454645*/
/*#define BUILD_CONVERT_16_RGB_ROT0*/
/*#define BUILD_CONVERT_16_RGB_ROT270*/
/*#define BUILD_CONVERT_24_RGB_888*/
/*#define BUILD_CONVERT_24_BGR_888*/
/*#define BUILD_CONVERT_32_RGB_8888*/
/*#define BUILD_CONVERT_32_BGR_8888*/
/*#define BUILD_CONVERT_32_RGB_ROT0*/
/*#define BUILD_CONVERT_32_RGB_ROT270*/

/*#define BUILD_SCALE_SAMPLE*/
/*#define BUILD_SCALE_SMOOTH*/
/*#define BUILD_SCALE_TRILINEAR*/

/*#define BUILD_MMX*/
/*#define BUILD_SSE*/
/*#define BUILD_C*/

/*#define BUILD_LOADER_PNG*/
/*#define BUILD_LOADER_JPEG*/
/*#define BUILD_LOADER_EET*/
/*#define BUILD_LOADER_EDB*/

/*#define BUILD_FMEMOPEN*/

/* check in that the user configured it right */
#ifndef BUILD_MMX
# ifndef BUILD_SSE
#  ifndef BUILD_C
#   error "Please Read the README"
#  endif
# endif
#endif

#ifdef BUILD_FMEMOPEN
# define _GNU_SOURCE
#endif

#define DIRECT_SCALE

#endif

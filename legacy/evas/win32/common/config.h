/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */

/* Build Altivec Code */
/* #undef BUILD_ALTIVEC */

/* Build plain C code */
#define BUILD_C 1

/* 16bpp BGR 565 Converter Support */
#define BUILD_CONVERT_16_BGR_565 1

/* 16bpp RGB 444 Converter Support */
#define BUILD_CONVERT_16_RGB_444 1

/* 16bpp 565 (444 ipaq) Converter Support */
#define BUILD_CONVERT_16_RGB_454645 1

/* 16bpp RGB 555 Converter Support */
#define BUILD_CONVERT_16_RGB_555 1

/* 16bpp RGB 565 Converter Support */
#define BUILD_CONVERT_16_RGB_565 1

/* 16bpp RGB Rotation 0 Converter Support */
#define BUILD_CONVERT_16_RGB_ROT0 1

/* 16bpp RGB Rotation 180 Converter Support */
#define BUILD_CONVERT_16_RGB_ROT180 1

/* 16bpp RGB Rotation 270 Converter Support */
#define BUILD_CONVERT_16_RGB_ROT270 1

/* 16bpp RGB Rotation 90 Converter Support */
#define BUILD_CONVERT_16_RGB_ROT90 1

/* 24bpp BGR 888 Converter Support */
#define BUILD_CONVERT_24_BGR_888 1

/* 24bpp RGB 888 Converter Support */
#define BUILD_CONVERT_24_RGB_888 1

/* 32bpp BGRX 8888 Converter Support */
#define BUILD_CONVERT_32_BGRX_8888 1

/* 32bpp BGR 8888 Converter Support */
#define BUILD_CONVERT_32_BGR_8888 1

/* 32bpp RGBX 8888 Converter Support */
#define BUILD_CONVERT_32_RGBX_8888 1

/* 32bpp RGB 8888 Converter Support */
#define BUILD_CONVERT_32_RGB_8888 1

/* 32bpp RGB Rotation 0 Converter Support */
#define BUILD_CONVERT_32_RGB_ROT0 1

/* 32bpp RGB Rotation 180 Converter Support */
#define BUILD_CONVERT_32_RGB_ROT180 1

/* 32bpp RGB Rotation 270 Converter Support */
#define BUILD_CONVERT_32_RGB_ROT270 1

/* 32bpp RGB Rotation 90 Converter Support */
#define BUILD_CONVERT_32_RGB_ROT90 1

/* 8bpp RGB 111 Converter Support */
#define BUILD_CONVERT_8_RGB_111 1

/* 8bpp RGB 121 Converter Support */
#define BUILD_CONVERT_8_RGB_121 1

/* 8bpp RGB 221 Converter Support */
#define BUILD_CONVERT_8_RGB_221 1

/* 8bpp RGB 222 Converter Support */
#define BUILD_CONVERT_8_RGB_222 1

/* 8bpp RGB 232 Converter Support */
#define BUILD_CONVERT_8_RGB_232 1

/* 8bpp RGB 332 Converter Support */
#define BUILD_CONVERT_8_RGB_332 1

/* 8bpp RGB 666 Converter Support */
#define BUILD_CONVERT_8_RGB_666 1

/* YUV Converter Support */
#define BUILD_CONVERT_YUV 1

/* Buffer Rendering Backend */
#define BUILD_ENGINE_BUFFER 1

/* Generic Cairo Rendering Support */
/* #undef BUILD_ENGINE_CAIRO_COMMON */

/* Cairo X11 Rendering Backend */
/* #undef BUILD_ENGINE_CAIRO_X11 */

/* Direct3D Rendering Backend */
#define BUILD_ENGINE_DIRECT3D 1

/* DirectFB Rendering Backend */
/* #undef BUILD_ENGINE_DIRECTFB */

/* Linux FB Rendering Backend */
/* #undef BUILD_ENGINE_FB */

/* Glitz X11 Rendering Backend */
/* #undef BUILD_ENGINE_GLITZ_X11 */

/* Generic OpenGL Rendering Support */
#define BUILD_ENGINE_GL_COMMON 1

/* OpenGL Glew Rendering Backend */
#define BUILD_ENGINE_GL_GLEW 1

/* OpenGL X11 Rendering Backend */
/* #undef BUILD_ENGINE_GL_X11 */

/* SDL Rendering Backend */
#define BUILD_ENGINE_SDL 1

/* 16bit Software DirectDraw Rendering Backend */
/* #undef BUILD_ENGINE_SOFTWARE_16_DDRAW */

/* Software 16bit X11 Rendering Backend */
/* #undef BUILD_ENGINE_SOFTWARE_16_X11 */

/* Software DirectDraw Rendering Backend */
/* #undef BUILD_ENGINE_SOFTWARE_DDRAW */

/* Qtopia Rendering Backend */
/* #undef BUILD_ENGINE_SOFTWARE_QTOPIA */

/* Software X11 Rendering Backend */
/* #undef BUILD_ENGINE_SOFTWARE_X11 */

/* Software XCB Rendering Backend */
/* #undef BUILD_ENGINE_SOFTWARE_XCB */

/* XRender X11 Rendering Backend */
/* #undef BUILD_ENGINE_XRENDER_X11 */

/* Xrender XCB Rendering Backend */
/* #undef BUILD_ENGINE_XRENDER_XCB */

/* EET Font Loader Support */
#define BUILD_FONT_LOADER_EET 1

/* EDB Image Loader Support */
/* #undef BUILD_LOADER_EDB */

/* EET Image Loader Support */
#define BUILD_LOADER_EET 1

/* GIF Image Loader Support */
#define BUILD_LOADER_GIF 1

/* JPEG Image Loader Support */
#define BUILD_LOADER_JPEG 1

/* TIFF Image Loader Support */
#define BUILD_LOADER_TIFF 1

/* Build MMX Code */
/* #define BUILD_MMX 1 */

/* No Dither Mask Support */
/* #undef BUILD_NO_DITHER_MASK */

/* Build Threaded Rendering */
/* #undef BUILD_PTHREAD */

/* Sampling Scaler Support */
#define BUILD_SCALE_SAMPLE 1

/* Smooth Scaler Support */
#define BUILD_SCALE_SMOOTH 1

/* Small Dither Mask Support */
/* #undef BUILD_SMALL_DITHER_MASK */

/* Build SSE Code */
/* #define BUILD_SSE 1 */

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
/* #undef CRAY_STACKSEG_END */

/* Define to 1 if using `alloca.c'. */
/* #undef C_ALLOCA */

/* Define to mention that evas is built */
#define EFL_EVAS_BUILD 1

/* Define to 1 if you have `alloca', as a function or macro. */
#define HAVE_ALLOCA 1

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
/* #undef HAVE_ALLOCA_H */

/* Have altivec.h header file */
/* #undef HAVE_ALTIVEC_H */

/* Define to 1 if you have the <d3d9.h> header file. */
#define HAVE_D3D9_H 1

/* Define to 1 if you have the <d3dx9.h> header file. */
/* #undef HAVE_D3DX9_H */

/* Define to 1 if you have the `dladdr' function. */
#define HAVE_DLADDR 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `dlopen' function. */
/* #undef HAVE_DLOPEN */

/* Set to 1 if evil package is installed */
#define HAVE_EVIL 1

/* Define to 1 if you have the `fnmatch' function. */
/* #undef HAVE_FNMATCH */

/* have fontconfig searching capabilities */
/* #define HAVE_FONTCONFIG 1 */

/* Define to 1 if you have the <GL/glew.h> header file. */
#define HAVE_GL_GLEW_H 1

/* Define to 1 if you have the <GL/glu.h> header file. */
#define HAVE_GL_GLU_H 1

/* Define to 1 if you have the <GL/gl.h> header file. */
#define HAVE_GL_GL_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <pthread.h> header file. */
/* #undef HAVE_PTHREAD_H */

/* Define to 1 if you have the <sched.h> header file. */
/* #undef HAVE_SCHED_H */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Valgrind support */
/* #undef HAVE_VALGRIND */

/* Define to 1 if you have the <X11/extensions/Xrender.h> header file. */
/* #undef HAVE_X11_EXTENSIONS_XRENDER_H */

/* Define to 1 if you have the <X11/X.h> header file. */
/* #undef HAVE_X11_X_H */

/* "Module architecture" */
#define MODULE_ARCH "mingw32-i686"

/* Name of package */
#define PACKAGE "evas"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "enlightenment-devel@lists.sourceforge.net"

/* Define to the full name of this package. */
#define PACKAGE_NAME "evas"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "evas 0.9.9.042"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "evas"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.9.9.042"

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at runtime.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown */
/* #undef STACK_DIRECTION */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "0.9.9.042"

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef WORDS_BIGENDIAN */

/* Define to 1 if the X Window System is missing or not being used. */
#define X_DISPLAY_MISSING 1

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

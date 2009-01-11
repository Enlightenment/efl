# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT(evas, 0.9.9.050, enlightenment-devel@lists.sourceforge.net)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(configure.ac)
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_ISC_POSIX

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_BIGENDIAN
AC_C_CONST
AC_C___ATTRIBUTE__

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

VMAJ=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $1);}'`
VMIN=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $2);}'`
VMIC=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $3);}'`
SNAP=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $4);}'`
version_info=`expr $VMAJ + $VMIN`":$VMIC:$VMIN"
AC_SUBST(version_info)


want_evas_engine_buffer="yes"
want_evas_engine_software_x11="no"
want_evas_engine_xrender_x11="no"
want_evas_engine_gl_x11="no"
want_evas_engine_glitz_x11="no"
want_evas_engine_cairo_x11="no"
want_evas_engine_software_xcb="no"
want_evas_engine_xrender_xcb="no"
want_evas_engine_software_ddraw="no"
want_evas_engine_direct3d="no"
want_evas_engine_quartz="no"
want_evas_engine_gl_glew="no"
want_evas_engine_software_sdl="no"
want_evas_engine_fb="no"
want_evas_engine_directfb="no"
want_evas_engine_software_qtopia="no"
want_evas_engine_software_16_x11="no"
want_evas_engine_software_16_ddraw="no"
want_evas_engine_software_16_wince="no"

want_evas_image_loader_edb="yes"
want_evas_image_loader_eet="yes"
want_evas_image_loader_gif="yes"
want_evas_image_loader_jpeg="yes"
want_evas_image_loader_pmaps="yes"
want_evas_image_loader_png="yes"
want_evas_image_loader_svg="yes"
want_evas_image_loader_tiff="yes"
want_evas_image_loader_xpm="yes"

want_evas_font_loader_eet="yes"

case "$host_os" in
   mingw32ce* | cegcc*)
      want_evas_engine_software_16_wince="yes"
      want_evas_image_loader_edb="no"
      want_evas_image_loader_gif="no"
      want_evas_image_loader_svg="no"
      want_evas_image_loader_tiff="no"
      ;;
   mingw*)
      want_evas_engine_software_ddraw="yes"
      want_evas_engine_direct3d="auto"
      want_evas_engine_software_16_ddraw="yes"
      want_evas_image_loader_edb="no"
      want_evas_image_loader_svg="no"
      ;;
   darwin*)
      want_evas_engine_quartz="auto"
      ;;
   *)
      want_evas_engine_software_x11="auto"
      want_evas_engine_xrender_x11="auto"
      want_evas_engine_software_16_x11="auto"
      ;;
esac

LT_PROG_RC
AC_FUNC_ALLOCA

MODULE_ARCH="$host_os-$host_cpu"
AC_SUBST(MODULE_ARCH)
AC_DEFINE_UNQUOTED(MODULE_ARCH, "$MODULE_ARCH", "Module architecture")

PKG_PROG_PKG_CONFIG

dnl wierd debian etch bug where pthread_barrier doesn't work without adding
dnl this.
CFLAGS="${CFLAGS=} -D_GNU_SOURCE"
AC_SUBST(CFLAGS)

WIN32_CPPFLAGS=""
WIN32_CFLAGS=""
lt_enable_auto_import=""
case "$host_os" in
        mingw*|cegcc*)
		PKG_CHECK_MODULES(EVIL, evil)
		AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if evil package is installed])
		dnl needed for correct definition of EAPI
		AC_DEFINE(EFL_EVAS_BUILD, 1, [Define to mention that evas is built])
                if test "$host_os" = "cegcc" ; then
                        WIN32_CPPFLAGS="-D_WIN32_WCE=0x0420"
                        WIN32_CFLAGS="-mwin32"
			lt_enable_auto_import="-Wl,--enable-auto-import"
                fi
                if test "$host_os" = "mingw32ce" ; then
                        WIN32_CPPFLAGS="-D_WIN32_WCE=0x0420"
                fi
                ;;
esac
AC_SUBST(WIN32_CPPFLAGS)
AC_SUBST(WIN32_CFLAGS)
AC_SUBST(lt_enable_auto_import)


dnl when used, that option makes configure script fails when
dnl a requirement is selected, but not met.
AC_ARG_ENABLE(strict,
  AC_HELP_STRING(
    [enable strict mode]),
  [use_strict="yes"],
  [use_strict="no"]
)

#####################################################################
# Check evas strong dependencie

PKG_CHECK_MODULES([EINA], [eina-0])

PKG_CHECK_MODULES([FREETYPE], [freetype2 >= 9.3.0])

#######################################
## Simple X11 build/link

AC_ARG_ENABLE(simple-x11,
  AC_HELP_STRING([--enable-simple-x11], [enable simple x11 linking]),
  [ want_evas_simple_x11=$enableval ]
)

#######################################
## FontConfig

want_fontconfig="auto"
have_fontconfig="no"
AC_ARG_ENABLE(fontconfig,
  AC_HELP_STRING(
    [--disable-fontconfig],
    [disable fontconfig for finding fonts. [[default=enabled]]]
  ),
  [ want_fontconfig=$enableval ]
)

if test "x$want_fontconfig" = "xyes" -o "x$want_fontconfig" = "xauto" ; then
  # Check if really available
  PKG_CHECK_MODULES(FONTCONFIG, fontconfig,
    [
      have_fontconfig="yes"
      AC_DEFINE(HAVE_FONTCONFIG, 1, [have fontconfig searching capabilities])
    ],
    [
      if test "x$want_fontconfig" = "xyes" -a "x$use_strict" = "xyes" ; then
        AC_MSG_ERROR([Fontconfig not found (strict dependencies checking)])
      fi
    ])
fi

###############
## dlopen

dlopen_libs=""
case "$host_os" in
  mingw32ce* | cegcc*)
dnl managed by evil
    AC_DEFINE(HAVE_DLADDR)
  ;;
  mingw*)
dnl nothing on mingw platform
  ;;
  *)
    AC_CHECK_FUNCS(dlopen, res=yes, res=no)
    if test "x$res" = "xyes"; then
      AC_CHECK_FUNCS(dladdr, AC_DEFINE(HAVE_DLADDR))
    else
      AC_CHECK_LIB(dl, dlopen, res=yes, res=no)
      if test "x$res" = "xyes"; then
        AC_CHECK_LIB(dl, dladdr, AC_DEFINE(HAVE_DLADDR))
        dlopen_libs=-ldl
      else
        AC_MSG_ERROR(Cannot find dlopen)
      fi
    fi
esac
AC_SUBST(dlopen_libs)

AC_CHECK_HEADER([fnmatch.h],
   [dummy="yes"],
   [AC_MSG_ERROR([Cannot find fnmatch.h. Make sure your CFLAGS environment variable contains include lines for the location of this file. MinGW users: see the INSTALL file])])

AC_CHECK_FUNCS([fnmatch], [res="yes"], [res="no"])
if test "x$res" = "xno"; then
   AC_SEARCH_LIBS([fnmatch],
      [fnmatch evil iberty],
      [res="yes"],
      [res="no"])
   if test "x$res" = "xno"; then
      AC_MSG_ERROR([Cannot find fnmatch() in neither libc nor libfnmatch, nor libiberty, nor libevil])
   fi
fi

#####################################################################
## Engines

EVAS_CHECK_ENGINE([buffer], [${want_evas_engine_buffer}], [no], [Buffer])

EVAS_CHECK_ENGINE([software-x11], [${want_evas_engine_software_x11}], [yes], [Software X11])

EVAS_CHECK_ENGINE([xrender-x11], [${want_evas_engine_xrender_x11}], [yes], [XRender X11])

EVAS_CHECK_ENGINE([gl-x11], [${want_evas_engine_gl_x11}], [yes], [OpenGL X11])

EVAS_CHECK_ENGINE([glitz-x11], [${want_evas_engine_glitz_x11}], [yes], [Glitz X11])

EVAS_CHECK_ENGINE([cairo-x11], [${want_evas_engine_cairo_x11}], [yes], [Cairo X11])

EVAS_CHECK_ENGINE([software-xcb], [${want_evas_engine_software_xcb}], [no], [Software XCB])

EVAS_CHECK_ENGINE([xrender-xcb], [${want_evas_engine_xrender_xcb}], [no], [XRender XCB])

EVAS_CHECK_ENGINE([software-ddraw], [${want_evas_engine_software_ddraw}], [no], [Software DirectDraw])

EVAS_CHECK_ENGINE([direct3d], [${want_evas_engine_direct3d}], [no], [Direct3D])

EVAS_CHECK_ENGINE([quartz], [${want_evas_engine_quartz}], [no], [Quartz])

EVAS_CHECK_ENGINE([gl-glew], [${want_evas_engine_gl_glew}], [no], [OpenGL Glew])

EVAS_CHECK_ENGINE([software-sdl], [${want_evas_engine_software_sdl}], [no], [Software SDL])

EVAS_CHECK_ENGINE([fb], [${want_evas_engine_fb}], [no], [Framebuffer])

EVAS_CHECK_ENGINE([directfb], [${want_evas_engine_directfb}], [no], [DirectFB])

EVAS_CHECK_ENGINE([software-qtopia], [${want_evas_engine_software_qtopia}], [no], [Qtopia])

EVAS_CHECK_ENGINE([software-16-x11], [${want_evas_engine_software_16_x11}], [yes], [Software X11 16 bits])

EVAS_CHECK_ENGINE([software-16-ddraw], [${want_evas_engine_software_16_ddraw}], [no], [Software DirectDraw 16 bits])

EVAS_CHECK_ENGINE([software-16-wince], [${want_evas_engine_software_16_wince}], [no], [Software Windows CE 16 bits])

# common cairo
have_evas_engine_cairo_common="no"
if test "x${have_evas_engine_cairo_x11}" = "xyes" ; then
   have_evas_engine_cairo_common="yes"
   AC_DEFINE([BUILD_ENGINE_CAIRO_COMMON], [1], [Generic Cairo Rendering Support])
fi
AM_CONDITIONAL([BUILD_ENGINE_CAIRO_COMMON], [test "x$have_evas_engine_cairo_common" = "xyes"])

# common gl
have_evas_engine_gl_common="no"
if test "x$have_evas_engine_gl_x11" = "xyes" -o "x$have_evas_engine_gl_glew" = "xyes"; then
   AC_DEFINE(BUILD_ENGINE_GL_COMMON, 1, [Generic OpenGL Rendering Support])
   have_evas_engine_gl_common="yes"
   evas_engine_gl_common_libs="-lglu32"
fi

if test "x$have_evas_engine_gl_x11" = "xyes" ; then
   evas_engine_gl_common_libs="-lGL -lGLU -lpthread"
fi

if test "x$have_evas_engine_gl_glew" = "xyes" ; then
   evas_engine_gl_common_libs="-lglu32"
fi
AC_SUBST([evas_engine_gl_common_libs])

AM_CONDITIONAL(BUILD_ENGINE_GL_COMMON, test "x$have_evas_engine_gl_common" = "xyes")

# SDL primitive
sdl_primitive="no"

AC_ARG_ENABLE([sdl-primitive],
   [AC_HELP_STRING([--enable-sdl-primitive], [])],
   [sdl_primitive=${enableval}]
)
AC_MSG_CHECKING([whether to use SDL primitive when possible])
AC_MSG_RESULT([${sdl_primitive}])

if test "x${sdl_primitive}" = "xyes" ; then
   AC_DEFINE([ENGINE_SDL_PRIMITIVE], [1], [Use SDL primitive when possible])
fi

# if software 16 x11 is enabled - build software_16 (the generic 16bit
# engine). later enable it fb_16 or other "16" bit engines are enabled.
have_evas_engine_software_16="no"
if test "x$have_evas_engine_software_16_x11" = "xyes"; then
   have_evas_engine_software_16="yes"
fi
if test "x$have_evas_engine_software_sdl" = "xyes"; then
   have_evas_engine_software_16="yes"
fi
if test "x$have_evas_engine_software_16_ddraw" = "xyes"; then
   have_evas_engine_software_16="yes"
fi
if test "x$have_evas_engine_software_16_wince" = "xyes"; then
   have_evas_engine_software_16="yes"
fi
AM_CONDITIONAL(BUILD_ENGINE_SOFTWARE_16, test "x$have_evas_engine_software_16" = "xyes")


#####################################################################
## Image loaders

EVAS_CHECK_IMAGE_LOADER([Edb], [${want_evas_image_loader_edb}])

EVAS_CHECK_IMAGE_LOADER([Eet], [${want_evas_image_loader_eet}])
EVAS_CHECK_FONT_LOADER([${want_evas_font_loader_eet}])

EVAS_CHECK_IMAGE_LOADER([Gif], [${want_evas_image_loader_gif}])

have_evas_image_saver_jpeg="no"
EVAS_CHECK_IMAGE_LOADER([Jpeg], [${want_evas_image_loader_jpeg}])

dnl Windows has no sigsetjmp function, nor equivalent.
dnl So we disable the jpeg saver.
dnl TODO: must find a workaround
case "$host_os" in
   mingw* | cegcc*)
      ;;
   *)
      if test "x${have_evas_image_loader_jpeg}" = "xyes" ; then
         have_evas_image_saver_jpeg="yes"
      fi
      ;;
esac
AM_CONDITIONAL([BUILD_SAVER_JPEG], [test "x${have_evas_image_saver_jpeg}" = "xyes"])

EVAS_CHECK_IMAGE_LOADER([PMAPS], [${want_evas_image_loader_pmaps}])

EVAS_CHECK_IMAGE_LOADER([PNG], [${want_evas_image_loader_png}])

EVAS_CHECK_IMAGE_LOADER([SVG], [${want_evas_image_loader_svg}])

EVAS_CHECK_IMAGE_LOADER([Tiff], [${want_evas_image_loader_tiff}])

EVAS_CHECK_IMAGE_LOADER([XPM], [${want_evas_image_loader_xpm}])


#####################################################################
## Cpu based optimizations

#######################################
## PTHREADS
pthread_cflags=""
pthread_libs=""
build_pthreads="no"
has_pthreads="no"
need_pthreads="no"
# basic pthread support
AC_CHECK_HEADER(pthread.h,
  [
   has_pthreads="yes"
  ],
  [
   has_pthreads="no"
  ]
)

# sched_getaffinity pthread_attr_setaffinity_np
AC_CHECK_HEADERS(pthread.h sched.h,
      [
        AC_CHECK_LIB(pthread, pthread_attr_setaffinity_np,
          [
            AC_CHECK_LIB(pthread, pthread_barrier_wait,
              [ build_pthreads="yes" ],
              [ build_pthreads="no" ]
            )
          ],
          [ build_pthreads="no" ]
        )
      ],
      [ build_pthreads="no" ]
)
### disable pthreads by default for now - some wierd deadlock issue with
# barriers (makes no sense)
#build_pthreads="no"
AC_MSG_CHECKING(whether to build pthread code)
AC_ARG_ENABLE(pthreads,
  AC_HELP_STRING([--enable-pthreads], [enable threaded rendering]),
  [
      if test "x$enableval" = "xyes" ; then
        if test "x$build_pthreads" = "xyes"; then
          AC_MSG_RESULT(yes)
          AC_DEFINE(BUILD_PTHREAD, 1, [Build Threaded Rendering])
          build_pthreads="yes"
	  need_pthreads="yes"
        else
          if "x$use_strict" = "xyes"; then
            AC_MSG_ERROR(pthreads headers or functions not found (strict dependencies checking))
          else
            AC_MSG_RESULT(no: pthread headers or functions not found)
          fi
        fi
      else
        AC_MSG_RESULT(no)
        build_pthreads="no"
      fi
  ],
  [
    AC_MSG_RESULT($build_pthreads)
    if test "x$build_pthreads" = "xyes" ; then
      AC_DEFINE(BUILD_PTHREAD, 1, [Build Threaded Rendering])
      need_pthreads="yes"
    fi
  ]
)

#######################################
## Async events
build_async_events="auto"
AC_MSG_CHECKING(whether to build Async Events support)
AC_ARG_ENABLE(async-events,
  AC_HELP_STRING([--enable-async-events], [enable async events support]),
  [ build_async_events=$enableval ]
)
AC_MSG_RESULT($build_async_events)

AC_MSG_CHECKING(whether we can build Async Events support)
if test \( "x$build_async_events" = "xyes" -o "x$build_async_events" = "xauto" \) -a "x$has_pthreads" = "xyes"; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(BUILD_ASYNC_EVENTS, 1, [Build async events support])
  build_async_events="yes"
  need_pthreads="yes"
else
  AC_MSG_RESULT(no)
  build_async_events="no"
fi

#######################################
## Async image preload
build_async_preload="auto"
AC_MSG_CHECKING(whether to build Async Image Preload support)
AC_ARG_ENABLE(async-preload,
  AC_HELP_STRING([--enable-async-preload], [enable async image preloading support]),
  [ build_async_preload=$enableval ]
)
AC_MSG_RESULT($build_async_preload)

AC_MSG_CHECKING(whether we can build Async Image Preload support)
if test \( "x$build_async_preload" = "xyes" -o "x$build_async_preload" = "xauto" \) -a "x$build_async_events" = "xyes"; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(BUILD_ASYNC_PRELOAD, 1, [Build async image preload support])
  build_async_preload="yes"
  need_pthreads="yes"
else
  AC_MSG_RESULT(no)
  build_async_preload="no"
fi

#######################################
## Link with pthread if needed
AC_MSG_CHECKING(whether we should link with pthread)
if test "x$need_pthreads" = "xyes"; then
  AC_MSG_RESULT(yes)
  pthread_cflags=""
  pthread_libs="-lpthread"
else
  AC_MSG_RESULT(no)
fi

#######################################
## MMX
build_cpu_mmx="no"
case $host_cpu in
  i*86)
    build_cpu_mmx="yes"
    ;;
  x86_64)
    build_cpu_mmx="yes"
    ;;
esac
AC_MSG_CHECKING(whether to build mmx code)
AC_ARG_ENABLE(cpu-mmx,
  AC_HELP_STRING([--enable-cpu-mmx], [enable mmx code]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(BUILD_MMX, 1, [Build MMX Code])
        build_cpu_mmx="yes"
      else
        AC_MSG_RESULT(no)
        build_cpu_mmx="no"
      fi
  ],
  [
    AC_MSG_RESULT($build_cpu_mmx)
    if test "x$build_cpu_mmx" = "xyes" ; then
      AC_DEFINE(BUILD_MMX, 1, [Build MMX Code])
    fi
  ]
)

#######################################
## SSE
build_cpu_sse="no"
case $host_cpu in
  i*86)
    build_cpu_sse="yes"
    ;;
  x86_64)
    build_cpu_sse="yes"
    ;;
esac
AC_MSG_CHECKING(whether to build sse code)
AC_ARG_ENABLE(cpu-sse,
  AC_HELP_STRING([--enable-cpu-sse], [enable sse code]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(BUILD_SSE, 1, [Build SSE Code])
        build_cpu_sse="yes"
      else
        AC_MSG_RESULT(no)
        build_cpu_sse="no"
      fi
  ],
  [
    AC_MSG_RESULT($build_cpu_sse)
    if test "x$build_cpu_sse" = "xyes" ; then
      AC_DEFINE(BUILD_SSE, 1, [Build SSE Code])
    fi
  ]
)

#######################################
## ALTIVEC
build_cpu_altivec="no"
case $host_cpu in
  *power* | *ppc*)
    build_cpu_altivec="auto"
    ;;
esac
altivec_cflags=""
AC_MSG_CHECKING(whether to build altivec code)
AC_ARG_ENABLE(cpu-altivec,
  AC_HELP_STRING([--enable-cpu-altivec], [enable altivec code]),
  [ build_cpu_altivec=$enableval ],
  [
    if test ! "x$build_cpu_altivec" = "xauto"; then
      build_cpu_altivec="no"
    fi
  ]
)
AC_MSG_RESULT($build_cpu_altivec)

if test "x$build_cpu_altivec" = "xyes"; then
   AC_CHECK_HEADER(altivec.h,
     [
        AC_DEFINE(BUILD_ALTIVEC, 1, [Build Altivec Code])
        AC_DEFINE(HAVE_ALTIVEC_H, 1, [Have altivec.h header file])
        build_cpu_altivec="yes"
     ],
     [
       save_CFLAGS=$CFLAGS
       save_CPPFLAGS=$CPPFLAGS
       CFLAGS=$CFLAGS" -maltivec"
       CPPFLAGS=$CPPFLAGS" -maltivec"
       unset ac_cv_header_altivec_h
       AC_CHECK_HEADER(altivec.h,
         [
            AC_DEFINE(BUILD_ALTIVEC, 1, [Build Altivec Code])
            AC_DEFINE(HAVE_ALTIVEC_H, 1, [Have altivec.h header file])
            build_cpu_altivec="yes"
         ],
         [
            if test "x$build_cpu_altivec" = "xyes" -a "x$use_strict" = "xyes" ; then
              AC_MSG_ERROR(Altivec not found (strict dependencies checking))
            fi
            build_cpu_altivec="no"
         ]
       )
       CFLAGS=$save_CFLAGS
       CPPFLAGS=$save_CPPFLAGS
     ]
   )
fi

if test "x$build_cpu_altivec" = "xyes"; then
   AC_MSG_CHECKING(whether to use altivec compiler flag)
   if test "x$GCC" = "xyes"; then
      if echo "int main(){return 0;}" | ${CPP} -faltivec - > /dev/null 2>&1; then
         altivec_cflags="-faltivec"
         AC_DEFINE(BUILD_ALTIVEC, 1, [Build Altivec Code])
      elif echo "int main(){return 0;}" | ${CPP} -maltivec - > /dev/null 2>&1; then
         altivec_cflags="-maltivec"
         AC_DEFINE(BUILD_ALTIVEC, 1, [Build Altivec Code])
      fi
   fi
   AC_MSG_RESULT($altivec_cflags)
   CFLAGS="$CFLAGS $altivec_cflags"
fi


#######################################
## C
build_cpu_c="no"
build_cpu_c="yes"
AC_MSG_CHECKING(whether to build c code)
AC_ARG_ENABLE(cpu-c,
  AC_HELP_STRING([--enable-cpu-c], [enable C code]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(BUILD_C, 1, [Build plain C code])
        build_cpu_c="yes"
      else
        AC_MSG_RESULT(no)
        build_cpu_c="no"
      fi
  ], [
      AC_MSG_RESULT($build_cpu_c)
      if test "x$build_cpu_c" = "xyes" ; then
        AC_DEFINE(BUILD_C, 1, [Build plain C code])
      fi
  ]
)

#######################################
## MAGIC_DEBUG
want_evas_magic_debug="yes"
AC_MSG_CHECKING(whether to check magic for evas object)
AC_ARG_ENABLE(evas-magic-debug,
  AC_HELP_STRING(
    [--disable-evas-magic-debug],
    [disable MAGIC_DEBUG check when people pass in wrong object type. [[default=enabled]]]
  ),
  [ want_evas_magic_debug="$enableval" ]
)
AC_MSG_RESULT($want_evas_magic_debug)

AM_CONDITIONAL(EVAS_MAGIC_DEBUG, test "x$want_evas_magic_debug" = "xyes")
if test "x$want_evas_magic_debug" = "xyes"; then
  AC_DEFINE(EVAS_MAGIC_DEBUG, 1, [complain when peole pass in wrong object types etc.])
fi

#####################################################################
## ARGB engine options

#######################################
## Nearest sampling scaler
scaler_sample="no"
scaler_sample="yes"
AC_MSG_CHECKING(whether to build sampling scaler)
AC_ARG_ENABLE(scale-sample,
  AC_HELP_STRING([--enable-scale-sample], [enable sampling scaler code]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(BUILD_SCALE_SAMPLE, 1, [Sampling Scaler Support])
        scaler_sample="yes"
      else
        AC_MSG_RESULT(no)
        scaler_sample="no"
      fi
  ], [
      AC_MSG_RESULT($scaler_sample)
      if test "x$scaler_sample" = "xyes" ; then
        AC_DEFINE(BUILD_SCALE_SAMPLE, 1, [Sampling Scaler Support])
      fi
  ]
)

#######################################
## Smooth super and sub sampling scaler
scaler_smooth="no"
scaler_smooth="yes"
AC_MSG_CHECKING(whether to build smooth scaler)
AC_ARG_ENABLE(scale-smooth,
  AC_HELP_STRING([--enable-scale-smooth], [enable smooth scaler code]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(BUILD_SCALE_SMOOTH, 1, [Smooth Scaler Support])
        scaler_smooth="yes"
      else
        AC_MSG_RESULT(no)
        scaler_smooth="no"
      fi
  ], [
      AC_MSG_RESULT($scaler_smooth)
      if test "x$scaler_smooth" = "xyes" ; then
        AC_DEFINE(BUILD_SCALE_SMOOTH, 1, [Smooth Scaler Support])
      fi
  ]
)

#######################################
## YUV -> ARGB converter
conv_yuv="no"
conv_yuv="yes"
AC_MSG_CHECKING(whether to build yuv converter code)
AC_ARG_ENABLE(convert-yuv,
  AC_HELP_STRING([--enable-convert-yuv], [enable yuv converter code]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(BUILD_CONVERT_YUV, 1, [YUV Converter Support])
        conv_yuv="yes"
      else
        AC_MSG_RESULT(no)
        conv_yuv="no"
      fi
  ], [
      AC_MSG_RESULT($conv_yuv)
      if test "x$conv_yuv" = "xyes" ; then
        AC_DEFINE(BUILD_CONVERT_YUV, 1, [YUV Converter Support])
      fi
  ]
)

#####################################################################
## Output rendering features

#######################################
## Small dither mask instead of big one (lower quality)
conv_small_dither="no"
AC_MSG_CHECKING(whether to build small dither mask code)
AC_ARG_ENABLE(small-dither-mask,
  AC_HELP_STRING([--enable-small-dither-mask], [enable small dither mask code]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(BUILD_SMALL_DITHER_MASK, 1, [Small Dither Mask Support])
        conv_small_dither="yes"
      else
        AC_MSG_RESULT(no)
        conv_small_dither="no"
      fi
  ], [
      AC_MSG_RESULT($conv_small_dither)
      if test "x$conv_small_dither" = "xyes" ; then
        AC_DEFINE(BUILD_SMALL_DITHER_MASK, 1, [Small Dither Mask Support])
      fi
  ]
)

#######################################
## Alternate Line dither mask instead of big one (lower quality - but fastest)
conv_line_dither="no"
AC_MSG_CHECKING(whether to build line dither mask code)
AC_ARG_ENABLE(line-dither-mask,
  AC_HELP_STRING([--enable-line-dither-mask], [enable line dither mask code]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(BUILD_LINE_DITHER_MASK, 1, [Line Dither Mask Support])
        conv_line_dither="yes"
      else
        AC_MSG_RESULT(no)
        conv_line_dither="no"
      fi
  ], [
      AC_MSG_RESULT($conv_line_dither)
      if test "x$conv_line_dither" = "xyes" ; then
        AC_DEFINE(BUILD_LINE_DITHER_MASK, 1, [Line Dither Mask Support])
      fi
  ]
)

#######################################
## No dither mask at all for 16bpp
conv_no_dither="no"
AC_MSG_CHECKING(whether to build without dither mask for 16bpp)
AC_ARG_ENABLE(no-dither-mask,
  AC_HELP_STRING([--enable-no-dither-mask], [enable conversion to 16bpp without dither mask]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(BUILD_NO_DITHER_MASK, 1, [No Dither Mask Support])
        conv_no_dither="yes"
      else
        AC_MSG_RESULT(no)
        conv_no_dither="no"
      fi
  ], [
      AC_MSG_RESULT($conv_no_dither)
      if test "x$conv_no_dither" = "xyes" ; then
        AC_DEFINE(BUILD_NO_DITHER_MASK, 1, [No Dither Mask Support])
      fi
  ]
)

#######################################
## Convert to 8bpp RGB 332
EVAS_CONVERT_COLOR(8, RGB, 332, [yes])
## Convert to 8bpp RGB 666
EVAS_CONVERT_COLOR(8, RGB, 666, [yes])
## Convert to 8bpp RGB 232
EVAS_CONVERT_COLOR(8, RGB, 232, [yes])
## Convert to 8bpp RGB 222
EVAS_CONVERT_COLOR(8, RGB, 222, [yes])
## Convert to 8bpp RGB 221
EVAS_CONVERT_COLOR(8, RGB, 221, [yes])
## Convert to 8bpp RGB 121
EVAS_CONVERT_COLOR(8, RGB, 121, [yes])
## Convert to 8bpp RGB 111
EVAS_CONVERT_COLOR(8, RGB, 111, [yes])
## Convert to 16bpp RGB 565
EVAS_CONVERT_COLOR(16, RGB, 565)
## Convert to 16bpp BGR 565
EVAS_CONVERT_COLOR(16, BGR, 565)
## Convert to 16bpp RGB 555
EVAS_CONVERT_COLOR(16, RGB, 555)
## Convert to 16bpp RGB 444
EVAS_CONVERT_COLOR(16, RGB, 444)

#######################################
## Convert to 16bpp RGB 565 (444 ipaq)
conv_16_rgb_ipq="yes"
AC_MSG_CHECKING(whether to build 16bpp 565 (444 ipaq) converter code)
AC_ARG_ENABLE(convert-16-rgb-ipq,
  AC_HELP_STRING([--disable-convert-16-rgb-ipq], [disable 16bpp 565 (444 ipaq) converter code]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_DEFINE(BUILD_CONVERT_16_RGB_454645, 1, [16bpp 565 (444 ipaq) Converter Support])
        conv_16_rgb_ipq="yes"
      else
        conv_16_rgb_ipq="no"
      fi
  ], [
      if test "x$conv_16_rgb_ipq" = "xyes" ; then
        AC_DEFINE(BUILD_CONVERT_16_RGB_454645, 1, [16bpp 565 (444 ipaq) Converter Support])
      fi
  ]
)
AC_MSG_RESULT($conv_16_rgb_ipq)

#######################################
## Convert to 16bpp RGB with rotation of 0
EVAS_CONVERT_ROT(16, RGB, 0)
## Convert to 16bpp RGB with rotation of 180
EVAS_CONVERT_ROT(16, RGB, 180)
## Convert to 16bpp RGB with rotation of 270
EVAS_CONVERT_ROT(16, RGB, 270)
## Convert to 16bpp RGB with rotation of 90
EVAS_CONVERT_ROT(16, RGB, 90)

#######################################
## Convert to 24bpp RGB 888
EVAS_CONVERT_COLOR(24, RGB, 888)
## Convert to 24bpp BGR 888
EVAS_CONVERT_COLOR(24, BGR, 888)
## Convert to 32bpp RGB 8888
EVAS_CONVERT_COLOR(32, RGB, 8888)
## Convert to 32bpp RGBX 8888
EVAS_CONVERT_COLOR(32, RGBX, 8888)
## Convert to 32bpp BGR 8888
EVAS_CONVERT_COLOR(32, BGR, 8888)
## Convert to 32bpp BGRX 8888
EVAS_CONVERT_COLOR(32, BGRX, 8888)

#######################################
## Convert to 32bpp RGB with rotation of 0
EVAS_CONVERT_ROT(32, RGB, 0)
## Convert to 32bpp RGB with rotation of 180
EVAS_CONVERT_ROT(32, RGB, 180)
## Convert to 32bpp RGB with rotation of 270
EVAS_CONVERT_ROT(32, RGB, 270)
## Convert to 32bpp RGB with rotation of 90
EVAS_CONVERT_ROT(32, RGB, 90)

## valgrind
want_valgrind="no"
have_valgrind="no"

AC_MSG_CHECKING(whether to enable build with valgrind)
AC_ARG_ENABLE(valgrind,
  AC_HELP_STRING([--enable-valgrind], [enable valgrind fixes to stop false reports]),
  [ want_valgrind=$enableval ]
)
AC_MSG_RESULT($want_valgrind)

if test x$want_valgrind = "xyes"; then
  PKG_CHECK_MODULES(VALGRIND, valgrind >= 2.4.0,
    [
      AC_DEFINE(HAVE_VALGRIND, 1, [Valgrind support])
      have_valgrind=yes
    ],
    [
      if test "x$want_valgrind" = "xyes" -a "x$use_strict" = "xyes" ; then
        AC_MSG_ERROR([Valgrind not found (strict dependencies checking)])
      fi
    ]
  )
fi

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])
#####################################################################
## Fill in flags

AC_SUBST(altivec_cflags)
AC_SUBST(pthread_cflags)
AC_SUBST(pthread_libs)


#####################################################################
## Output

AC_OUTPUT([
Makefile
evas-cairo-x11.pc
evas-directfb.pc
evas-fb.pc
evas-glitz-x11.pc
evas-opengl-glew.pc
evas-opengl-x11.pc
evas-quartz.pc
evas-software-buffer.pc
evas-software-qtopia.pc
evas-software-x11.pc
evas-software-16-x11.pc
evas-software-xcb.pc
evas-xrender-x11.pc
evas-xrender-xcb.pc
evas-software-ddraw.pc
evas-software-16-ddraw.pc
evas-direct3d.pc
evas-software-16-wince.pc
evas-software-sdl.pc
evas.pc
doc/evas.dox
doc/Makefile
src/Makefile
src/lib/Makefile
src/lib/canvas/Makefile
src/lib/data/Makefile
src/lib/file/Makefile
src/lib/imaging/Makefile
src/lib/cache/Makefile
src/lib/engines/Makefile
src/lib/engines/common/Makefile
src/lib/engines/common/evas_op_add/Makefile
src/lib/engines/common/evas_op_blend/Makefile
src/lib/engines/common/evas_op_copy/Makefile
src/lib/engines/common/evas_op_mask/Makefile
src/lib/engines/common/evas_op_mul/Makefile
src/lib/engines/common/evas_op_sub/Makefile
src/lib/engines/common_16/Makefile
src/modules/Makefile
src/modules/engines/Makefile
src/modules/engines/software_generic/Makefile
src/modules/engines/software_ddraw/Makefile
src/modules/engines/direct3d/Makefile
src/modules/engines/software_16_wince/Makefile
src/modules/engines/software_x11/Makefile
src/modules/engines/fb/Makefile
src/modules/engines/buffer/Makefile
src/modules/engines/software_win32_gdi/Makefile
src/modules/engines/software_qtopia/Makefile
src/modules/engines/directfb/Makefile
src/modules/engines/gl_common/Makefile
src/modules/engines/gl_glew/Makefile
src/modules/engines/gl_x11/Makefile
src/modules/engines/quartz/Makefile
src/modules/engines/cairo_common/Makefile
src/modules/engines/cairo_x11/Makefile
src/modules/engines/xrender_x11/Makefile
src/modules/engines/xrender_xcb/Makefile
src/modules/engines/software_sdl/Makefile
src/modules/engines/glitz_x11/Makefile
src/modules/engines/software_16/Makefile
src/modules/engines/software_16_x11/Makefile
src/modules/engines/software_16_ddraw/Makefile
src/modules/engines/software_16_sdl/Makefile
src/modules/loaders/Makefile
src/modules/loaders/edb/Makefile
src/modules/loaders/eet/Makefile
src/modules/loaders/gif/Makefile
src/modules/loaders/jpeg/Makefile
src/modules/loaders/png/Makefile
src/modules/loaders/tiff/Makefile
src/modules/loaders/xpm/Makefile
src/modules/loaders/svg/Makefile
src/modules/loaders/pmaps/Makefile
src/modules/savers/Makefile
src/modules/savers/edb/Makefile
src/modules/savers/eet/Makefile
src/modules/savers/jpeg/Makefile
src/modules/savers/png/Makefile
src/modules/savers/tiff/Makefile
src/lib/include/Makefile
README
evas.spec
])

#####################################################################
## Info

echo
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE $VERSION"
echo "------------------------------------------------------------------------"
echo
echo "Configuration Options Summary:"
echo
echo "Engines:"
echo "  Software Memory Buffer.....: $have_evas_engine_buffer"
echo "  Software X11...............: $have_evas_engine_software_x11 (XCB: $have_evas_engine_software_xcb)"
echo "  XRender X11................: $have_evas_engine_xrender_x11"
echo "  OpenGL X11.................: $have_evas_engine_gl_x11"
echo "  Glitz X11..................: $have_evas_engine_glitz_x11"
echo "  Cairo X11..................: $have_evas_engine_cairo_x11"
echo "  XRender XCB................: $have_evas_engine_xrender_xcb"
echo "  Software DirectDraw........: $have_evas_engine_software_ddraw"
echo "  Direct3d...................: $have_evas_engine_direct3d"
echo "  Quartz.....................: $have_evas_engine_quartz"
echo "  OpenGL Glew................: $have_evas_engine_gl_glew"
echo "  Software SDL...............: $have_evas_engine_software_sdl (primitive: $sdl_primitive)"
echo "  Software Framebuffer.......: $have_evas_engine_fb"
echo "  DirectFB...................: $have_evas_engine_directfb"
echo "  Software Qtopia............: $have_evas_engine_software_qtopia"
echo "  Software 16bit ............: $have_evas_engine_software_16"
echo "  Software 16bit X11.........: $have_evas_engine_software_16_x11"
echo "  Software 16bit Directdraw..: $have_evas_engine_software_16_ddraw"
echo "  Software 16bit WinCE.......: $have_evas_engine_software_16_wince"
echo "  Software 16bit SDL.........: $have_evas_engine_software_sdl (primitive: $sdl_primitive)"
# FIXME: opengl engine needs to be fixed and tested lots for all drivers
# FIXME: xrender engine to be written
echo
echo "Image Loaders:"
echo "  EDB.....................: $have_evas_image_loader_edb"
echo "  EET.....................: $have_evas_image_loader_eet"
echo "  GIF.....................: $have_evas_image_loader_gif"
echo "  JPEG....................: $have_evas_image_loader_jpeg"
echo "  PMAPS...................: $have_evas_image_loader_pmaps"
echo "  PNG.....................: $have_evas_image_loader_png"
echo "  SVG.....................: $have_evas_image_loader_svg"
echo "  TIFF....................: $have_evas_image_loader_tiff"
echo "  XPM.....................: $have_evas_image_loader_xpm"
# FIXME: need to add modular image loader system
# FIXME: add more image loader modules
echo
echo "Font Sourcing Systems:"
echo "  EET.....................: $have_evas_font_loader_eet"
echo
echo "Font Searching Systems:"
echo "  Fontconfig..............: $have_fontconfig"
# FIXME: add non freetype2 font engine support
# FIXME: make freetype2 optional
echo
echo "CPU Specific Extensions:"
echo "  Fallback C Code.........: $build_cpu_c"
echo "  MMX.....................: $build_cpu_mmx"
echo "  SSE.....................: $build_cpu_sse"
echo "  ALTIVEC.................: $build_cpu_altivec"
echo "  Thread Support..........: $build_pthreads"
echo "  MAGIC_DEBUG.............: $want_evas_magic_debug"
echo
echo "Async Events..............: $build_async_events"
echo "Async Image Preload.......: $build_async_preload"
echo
echo "ARGB Software Engine Options:"
echo "  Sampling Scaler.........: $scaler_sample"
echo "  Smooth Scaler...........: $scaler_smooth"
# FIXME: add an mmx scaler routine
echo "  YUV Converter...........: $conv_yuv"
# FIXME: add more YUV format and colorvariant support
echo
echo "ARGB Conversion Options:"
echo "  Smaller Dither Mask.....: $conv_small_dither"
echo "  Line Dither Mask........: $conv_line_dither"
echo "  No Dither Mask for 16bpp: $conv_no_dither"
echo "  8bpp RGB 332............: $conv_8_rgb_332"
echo "  8bpp RGB 666............: $conv_8_rgb_666"
echo "  8bpp RGB 232............: $conv_8_rgb_232"
echo "  8bpp RGB 222............: $conv_8_rgb_222"
echo "  8bpp RGB 221............: $conv_8_rgb_221"
echo "  8bpp RGB 121............: $conv_8_rgb_121"
echo "  8bpp RGB 111............: $conv_8_rgb_111"
# FIXME: add grayscale and B&W support
echo "  16bpp RGB 565...........: $conv_16_rgb_565"
echo "  16bpp BGR 565...........: $conv_16_bgr_565"
echo "  16bpp RGB 555...........: $conv_16_rgb_555"
echo "  16bpp RGB 444...........: $conv_16_rgb_444"
echo "  16bpp RGB 565 (444 ipaq): $conv_16_rgb_ipq"
# FIXME: add 555 (444 ipaq) support
# FIXME: add 24bpp 666 support
# FIXME: add 32bpp 666 support
# FIXME: add 30bpp support
# FIXME: add palletted support
# FIXME: add 8bpp and below rotation
echo "  16bpp Rotation 0........: $conv_16_rgb_rot_0"
echo "  16bpp Rotation 90.......: $conv_16_rgb_rot_90"
echo "  16bpp Rotation 180......: $conv_16_rgb_rot_180"
echo "  16bpp Rotation 270......: $conv_16_rgb_rot_270"
echo "  24bpp RGB 888...........: $conv_24_rgb_888"
echo "  24bpp BGR 888...........: $conv_24_bgr_888"
# FIXME: add 24bpp rotation
echo "  32bpp RGB 8888..........: $conv_32_rgb_8888"
echo "  32bpp RGBX 8888.........: $conv_32_rgbx_8888"
echo "  32bpp BGR 8888..........: $conv_32_bgr_8888"
echo "  32bpp BGRX 8888.........: $conv_32_bgrx_8888"
echo "  32bpp Rotation 0........: $conv_32_rgb_rot_0"
echo "  32bpp Rotation 90.......: $conv_32_rgb_rot_90"
echo "  32bpp Rotation 180......: $conv_32_rgb_rot_180"
echo "  32bpp Rotation 270......: $conv_32_rgb_rot_270"
echo
echo "Documentation.............: ${build_doc}"
echo
echo "------------------------------------------------------------------------"
echo
echo "Now type 'make' ('gmake' on some systems) to compile $PACKAGE,"
echo "and then afterwards as root (or the user who will install this), type"
echo "'make install'. Change users with 'su' or 'sudo' appropriately."
echo

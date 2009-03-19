# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT([ecore], [0.9.9.050], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.52])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_ISC_POSIX

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

VMAJ=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $1);}'`
VMIN=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $2);}'`
VMIC=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $3);}'`
SNAP=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $4);}'`
version_info=`expr $VMAJ + $VMIN`":$VMIC:$VMIN"
AC_SUBST(version_info)

AC_DEFINE_UNQUOTED(SHARED_LIB_SUFFIX, "$shrext_cmds", [Suffix for shared objects])


### Default options with respect to host

# dependencies and options
want_curl="no"
want_abstract_sockets="no"
want_gnutls="no"
want_openssl="no"
want_cipher="no"
want_signature="no"
want_poll="yes"
want_inotify="no"
want_tslib="no"

# core modules
want_ecore_job="yes"
want_ecore_txt="yes"
want_ecore_con="no"
want_ecore_ipc="no"
want_ecore_file="yes"
want_ecore_config="no"
want_ecore_imf="no"
want_ecore_input="yes"

# graphic system modules
want_evas_simple_x11="no"
want_ecore_x_xcb="no"
want_ecore_x="no"
want_ecore_win32="no"
want_ecore_quartz="no"
want_ecore_sdl="no"
want_ecore_fb="no"
want_ecore_directfb="no"
want_ecore_wince="no"

# ecore_evas modules
want_ecore_evas="yes"
want_ecore_evas_software_buffer="yes"
want_ecore_evas_software_x11="no"
want_ecore_evas_xrender_x11="no"
want_ecore_evas_opengl_x11="no"
want_ecore_evas_software_16_x11="no"
want_ecore_evas_software_xcb="no"
want_ecore_evas_xrender_xcb="no"
want_ecore_evas_software_ddraw="no"
want_ecore_evas_direct3d="no"
want_ecore_evas_opengl_glew="no"
want_ecore_evas_software_16_ddraw="no"
want_ecore_evas_quartz="no"
want_ecore_evas_software_sdl="no"
want_ecore_evas_directfb="no"
want_ecore_evas_fb="no"
want_ecore_evas_software_16_wince="no"

case "$host_os" in
   mingw32ce* | cegcc*)
      want_ecore_wince="yes"
      want_ecore_evas_software_16_wince="yes"
      ;;
   mingw*)
      want_curl="yes"
      want_ecore_imf="yes"
      want_ecore_win32="yes"
      want_ecore_evas_software_ddraw="yes"
      want_ecore_evas_direct3d="yes"
      want_ecore_evas_opengl_glew="yes"
      want_ecore_evas_software_16_ddraw="auto"
      want_ecore_evas_software_sdl="yes"
      ;;
   darwin*)
      want_curl="yes"
      want_gnutls="auto"
      want_openssl="auto"
      want_ecore_con="yes"
      want_ecore_ipc="yes"
      want_ecore_config="yes"
      want_ecore_imf="yes"
      want_ecore_quartz="yes"
      want_ecore_evas_quartz="yes"
      want_ecore_evas_software_sdl="yes"
      ;;
   *)
      want_curl="yes"
      want_abstract_sockets="yes"
      want_gnutls="auto"
      want_openssl="auto"
      want_cipher="yes"
      want_signature="yes"
      want_inotify="yes"
      want_tslib="yes"
      want_ecore_con="yes"
      want_ecore_ipc="yes"
      want_ecore_config="yes"
      want_ecore_imf="yes"
      want_ecore_x="yes"
      want_ecore_evas_software_x11="yes"
      want_ecore_evas_xrender_x11="yes"
      want_ecore_evas_opengl_x11="yes"
      want_ecore_evas_software_16_x11="yes"
      want_ecore_evas_software_xcb="yes"
      want_ecore_evas_xrender_xcb="yes"
      want_ecore_evas_software_sdl="yes"
      want_ecore_evas_directfb="yes"
      want_ecore_evas_fb="yes"
      ;;
esac

requirements_ecore=""
requirements_ecore_con=""
requirements_ecore_config=""
requirements_ecore_directfb=""
requirements_ecore_evas=""
requirements_ecore_fb=""
requirements_ecore_file=""
requirements_ecore_imf=""
requirements_ecore_imf_evas=""
requirements_ecore_input=""
requirements_ecore_ipc=""
requirements_ecore_job=""
requirements_ecore_quartz=""
requirements_ecore_sdl=""
requirements_ecore_txt=""
requirements_ecore_x=""
requirements_ecore_win32=""
requirements_ecore_wince=""


### Additional options to configure

# abstract sockets (ecore_con.c)
AC_ARG_ENABLE([abstract-sockets],
   [AC_HELP_STRING([--disable-abstract-sockets], [disable abstract sockets.])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_abstract_sockets="yes"
    else
       want_abstract_sockets="no"
    fi
   ],
   [want_abstract_sockets="yes"])

if test "x${want_abstract_sockets}" = "xyes" ; then
   AC_DEFINE([HAVE_ABSTRACT_SOCKETS], [1], [Have abstract sockets namespace])
fi

# Simple X11 build/link

AC_ARG_ENABLE(simple-x11,
  AC_HELP_STRING([--enable-simple-x11], [enable simple x11 linking]),
  [want_evas_simple_x11=$enableval])


### Checks for programs

m4_ifdef([AC_PROG_OBJC],
   [
    AC_PROG_OBJC
    _AM_DEPENDENCIES(OBJC)
   ],
   [
    AC_CHECK_TOOL([OBJC], [gcc])
    AC_SUBST([OBJC])
    AC_SUBST([OBJCFLAGS])
   ])
AC_PROG_CXX
AC_PROG_CC

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])

# The first call to PKG_CHECK_MODULES is done conditionally,
# so we should include this here:
PKG_PROG_PKG_CONFIG


### Checks for libraries

# Evil library for compilation on Windows

case "$host_os" in
   mingw* | cegcc*)
      PKG_CHECK_MODULES([EVIL], [evil])
      AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if Evil library is installed])
      requirements_ecore="evil ${requirements_ecore}"
      requirements_ecore_evas="evil ${requirements_ecore_evas}"
      requirements_ecore_file="evil ${requirements_ecore_file}"
      requirements_ecore_imf="evil ${requirements_ecore_imf}"
      requirements_ecore_imf_evas="evil ${requirements_ecore_imf_evas}"
      requirements_ecore_job="evil ${requirements_ecore_job}"
      requirements_ecore_txt="evil ${requirements_ecore_txt}"
      EFL_ECORE_BUILD="-DEFL_ECORE_BUILD"
      EFL_ECORE_JOB_BUILD="-DEFL_ECORE_JOB_BUILD"
      EFL_ECORE_TXT_BUILD="-DEFL_ECORE_TXT_BUILD"
      EFL_ECORE_FILE_BUILD="-DEFL_ECORE_FILE_BUILD"
      EFL_ECORE_EVAS_BUILD="-DEFL_ECORE_EVAS_BUILD"
      EFL_ECORE_IMF_BUILD="-DEFL_ECORE_IMF_BUILD"
      EFL_ECORE_INPUT_BUILD="-DEFL_ECORE_INPUT_BUILD"
      ;;
esac

have_win32=""
have_wince=""
case "$host_os" in
   mingw32ce* | cegcc*)
      EFL_ECORE_WINCE_BUILD="-DEFL_ECORE_WINCE_BUILD"
      requirements_ecore_wince="evil ${requirements_ecore_wince}"
      have_wince="yes"
      ;;
   mingw*)
      EFL_ECORE_WIN32_BUILD="-DEFL_ECORE_WIN32_BUILD"
      EFL_ECORE_SDL_BUILD="-DEFL_ECORE_SDL_BUILD"
      requirements_ecore_win32="evil ${requirements_ecore_win32}"
      requirements_ecore_sdl="evil ${requirements_ecore_sdl}"
      have_win32="yes"
      ;;
esac

AC_SUBST(EFL_ECORE_BUILD)
AC_SUBST(EFL_ECORE_JOB_BUILD)
AC_SUBST(EFL_ECORE_TXT_BUILD)
AC_SUBST(EFL_ECORE_FILE_BUILD)
AC_SUBST(EFL_ECORE_EVAS_BUILD)
AC_SUBST(EFL_ECORE_IMF_BUILD)
AC_SUBST(EFL_ECORE_INPUT_BUILD)
AC_SUBST(EFL_ECORE_WINCE_BUILD)
AC_SUBST(EFL_ECORE_WIN32_BUILD)
AC_SUBST(EFL_ECORE_SDL_BUILD)

AM_CONDITIONAL(ECORE_HAVE_WINCE, test "x${have_wince}" = "xyes")
AM_CONDITIONAL(ECORE_HAVE_WIN32, test "x${have_win32}" = "xyes")

WIN32_LIBS=""
case "$host_os" in
   mingw32ce* | cegcc*)
      WIN32_LIBS="-lws2"
      dlopen_libs="-ldl"
      ;;
   mingw*)
      WIN32_LIBS="-lws2_32"
      dlopen_libs="-ldl"
      ;;
   *)
      AC_CHECK_LIB(dl, dlopen, dlopen_libs=-ldl)
      ;;
esac
AC_SUBST(WIN32_LIBS)
AC_SUBST(dlopen_libs)

# Eina library

PKG_CHECK_MODULES(EINA, [eina-0])
#FIXME check all the requirements when the eina move will be finished
requirements_ecore="eina-0 ${requirements_ecore}"
requirements_ecore_con="ecore eina-0 ${requirements_ecore_con}"
requirements_ecore_config="ecore eina-0 ${requirements_ecore_config}"
requirements_ecore_directfb="ecore eina-0 ${requirements_ecore_directfb}"
requirements_ecore_evas="ecore eina-0 ${requirements_ecore_evas}"
requirements_ecore_fb="ecore eina-0 ${requirements_ecore_fb}"
requirements_ecore_file="ecore eina-0 ${requirements_ecore_file}"
requirements_ecore_imf="ecore eina-0 ${requirements_ecore_imf}"
requirements_ecore_imf_evas="ecore eina-0 ${requirements_ecore_imf_evas}"
requirements_ecore_input="ecore eina-0 ${requirements_ecore_input}"
requirements_ecore_ipc="ecore eina-0 ${requirements_ecore_ipc}"
requirements_ecore_job="ecore eina-0 ${requirements_ecore_job}"
requirements_ecore_quartz="ecore eina-0 ${requirements_ecore_quartz}"
requirements_ecore_sdl="ecore eina-0 ${requirements_ecore_sdl}"
requirements_ecore_x="ecore eina-0 ${requirements_ecore_x}"
requirements_ecore_win32="ecore eina-0 ${requirements_ecore_win32}"
requirements_ecore_wince="ecore eina-0 ${requirements_ecore_wince}"


# iconv library (ecore_txt)

iconv_cflags=""
iconv_libs=""
have_iconv="no"
AC_ARG_WITH([iconv-link],
   AC_HELP_STRING([--with-iconv-link=ICONV_LINK], [explicitly specify an iconv link option]),
   [
    iconv_libs=$withval
    have_iconv="yes"
   ]
)

if test "x${have_iconv}" = "xno" ; then
   AC_CHECK_LIB(c, iconv,
      [have_iconv="yes"]   )

   if ! test "x${have_iconv}" = "xyes" ; then
      AC_CHECK_LIB(iconv, iconv,
         [
          iconv_libs="-liconv"
          have_iconv="yes"
         ])
   fi

   if test "x$have_iconv" != "xyes"; then
      AC_CHECK_LIB(iconv_plug, iconv,
         [
          iconv_libs="-liconv -liconv_plug"
          have_iconv="yes"
         ])
   fi
fi
AC_MSG_CHECKING(for explicit iconv link options)
AC_MSG_RESULT($iconv_libs)

AC_SUBST(iconv_cflags)
AC_SUBST(iconv_libs)


# SDL library (ecore_sdl)

have_sdl="no"
SDL_CONFIG="sdl-config"
AC_ARG_WITH([sdl-config],
   [AC_HELP_STRING([--with-sdl-config=PATH], [use sdl-config specified])],
   [
    SDL_CONFIG=$withval
    AC_MSG_NOTICE([using ${SDL_CONFIG} for sdl-config])
   ])

AC_PATH_PROG([SDL_CONFIG], ["sdl-config"], [""], [$PATH])

if test -z "$SDL_CONFIG" ; then
   SDL_CFLAGS=`$SDL_CONFIG --cflags`
   SDL_LIBS=`$SDL_CONFIG --libs`
   AC_SUBST(SDL_CFLAGS)
   AC_SUBST(SDL_LIBS)
   have_sdl="no"
else
   PKG_CHECK_MODULES([SDL], [sdl >= 1.2.0], [have_sdl="yes"], [have_sdl="no"])
fi

if test "x${have_sdl}" = "xyes" ; then
   PKG_CHECK_EXISTS([sdl >= 1.3.0],
      [AC_DEFINE(BUILD_ECORE_EVAS_SDL_130, 1, [Support for SVN SDL])])
fi


# DirectFB library (ecore_directfb)

PKG_CHECK_MODULES([DIRECTFB], [directfb >= 0.9.16],
   [
    have_directfb="yes"
    requirements_ecore_directfb="evas ${requirements_ecore_directfb}"
   ],
   [have_directfb="no"])


# Eet library (ecore_config)

PKG_CHECK_MODULES(EET, [eet >= 1.0.0],
  [ have_eet="yes" ],
  [ have_eet="no" ])


# Xlib and XCB (ecore_x)

have_x="no"
have_ecore_x="no"
have_ecore_x_xlib="no"
have_ecore_x_xcb="no"

x_dir="";
x_includes="";
x_cflags="";
x_libs="";

AC_ARG_ENABLE(ecore-x-xcb,
   [AC_HELP_STRING([--enable-ecore-x-xcb],
       [enable the ecore_x module with XCB backend. [[default=disabled]]])],
   [want_ecore_x_xcb=$enableval],
   [want_ecore_x_xcb="no"])

AC_MSG_CHECKING(whether ecore_x with XCB backend is to be built)
AC_MSG_RESULT($want_ecore_x_xcb)

if test "x$want_ecore_x_xcb" = "xyes" ; then
  PKG_CHECK_MODULES(XCB, xcb xcb-icccm xcb-image xcb-keysyms,
    [ have_ecore_x_xcb="yes" ],
    [ have_ecore_x_xcb="no" ])

  if test "x$have_ecore_x_xcb" = "xyes" ; then
    PKG_CHECK_MODULES(XCB_COMPOSITE, xcb-composite,
      [ have_ecore_x_xcb_composite="yes"
        AC_DEFINE(ECORE_XCB_COMPOSITE, 1, [Build support for XCB composite]) ],
      [ have_ecore_x_xcb_composite="no" ])

    PKG_CHECK_MODULES(XCB_DAMAGE, xcb-damage,
      [ have_ecore_x_xcb_damage="yes"
        AC_DEFINE(ECORE_XCB_DAMAGE, 1, [Build support for XCB damage]) ],
      [ have_ecore_x_xcb_damage="no" ])

    PKG_CHECK_MODULES(XCB_DPMS, xcb-dpms,
      [ have_ecore_x_xcb_dpms="yes"
        AC_DEFINE(ECORE_XCB_DPMS, 1, [Build support for XCB dpms]) ],
      [ have_ecore_x_xcb_dpms="no" ])

    PKG_CHECK_MODULES(XCB_RANDR, xcb-randr,
      [ have_ecore_x_xcb_randr="yes"
        AC_DEFINE(ECORE_XCB_RANDR, 1, [Build support for XCB randr]) ],
      [ have_ecore_x_xcb_randr="no" ])

    PKG_CHECK_MODULES(XCB_RENDER, xcb-render,
      [ have_ecore_x_xcb_render="yes"
        AC_DEFINE(ECORE_XCB_RENDER, 1, [Build support for XCB render]) ],
      [ have_ecore_x_xcb_render="no" ])

    PKG_CHECK_MODULES(XCB_SCREENSAVER, xcb-screensaver,
      [ have_ecore_x_xcb_screensaver="yes"
        AC_DEFINE(ECORE_XCB_SCREENSAVER, 1, [Build support for XCB screensaver]) ],
      [ have_ecore_x_xcb_screensaver="no" ])

    PKG_CHECK_MODULES(XCB_SHAPE, xcb-shape,
      [ have_ecore_x_xcb_shape="yes"
        AC_DEFINE(ECORE_XCB_SHAPE, 1, [Build support for XCB shape]) ],
      [ have_ecore_x_xcb_shape="no" ])

    PKG_CHECK_MODULES(XCB_SYNC, xcb-sync,
      [ have_ecore_x_xcb_sync="yes"
        AC_DEFINE(ECORE_XCB_SYNC, 1, [Build support for XCB sync]) ],
      [ have_ecore_x_xcb_sync="no" ])

    PKG_CHECK_MODULES(XCB_XFIXES, xcb-xfixes,
      [ have_ecore_x_xcb_xfixes="yes"
        AC_DEFINE(ECORE_XCB_FIXES, 1, [Build support for XCB xfixes]) ],
      [ have_ecore_x_xcb_xfixes="no" ])

    PKG_CHECK_MODULES(XCB_XINERAMA, xcb-xinerama,
      [ have_ecore_x_xcb_xinerama="yes"
        AC_DEFINE(ECORE_XCB_XINERAMA, 1, [Build support for XCB xinerama]) ],
      [ have_ecore_x_xcb_xinerama="no" ])

    PKG_CHECK_MODULES(XCB_XPRINT, xcb-xprint,
      [ have_ecore_x_xcb_xprint="yes"
        AC_DEFINE(ECORE_XCB_XPRINT, 1, [Build support for XCB xprint]) ],
      [ have_ecore_x_xcb_xprint="no" ])

    PKG_CHECK_MODULES(XCB_XTEST, xcb-xtest,
      [ have_ecore_x_xcb_xtest="yes"
        AC_DEFINE(ECORE_XCB_XTEST, 1, [Build support for XCB xtest]) ],
      [ have_ecore_x_xcb_xtest="no" ])

    AC_DEFINE(HAVE_ECORE_X_XCB, 1, [Defined to 1 if XCB is enabled.])

    x_cflags=$XCB_CFLAGS
    x_libs=$XCB_LIBS
    have_x="yes"

    have_ecore_x_xcb_define="-DHAVE_ECORE_X_XCB"
    AC_SUBST(have_ecore_x_xcb_define)
  fi
fi

if ! test "x$have_ecore_x_xcb" = "xyes" ; then
  AC_PATH_XTRA
  AC_CHECK_HEADER(X11/X.h,
    [
     if test "x$want_evas_simple_x11" = "xyes"; then
       x_libs="${x_libs} -lX11 -lXext"
     else
       x_dir=${x_dir:-/usr/X11R6}
       x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
       x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext"
     fi
     have_ecore_x_xlib="yes"
    ]
  )

  if test "x$have_ecore_x_xlib" = "xyes"; then
    Xcursor_libs=""
    Xcursor_cflags=""
    use_Xcursor="no"
    PCFLAGS=$CFLAGS
    CFLAGS="$x_cflags $x_includes"
    AC_CHECK_HEADER(X11/Xcursor/Xcursor.h,
      [
        AC_CHECK_LIB(Xcursor, XcursorImageLoadCursor,
          [
            AC_DEFINE(ECORE_XCURSOR, 1, [Build support for Xcursor])
            Xcursor_cflags=""
            Xcursor_libs="-lXcursor"
            use_Xcursor="yes"
          ], [
            Xcursor_cflags=""
            Xcursor_libs=""
            use_Xcursor="no"
          ], [
            $x_libs -lXrender
          ]
        )
      ], [
        Xcursor_cflags=""
        Xcursor_libs=""
        use_Xcursor="no"
      ], [
        #include <X11/Xlib.h>
      ]
    )
    CFLAGS=$PCFLAGS

    AC_SUBST(Xcursor_cflags)
    AC_SUBST(Xcursor_libs)

    ECORE_CHECK_X_EXTENSION([Xkb], [XKB.h], [X11], [XkbSetDetectableAutoRepeat])
    ECORE_CHECK_X_EXTENSION([Xcomposite], [Xcomposite.h], [Xcomposite], [XCompositeQueryExtension])
    ECORE_CHECK_X_EXTENSION([Xdamage], [Xdamage.h], [Xdamage], [XDamageSubtract])
    ECORE_CHECK_X_EXTENSION([Xdpms], [dpms.h], [Xdpms], [DPMSQueryExtension])
    if test "x$use_xdpms" = "xno" ; then
      ECORE_CHECK_X_EXTENSION([Xdpms], [dpms.h], [Xext], [DPMSQueryExtension])
    fi
    ECORE_CHECK_X_EXTENSION([Xfixes], [Xfixes.h], [Xfixes], [XFixesExpandRegion])
    ECORE_CHECK_X_EXTENSION([Xinerama], [Xinerama.h], [Xinerama], [XineramaQueryScreens])
    ECORE_CHECK_X_EXTENSION([Xprint], [Print.h], [Xp], [XpQueryScreens])
    ECORE_CHECK_X_EXTENSION([Xrandr], [Xrandr.h], [Xrandr], [XRRSizes])
    ECORE_CHECK_X_EXTENSION([Xrender], [Xrender.h], [Xrender], [XRenderFindVisualFormat])
    ECORE_CHECK_X_EXTENSION([Xtest], [XTest.h], [Xtst], [XTestFakeKeyEvent])
    ECORE_CHECK_X_EXTENSION([Xss], [scrnsaver.h], [Xss], [XScreenSaverSelectInput])

    AC_DEFINE(HAVE_ECORE_X_XLIB, 1, [Defined to 1 if Xlib is enabled.])
    have_x="yes"

    have_ecore_x_xlib="yes"
  fi
fi

AC_SUBST(x_cflags)
AC_SUBST(x_includes)
AC_SUBST(x_libs)

AM_CONDITIONAL(BUILD_ECORE_X_XLIB, test $have_ecore_x_xlib = yes)
AM_CONDITIONAL(BUILD_ECORE_X_XCB, test $have_ecore_x_xcb = yes)


# Evas library (ecore_evas and ecore_imf_evas)

PKG_CHECK_MODULES([EVAS], [evas >= 0.9.9],
   [
    have_evas="yes"
    requirements_ecore_evas="evas ${requirements_ecore_evas}"
   ],
   [have_evas="no"])


### Checks for header files

AC_HEADER_SYS_WAIT

case "$host_os" in
   mingw* | cegcc*)
      AC_DEFINE(HAVE_DLFCN_H, 1, [Define to 1 if you have the <dlfcn.h> header file.])
      AC_DEFINE(HAVE_SYS_MMAN_H, 1, [Define to 1 if you have the <sys/mman.h> header file.])
      AC_DEFINE(HAVE_SYS_TIME_H, 1, [Define to 1 if you have the <sys/time.h> header file.])
      ;;
   *)
      AC_CHECK_HEADERS([dlfcn.h features.h langinfo.h locale.h netdb.h netinet/in.h sys/time.h sys/mman.h signal.h sys/resource.h])
      ;;
esac

# Framebuffer (ecore_fb)
have_fb="no"
AC_CHECK_HEADER([linux/fb.h],
   [AC_CHECK_HEADER([linux/input.h], [have_fb="yes"])])

# Quartz header files (ecore_quartz)

quartz_cflags="";
quartz_libs="";
have_quartz="no"
AC_CHECK_HEADER([Cocoa/Cocoa.h],
   [
    have_quartz="yes"
    quartz_cflags="-framework Cocoa";
    quartz_libs="-framework Cocoa"
   ],
   [have_quartz="no"])

AC_SUBST(quartz_cflags)
AC_SUBST(quartz_libs)

### Checks for types
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)


### Checks for structures


### Checks for compiler characteristics
AM_PROG_CC_STDC
AC_C_CONST
AC_C_BIGENDIAN
AC_HEADER_STDC
AC_C___ATTRIBUTE__

WIN32_CPPFLAGS=""
WIN32_CFLAGS=""
case "$host_os" in
   mingw32ce*)
      WIN32_CPPFLAGS="-D_WIN32_WCE=0x0420"
      ;;
   cegcc*)
      WIN32_CPPFLAGS="-D_WIN32_WCE=0x0420"
      WIN32_CFLAGS="-mwin32"
      ;;
   mingw*)
      WIN32_CPPFLAGS="-D_WIN32_WINNT=0x0500"
      ;;
esac
AC_SUBST(WIN32_CPPFLAGS)
AC_SUBST(WIN32_CFLAGS)


### Checks for linker characteristics

# use --enable-auto-import on Windows

lt_enable_auto_import=""
case "$host_os" in
   mingw* | cegcc*)
      lt_enable_auto_import="-Wl,--enable-auto-import"
      ;;
esac
AC_SUBST(lt_enable_auto_import)


### Checks for library functions
AC_FUNC_ALLOCA
AC_CHECK_FUNCS(gettimeofday strlcpy)


### Ecore modules

## Core modules

# ecore_job
ECORE_CHECK_MODULE([Job], [${want_ecore_job}])

# ecore_txt
ECORE_CHECK_MODULE([Txt], [${want_ecore_txt}], [$have_iconv],
   [ecore_txt_libs="$ecore_txt_libs $iconv_libs"])

# ecore_con
ECORE_CHECK_MODULE([Con], [${want_ecore_con}])

have_curl="no"
have_gnutls="no"
have_openssl="no"
if test "x${have_ecore_con}" = "xyes" ; then
   requirements_ecore_con="ecore ${requirements_ecore_con}"

   ECORE_CHECK_CURL([${want_curl}],
      [
       requirements_ecore_con="libcurl ${requirements_ecore_con}"
       have_curl="yes"
      ],
      [have_curl="no"])

   ECORE_CHECK_GNUTLS([${want_gnutls}],
      [have_gnutls="yes"],
      [have_gnutls="no"])

   ECORE_CHECK_OPENSSL([${want_openssl}],
      [have_openssl="yes"],
      [have_openssl="no"])

   if test "x${have_gnutls}" = "xyes" ; then
      requirements_ecore_con="gnutls ${requirements_ecore_con}"
      # no need to add it to req_ecore_ipc, since they
      # depends on ecore_con anyway.
   else
      if test "x${have_openssl}" = "xyes" ; then
         requirements_ecore_con="openssl ${requirements_ecore_con}"
         # no need to add it to req_ecore_ipc, since they
         # depends on ecore_con anyway.
      fi
   fi
fi

# ecore_ipc
ECORE_CHECK_MODULE([Ipc], [${want_ecore_ipc}], [$have_ecore_con],
  [requirements_ecore_ipc="ecore-con ${requirements_ecore_ipc}"])

# ecore_file
ECORE_CHECK_MODULE([File], [${want_ecore_file}])

have_poll="no"
have_inotify="no"
if test "x${have_ecore_file}" = "xyes" ; then
   ECORE_CHECK_POLL([${want_poll}], [have_poll="yes"], [have_poll="no"])
   ECORE_CHECK_INOTIFY([${want_inotify}], [have_inotify="yes"], [have_inotify="no"])

   if test "x${have_ecore_con}" = "xyes" ; then
      requirements_ecore_file="${requirements_ecore_con} ${requirements_ecore_file}"
   fi
fi

# ecore_config
ecore_config_deps="no"
if test "x$have_eet" = "xyes" -a "x$have_ecore_ipc" = "xyes" ; then
  ecore_config_deps="yes"
fi

ECORE_CHECK_MODULE([Config], [${want_ecore_config}], [$ecore_config_deps],
  [requirements_ecore_config="ecore-ipc eet ecore ${requirements_ecore_config}"])

# ecore_imf

ECORE_CHECK_MODULE([IMF], [${want_ecore_imf}])

# ecore_imf_evas

ecore_imf_evas_deps="no"
if test "x${have_ecore_imf}" = "xyes" -a "x${have_evas}" = "xyes" ; then
   ecore_imf_evas_deps="yes"
fi

ECORE_CHECK_MODULE([IMF_EVAS], [${want_ecore_imf}], [${ecore_imf_evas_deps}],
  [requirements_ecore_imf_evas="ecore-imf evas ${requirements_ecore_imf_evas}"])

# ecore_input
ECORE_CHECK_MODULE([Input], [${want_ecore_input}], [${have_evas}],
  [requirements_ecore_input="evas ${requirements_ecore_input}"])

## Graphic systems

# ecore_x{cb}

ecore_x_deps="no"
if test "x${have_x}" = "xyes" -a "x${have_ecore_input}" = "xyes" ; then
   ecore_x_deps="yes"
fi

ECORE_CHECK_MODULE([X], [${want_ecore_x}], [${ecore_x_deps}],
  [
   ecore_x_libs="$ecore_x_libs $x_libs"
   requirements_ecore_x="ecore-input ${requirements_ecore_x}"
  ])

# ecore_win32

ECORE_CHECK_MODULE([Win32], [${want_ecore_win32}], [yes],
  [ecore_win32_libs="-lole32 -lgdi32"])
AC_SUBST(ecore_win32_libs)

# ecore_quartz

ECORE_CHECK_MODULE([QUARTZ], [${want_ecore_quartz}], [$have_quartz])

# ecore_sdl

ecore_sdl_deps="no"
if test "x${have_sdl}" = "xyes" -a "x${have_ecore_input}" = "xyes" ; then
   ecore_sdl_deps="yes"
fi

ECORE_CHECK_MODULE([SDL], [${want_ecore_sdl}], [${ecore_sdl_deps}],
  [requirements_ecore_sdl="ecore-input ${requirements_ecore_sdl}"])

# ecore_fb
ECORE_CHECK_MODULE([FB], [${want_ecore_fb}], [$have_fb])

if test "x${have_ecore_fb}" = "xyes" ; then
   ECORE_CHECK_TSLIB([${want_tslib}], [have_tslib="no"], [have_tslib="no"])
fi

# ecore_directfb

ECORE_CHECK_MODULE([DirectFB], [${want_ecore_directfb}], [${have_directfb}],
  [requirements_ecore_directfb="directfb ${requirements_ecore_directfb}"])

# ecore_wince

ECORE_CHECK_MODULE([WinCE], [${want_ecore_wince}])

## Ecore Evas

# ecore_evas

ECORE_CHECK_MODULE([Evas], [${want_ecore_evas}], [$have_ecore_input],
  [requirements_ecore_evas="ecore-input ${requirements_ecore_evas}"])

# ecore_evas_buffer

ECORE_EVAS_CHECK_MODULE([software-buffer],
   [${want_ecore_evas_software_buffer}],
   [Software Buffer],
   [yes])

# ecore_evas_x11

# ecore_evas_software_x11

ecore_evas_xlib_deps="no"
if test "x${have_ecore_x}" = "xyes" -a "x${have_ecore_x_xlib}" = "xyes" ; then
   ecore_evas_xlib_deps="yes"
fi

ECORE_EVAS_CHECK_MODULE([software-x11],
   [${want_ecore_evas_software_x11}],
   [Software Xlib],
   [${ecore_evas_xlib_deps}])

# ecore_evas_xrender_x11

ECORE_EVAS_CHECK_MODULE([xrender-x11],
   [${want_ecore_evas_xrender_x11}],
   [XRender Xlib],
   [${ecore_evas_xlib_deps}])

# ecore_evas_opengl_x11

ECORE_EVAS_CHECK_MODULE([opengl-x11],
   [${want_ecore_evas_opengl_x11}],
   [OpenGL Xlib],
   [${ecore_evas_xlib_deps}])

# ecore_evas_software_x11 16 bits

ECORE_EVAS_CHECK_MODULE([software-16-x11],
   [${want_ecore_evas_software_16_x11}],
   [Software Xlib 16 bits],
   [${ecore_evas_xlib_deps}])

# ecore_evas_software_xcb

ecore_evas_xcb_deps="no"
if test "x${have_ecore_x}" = "xyes" -a "x${have_ecore_x_xcb}" = "xyes" ; then
   ecore_evas_xcb_deps="yes"
fi

ECORE_EVAS_CHECK_MODULE([software-xcb],
   [${want_ecore_evas_software_xcb}],
   [Software XCB],
   [${ecore_evas_xcb_deps}])

# ecore_evas_xrender_xcb

ECORE_EVAS_CHECK_MODULE([xrender-xcb],
   [${want_ecore_evas_xrender_xcb}],
   [XRender XCB],
   [${ecore_evas_xcb_deps}])

if test "x$have_ecore_evas_software_x11" = "xyes" -o \
        "x$have_ecore_evas_xrender_x11" = "xyes" -o \
        "x$have_ecore_evas_opengl_x11" = "xyes" -o \
        "x$have_ecore_evas_software_16_x11" = "xyes" -o \
        "x$have_ecore_evas_software_xcb" = "xyes" -o \
        "x$have_ecore_evas_xrender_xcb" = "xyes"; then
   AC_DEFINE(BUILD_ECORE_EVAS_X11, 1, [Support for X Window Engines in Ecore_Evas])
   requirements_ecore_evas="ecore-x ${requirements_ecore_evas}"
fi

# ecore_evas_win32

ECORE_EVAS_CHECK_MODULE([software-ddraw],
   [${want_ecore_evas_software_ddraw}],
   [Software DirectDraw],
   [${have_ecore_win32}])

ECORE_EVAS_CHECK_MODULE([direct3d],
   [${want_ecore_evas_direct3d}],
   [Direct3d],
   [${have_ecore_win32}])

ECORE_EVAS_CHECK_MODULE([opengl-glew],
   [${want_ecore_evas_opengl_glew}],
   [Glew OpenGL],
   [${have_ecore_win32}])

ECORE_EVAS_CHECK_MODULE([software-16-ddraw],
   [${want_ecore_evas_software_16_ddraw}],
   [16 bpp Software DirectDraw],
   [${have_ecore_win32}])

if test "x${have_ecore_evas_software_ddraw}" = "xyes" -o \
        "x${have_ecore_evas_direct3d}" = "xyes" -o \
        "x${have_ecore_evas_opengl_glew}" = "xyes" -o \
        "x${have_ecore_evas_software_16_ddraw}" = "xyes" ; then
   AC_DEFINE(BUILD_ECORE_EVAS_WIN32, 1, [Support for Win32 Engine in Ecore_Evas])
   requirements_ecore_evas="ecore-win32 ${requirements_ecore_evas}"
fi

# ecore_evas_quartz

ECORE_EVAS_CHECK_MODULE([quartz],
   [${want_ecore_evas_quartz}],
   [Quartz],
   [${have_ecore_quartz}],
   [requirements_ecore_evas="ecore-quartz ${requirements_ecore_evas}"])

# ecore_evas_software_sdl

ECORE_EVAS_CHECK_MODULE([software-sdl],
   [${want_ecore_evas_software_sdl}],
   [Software SDL],
   [${have_ecore_sdl}],
   [requirements_ecore_evas="ecore-sdl ${requirements_ecore_evas}"])

# ecore_evas_directfb

ECORE_EVAS_CHECK_MODULE([directfb],
   [${want_ecore_evas_directfb}],
   [DirectFB],
   [${have_ecore_directfb}],
   [requirements_ecore_evas="ecore-directfb ${requirements_ecore_evas}"])

# ecore_evas_fb

ECORE_EVAS_CHECK_MODULE([fb],
   [${want_ecore_evas_fb}],
   [Linux Framebuffer],
   [${have_ecore_fb}],
   [requirements_ecore_evas="ecore-fb ${requirements_ecore_evas}"])

# ecore_evas_wince

ECORE_EVAS_CHECK_MODULE([software-16-wince],
   [${want_ecore_evas_software_16_wince}],
   [16 bpp Software Windows CE],
   [${have_ecore_wince}],
   [requirements_ecore_evas="ecore-wince ${requirements_ecore_evas}"])



### requirements

AC_SUBST(requirements_ecore)
AC_SUBST(requirements_ecore_con)
AC_SUBST(requirements_ecore_config)
AC_SUBST(requirements_ecore_directfb)
AC_SUBST(requirements_ecore_evas)
AC_SUBST(requirements_ecore_fb)
AC_SUBST(requirements_ecore_file)
AC_SUBST(requirements_ecore_imf)
AC_SUBST(requirements_ecore_imf_evas)
AC_SUBST(requirements_ecore_input)
AC_SUBST(requirements_ecore_ipc)
AC_SUBST(requirements_ecore_job)
AC_SUBST(requirements_ecore_quartz)
AC_SUBST(requirements_ecore_sdl)
AC_SUBST(requirements_ecore_txt)
AC_SUBST(requirements_ecore_x)
AC_SUBST(requirements_ecore_win32)
AC_SUBST(requirements_ecore_wince)

AC_OUTPUT([
Makefile
ecore-con.pc
ecore-config.pc
ecore-directfb.pc
ecore-evas.pc
ecore-fb.pc
ecore-file.pc
ecore-imf.pc
ecore-imf-evas.pc
ecore-ipc.pc
ecore-job.pc
ecore-txt.pc
ecore-x.pc
ecore-input.pc
ecore-win32.pc
ecore-sdl.pc
ecore-quartz.pc
ecore-wince.pc
ecore.pc
doc/ecore.dox
doc/Makefile
src/Makefile
src/bin/Makefile
src/lib/Makefile
src/lib/ecore/Makefile
src/lib/ecore_job/Makefile
src/lib/ecore_x/Makefile
src/lib/ecore_x/xlib/Makefile
src/lib/ecore_x/xcb/Makefile
src/lib/ecore_fb/Makefile
src/lib/ecore_sdl/Makefile
src/lib/ecore_quartz/Makefile
src/lib/ecore_evas/Makefile
src/lib/ecore_con/Makefile
src/lib/ecore_imf/Makefile
src/lib/ecore_imf_evas/Makefile
src/lib/ecore_input/Makefile
src/lib/ecore_ipc/Makefile
src/lib/ecore_txt/Makefile
src/lib/ecore_config/Makefile
src/lib/ecore_file/Makefile
src/lib/ecore_directfb/Makefile
src/lib/ecore_win32/Makefile
src/lib/ecore_wince/Makefile
README
ecore.spec
])

echo
echo "$PACKAGE $VERSION"
echo
echo "Optional Modules:"
echo
echo " Core:"
echo
echo "  Ecore........................: always"
echo "  Ecore_Job....................: $have_ecore_job"
echo "  Ecore_Txt....................: $have_ecore_txt"
echo "  Ecore_Con....................: $have_ecore_con"
if test "x$have_ecore_con" = "xyes" ; then
  echo -n "    OpenSSL....................: $have_openssl"
if test "x$have_gnutls" = "xyes" ; then
  echo " (disabled)"
else
  echo
fi
  echo "    GnuTLS.....................: $have_gnutls"
  echo "    CURL.......................: $have_curl"
  echo "    Abstract Sockets...........: $want_abstract_sockets"
fi
echo "  Ecore_Ipc....................: $have_ecore_ipc"
if test "x$have_ecore_ipc" = "xyes" ; then
  echo -n "    OpenSSL....................: $have_openssl"
if test "x$have_gnutls" = "xyes" ; then
  echo " (disabled)"
else
  echo
fi
  echo "    GnuTLS.....................: $have_gnutls"
fi
echo "  Ecore_File...................: $have_ecore_file"
if test "x$have_ecore_file" = "xyes" ; then
  echo "    Inotify....................: $have_inotify"
  echo "    Poll.......................: $have_poll"
  echo "    CURL.......................: $have_curl"
fi
echo "  Ecore_Config.................: $have_ecore_config"
echo "  Ecore_IMF....................: $have_ecore_imf"
echo "  Ecore_IMF_Evas...............: $have_ecore_imf_evas"
echo "  Ecore_Input..................: $have_ecore_input"

echo
echo " Graphic systems:"
echo

if test "x$have_ecore_x" = "xyes" ; then
  if test "x$have_ecore_x_xcb" = "xyes" ; then
    echo "  Ecore_X (XCB backend)........: $have_ecore_x_xcb"
    echo "    Xprint.....................: $have_ecore_x_xcb_xprint"
    echo "    Xinerama...................: $have_ecore_x_xcb_xinerama"
    echo "    Xrandr.....................: $have_ecore_x_xcb_randr"
    echo "    Xscreensaver...............: $have_ecore_x_xcb_screensaver"
    echo "    Xshape.....................: $have_ecore_x_xcb_shape"
    echo "    Xsync......................: $have_ecore_x_xcb_sync"
    echo "    Xrender....................: $have_ecore_x_xcb_render"
    echo "    Xcomposite.................: $have_ecore_x_xcb_composite"
    echo "    Xfixes.....................: $have_ecore_x_xcb_xfixes"
    echo "    Xdamage....................: $have_ecore_x_xcb_damage"
    echo "    Xdpms......................: $have_ecore_x_xcb_dpms"
    echo "    Xtest......................: $have_ecore_x_xcb_xtest"
  else
    echo "  Ecore_X (Xlib backend).......: $have_ecore_x"
    echo "    Xcursor....................: $use_Xcursor"
    echo "    Xkb........................: $use_xkb"
    echo "    Xprint.....................: $use_xprint"
    echo "    Xinerama...................: $use_xinerama"
    echo "    Xrandr.....................: $use_xrandr"
    echo "    Xscreensaver...............: $use_xss"
    echo "    Xrender....................: $use_xrender"
    echo "    Xcomposite.................: $use_xcomposite"
    echo "    Xfixes.....................: $use_xfixes"
    echo "    Xdamage....................: $use_xdamage"
    echo "    Xdpms......................: $use_xdpms"
    echo "    Xtest......................: $use_xtest"
  fi
else
  echo "  Ecore_X......................: $have_ecore_x"
fi
echo "  Ecore_Win32..................: $have_ecore_win32"
echo "  Ecore_Quartz.................: $have_ecore_quartz"
echo "  Ecore_SDL....................: $have_ecore_sdl"
echo "  Ecore_FB.....................: $have_ecore_fb"
if test "x${have_ecore_fb}" = "xyes" ; then
  echo "    Touchscreen................: $have_tslib"
fi
echo "  Ecore_DirectFB...............: $have_ecore_directfb"
echo "  Ecore_WinCE..................: $have_ecore_wince"

echo
echo " Ecore Evas:"
echo

echo "  Ecore_Evas...................: $have_ecore_evas"
if test "x${have_ecore_evas}" = "xyes" ; then
  echo "    Software Memory Buffer.....: $have_ecore_evas_software_buffer"
  echo "    Software X11...............: $have_ecore_evas_software_x11"
  echo "    XRender X11................: $have_ecore_evas_xrender_x11"
  echo "    OpenGL X11.................: $have_ecore_evas_opengl_x11"
  echo "    Software XCB...............: $have_ecore_evas_software_xcb"
  echo "    XRender XCB................: $have_ecore_evas_xrender_xcb"
  echo "    Software DirectDraw........: $have_ecore_evas_software_ddraw"
  echo "    Direct3D...................: $have_ecore_evas_direct3d"
  echo "    OpenGL Glew................: $have_ecore_evas_opengl_glew"
  echo "    Quartz.....................: $have_ecore_evas_quartz"
  echo "    Software SDL...............: $have_ecore_evas_software_sdl"
  echo "    DirectFB...................: $have_ecore_evas_directfb"
  echo "    Software Framebuffer.......: $have_ecore_evas_fb"
  echo "    Software 16bit X11.........: $have_ecore_evas_software_16_x11"
  echo "    Software 16bit DirectDraw..: $have_ecore_evas_software_16_ddraw"
  echo "    Software 16bit WinCE.......: $have_ecore_evas_software_16_wince"
fi
echo
echo " Documentation.................: ${build_doc}"
echo
echo " Compilation...................: make (gmake on some systems)"
if test "x${build_doc}" = "xyes" ; then
   echo " Build of the documentation....: make doc"
fi
echo
echo " Installation..................: make install"
echo
echo "    prefix.....................: $prefix"
echo

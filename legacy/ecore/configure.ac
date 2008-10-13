# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT(ecore, 0.9.9.050, enlightenment-devel@lists.sourceforge.net)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(configure.ac)
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_ISC_POSIX

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

AC_PROG_CXX
AC_C_BIGENDIAN
AC_PROG_CC
AM_PROG_CC_STDC
m4_ifdef([AC_PROG_OBJC],
         [AC_PROG_OBJC],
         [AC_CHECK_TOOL([OBJC], [gcc])
          AC_SUBST([OBJC])
          AC_SUBST([OBJCFLAGS])
         ])
AC_HEADER_STDC
AC_C_CONST
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_C___ATTRIBUTE__
AC_HEADER_SYS_WAIT

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL
AC_DEFINE_UNQUOTED(SHARED_LIB_SUFFIX, "$shrext_cmds", [Suffix for shared objects])

VMAJ=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $1);}'`
VMIN=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $2);}'`
VMIC=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $3);}'`
SNAP=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $4);}'`
version_info=`expr $VMAJ + $VMIN`":$VMIC:$VMIN"
AC_SUBST(version_info)

# define pkg-config requirements for all our submodules.
# only the trivial requirements go here.
requirements_ecore_con=""
requirements_ecore_config=""
requirements_ecore_desktop=""
requirements_ecore_directfb=""
requirements_ecore_evas="evas"
requirements_ecore_fb=""
requirements_ecore_file=""
requirements_ecore_ipc=""
requirements_ecore_job=""
requirements_ecore_txt=""
requirements_ecore_x=""
requirements_ecore_win32=""
requirements_ecore_wince=""
requirements_ecore_sdl=""
requirements_ecore_quartz=""

dnl The first call to PKG_CHECK_MODULES is done conditionally,
dnl so we should include this here:
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([EINA], [eina-0])

dnl add windows requirements
WIN32_CFLAGS=""
WIN32_LIBS=""
lt_no_undefined=""
lt_enable_auto_import=""
case "$host_os" in
	mingw*|cegcc)
		PKG_CHECK_MODULES([EVIL], [evil])
		AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if evil package is installed.])
		dnl needed for correct definition of EAPI
		AC_DEFINE(EFL_ECORE_BUILD, 1, [Define to mention that ecore is built.])
		AC_DEFINE(EFL_ECORE_EVAS_BUILD, 1, [Define to mention that ecore evas is built.])
		AC_DEFINE(EFL_ECORE_FILE_BUILD, 1, [Define to mention that ecore file is built.])
		AC_DEFINE(EFL_ECORE_IMF_BUILD, 1, [Define to mention that ecore imf is built.])
		AC_DEFINE(EFL_ECORE_JOB_BUILD, 1, [Define to mention that ecore job is built.])
		AC_DEFINE(EFL_ECORE_SDL_BUILD, 1, [Define to mention that ecore SDL is built.])
		AC_DEFINE(EFL_ECORE_QUARTZ_BUILD, 1, [Define to mention that ecore Quartz is built.])
		AC_DEFINE(EFL_ECORE_TXT_BUILD, 1, [Define to mention that ecore txt is built.])
		AC_DEFINE(EFL_ECORE_WIN32_BUILD, 1, [Define to mention that ecore Win32 is built.])
		AC_DEFINE(HAVE_DLFCN_H, 1, [Define to 1 if you have the <dlfcn.h> header file.])
		AC_DEFINE(HAVE_SYS_MMAN_H, 1, [Define to 1 if you have the <sys/mman.h> header file.])
		AC_DEFINE(HAVE_SYS_TIME_H, 1, [Define to 1 if you have the <sys/time.h> header file.])
                if test "$host_os" = "cegcc" ; then
			AC_DEFINE(EFL_ECORE_WINCE_BUILD, 1, [Define to mention that ecore WinCE is built.])
                        WIN32_CFLAGS="-mwin32"
                        WIN32_LIBS="-lws2 -laygshell"
			lt_enable_auto_import="-Wl,--enable-auto-import"
                else
                        WIN32_LIBS="-lws2_32"
                fi
		lt_no_undefined="-no-undefined"
		;;
	*)
		AC_CHECK_HEADERS([dlfcn.h features.h langinfo.h locale.h netdb.h netinet/in.h sys/time.h sys/mman.h])
		AC_CHECK_LIB(dl, dlopen, dlopen_libs=-ldl)
		AC_SUBST(dlopen_libs)
		;;
esac
AC_SUBST(WIN32_CFLAGS)
AC_SUBST(WIN32_LIBS)
AC_SUBST(lt_no_undefined)
AC_SUBST(lt_enable_auto_import)

dnl check common functions and headers
AC_CHECK_FUNCS(gettimeofday)
AC_CHECK_FUNCS(strlcpy)

dnl openssl
want_openssl="yes"
have_openssl="no"
AC_ARG_ENABLE(openssl,
  AC_HELP_STRING(
    [--disable-openssl],
    [disable openssl support]
  ),
  [ want_openssl=$enableval ]
)

if test "x$want_openssl" = "xyes"; then
    PKG_CHECK_MODULES([SSL], [openssl], [
        AC_DEFINE(USE_OPENSSL, 1, [Use OpenSSL])
        requirements_ecore_con="$requirements_ecore_con openssl"
        # no need to add it to req_ecore_ipc, since they
        # depends on ecore_con anyway.
        have_openssl="yes"
    ], [
        have_openssl="no"
    ])
fi

dnl curl
want_curl="yes"
have_curl="no"
AC_ARG_ENABLE(curl,
  AC_HELP_STRING(
    [--disable-curl],
    [disable curl support]
  ),
  [ want_curl=$enableval ]
)

if test "x$want_curl" = "xyes"; then
  PKG_CHECK_MODULES(CURL, libcurl,
    [
      AC_DEFINE(HAVE_CURL, 1, [ Downloading with CURL ])
      requirements_ecore_con="$requirements_ecore_con libcurl"
      have_curl="yes"
    ],
    [ have_curl="no" ]
  )
fi

dnl abstract sockets
want_abstract_sockets="yes"
AC_ARG_ENABLE(abstract-sockets,
  AC_HELP_STRING(
    [--disable-abstract-sockets],
    [disable abstract sockets.]
  ),
  [ want_abstract_sockets=$enableval ]
)
if test "x$want_abstract_sockets" = "xyes"; then
  AC_DEFINE(HAVE_ABSTRACT_SOCKETS, 1, [Have abstract sockets namespace])
fi

dnl ecore_job
ECORE_CHECK_MODULE([Job], [yes])

dnl ecore_con
ECORE_CHECK_MODULE([Con], [yes])

dnl ecore_ipc
ECORE_CHECK_MODULE([Ipc], [yes], [$have_ecore_con])

dnl ecore_config
PKG_CHECK_MODULES(EET, [eet >= 1.0.0],
  [ have_eet="yes" ],
  [ have_eet="no" ]
)
ecore_config_deps="no"
if test "x$have_eet" = "xyes" -a "x$have_ecore_ipc" = "xyes" ; then
  ecore_config_deps="yes"
fi
ECORE_CHECK_MODULE([Config], [yes], [$ecore_config_deps],
  [requirements_ecore_config="$requirements_ecore_config ecore-ipc eet"])

dnl ecore_txt
iconv_cflags=""
iconv_libs=""
have_iconv="no"
AC_ARG_WITH(iconv-link,
  AC_HELP_STRING([--with-iconv-link=ICONV_LINK], [explicitly specify an iconv link option]),
  [
    iconv_libs=$withval
    have_iconv="yes"
  ]
)

if test "x$have_iconv" = "xno" ; then
  AC_CHECK_LIB(c, iconv,
    [ have_iconv="yes" ]
  )

  if test "x$have_iconv" != "xyes"; then
    AC_CHECK_LIB(iconv, iconv,
      [
       iconv_libs="-liconv"
       have_iconv="yes"
      ]
    )
  fi
fi
AC_MSG_CHECKING(for explicit iconv link options)
AC_MSG_RESULT($iconv_libs)

AC_SUBST(iconv_cflags)
AC_SUBST(iconv_libs)

ECORE_CHECK_MODULE([Txt], [yes], [$have_iconv],
  [ecore_txt_libs="$ecore_txt_libs $iconv_libs"])

dnl ecore_evas
PKG_CHECK_MODULES(EVAS, [evas >= 0.9.9],
  [ have_evas="yes" ],
  [ have_evas="no" ]
)

ECORE_CHECK_MODULE([Evas], [yes], [$have_evas])

#######################################
## Simple X11 build/link

AC_ARG_ENABLE(simple-x11,
  AC_HELP_STRING([--enable-simple-x11], [enable simple x11 linking]),
  [ want_evas_simple_x11=$enableval ]
)

have_x="no"
have_ecore_x="no"
have_ecore_x_xlib="no"
have_ecore_x_xcb="no"

x_dir="";
x_includes="";
x_cflags="";
x_libs="";

dnl ecore_x{cb}

AC_ARG_ENABLE(ecore-x-xcb,
  AC_HELP_STRING(
    [--enable-ecore-x-xcb],
    [enable the ecore_x module with XCB backend. [[default=disabled]]]),
  [ want_ecore_x_xcb=$enableval ],
  [ want_ecore_x_xcb="no" ])

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

ECORE_CHECK_MODULE([X], [yes], [$have_x],
  [ecore_x_libs="$ecore_x_libs $x_libs"])

dnl ecore_evas_x11
have_ecore_evas_software_x11="no"
have_ecore_evas_xrender_x11="no";
have_ecore_evas_opengl_x11="no";
have_ecore_evas_software_xcb="no"
have_ecore_evas_xrender_xcb="no";
have_ecore_evas_software_16_x11="no";
if test "x$have_ecore_x" = "xyes" -a "x$have_ecore_evas" = "xyes"; then

  dnl ecore_evas_x Xlib
  if test "x$have_ecore_x_xlib" = "xyes"; then
    want_ecore_evas_software_x11="yes"
    dnl disabling this will make things go haywire
    #AC_ARG_ENABLE(ecore-evas-x11,
    #  AC_HELP_STRING(
    #    [--enable-ecore-evas-x11],
    #    [enable X11 support in the ecore_evas module.]
    #  ),
    #  [ want_ecore_evas_x11=$enableval ]
    #)
    AC_MSG_CHECKING(whether ecore_evas Software Xlib support is to be built)
    AC_MSG_RESULT($want_ecore_evas_software_x11)

    if test "x$want_ecore_evas_software_x11" = "xyes"; then
      PKG_CHECK_MODULES(EVAS_SOFTWARE_X11, evas-software-x11, [
          AC_DEFINE(BUILD_ECORE_EVAS_SOFTWARE_X11, 1, [Support for Software Xlib Engine in Ecore_Evas])
          have_ecore_evas_software_x11="yes";
      ], [
          have_ecore_evas_software_x11="no";
      ])
    fi

    dnl ecore_evas_opengl_x11
    want_ecore_evas_opengl_x11=yes
    AC_ARG_ENABLE(ecore-evas-opengl-x11,
      AC_HELP_STRING(
        [--enable-ecore-evas-opengl-x11],
        [enable OpenGL Xlib support in the ecore_evas module.]
      ),
      [ want_ecore_evas_opengl_x11=$enableval ]
    )
    AC_MSG_CHECKING(whether ecore_evas OpenGL Xlib support is to be built)
    AC_MSG_RESULT($want_ecore_evas_opengl_x11)

    if test "x$want_ecore_evas_opengl_x11" = "xyes"; then
      PKG_CHECK_MODULES(EVAS_OPENGL_X11, evas-opengl-x11, [
          AC_DEFINE(BUILD_ECORE_EVAS_OPENGL_X11, 1, [Support for OpenGL Xlib Engine in Ecore_Evas])
          have_ecore_evas_opengl_x11="yes";
      ], [
          have_ecore_evas_opengl_x11="no"
      ])
    fi

    dnl ecore_evas_xrender_x11
    want_ecore_evas_xrender_x11="yes"
    AC_ARG_ENABLE(ecore-evas-xrender-x11,
      AC_HELP_STRING(
        [--disable-ecore-evas-xrender-x11],
        [disable XRender Xlib support in the ecore_evas module. [[default=enabled]]]
      ),
      [ want_ecore_evas_xrender_x11=$enableval ]
    )

    AC_MSG_CHECKING(whether ecore_evas XRender Xlib support is to be built)
    AC_MSG_RESULT($want_ecore_evas_xrender_x11)

    if test "x$want_ecore_evas_xrender_x11" = "xyes"; then
      PKG_CHECK_MODULES(EVAS_XRENDER_X11, evas-xrender-x11, [
          AC_DEFINE(BUILD_ECORE_EVAS_XRENDER_X11, 1, [Support for XRender Xlib Engine in Ecore_Evas])
          have_ecore_evas_xrender_x11="yes"
      ], [
          have_ecore_evas_xrender_x11="no"
      ])
    fi

    dnl ecore_evas_software_16_x11
    want_ecore_evas_software_16_x11=no
    AC_ARG_ENABLE(ecore-evas-software-16-x11,
      AC_HELP_STRING(
        [--enable-ecore-evas-software-16-x11],
        [enable 16bpp optimized mode support in the ecore_evas module. [[default=disabled]]]
      ),
      [ want_ecore_evas_software_16_x11=$enableval ]
    )

    AC_MSG_CHECKING(whether ecore_evas x11-16 support is to be built)
    AC_MSG_RESULT($want_ecore_evas_software_16_x11)

    if test "x$want_ecore_evas_software_16_x11" = "xyes"; then
      PKG_CHECK_MODULES(EVAS_SOFTWARE_16_X11, evas-software-16-x11,
        [
          AC_DEFINE(BUILD_ECORE_EVAS_SOFTWARE_16_X11, 1, [Support for X11 16bpp Engine in Ecore_Evas])
          have_ecore_evas_software_16_x11="yes";
        ]
      )
    fi
  else
    want_ecore_evas_software_xcb="yes"
    ECORE_EVAS_CHECK_MODULE([software-xcb],
      [$want_ecore_evas_software_xcb],
      [Software XCB],
      $have_ecore_x_xcb)

    want_ecore_evas_xrender_xcb="auto"
    ECORE_EVAS_CHECK_MODULE([xrender-xcb],
      [$want_ecore_evas_xrender_xcb],
      [XRender XCB],
      $have_ecore_x_xcb)
  fi

  if test "x$have_ecore_evas_software_x11" = "xyes" -o \
	  "x$have_ecore_evas_opengl_x11" = "xyes" -o \
	  "x$have_ecore_evas_xrender_x11" = "xyes" -o \
	  "x$have_ecore_evas_software_16_x11" = "xyes" -o \
          "x$have_ecore_evas_software_xcb" = "xyes" -o \
	  "x$have_ecore_evas_xrender_xcb" = "xyes"; then
    AC_DEFINE(BUILD_ECORE_EVAS_X11, 1, [Support for X Window Engines in Ecore_Evas])
    requirements_ecore_evas="$requirements_ecore_evas ecore-x"
  fi

fi

dnl ecore_win32

ECORE_CHECK_MODULE([Win32], [no], [yes],
  [ecore_win32_libs="-lgdi32"])
AC_SUBST(ecore_win32_libs)

want_ecore_evas_software_ddraw="yes"
ECORE_EVAS_CHECK_MODULE([software-ddraw],
  [$want_ecore_evas_software_ddraw],
  [Software DirectDraw],
  $have_ecore_win32)

want_ecore_evas_direct3d="yes"
ECORE_EVAS_CHECK_MODULE([direct3d],
  [$want_ecore_evas_direct3d],
  [Direct3d],
  $have_ecore_win32)

want_ecore_evas_opengl_glew="yes"
ECORE_EVAS_CHECK_MODULE([opengl-glew],
  [$want_ecore_evas_opengl_glew],
  [Glew OpenGL],
  $have_ecore_win32)

want_ecore_evas_software_16_ddraw="yes"
ECORE_EVAS_CHECK_MODULE([software-16-ddraw],
  [$want_ecore_evas_software_16_ddraw],
  [16 bpp Software DirectDraw],
  $have_ecore_win32)

if test "x$have_ecore_evas_software_ddraw" = "xyes" -o \
        "x$have_ecore_evas_direct3d" = "xyes" -o \
        "x$have_ecore_evas_opengl_glew" = "xyes" -o \
        "x$have_ecore_evas_software_16_ddraw" = "xyes"; then
  AC_DEFINE(BUILD_ECORE_EVAS_WIN32, 1, [Support for Win32 Engine in Ecore_Evas])
  requirements_ecore_evas="$requirements_ecore_evas ecore-win32"
fi

dnl ecore_wince

ECORE_CHECK_MODULE([WinCE], [no])

want_ecore_evas_software_16_wince="yes"
ECORE_EVAS_CHECK_MODULE([software-16-wince],
  [$want_ecore_evas_software_16_wince],
  [16 bpp Software Windows CE],
  $have_ecore_wince)

if test "x$have_ecore_evas_software_16_wince" = "xyes"; then
  requirements_ecore_evas="$requirements_ecore_evas ecore-wince"
fi

dnl ecore_dfb
PKG_CHECK_MODULES(DIRECTFB, directfb >= 0.9.16,
  [ have_directfb="yes" ],
  [ have_directfb="no" ]
)

ECORE_CHECK_MODULE([DirectFB], [no], [$have_directfb],
  [requirements_ecore_directfb="directfb"])

dnl ecore_evas_dfb
have_ecore_evas_dfb="no"
if test "x$have_ecore_directfb" = "xyes" -a "x$have_ecore_evas" = "xyes"; then
  want_ecore_evas_dfb="yes"
  AC_MSG_CHECKING(whether ecore_evas DirectFB support is to be built)
  AC_ARG_ENABLE(ecore-evas-dfb,
    AC_HELP_STRING([--disable-ecore-evas-dfb], [disable DirectFB in the ecore_evas module]),
    [ want_ecore_evas_dfb=$enableval ]
  )
  AC_MSG_RESULT($want_ecore_evas_dfb)

  if test "x$want_ecore_evas_dfb" = "xyes" -a "x$have_ecore_directfb" = "xyes" ; then
    save_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS $DIRECTFB_CFLAGS"
    PKG_CHECK_MODULES(EVAS_DIRECTFB, evas-directfb,
      [
        AC_DEFINE(BUILD_ECORE_EVAS_DIRECTFB, 1, [Support for DirectFB in Ecore_Evas])
        have_ecore_evas_dfb="yes"
        requirements_ecore_evas="$requirements_ecore_evas ecore-directfb"
      ]
    )
    CFLAGS="$save_CFLAGS"
  fi
fi

dnl ecore_fb
have_fb="no"
AC_CHECK_HEADER(linux/fb.h,
  [ AC_CHECK_HEADER(linux/input.h, [ have_fb="yes" ]) ]
)
ECORE_CHECK_MODULE([FB], [no], [$have_fb])

tslib_libs=""
if test "x$have_ecore_fb" = "xyes" ; then
AC_CHECK_HEADER(tslib.h,
  [
   AC_CHECK_LIB(ts, ts_open, tslib_libs="-lts", tslib_libs="-ltslib")
   AC_DEFINE(HAVE_TSLIB, 1, [Build Ecore_FB Touchscreen Code])
  ]
)
fi
AC_SUBST(tslib_libs)

dnl ecore_evas_fb
have_ecore_evas_fb="no";
if test "x$have_ecore_fb" = "xyes" -a "x$have_ecore_evas" = "xyes"; then
  want_ecore_evas_fb="yes";
  AC_MSG_CHECKING(whether ecore_evas fb support is to be built)
  AC_ARG_ENABLE(ecore-evas-fb,
    AC_HELP_STRING([--disable-ecore-evas-fb], [disable fb in the ecore_evas module]),
    [ want_ecore_evas_fb=$enableval ]
  )
  AC_MSG_RESULT($want_ecore_evas_fb)

  if test "x$want_ecore_evas_fb" = "xyes"; then
    PKG_CHECK_MODULES(EVAS_FB, evas-fb,
      [
        AC_DEFINE(BUILD_ECORE_EVAS_FB, 1, [Support for Linux FB in Ecore_Evas])
        have_ecore_evas_fb="yes"
        requirements_ecore_evas="$requirements_ecore_evas ecore-fb"
      ]
    )
  fi
fi

dnl ecore_sdl
AC_ARG_WITH(sdl-config, AC_HELP_STRING([--with-sdl-config=PATH], [use sdl-config specified]), [
    SDL_CONFIG=$withval;
    echo "using "$SDL_CONFIG" for sdl-config";
], [
    if test -z "$SDL_CONFIG"; then
        AC_PATH_PROG(SDL_CONFIG, "sdl-config", "", $PATH)
    fi
])
if test -z "$SDL_CONFIG" ; then SDL_CONFIG="sdl-config"; fi

sdl_cflags="";
sdl_libs="";
have_sdl="no"
AC_CHECK_HEADER(SDL/SDL.h, [ have_sdl="yes" ], [ have_sdl="no" ])

ECORE_CHECK_MODULE([SDL], [no], [$have_sdl])

dnl ecore_evas_sdl
have_ecore_evas_sdl="no";
if test "x$have_ecore_sdl" = "xyes" -a "x$have_ecore_evas" = "xyes"; then
    want_ecore_evas_sdl="yes";
    AC_MSG_CHECKING([whether ecore_evas SDL support is to be built])
    AC_ARG_ENABLE(ecore-evas-sdl,
        AC_HELP_STRING([--disable-ecore-evas-sdl], [disable SDL in the ecore_evas module]),
        [ want_ecore_evas_sdl=$enableval ]
    )
    AC_MSG_RESULT($want_ecore_evas_sdl)

    if test "x$want_ecore_evas_sdl" = "xyes"; then
        save_CFLAGS=$CFLAGS
        SDL_CFLAGS=`$SDL_CONFIG --cflags`
        SDL_LIBS=`$SDL_CONFIG --libs`
        CFLAGS="$CFLAGS $SDL_CFLAGS"
        AC_SUBST(SDL_CFLAGS)
        AC_SUBST(SDL_LIBS)

        PKG_CHECK_MODULES(EVAS_SOFTWARE_SDL, evas-software-sdl, [
            AC_DEFINE(BUILD_ECORE_EVAS_SDL, 1, [Support for SDL in Ecore_Evas])
            have_ecore_evas_sdl="yes"
            requirements_ecore_evas="$requirements_ecore_evas ecore-sdl"
            PKG_CHECK_MODULES(SDL, sdl >= 1.3.0, [
                AC_DEFINE(BUILD_ECORE_EVAS_SDL_130, 1, [Support for SVN SDL])
            ], [
                dummy="no"
            ])
        ], [
            have_ecore_evas_sdl="no"
        ])
    CFLAGS=$save_CFLAGS
  fi
fi

dnl ecore_quartz

quartz_cflags="";
quartz_libs="";
have_quartz="no"
AC_CHECK_HEADER(Cocoa/Cocoa.h, 
               [ have_quartz="yes"; 
	         quartz_cflags="-framework Cocoa"; 
		 quartz_libs="-framework Cocoa" ], 
	       [ have_quartz="no" ])

ECORE_CHECK_MODULE([QUARTZ], [no], [$have_quartz])

dnl ecore_evas_quartz
have_ecore_evas_quartz="no";
if test "x$have_ecore_quartz" = "xyes" -a "x$have_ecore_evas" = "xyes"; then
    want_ecore_evas_quartz="yes";
    AC_MSG_CHECKING([whether ecore_evas Quartz support is to be built])
    AC_ARG_ENABLE(ecore-evas-quartz,
        AC_HELP_STRING([--disable-ecore-evas-quartz], [disable Quartz in the ecore_evas module]),
        [ want_ecore_evas_quartz=$enableval ]
    )
    AC_MSG_RESULT($want_ecore_evas_quartz)

    if test "x$want_ecore_evas_quartz" = "xyes"; then
        save_CFLAGS=$CFLAGS
        QUARTZ_CFLAGS=`-framework Cocoa`
        QUARTZ_LIBS=`-framework Cocoa`
        CFLAGS="$CFLAGS $QUARTZ_CFLAGS"
        AC_SUBST(QUARTZ_CFLAGS)
        AC_SUBST(QUARTZ_LIBS)

        PKG_CHECK_MODULES(EVAS_QUARTZ, evas-quartz, [
            AC_DEFINE(BUILD_ECORE_EVAS_QUARTZ, 1, [Support for Quartz in Ecore_Evas])
            have_ecore_evas_quartz="yes"
            requirements_ecore_evas="$requirements_ecore_evas ecore-quartz"
        ], [
            have_ecore_evas_quartz="no"
        ])
    CFLAGS=$save_CFLAGS
  fi
fi

dnl ecore_evas_buffer
want_ecore_evas_buffer="yes";
have_ecore_evas_buffer="no";
AC_MSG_CHECKING(whether ecore_evas buffer support is to be built)
AC_ARG_ENABLE(ecore-evas-buffer,
  AC_HELP_STRING([--disable-ecore-evas-buffer], [disable buffer in the ecore_evas module]),
  [ want_ecore_evas_buffer=$enableval ]
)
AC_MSG_RESULT($want_ecore_evas_buffer)

if test "x$want_ecore_evas_buffer" = "xyes" -a "x$have_ecore_evas" = "xyes"; then
  PKG_CHECK_MODULES(EVAS_SOFTWARE_BUFFER, evas-software-buffer,
    [
      AC_DEFINE(BUILD_ECORE_EVAS_BUFFER, 1, [Support for Buffers in Ecore_Evas])
      have_ecore_evas_buffer="yes"
    ], [
      have_ecore_evas_buffer="no"
  ])
fi

dnl ecore_file
ECORE_CHECK_MODULE([File], [yes])
have_inotify="no"
have_poll="no"
if test "x$have_ecore_file" = "xyes"; then

  dnl We need to check if the right inotify version is accessible
  want_inotify="yes"
  AC_MSG_CHECKING(whether inotify is to be used for filemonitoring)
  AC_ARG_ENABLE(inotify,
    AC_HELP_STRING([--disable-inotify], [disable inotify in the ecore_file module]),
    [ want_inotify=$enableval ]
  )
  AC_MSG_RESULT($want_inotify)

  dnl It's hard to find a good test on how to check the correct
  dnl inotify version. They changed the headers a lot.
  dnl in kernel 2.6.13 __NR_inotify_init was added to the defined syscalls
  dnl in asm/unistd.h and IN_MOVE_SELF was added to linux/inotify.h
  dnl so with this check you need a very new kernel and kernel-headers!
  if test "x$want_inotify" = "xyes"; then
    AC_CHECK_LIB(c, inotify_init, [
        AC_DEFINE(HAVE_INOTIFY, 1, [ File monitoring with Inotify ])
        AC_DEFINE(HAVE_SYS_INOTIFY, 1, [ File monitoring with Inotify - sys/inotify.h ])
	have_inotify="yes"
      ],
      [
        AC_TRY_COMPILE(
          [
            #include <asm/unistd.h>
            #include <linux/inotify.h>
          ],
          [ int a = __NR_inotify_init; int b = IN_MOVE_SELF; ],
          [
            AC_DEFINE(HAVE_INOTIFY, 1, [ File monitoring with Inotify ])
	    have_inotify="yes"
          ], [
            have_inotify="no"
          ]
	)
      ]
    )
  fi

  want_poll="yes"
  AC_MSG_CHECKING(whether polling is to be used for filemonitoring)
  AC_ARG_ENABLE(poll,
    AC_HELP_STRING([--disable-poll], [disable poll in the ecore_file module]),
    [ want_poll=$enableval ]
  )
  AC_MSG_RESULT($want_poll)

  if test "x$want_poll" = "xyes"; then
    AC_DEFINE(HAVE_POLL, 1, [ File monitoring with polling ])
    have_poll="yes"
  fi
  requirements_ecore_file="$requirements_ecore_file $requirements_ecore_con"
fi

dnl ecore_desktop
ECORE_CHECK_MODULE([Desktop], [no], [$have_ecore_file],
  [requirements_ecore_desktop="ecore-file"])

dnl ecore_imf
ECORE_CHECK_MODULE([IMF], [yes])

try_ecore_imf_evas=no
if test "x$have_ecore_imf" = "xyes" -a "x$have_evas" = "xyes"; then
  try_ecore_imf_evas=yes
fi

ECORE_CHECK_MODULE([IMF_EVAS], [yes], [$try_ecore_imf_evas])

dnl requirements
AC_SUBST(requirements_ecore_con)
AC_SUBST(requirements_ecore_config)
AC_SUBST(requirements_ecore_desktop)
AC_SUBST(requirements_ecore_directfb)
AC_SUBST(requirements_ecore_evas)
AC_SUBST(requirements_ecore_fb)
AC_SUBST(requirements_ecore_file)
AC_SUBST(requirements_ecore_ipc)
AC_SUBST(requirements_ecore_job)
AC_SUBST(requirements_ecore_txt)
AC_SUBST(requirements_ecore_x)
AC_SUBST(requirements_ecore_sdl)
AC_SUBST(requirements_ecore_quartz)

# set up conditionals
AM_CONDITIONAL(BUILD_ECORE_X, test $have_ecore_x = yes)
AM_CONDITIONAL(BUILD_ECORE_X_XLIB, test $have_ecore_x_xlib = yes)
AM_CONDITIONAL(BUILD_ECORE_X_XCB, test $have_ecore_x_xcb = yes)

AC_OUTPUT([
Makefile
ecore-con.pc
ecore-config.pc
ecore-desktop.pc
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
ecore-win32.pc
ecore-sdl.pc
ecore-quartz.pc
ecore-wince.pc
ecore.pc
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
src/lib/ecore_ipc/Makefile
src/lib/ecore_txt/Makefile
src/lib/ecore_config/Makefile
src/lib/ecore_file/Makefile
src/lib/ecore_desktop/Makefile
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
echo "  Ecore_Job....................: $have_ecore_job"
echo "  Ecore_Txt....................: $have_ecore_txt"
echo "  Ecore_File...................: $have_ecore_file"
if test "x$have_ecore_file" = "xyes" ; then
  echo "    Inotify....................: $have_inotify"
  echo "    Poll.......................: $have_poll"
  echo "    CURL.......................: $have_curl"
fi
echo "  Ecore_Desktop................: $have_ecore_desktop"
echo "  Ecore_Con....................: $have_ecore_con"
if test "x$have_ecore_con" = "xyes" ; then
  echo "    OpenSSL....................: $have_openssl"
  echo "    CURL.......................: $have_curl"
fi
echo "  Ecore_Ipc....................: $have_ecore_ipc"
if test "x$have_ecore_con" = "xyes" ; then
  echo "    OpenSSL....................: $have_openssl"
fi
echo "  Ecore_Config.................: $have_ecore_config"
echo "  Ecore_IMF....................: $have_ecore_imf"
echo "  Ecore_IMF_Evas...............: $have_ecore_imf_evas"

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
echo "  Ecore_SDL....................: $have_ecore_sdl"
echo "  Ecore_Quartz.................: $have_ecore_quartz"
echo "  Ecore_FB.....................: $have_ecore_fb"
echo "  Ecore_DFB....................: $have_ecore_directfb"
echo "  Ecore_WinCE..................: $have_ecore_wince"

echo
echo " Ecore Evas:"
echo

echo "  Ecore_Evas...................: $have_ecore_evas"
if test "x$have_ecore_evas" = "xyes" ; then
  echo "    Software Memory Buffer.....: $have_ecore_evas_buffer"
  echo "    Software X11...............: $have_ecore_evas_software_x11"
  echo "    XRender X11................: $have_ecore_evas_xrender_x11"
  echo "    OpenGL X11.................: $have_ecore_evas_opengl_x11"
  echo "    Software XCB...............: $have_ecore_evas_software_xcb"
  echo "    XRender XCB................: $have_ecore_evas_xrender_xcb"
  echo "    Software DirectDraw........: $have_ecore_evas_software_ddraw"
  echo "    Direct3D...................: $have_ecore_evas_direct3d"
  echo "    OpenGL Glew................: $have_ecore_evas_opengl_glew"
  echo "    Software SDL...............: $have_ecore_evas_sdl"
  echo "    Quartz.....................: $have_ecore_evas_quartz"
  echo "    DirectFB...................: $have_ecore_evas_dfb"
  echo "    Software Framebuffer.......: $have_ecore_evas_fb"
  echo "    Software 16bit X11.........: $have_ecore_evas_software_16_x11"
  echo "    Software 16bit DirectDraw..: $have_ecore_evas_software_16_ddraw"
  echo "    Software 16bit WinCE.......: $have_ecore_evas_software_16_wince"
fi
echo
echo "Now type 'make' ('gmake' on some systems) to compile $PACKAGE."
echo

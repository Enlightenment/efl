##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_define([v_maj], [1])
m4_define([v_min], [7])
m4_define([v_mic], [99])
m4_define([v_rev], m4_esyscmd([(svnversion "${SVN_REPO_PATH:-.}" | grep -v '\(export\|Unversioned directory\)' || echo 0) | awk -F : '{printf("%s\n", $1);}' | tr -d ' :MSP\n']))
m4_if(v_rev, [0], [m4_define([v_rev], m4_esyscmd([git log 2> /dev/null | (grep -m1 git-svn-id || echo 0) | sed -e 's/.*@\([0-9]*\).*/\1/' | tr -d '\n']))])
##--   When released, remove the dnl on the below line
dnl m4_undefine([v_rev])
##--   When doing snapshots - change soname. remove dnl on below line
dnl m4_define([relname], [ver-pre-svn-07])
dnl m4_define([v_rel], [-release relname])
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], [m4_define([v_ver], [v_maj.v_min.v_mic.v_rev])], [m4_define([v_ver], [v_maj.v_min.v_mic])])
m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], v_mic)
m4_define([lt_age], v_min)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##

# rw_PROG_OBJC_WORKS
# Check whether the Objective C compiler works.
AC_DEFUN([rw_PROG_OBJC_WORKS],
[m4_ifdef([AC_PROG_OBJC], [AC_REQUIRE([AC_PROG_OBJC])dnl
AC_CACHE_CHECK([whether the Objective C compiler works],
               [rw_cv_prog_objc_works],
               [AC_LANG_PUSH([Objective C])
                AC_LINK_IFELSE([AC_LANG_PROGRAM([], [])],
                               [rw_cv_prog_objc_works=yes],
                               [rw_cv_prog_objc_works=no])
                AC_LANG_POP([Objective C])])], [rw_cv_prog_objc_works=no])
])

AC_INIT([evas], [v_ver], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.52])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])
AH_TOP([
#ifndef EFL_CONFIG_H__
#define EFL_CONFIG_H__
])
AH_BOTTOM([
#endif /* EFL_CONFIG_H__ */
])

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_GNU_SOURCE

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], , [m4_define([v_rev], [0])])
m4_ifdef([v_rel], , [m4_define([v_rel], [])])
AC_DEFINE_UNQUOTED(VMAJ, [v_maj], [Major version])
AC_DEFINE_UNQUOTED(VMIN, [v_min], [Minor version])
AC_DEFINE_UNQUOTED(VMIC, [v_mic], [Micro version])
AC_DEFINE_UNQUOTED(VREV, [v_rev], [Revison])
version_info="lt_cur:lt_rev:lt_age"
release_info="v_rel"
AC_SUBST(version_info)
AC_SUBST(release_info)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
VMAJ=v_maj
AC_SUBST(VMAJ)

### Needed information

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

case "$host_os" in
   mingw32ce*)
      MODULE_ARCH="$host_os-$host_cpu"
      ;;
   *)
      MODULE_ARCH="$host_os-$host_cpu-v_maj.v_min.v_mic"
      ;;
esac
AC_SUBST(MODULE_ARCH)
AC_DEFINE_UNQUOTED(MODULE_ARCH, "$MODULE_ARCH", "Module architecture")

want_fontconfig="yes"
want_fribidi="yes"
want_harfbuzz="auto"
want_pixman="no"
want_evas_cserve2="yes"

want_evas_engine_buffer="yes"
want_evas_engine_software_xlib="no"
want_evas_engine_software_xcb="no"
want_evas_engine_software_gdi="no"
want_evas_engine_software_ddraw="no"
want_evas_engine_gl_xlib="no"
want_evas_engine_gl_xcb="no"
want_evas_engine_gl_sdl="no"
want_evas_engine_gl_cocoa="no"
want_evas_engine_direct3d="no"
want_evas_engine_fb="no"
want_evas_engine_directfb="no"
want_evas_engine_psl1ght="no"
want_evas_engine_wayland_shm="no"
want_evas_engine_wayland_egl="no"

want_evas_image_loader_webp="yes"
want_evas_image_loader_gif="yes"
want_evas_image_loader_tiff="yes"
want_evas_image_loader_svg="yes"

case "$host_os" in
   mingw32ce*)
      want_fontconfig="no"
      want_evas_image_loader_gif="no"
      want_evas_image_loader_tiff="no"
      want_evas_image_loader_svg="no"
      ;;
   mingw*)
      want_evas_engine_software_gdi="yes"
      want_evas_engine_software_ddraw="yes"
      want_evas_engine_direct3d="yes"
      want_evas_image_loader_svg="yes"
      ;;
   darwin*)
      want_evas_engine_software_xlib="auto"
      want_evas_engine_gl_cocoa="auto"
      ;;
   *)
      want_evas_engine_software_xlib="auto"
      want_evas_engine_gl_xlib="auto"
      want_evas_engine_software_xcb="no"
      want_evas_engine_gl_xcb="no"
      want_evas_engine_fb="auto"
      want_evas_engine_wayland_shm="auto"
      want_evas_engine_wayland_egl="auto"
      ;;
esac

case "$host_vendor" in
  ps3*)
      want_evas_engine_psl1ght="static"
      ;;
esac

requirement_evas=""


### Additional options to configure

# when used, that option makes configure script fails when
# a requirement is selected, but not met.
use_strict="no"
AC_ARG_ENABLE([strict],
   AC_HELP_STRING([--enable-strict], [enable strict checking mode. @<:@default==disabled@:>@]),
   [use_strict="yes"],
   [use_strict="no"])

# Simple X11 build/link
AC_ARG_ENABLE([simple-x11],
   AC_HELP_STRING([--enable-simple-x11], [enable simple x11 linking]),
   [
    if test "x${enableval}" = "xyes" ; then
       want_evas_simple_x11="yes"
    else
       want_evas_simple_x11="no"
    fi
   ])


### Checks for programs

m4_ifdef([AC_PROG_OBJC],
   [
    AC_PROG_OBJC
    _AM_DEPENDENCIES(OBJC)
   ],
   [
    dnl#AC_CHECK_TOOL([OBJC], [gcc])
    dnl#AC_SUBST([OBJC])
    dnl#AC_SUBST([OBJCFLAGS])
   ])

rw_PROG_OBJC_WORKS

AC_PROG_CXX
AC_PROG_CC

# pkg-config
PKG_PROG_PKG_CONFIG

# Check whether pkg-config supports Requires.private
if $PKG_CONFIG --atleast-pkgconfig-version 0.22; then
   pkgconfig_requires_private="Requires.private"
else
   pkgconfig_requires_private="Requires"
fi
AC_SUBST(pkgconfig_requires_private)

# doxygen program for documentation building
EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])


### Checks for libraries

# Evil library for compilation on Windows CE

case "$host_os" in
   mingw*)
      PKG_CHECK_MODULES([EVIL], [evil >= 1.0.0])
      AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if evil package is installed])
      requirement_evas="evil ${requirement_evas}"
   ;;
esac

### Checks for portability layer

PKG_CHECK_MODULES([EXOTIC],
   [exotic],
   [enable_exotic="yes"],
   [enable_exotic="no"])

if test "x${enable_exotic}" = "xyes"; then
    requirement_evas="exotic ${requirement_evas}"

    AC_DEFINE([HAVE_EXOTIC], [1], [Define to 1 if you have Exotic.])
fi
AM_CONDITIONAL([EVAS_HAVE_EXOTIC], [test "x${enable_exotic}" = "xyes"])

## strong dependencies

# Eina
PKG_CHECK_MODULES([EINA], [eina >= 1.6.0])
CPPFLAGS_SAVE="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $EINA_CFLAGS"
AC_CHECK_SIZEOF(Eina_Unicode,, [#include <Eina.h>])
CPPFLAGS="$CPPFLAGS_SAVE"

# Freetype
PKG_CHECK_MODULES([FREETYPE], [freetype2 >= 9.3.0])

requirement_evas="freetype2 eina >= 1.6.0 ${requirement_evas}"

# Eo

PKG_CHECK_MODULES([EO], [eo])
requirement_evas="eo ${requirement_evas}"

## optional dependencies

# FontConfig
have_fontconfig="no"
AC_ARG_ENABLE([fontconfig],
   AC_HELP_STRING([--disable-fontconfig],
      [disable fontconfig for finding fonts. @<:@default=enabled@:>@]),
   [
    if test "x${enableval}" = "xyes" ; then
       want_fontconfig="yes"
    else
       want_fontconfig="no"
    fi
   ])

if test "x${want_fontconfig}" = "xyes"; then
   PKG_CHECK_MODULES([FONTCONFIG],
      [fontconfig >= 2.5.0],
      [
       have_fontconfig="yes"
       AC_DEFINE(HAVE_FONTCONFIG, 1, [have fontconfig searching capabilities])
       requirement_evas="fontconfig ${requirement_evas}"
      ],
      [AC_MSG_ERROR([Fontconfig not found])])
fi

# linebreak
have_linebreak="no"
want_linebreak="yes"
AC_ARG_ENABLE([liblinebreak],
   AC_HELP_STRING([--disable-liblinebreak],
      [disable linking against liblinebreak. @<:@default=enabled@:>@]),
   [
    if test "x${enableval}" = "xyes" ; then
       want_linebreak="yes"
    else
       want_linebreak="no"
    fi
   ])


AM_CONDITIONAL(EVAS_USE_LINEBREAK, test "x${want_linebreak}" = "xyes")
if test "x${want_linebreak}" = "xyes" ; then
   have_linebreak="yes"
   LINEBREAK_CFLAGS='-I$(top_srcdir)/src/static_deps/liblinebreak'
   LINEBREAK_LIBS='$(top_builddir)/src/static_deps/liblinebreak/liblinebreak.la'
   AC_SUBST(LINEBREAK_CFLAGS)
   AC_SUBST(LINEBREAK_LIBS)
   AC_DEFINE(HAVE_LINEBREAK, 1, [have liblinebreak support])
else
   have_linebreak="no"
fi

# fribidi support
have_fribidi="no"
AC_ARG_ENABLE([fribidi],
   AC_HELP_STRING([--disable-fribidi],
      [disable bidirectional text support. @<:@default=enabled@:>@]),
   [
    if test "x${enableval}" = "xyes" ; then
       want_fribidi="yes"
    else
       want_fribidi="no"
    fi
   ])

if test "x${want_fribidi}" = "xyes"; then
   PKG_CHECK_MODULES([FRIBIDI],
      [fribidi >= 0.19.2],
      [
       have_fribidi="yes"
       AC_DEFINE(HAVE_FRIBIDI, 1, [have fribidi support])
       requirement_evas="fribidi ${requirement_evas}"
       CPPFLAGS_SAVE="$CPPFLAGS"
       CPPFLAGS="$CPPFLAGS $FRIBIDI_CFLAGS"
       AC_CHECK_SIZEOF(FriBidiChar,, [#include <fribidi/fribidi.h>])
       CPPFLAGS="$CPPFLAGS_SAVE"
      ],
      [
       AC_MSG_ERROR([Fribidi not found])
      ])
fi

# harfbuzz support
have_harfbuzz="no"
have_harfbuzz_ft="no"
AC_ARG_ENABLE([harfbuzz],
   AC_HELP_STRING([--disable-harfbuzz],
      [disable complex text shaping and layouting support. @<:@default=enabled@:>@]),
   [
    if test "x${enableval}" = "xyes" ; then
       want_harfbuzz="yes"
    else
       want_harfbuzz="no"
    fi
   ])

if test "x${want_harfbuzz}" = "xyes" -o "x${want_harfbuzz}" = "xauto" ; then
   PKG_CHECK_MODULES([HARFBUZZ],
      [harfbuzz >= 0.9.0],
      [
       have_harfbuzz="yes"
       requirement_evas="harfbuzz ${requirement_evas}"
      ],
      [
       if test "x$want_harfbuzz" = "xyes" -a "x$use_strict" = "xyes" ; then
          AC_MSG_ERROR([Harfbuzz not found (strict dependencies checking)])
       fi
      ])
   if test "x$have_harfbuzz" = "xyes" ; then

      CPPFLAGS_SAVE="$CPPFLAGS"
      CPPFLAGS="$CPPFLAGS $HARFBUZZ_CFLAGS $FREETYPE_CFLAGS"
# must have for usage with harfbuzz although harfbuzz may not have it.

      AC_CHECK_HEADER(hb-ft.h,
          [
           have_harfbuzz_ft="yes"
           #Depend on harfbuzz ft for harfbuzz support
           AC_DEFINE(HAVE_HARFBUZZ, 1, [have harfbuzz support])
          ],
          [
           if test "x$want_harfbuzz" = "xyes" -a "x$use_strict" = "xyes" ; then
              AC_MSG_ERROR([Harfbuzz-ft not found (strict dependencies checking)])
           fi
           have_harfbuzz="no"
          ])
      CPPFLAGS="$CPPFLAGS_SAVE"
   fi
fi

if test "x${have_harfbuzz}" = "xno"; then
  HAS_HARFBUZZ=0
fi

# Pixman
have_pixman="no"
AC_ARG_ENABLE([pixman],
   AC_HELP_STRING([--enable-pixman],
      [enable pixman for software rendering. @<:@default=enabled@:>@]),
   [
    if test "x${enableval}" = "xyes" ; then
       want_pixman="yes"
    else
       want_pixman="no"
    fi
   ])

if test "x${want_pixman}" = "xyes" -o "x${want_pixman}" = "xauto" ; then
   PKG_CHECK_MODULES([PIXMAN],
      [pixman-1],
      [
       have_pixman="yes"
       AC_DEFINE(HAVE_PIXMAN, 1, [have pixman for rendering])
       requirement_evas="pixman-1 ${requirement_evas}"
      ],
      [
       if test "x${want_pixman}" = "xyes" -a "x${use_strict}" = "xyes" ; then
          AC_MSG_ERROR([Pixman not found (strict dependencies checking)])
       fi
      ])
fi

have_pixman_font="no"
AC_ARG_ENABLE(pixman-font,
  AC_HELP_STRING([--enable-pixman-font], [Allow pixman to render fonts]),
  [ 
    have_pixman_font="yes"
    AC_DEFINE(PIXMAN_FONT, 1, [Allow pixman to render fonts])
  ]
)

have_pixman_rect="no"
AC_ARG_ENABLE(pixman-rect,
  AC_HELP_STRING([--enable-pixman-rect], [Allow pixman to render rects]),
  [ 
    have_pixman_rect="yes"
    AC_DEFINE(PIXMAN_RECT, 1, [Allow pixman to render rects])
  ]
)

have_pixman_line="no"
AC_ARG_ENABLE(pixman-line,
  AC_HELP_STRING([--enable-pixman-line], [Allow pixman to render lines]),
  [ 
    have_pixman_line="yes"
    AC_DEFINE(PIXMAN_LINE, 1, [Allow pixman to render lines])
  ]
)

have_pixman_poly="no"
AC_ARG_ENABLE(pixman-poly,
  AC_HELP_STRING([--enable-pixman-poly], [Allow pixman to render polys]),
  [ 
    have_pixman_poly="yes"
    AC_DEFINE(PIXMAN_POLY, 1, [Allow pixman to render polys])
  ]
)

have_pixman_image="no"
AC_ARG_ENABLE(pixman-image,
  AC_HELP_STRING([--enable-pixman-image], [Allow pixman to render images]),
  [ 
    have_pixman_image="yes"
    AC_DEFINE(PIXMAN_IMAGE, 1, [Allow pixman to render images])
  ]
)

have_pixman_image_scale_sample="no"
AC_ARG_ENABLE(pixman-image-scale-sample,
  AC_HELP_STRING([--enable-pixman-image-scale-sample], [Allow pixman to render sampled scaled images]),
  [ 
    have_pixman_image_scale_sample="yes"
    AC_DEFINE(PIXMAN_IMAGE_SCALE_SAMPLE, 1, [Allow pixman to render image sampled scaling])
  ]
)

have_tile_rotate="no"
AC_ARG_ENABLE(tile-rotate,
  AC_HELP_STRING([--enable-tile-rotate], [Enable tiled rotate algorithm]),
  [ 
    have_tile_rotate="yes"
    AC_DEFINE(TILE_ROTATE, 1, [Enable tiled rotate algorithm])
  ]
)

### Checks for header files
AC_HEADER_STDC
AC_CHECK_HEADERS([unistd.h stdint.h sys/param.h netinet/in.h sys/mman.h])
EFL_CHECK_PATH_MAX

### Checks for types
AC_CHECK_TYPES([struct sigaction], [], [],
               [[#include <signal.h>]])


### Checks for structures


### Checks for compiler characteristics
AC_C_CONST
AC_C_BIGENDIAN
AC_PROG_CC_STDC
EFL_ATTRIBUTE_UNUSED
EFL_ATTRIBUTE_VECTOR
EFL_ATTRIBUTE_ALWAYS_INLINE


### Checks for linker characteristics
lt_enable_auto_import=""
case "${host_os}" in
   mingw*)
      AC_DEFINE(EFL_EVAS_BUILD, 1, [Define to mention that evas is built])
      lt_enable_auto_import="-Wl,--enable-auto-import"
   ;;
esac
AC_SUBST(lt_enable_auto_import)


### Checks for library functions

AC_ISC_POSIX
AC_CHECK_FUNCS([siglongjmp])

# alloca
AC_FUNC_ALLOCA

# fnmatch
EFL_CHECK_FNMATCH([], [AC_MSG_ERROR([Cannot find fnmatch()])])

# dlopen
dlopen_libs=""
case "$host_os" in
  mingw32ce*)
# managed by evil
    AC_DEFINE(HAVE_DLSYM)
  ;;
  mingw*)
# nothing on mingw platform
  ;;
  *)
    AC_CHECK_FUNCS(dlsym, res=yes, res=no)
    if test "x$res" = "xyes"; then
      AC_DEFINE(HAVE_DLSYM)
    else
      AC_CHECK_LIB(dl, dlsym, res=yes, res=no)
      if test "x$res" = "xyes"; then
        dlopen_libs=-ldl
        AC_DEFINE(HAVE_DLSYM)
      else
        want_evas_engine_gl_xlib="no"
        want_evas_engine_gl_xcb="no"
        want_evas_engine_gl_sdl="no"
      fi
    fi
esac
AC_SUBST(dlopen_libs)

res=no
AC_CHECK_LIB(m, lround, res=yes, res=no)
if test "x$res" = "xyes"; then
  AC_DEFINE(HAVE_LROUND, 1, C99 lround function exists)
fi

if test "x${want_evas_image_loader_generic}" = "xyes" ; then
   EFL_CHECK_SHM_OPEN
fi

want_evas_cserve2="yes"
# cserve2 only works on Linux so far.
if test "x${want_evas_cserve2}" = "xyes" ; then
   AC_CHECK_HEADERS(
      [sys/epoll.h sys/inotify.h sys/signalfd.h],
      [],
      [ want_evas_cserve2="no" ]
   )
fi

shm_open_libs=""
if test "x${want_evas_cserve2}" = "xyes" ; then
   EFL_CHECK_SHM_OPEN(
      [ shm_open_libs=${EFL_SHM_OPEN_LIBS} ],
      [ want_evas_cserve2="no" ])
fi
AC_SUBST([shm_open_libs])

if test "x${want_evas_cserve2}" = "xyes" ; then
  AC_DEFINE(EVAS_CSERVE2, 1, [Shared cache server.])
fi
AM_CONDITIONAL([EVAS_CSERVE2], [test "x${want_evas_cserve2}" = "xyes"])

#######################################
## GL force flavor gles
gl_flavor_gles="no"
AC_MSG_CHECKING(whether to build GLES flavor of GL)
AC_ARG_ENABLE(gl-flavor-gles,
  AC_HELP_STRING([--enable-gl-flavor-gles], [enable gles flavor of gl instead of standard]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(GL_GLES, 1, [GLSL runtime shader GLES2 support])
        gl_flavor_gles="yes"
        gles_variety_sgx="yes"
      else
        AC_MSG_RESULT(no)
        gl_flavor_gles="no"
      fi
  ], [
      AC_MSG_RESULT(no)
      gl_flavor_gles="no"
  ]
)

#####################################################################
## Engines

EVAS_CHECK_ENGINE([buffer], [${want_evas_engine_buffer}], [yes], [Buffer])
EVAS_CHECK_ENGINE([software-gdi], [${want_evas_engine_software_gdi}], [no], [Software GDI])
EVAS_CHECK_ENGINE([software-ddraw], [${want_evas_engine_software_ddraw}], [no], [Software DirectDraw])
EVAS_CHECK_ENGINE([direct3d], [${want_evas_engine_direct3d}], [no], [Direct3D])
EVAS_CHECK_ENGINE([psl1ght], [${want_evas_engine_psl1ght}], [no], [PSL1GHT])
EVAS_CHECK_ENGINE([gl-sdl], [${want_evas_engine_gl_sdl}], [no], [OpenGL SDL])
EVAS_CHECK_ENGINE([gl-cocoa], [${want_evas_engine_gl_cocoa}], [no], [OpenGL Cocoa])
EVAS_CHECK_ENGINE([fb], [${want_evas_engine_fb}], [no], [Framebuffer])
EVAS_CHECK_ENGINE([directfb], [${want_evas_engine_directfb}], [no], [DirectFB])
EVAS_CHECK_ENGINE([wayland-shm], [${want_evas_engine_wayland_shm}], [no], [Wayland Shm])
EVAS_CHECK_ENGINE([wayland-egl], [${want_evas_engine_wayland_egl}], [no], [Wayland Egl])

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

# if software generic need to be build as part of libevas.so
have_static_software_generic="no"
AC_ARG_ENABLE([static-software-generic],
   [AC_HELP_STRING([--enable-static-software-generic], [Build software generic engine as part of libevas])],
   [have_static_software_generic=${enableval}]
)
AC_MSG_CHECKING([Whether to build software generic engine as part of libevas])
AC_MSG_RESULT([${have_static_software_generic}])

if test "x${have_static_module}" = "xyes" -a "x${have_static_software_generic}" = "xno"; then
   AC_MSG_WARN([Forcing --enable-static-software-generic as engines must be compiled statically])
   have_static_software_generic="yes"
fi

AM_CONDITIONAL(EVAS_STATIC_BUILD_SOFTWARE_GENERIC, test "x${have_static_software_generic}" = "xyes")
if test "x${have_static_software_generic}" = "xyes"; then
   AC_DEFINE(EVAS_STATIC_BUILD_SOFTWARE_GENERIC, [1], [Build software generic engine as part of libevas])
fi


######## Software Engine ########
have_evas_engine_software_x11="no"
have_evas_engine_software_xcb="no"
have_evas_engine_software_xlib="no"
build_software_xcb=

## XCB Checks ##
want_xcb="no"
AC_MSG_CHECKING(whether to build Software XCB Engine)
AC_ARG_ENABLE(software-xcb,
  AC_HELP_STRING([--enable-software-xcb], [enable Software XCB Engine. @<:@default=disabled@:>@]), [want_xcb=$enableval]
)
if test "x$want_xcb" != "xno" ; then
  EVAS_CHECK_ENGINE([software-xcb], [${want_evas_engine_software_xcb}], [no], [Software XCB])
  if test "x$have_evas_engine_software_xcb" != "xno" ; then
    build_software_xcb="yes"
    have_evas_engine_software_x11="yes"
    AC_DEFINE_UNQUOTED(BUILD_ENGINE_SOFTWARE_X11, [1], [Build software X11 engine])
    AC_DEFINE_UNQUOTED(BUILD_ENGINE_SOFTWARE_XCB, [1], [Build software XCB backend])
  fi
else
  AC_MSG_RESULT(no)
  build_software_xcb="no"
fi

## If we are not building XCB, then we can build XLib ##

if test "x$build_software_xcb" = "xno" ; then
  EVAS_CHECK_ENGINE([software-xlib], [${want_evas_engine_software_xlib}], [no], [Software Xlib])
  # if software_x11 is available, define everything needed for X11
  if test "x$have_evas_engine_software_xlib" != "xno" ; then
    have_evas_engine_software_x11="yes"
    AC_DEFINE_UNQUOTED(BUILD_ENGINE_SOFTWARE_X11, [1], [Build software X11 engine])
    AC_DEFINE_UNQUOTED(BUILD_ENGINE_SOFTWARE_XLIB, [1], [Build software Xlib backend])
  fi
else
  have_evas_engine_software_xlib="no"
fi

if test "x$have_evas_engine_software_xlib" = "xstatic" -o "x$have_evas_engine_software_xcb" = "xstatic"; then
   AC_DEFINE_UNQUOTED(EVAS_STATIC_BUILD_SOFTWARE_X11, [1], [Build software X11 engine as part of libevas])
fi

AC_SUBST(have_evas_engine_software_xcb)
AC_SUBST(have_evas_engine_software_xlib)

AM_CONDITIONAL([BUILD_ENGINE_SOFTWARE_XLIB], [test "x${have_evas_engine_software_xlib}" != "xno"])
AM_CONDITIONAL([BUILD_ENGINE_SOFTWARE_XCB], [test "x${have_evas_engine_software_xcb}" != "xno"])
AM_CONDITIONAL([BUILD_ENGINE_SOFTWARE_X11], [test "x${have_evas_engine_software_x11}" != "xno"])

AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_X11], [test "x${have_evas_engine_software_xcb}" = "xstatic" -o "x${have_evas_engine_software_xlib}" = "xstatic"])
AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_XLIB], [test "x${have_evas_engine_software_xlib}" = "xstatic"])
AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_XCB], [test "x${have_evas_engine_software_xcb}" = "xstatic"])

AC_SUBST(have_evas_engine_software_x11)


######## OpenGL Engine ########
have_evas_engine_gl_x11="no"
have_evas_engine_gl_xcb="no"
build_gl_xcb="no"

## XCB Checks ##
want_gl_xcb="no"
AC_MSG_CHECKING(whether to build OpenGL XCB Engine)
AC_ARG_ENABLE(gl-xcb,
  AC_HELP_STRING([--enable-gl-xcb], [enable OpenGL XCB Engine. @<:@default=disabled@:>@]), [want_gl_xcb=$enableval]
)
if test "x$want_gl_xcb" = "xyes" ; then
  EVAS_CHECK_ENGINE([gl-xcb], [${want_evas_engine_gl_xcb}], [no], [OpenGL XCB])
  if test "x$have_evas_engine_gl_xcb" != "xno" ; then
    build_gl_xcb="yes"
    have_evas_engine_gl_x11="yes"
   AC_DEFINE(BUILD_ENGINE_GL_X11, [1], [Build GL X11 engine])
   AC_DEFINE(BUILD_ENGINE_GL_XCB, [1], [Build GL Xcb backend])
    if test "x$have_evas_engine_gl_xcb" = "xstatic" ; then
      AC_DEFINE(EVAS_STATIC_BUILD_GL_X11, [1], [Build GL X11 engine as part of libevas])
    fi
  fi
  AC_SUBST(have_evas_engine_gl_xcb)
else
  AC_MSG_RESULT(no)
  build_gl_xcb="no"
fi

## If we are not building XCB, then we can build XLib ##

if test "x$build_gl_xcb" = "xno" ; then
  EVAS_CHECK_ENGINE([gl-xlib], [${want_evas_engine_gl_xlib}], [no], [OpenGL XLib])
  if test "x$have_evas_engine_gl_xlib" != "xno" ; then
    have_evas_engine_gl_x11="yes"
    AC_DEFINE_UNQUOTED(BUILD_ENGINE_GL_X11, [1], [Build OpenGL X11 engine])
    AC_DEFINE_UNQUOTED(BUILD_ENGINE_GL_XLIB, [1], [Build OpenGL Xlib backend])
    if test "x$have_evas_engine_gl_xlib" = "xstatic" ; then
      AC_DEFINE_UNQUOTED(EVAS_STATIC_BUILD_GL_X11, [1], [Build OpenGL X11 engine as part of libevas])
    fi
  fi
else
  have_evas_engine_gl_xlib="no"
fi
AC_SUBST(have_evas_engine_gl_xlib)

AM_CONDITIONAL([BUILD_ENGINE_GL_XLIB], [test "x${have_evas_engine_gl_xlib}" != "xno"])
AM_CONDITIONAL([BUILD_ENGINE_GL_XCB], [test "x${have_evas_engine_gl_xcb}" != "xno"])
AM_CONDITIONAL([BUILD_ENGINE_GL_X11], [test "x${have_evas_engine_gl_x11}" != "xno"])

AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_X11], [test "x${have_evas_engine_gl_xcb}" = "xstatic" -o "x${have_evas_engine_gl_xlib}" = "xstatic"])
AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_XLIB], [test "x${have_evas_engine_gl_xlib}" = "xstatic"])
AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_XCB], [test "x${have_evas_engine_gl_xcb}" = "xstatic"])

AC_SUBST(have_evas_engine_gl_x11)

# common gl
have_evas_engine_gl_common="no"
have_static_evas_engine_gl_common="no"
if test "x$have_evas_engine_gl_xlib" = "xyes" \
   || test "x$have_evas_engine_gl_xcb" = "xyes" \
   || test "x$have_evas_engine_gl_sdl" = "xyes" \
   || test "x$have_evas_engine_gl_cocoa" = "xyes" \
   || test "x$have_evas_engine_wayland_egl" = "xyes"; then
   have_evas_engine_gl_common="yes"
fi
if test "x$have_evas_engine_gl_xlib" = "xstatic" \
   || test "x$have_evas_engine_gl_xcb" = "xstatic" \
   || test "x$have_evas_engine_gl_sdl" = "xstatic" \
   || test "x$have_evas_engine_gl_cocoa" = "xstatic" \
   || test "x$have_evas_engine_wayland_egl" = "xstatic"; then
   have_evas_engine_gl_common="yes"
   have_static_evas_engine_gl_common="yes"
fi

if test "x$have_evas_engine_gl_common" = "xyes"; then
   evas_engine_gl_common_libs=""
   AC_DEFINE(BUILD_ENGINE_GL_COMMON, [1], [Generic OpenGL Rendering Support])
fi

AC_SUBST([evas_engine_gl_common_libs])

AM_CONDITIONAL(BUILD_ENGINE_GL_COMMON, [test "x$have_evas_engine_gl_common" = "xyes"])

AM_CONDITIONAL(EVAS_STATIC_BUILD_GL_COMMON, test "x${have_static_evas_engine_gl_common}" = "xyes")
if test "x${have_static_evas_engine_gl_common}" = "xyes"; then
   AC_DEFINE(EVAS_STATIC_BUILD_GL_COMMON, [1], [Build GL generic engine as part of libevas])
fi

# gl_sdl
if test "x$have_evas_engine_gl_sdl" = "xyes" || test "x$have_evas_engine_gl_sdl" = "xstatic" ; then
   AC_CHECK_DECL(SDL_GL_CONTEXT_MAJOR_VERSION,
      [AC_DEFINE(HAVE_SDL_GL_CONTEXT_VERSION, [1], [SDL_GL version attributes present])],,
      [#include <SDL/SDL_video.h>])
fi

if test "x$gl_flavor_gles" = "xyes"; then
   AC_CHECK_DECL(SDL_OPENGLES,
      [AC_DEFINE(HAVE_SDL_FLAG_OPENGLES, [1], [SDL_OPENGLES flag is present])],,
      [#include <SDL/SDL_video.h>])
fi

#####################################################################
## Font Loaders

PKG_CHECK_MODULES([EET], [eet >= 1.7.0], [], [AC_MSG_ERROR([EET not found])])
AC_DEFINE([BUILD_FONT_LOADER_EET], [1], [EET Font Loader Support])
requirement_evas="eet >= 1.7.0 ${requirement_evas}"

#####################################################################
## Image loaders

#have_static_module="yes"
do_module="true"
do_static="false"

#####################################################################
evas_image_loader_generic_cflags=""
evas_image_loader_generic_libs=""
AC_SUBST([evas_image_loader_generic_cflags])
AC_SUBST([evas_image_loader_generic_libs])
AM_CONDITIONAL(BUILD_LOADER_GENERIC, [${do_module}])
AM_CONDITIONAL(EVAS_STATIC_BUILD_GENERIC, [${do_static}])
AC_DEFINE(BUILD_LOADER_GENERIC, [1], [Generic Loader])

#####################################################################
evas_image_loader_xpm_cflags=""
evas_image_loader_xpm_libs=""
AC_SUBST([evas_image_loader_xpm_cflags])
AC_SUBST([evas_image_loader_xpm_libs])
AM_CONDITIONAL(BUILD_LOADER_XPM, [${do_module}])
AM_CONDITIONAL(EVAS_STATIC_BUILD_XPM, [${do_static}])
AC_DEFINE(BUILD_LOADER_XPM, [1], [XPM Loader])

#####################################################################
evas_image_loader_bmp_cflags=""
evas_image_loader_bmp_libs=""
AC_SUBST([evas_image_loader_bmp_cflags])
AC_SUBST([evas_image_loader_bmp_libs])
AM_CONDITIONAL(BUILD_LOADER_BMP, [${do_module}])
AM_CONDITIONAL(EVAS_STATIC_BUILD_BMP, [${do_static}])
AC_DEFINE(BUILD_LOADER_BMP, [1], [BMP Loader])

#####################################################################
evas_image_loader_ico_cflags=""
evas_image_loader_ico_libs=""
AC_SUBST([evas_image_loader_ico_cflags])
AC_SUBST([evas_image_loader_ico_libs])
AM_CONDITIONAL(BUILD_LOADER_ICO, [${do_module}])
AM_CONDITIONAL(EVAS_STATIC_BUILD_ICO, [${do_static}])
AC_DEFINE(BUILD_LOADER_ICO, [1], [ICO Loader])

#####################################################################
evas_image_loader_pmaps_cflags=""
evas_image_loader_pmaps_libs=""
AC_SUBST([evas_image_loader_pmaps_cflags])
AC_SUBST([evas_image_loader_pmaps_libs])
AM_CONDITIONAL(BUILD_LOADER_PMAPS, [${do_module}])
AM_CONDITIONAL(EVAS_STATIC_BUILD_PMAPS, [${do_static}])
AC_DEFINE(BUILD_LOADER_PMAPS, [1], [PMAPS Loader])

#####################################################################
evas_image_loader_psd_cflags=""
evas_image_loader_psd_libs=""
AC_SUBST([evas_image_loader_psd_cflags])
AC_SUBST([evas_image_loader_psd_libs])
AM_CONDITIONAL(BUILD_LOADER_PSD, [${do_module}])
AM_CONDITIONAL(EVAS_STATIC_BUILD_PSD, [${do_static}])
AC_DEFINE(BUILD_LOADER_PSD, [1], [PSD Loader])

#####################################################################
evas_image_loader_tga_cflags=""
evas_image_loader_tga_libs=""
AC_SUBST([evas_image_loader_tga_cflags])
AC_SUBST([evas_image_loader_tga_libs])
AM_CONDITIONAL(BUILD_LOADER_TGA, [${do_module}])
AM_CONDITIONAL(EVAS_STATIC_BUILD_TGA, [${do_static}])
AC_DEFINE(BUILD_LOADER_TGA, [1], [TGA Loader])

#####################################################################
evas_image_loader_wbmp_cflags=""
evas_image_loader_wbmp_libs=""
AC_SUBST([evas_image_loader_wbmp_cflags])
AC_SUBST([evas_image_loader_wbmp_libs])
AM_CONDITIONAL(BUILD_LOADER_WBMP, [${do_module}])
AM_CONDITIONAL(EVAS_STATIC_BUILD_WBMP, [${do_static}])
AC_DEFINE(BUILD_LOADER_WBMP, [1], [WBMP Loader])

#####################################################################
PKG_CHECK_MODULES([EET], [eet >= 1.7.0], [requirement="eet >= 1.7.0"], [AC_MSG_ERROR([Eet not found])])
dnl requirement_evas="${requirement} ${requirement_evas}"
evas_image_loader_eet_cflags="${EET_CFLAGS}"
evas_image_loader_eet_libs="${EET_LIBS}"
AC_SUBST([evas_image_loader_eet_cflags])
AC_SUBST([evas_image_loader_eet_libs])
AM_CONDITIONAL(BUILD_LOADER_EET, [${do_module}])
AM_CONDITIONAL(EVAS_STATIC_BUILD_EET, [${do_static}])
AC_DEFINE(BUILD_LOADER_EET, [1], [EET Loader])

#####################################################################
have_evas_image_saver_jpeg="no"
evas_image_loader_jpeg_cflags=""
AC_CHECK_HEADER([jpeglib.h], [], [AC_MSG_ERROR([JPEG header not found])])
AC_CHECK_LIB([jpeg], [jpeg_CreateDecompress],
             [ evas_image_loader_jpeg_libs="-ljpeg"
               AC_COMPILE_IFELSE(
               [  AC_LANG_PROGRAM([[
#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>
                  ]],
                  [[
struct jpeg_decompress_struct decomp;
decomp.region_x = 0;
                  ]])],
                  [have_jpeg_region="yes"],
                  [have_jpeg_region="no"])
             ],[ AC_MSG_ERROR([JPEG missing jpeg_CreateDecompress]) ])
AC_SUBST([evas_image_loader_jpeg_cflags])
AC_SUBST([evas_image_loader_jpeg_libs])
AM_CONDITIONAL(BUILD_LOADER_JPEG, [${do_module}])
AM_CONDITIONAL(EVAS_STATIC_BUILD_JPEG, [${do_static}])
AC_DEFINE(BUILD_LOADER_JPEG, [1], [JPEG Loader])
case "$host_os" in
   mingw*)
      ;;
   *)
      AC_DEFINE(EVAS_BUILD_SAVER_JPEG, [1], [Build JPEG saver])
      AM_CONDITIONAL([BUILD_SAVER_JPEG], [true])
      have_evas_image_saver_jpeg="yes"
      ;;
esac

#####################################################################
PKG_CHECK_MODULES([PNG], [libpng >= 1.2.10], [requirement="libpng >= 1.2.10"], [
  PKG_CHECK_MODULES([PNG], [libpng15], [requirement="libpng15"], [
    PKG_CHECK_MODULES([PNG], [libpng14], [requirement="libpng14"], [
      PKG_CHECK_MODULES([PNG], [libpng12], [requirement="libpng12"], [
        AC_MSG_ERROR([PNG not found])
      ])
    ])
  ])
])
dnl requirement_evas="${requirement} ${requirement_evas}"
evas_image_loader_png_cflags="${PNG_CFLAGS}"
evas_image_loader_png_libs="${PNG_LIBS}"
AC_SUBST([evas_image_loader_png_cflags])
AC_SUBST([evas_image_loader_png_libs])
AM_CONDITIONAL(BUILD_LOADER_PNG, [${do_module}])
AM_CONDITIONAL(EVAS_STATIC_BUILD_PNG, [${do_static}])
AC_DEFINE(BUILD_LOADER_PNG, [1], [PNG Loader])

EVAS_CHECK_IMAGE_LOADER([WEBP], [${want_evas_image_loader_webp}])
EVAS_CHECK_IMAGE_LOADER([Gif], [${want_evas_image_loader_gif}])
EVAS_CHECK_IMAGE_LOADER([Tiff], [${want_evas_image_loader_tiff}])
EVAS_CHECK_IMAGE_LOADER([SVG], [${want_evas_image_loader_svg}])

#####################################################################
## Cpu based optimizations

#######################################
## PTHREADS

# basic pthread support
AC_CHECK_HEADER(pthread.h, [], [AC_MSG_ERROR([Pthread support missing.])])
# sched_getaffinity pthread_attr_setaffinity_np
AC_CHECK_HEADERS([pthread.h sched.h],
   [
    CFLAGS_save="${CFLAGS}"
    CFLAGS="${CFLAGS} -pthread"
    LIBS_save="${LIBS}"
    LIBS="${LIBS} -pthread"
    AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([[
#include <pthread.h>
#include <sched.h>
                       ]],
                       [[
pthread_attr_setaffinity_np(NULL, 0, NULL);
pthread_barrier_wait(NULL);
                       ]])], [],
      [AC_MSG_ERROR([Missing pthread_attr_setaffinity_np or pthread_barrier_wait.])])
   CFLAGS=${CFLAGS_save}
   LIBS=${LIBS_save}
  ],
  [AC_MSG_ERROR([Missing sched.h])])
pthread_cflags="-pthread"
pthread_libs="-pthread"
AC_DEFINE(BUILD_ASYNC_EVENTS, 1, [Build async events support])
AC_DEFINE(BUILD_ASYNC_PRELOAD, 1, [Build async image preload support])

#######################################
## Pipe Renderer
build_pipe_render="no"

# Let disable pipe render strongly as it doesn't massively improve performance, but
# just hammer more watt for a few more percent. The code is kept as the understanding
# is that with an async pipeline things will be better.
#
#AC_MSG_CHECKING(whether to build Threaded Pipe Rendering support)
#AC_ARG_ENABLE(pipe-render,
#  AC_HELP_STRING([--enable-pipe-render], [enable threaded pipe rendering support]),
#  [ build_pipe_render=$enableval ]
#)
#AC_MSG_RESULT($build_pipe_render)
#AC_MSG_CHECKING(whether we can build Threaded Pipe Rendering support)
#if test \( "x$build_pipe_render" = "xyes" -o "x$build_pipe_render" = "xauto" \); then
#  AC_MSG_RESULT(yes)
#  AC_DEFINE(BUILD_PIPE_RENDER, 1, [Build pipe render support])
#  build_pipe_render="yes"
#else
#  AC_MSG_RESULT(no)
#  build_pipe_render="no"
#fi

#######################################
## CPU architecture specific assembly

build_cpu_mmx="no"
build_cpu_sse3="no"
build_cpu_altivec="no"
build_cpu_neon="no"

case $host_cpu in
  i*86|x86_64|amd64)
    AC_DEFINE(BUILD_MMX, 1, [Build MMX Code])
    build_cpu_mmx="yes"
    build_cpu_sse3="yes"
    ;;
    
  *power* | *ppc*)
    build_cpu_altivec="yes"
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
            build_cpu_altivec="no"
          ]
        )
        CFLAGS=$save_CFLAGS
        CPPFLAGS=$save_CPPFLAGS
      ]
    )
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
    ;;
    
  arm*)
    build_cpu_neon="yes"
    AC_TRY_COMPILE([#include <arm_neon.h>],
      [asm volatile ("vqadd.u8 d0, d1, d0\n")],
      [
        AC_MSG_RESULT(yes)
        AC_DEFINE(BUILD_NEON, 1, [Build NEON Code])
        build_cpu_neon="yes"
      ],[
        AC_MSG_RESULT(no)
        build_cpu_neon="no"
      ])
    ;;
esac

#######################################
## CHECK SSE3
## sse3 is still an option because older gcc's dont have support for
## intrinsics used
AC_MSG_CHECKING(whether to build sse3 code)
AC_ARG_ENABLE(cpu-sse3,
  AS_HELP_STRING([--enable-cpu-sse3], [enable sse3 code]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_CHECK_HEADER(immintrin.h,
          [
            AC_MSG_RESULT(yes)
            AC_DEFINE(BUILD_SSE3, 1, [Build SSE3 Code])
            build_cpu_sse3="yes"
           ],
          [
            AC_MSG_RESULT(no)
            build_cpu_sse3="no"
          ])
      else
        AC_MSG_RESULT(no)
        build_cpu_sse3="no"
      fi
  ],
  [
    AC_MSG_RESULT($build_cpu_sse3)
    if test "x$build_cpu_sse3" = "xyes" ; then
      AC_CHECK_HEADER(immintrin.h,
        [
          AC_MSG_RESULT(yes)
          AC_DEFINE(BUILD_SSE3, 1, [Build SSE3 Code])
          build_cpu_sse3="yes"
         ],
        [
          AC_MSG_RESULT(no)
          build_cpu_sse3="no"
        ])
    fi
  ]
)

EVAS_SSE3_CFLAGS=" "
if test "x$build_cpu_sse3" = "xyes" ; then
    EVAS_SSE3_CFLAGS="-msse3 "
fi

AC_SUBST(CFLAGS)
AC_SUBST(EVAS_SSE3_CFLAGS)

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
  AC_DEFINE(EVAS_MAGIC_DEBUG, 1, [complain when people pass in wrong object types etc.])
fi

#####################################################################
## dither options

## Small dither mask instead of big one (lower quality)
EVAS_CHECK_DITHER([small-dither-mask], [small dither mask], [conv_small_dither="yes"], [conv_small_dither="no"])
## Alternate Line dither mask instead of big one (lower quality - but fastest)
EVAS_CHECK_DITHER([line-dither-mask], [line dither mask], [conv_line_dither="yes"], [conv_line_dither="no"])
## No dither mask at all for 16bpp
EVAS_CHECK_DITHER([no-dither-mask], [conversion to 16bpp without dither mask], [conv_no_dither="yes"], [conv_no_dither="no"])

#####################################################################
## no longer options - turn on.

AC_DEFINE(BUILD_CONVERT_8_RGB_332, 1, [8bpp RGB 332])
AC_DEFINE(BUILD_CONVERT_8_RGB_666, 1, [8bpp RGB 666])
AC_DEFINE(BUILD_CONVERT_8_RGB_232, 1, [8bpp RGB 232])
AC_DEFINE(BUILD_CONVERT_8_RGB_222, 1, [8bpp RGB 222])
AC_DEFINE(BUILD_CONVERT_8_RGB_221, 1, [8bpp RGB 221])
AC_DEFINE(BUILD_CONVERT_8_RGB_121, 1, [8bpp RGB 121])
AC_DEFINE(BUILD_CONVERT_8_RGB_111, 1, [8bpp RGB 111])
AC_DEFINE(BUILD_CONVERT_16_BGR_565, 1, [16bpp BGR 565])
AC_DEFINE(BUILD_CONVERT_16_RGB_565, 1, [16bpp RGB 565])
AC_DEFINE(BUILD_CONVERT_16_RGB_555, 1, [16bpp RGB 555])
AC_DEFINE(BUILD_CONVERT_16_RGB_444, 1, [16bpp RGB 444])
AC_DEFINE(BUILD_CONVERT_16_RGB_454645, 1, [16bpp RGB 565 (444 ipaq)])
AC_DEFINE(BUILD_CONVERT_16_RGB_ROT0, 1, [16bpp 0 dgeree rotation])
AC_DEFINE(BUILD_CONVERT_16_RGB_ROT90, 1, [16bpp 90 dgeree rotation])
AC_DEFINE(BUILD_CONVERT_16_RGB_ROT180, 1, [16bpp 180 dgeree rotation])
AC_DEFINE(BUILD_CONVERT_16_RGB_ROT270, 1, [16bpp 270 dgeree rotation])
AC_DEFINE(BUILD_CONVERT_24_RGB_888, 1, [24bpp RGB 888])
AC_DEFINE(BUILD_CONVERT_24_BGR_888, 1, [24bpp BGR 888])
AC_DEFINE(BUILD_CONVERT_32_RGB_8888, 1, [32bpp RGB 8888])
AC_DEFINE(BUILD_CONVERT_32_RGBX_8888, 1, [32bpp RGBX 8888])
AC_DEFINE(BUILD_CONVERT_32_BGR_8888, 1, [32bpp BGR 8888])
AC_DEFINE(BUILD_CONVERT_32_BGRX_8888, 1, [32bpp BGRX 8888])
AC_DEFINE(BUILD_CONVERT_24_RGB_666, 1, [24bpp RGB 666 (666 ezx)])
AC_DEFINE(BUILD_CONVERT_32_RGB_666, 1, [32bpp RGB 666 (666 ezx)])
AC_DEFINE(BUILD_CONVERT_32_RGB_ROT0, 1, [32bpp 0 dgeree rotation])
AC_DEFINE(BUILD_CONVERT_32_RGB_ROT90, 1, [32bpp 90 dgeree rotation])
AC_DEFINE(BUILD_CONVERT_32_RGB_ROT180, 1, [32bpp 180 dgeree rotation])
AC_DEFINE(BUILD_CONVERT_32_RGB_ROT270, 1, [32bpp 270 dgeree rotation])
AC_DEFINE(BUILD_CONVERT_8_GRY_1, 1, [1bpp Mono])
AC_DEFINE(BUILD_CONVERT_8_GRY_16, 1, [4bpp Greyscale])
AC_DEFINE(BUILD_CONVERT_8_GRAYSCALE_64, 1, [32bpp Grayscale 64 palette])

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
      requirement_evas="valgrind ${requirement_evas}"
    ],
    [
      if test "x$want_valgrind" = "xyes" -a "x$use_strict" = "xyes" ; then
        AC_MSG_ERROR([Valgrind not found (strict dependencies checking)])
      fi
    ]
  )
fi

## Examples

install_examples="yes"
AC_ARG_ENABLE([install-examples],
   AC_HELP_STRING([--disable-install-examples],
                  [disable installing examples (compiled or just source).
                   @<:@default==enabled@:>@]),
   [
    if test "x${enableval}" = "xyes" ; then
       install_examples="yes"
    else
       install_examples="no"
    fi
   ],
   [install_examples="yes"])
AM_CONDITIONAL([INSTALL_EXAMPLES], [test "x${install_examples}" = "xyes"])

build_examples="no"
AC_ARG_ENABLE([build-examples],
   AC_HELP_STRING([--enable-build-examples],
                  [enable building examples (this requires extra denpendencies. if you don't have them installed yet, don't build with this option or it will fail. build again when you have it, overriding the previous installation). @<:@default==disabled@:>@]),
   [
    if test "x${enableval}" = "xyes" ; then
       build_examples="yes"
       # put in here the dependencies for Evas' examples. they are
       # meant to be 'real world' usage examples, thus one will be
       # using higher level libraries on these programs
       PKG_CHECK_MODULES([ECORE_EVAS], [ecore-evas ecore], [], [build_examples="no"])
       PKG_CHECK_MODULES([EDJE], [edje],
        [edje_cc=$($PKG_CONFIG --variable=prefix edje)/bin/edje_cc],
        [build_examples="no"])
       AC_SUBST(edje_cc)
    else
       build_examples="no"
    fi
   ],
   [build_examples="no"])
AM_CONDITIONAL([BUILD_EXAMPLES], [test "x${build_examples}" = "xyes"])

## Unit tests, coverage

EFL_CHECK_TESTS([enable_tests="yes"], [enable_tests="no"])

EFL_CHECK_COVERAGE([${enable_tests}], [enable_coverage="yes"], [enable_coverage="no"])
EVAS_CFLAGS="${EVAS_CFLAGS} ${EFL_COVERAGE_CFLAGS} ${EXOTIC_CFLAGS}"
EVAS_LIBS="${EVAS_LIBS} ${EFL_COVERAGE_LIBS} ${EXOTIC_LIBS}"
if test "x$enable_coverage" = "xyes" ; then
   EVAS_CFLAGS="${EVAS_CFLAGS} ${EFL_DEBUG_CFLAGS}"
fi

AC_SUBST(EVAS_CFLAGS)
AC_SUBST(EVAS_LIBS)

# General CFLAGS

EVAS_GENERAL_CFLAGS="${EINA_CFLAGS} ${EO_CFLAGS}"
EVAS_GENERAL_LIBS="${EINA_LIBS} ${EO_LIBS}"
AC_SUBST(EVAS_GENERAL_CFLAGS)
AC_SUBST(EVAS_GENERAL_LIBS)

#####################################################################
## Fill in flags

AC_SUBST(altivec_cflags)
AC_SUBST(pthread_cflags)
AC_SUBST(pthread_libs)
AC_SUBST(requirement_evas)

#####################################################################
## Output

AC_OUTPUT([
Makefile
evas-directfb.pc
evas-fb.pc
evas-opengl-x11.pc
evas-opengl-sdl.pc
evas-opengl-cocoa.pc
evas-software-buffer.pc
evas-software-x11.pc
evas-software-gdi.pc
evas-software-ddraw.pc
evas-direct3d.pc
evas-software-sdl.pc
evas-psl1ght.pc
evas-wayland-shm.pc
evas-wayland-egl.pc
evas.pc
doc/evas.dox
doc/Makefile
doc/Doxyfile
src/Makefile
src/bin/Makefile
src/bin/loaders/Makefile
src/bin/loaders/jpeg/Makefile
src/bin/loaders/png/Makefile
src/bin/loaders/eet/Makefile
src/bin/loaders/tiff/Makefile
src/bin/loaders/xpm/Makefile
src/bin/loaders/bmp/Makefile
src/bin/loaders/ico/Makefile
src/bin/loaders/tga/Makefile
src/bin/loaders/pmaps/Makefile
src/bin/loaders/wbmp/Makefile
src/bin/loaders/webp/Makefile
src/bin/loaders/psd/Makefile
src/lib/Makefile
src/lib/canvas/Makefile
src/lib/file/Makefile
src/lib/cache/Makefile
src/lib/cache2/Makefile
src/lib/cserve2/Makefile
src/lib/engines/Makefile
src/lib/engines/common/Makefile
src/lib/engines/common/evas_op_add/Makefile
src/lib/engines/common/evas_op_blend/Makefile
src/lib/engines/common/evas_op_copy/Makefile
src/lib/engines/common/evas_op_mask/Makefile
src/lib/engines/common/evas_op_mul/Makefile
src/lib/engines/common/evas_op_sub/Makefile
src/modules/Makefile
src/modules/engines/Makefile
src/modules/engines/software_generic/Makefile
src/modules/engines/software_gdi/Makefile
src/modules/engines/software_ddraw/Makefile
src/modules/engines/direct3d/Makefile
src/modules/engines/software_x11/Makefile
src/modules/engines/fb/Makefile
src/modules/engines/buffer/Makefile
src/modules/engines/directfb/Makefile
src/modules/engines/gl_common/Makefile
src/modules/engines/gl_x11/Makefile
src/modules/engines/gl_sdl/Makefile
src/modules/engines/gl_cocoa/Makefile
src/modules/engines/psl1ght/Makefile
src/modules/engines/wayland_shm/Makefile
src/modules/engines/wayland_egl/Makefile
src/modules/loaders/Makefile
src/modules/loaders/eet/Makefile
src/modules/loaders/gif/Makefile
src/modules/loaders/jpeg/Makefile
src/modules/loaders/png/Makefile
src/modules/loaders/tiff/Makefile
src/modules/loaders/xpm/Makefile
src/modules/loaders/bmp/Makefile
src/modules/loaders/ico/Makefile
src/modules/loaders/tga/Makefile
src/modules/loaders/svg/Makefile
src/modules/loaders/pmaps/Makefile
src/modules/loaders/wbmp/Makefile
src/modules/loaders/webp/Makefile
src/modules/loaders/psd/Makefile
src/modules/loaders/generic/Makefile
src/modules/savers/Makefile
src/modules/savers/eet/Makefile
src/modules/savers/jpeg/Makefile
src/modules/savers/png/Makefile
src/modules/savers/tiff/Makefile
src/static_deps/Makefile
src/static_deps/liblinebreak/Makefile
src/lib/include/Makefile
src/examples/Makefile
src/tests/Makefile
data/Makefile
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
echo "  Software X11...............: $have_evas_engine_software_x11 (Xlib: $have_evas_engine_software_xlib) (XCB: $have_evas_engine_software_xcb)"
echo $ECHO_N "  OpenGL X11.................: $have_evas_engine_gl_x11 (Xlib: $have_evas_engine_gl_xlib) (XCB: $have_evas_engine_gl_xcb) $ECHO_C"
if test "x$have_evas_engine_gl_x11" = "xyes"; then
  echo "(GLES: $gl_flavor_gles)"
else
  echo
fi
echo "  Software GDI...............: $have_evas_engine_software_gdi"
echo "  Software DirectDraw........: $have_evas_engine_software_ddraw"
echo "  Direct3d...................: $have_evas_engine_direct3d"
echo "  OpenGL SDL.................: $have_evas_engine_gl_sdl $ECHO_C"
if test "x$have_evas_engine_gl_sdl" = "xyes"; then
  echo "(GLES: $gl_flavor_gles)"
else
  echo
fi
echo "  OpenGL Cocoa...............: $have_evas_engine_gl_cocoa"
echo "  Software Framebuffer.......: $have_evas_engine_fb"
echo "  DirectFB...................: $have_evas_engine_directfb"
echo "  PSL1GHT....................: $have_evas_engine_psl1ght"
echo "  Wayland Shm................: $have_evas_engine_wayland_shm"
echo "  Wayland Egl................: $have_evas_engine_wayland_egl"
echo
echo "Image Loaders:"
echo "  JPEG region decode......: $have_jpeg_region"
echo "  WEBP....................: $have_evas_image_loader_webp"
echo "  GIF.....................: $have_evas_image_loader_gif"
echo "  TIFF....................: $have_evas_image_loader_tiff"
echo "  SVG.....................: $have_evas_image_loader_svg"
echo
echo "Font Searching Systems:"
echo "  Fontconfig..............: $have_fontconfig"
echo
echo "Font Rendering Helpers:"
echo "  Fribidi.................: $have_fribidi"
echo "  Harfbuzz................: $have_harfbuzz"
echo "  liblinebreak............: $have_linebreak"
# FIXME: add non freetype2 font engine support
# FIXME: make freetype2 optional
echo
echo "CPU Specific Extensions:"
echo "  MMX/SSE.................: $build_cpu_mmx"
echo "  SSE3....................: $build_cpu_sse3"
echo "  ALTIVEC.................: $build_cpu_altivec"
echo "  NEON....................: $build_cpu_neon"
echo
echo "Features:"
echo "  MAGIC_DEBUG.............: $want_evas_magic_debug"
echo "  Cache Server 2..........: $want_evas_cserve2"
echo "  Threaded Pipe Rendering.: $build_pipe_render"
echo
echo "  Pixman..................: $have_pixman"
echo "  Pixman Fonts............: $have_pixman_font"
echo "  Pixman Rects............: $have_pixman_rect"
echo "  Pixman Lines............: $have_pixman_line"
echo "  Pixman Polygons.........: $have_pixman_poly"
echo "  Pixman Images...........: $have_pixman_image"
echo "  Pixman Image ScaleSample: $have_pixman_image_scale_sample"
echo 
echo "  Tiled 32BPP rotate......: $have_tile_rotate"
echo
echo "Conversion Options:"
echo "  Smaller Dither Mask.....: $conv_small_dither"
echo "  Line Dither Mask........: $conv_line_dither"
echo "  No Dither Mask for 16bpp: $conv_no_dither"
echo
echo "Documentation.............: ${build_doc}"
echo "Examples..................: install:${install_examples} build:${build_examples}"
if test "x${enable_tests}" = "xyes" ; then
   echo "Tests.....................: ${enable_tests} (make check)"
else
   echo "Tests.....................: ${enable_tests}"
fi
if test "x${enable_coverage}" = "xyes" ; then
   echo "Coverage..................: ${enable_coverage} (make coverage)"
else
   echo "Coverage..................: ${enable_coverage}"
fi
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  CXXFLAGS.............: $CXXFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo
if test "x${have_static_module}" = "xyes" ; then
echo -e "\0033\01331;31mWarning\0033\01331;0m: You are trying to link statically one or more modules to Evas."
echo "         You must know what you are doing, or else you will have a lot of"
echo "         problems. And Kenny will be killed."
echo "         Think about that."
echo
fi

##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_define([v_maj], [1])
m4_define([v_min], [0])
m4_define([v_mic], [999])
m4_define([v_rev], m4_esyscmd([(svnversion "${SVN_REPO_PATH:-.}" | grep -v export || echo 0) | awk -F : '{printf("%s\n", $1);}' | tr -d ' :MSP\n']))
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
   mingw32ce* | cegcc*)
      MODULE_ARCH="$host_os-$host_cpu"
      ;;
   *)
      MODULE_ARCH="$host_os-$host_cpu-v_maj.v_min.v_mic"
      ;;
esac
AC_SUBST(MODULE_ARCH)
AC_DEFINE_UNQUOTED(MODULE_ARCH, "$MODULE_ARCH", "Module architecture")

want_fontconfig="auto"
want_fribidi="auto"
want_harfbuzz="auto"
want_pixman="no"
want_evas_cserve="yes"

want_evas_engine_buffer="yes"
want_evas_engine_software_xlib="no"
want_evas_engine_software_xcb="no"
want_evas_engine_software_gdi="no"
want_evas_engine_software_ddraw="no"
want_evas_engine_software_sdl="no"
want_evas_engine_software_8_x11="no"
want_evas_engine_software_16_x11="no"
want_evas_engine_software_16_ddraw="no"
want_evas_engine_software_16_wince="no"
want_evas_engine_gl_xlib="no"
want_evas_engine_gl_xcb="no"
want_evas_engine_gl_sdl="no"
want_evas_engine_direct3d="no"
want_evas_engine_fb="no"
want_evas_engine_directfb="no"

want_evas_image_loader_edb="yes"
want_evas_image_loader_eet="yes"
want_evas_image_loader_gif="yes"
want_evas_image_loader_jpeg="yes"
want_evas_image_loader_pmaps="yes"
want_evas_image_loader_png="yes"
want_evas_image_loader_svg="yes"
want_evas_image_loader_tiff="yes"
want_evas_image_loader_xpm="yes"
want_evas_image_loader_bmp="yes"
want_evas_image_loader_tga="yes"
want_evas_image_loader_wbmp="yes"
want_evas_image_loader_ico="yes"
want_evas_image_loader_psd="yes"
want_evas_image_loader_generic="yes"

want_evas_font_loader_eet="yes"

case "$host_os" in
   mingw32ce* | cegcc*)
      want_fontconfig="no"
      want_evas_engine_software_16_wince="yes"
      want_evas_image_loader_edb="no"
      want_evas_image_loader_gif="no"
      want_evas_image_loader_svg="no"
      want_evas_image_loader_tiff="no"
      ;;
   mingw*)
      want_evas_engine_software_gdi="yes"
      want_evas_engine_software_ddraw="yes"
      want_evas_engine_direct3d="yes"
      want_evas_engine_software_16_ddraw="yes"
      want_evas_image_loader_edb="no"
      want_evas_image_loader_svg="no"
      ;;
   darwin*)
      want_evas_engine_software_xlib="auto"
      ;;
   *)
      want_evas_engine_software_xlib="auto"
      want_evas_engine_gl_xlib="auto"
      want_evas_engine_software_xcb="no"
      want_evas_engine_gl_xcb="no"
      want_evas_engine_fb="auto"
### no - not ready/usable/complete
#      want_evas_engine_software_8_x11="auto"
#      want_evas_engine_software_16_x11="auto"
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
   mingw* | cegcc*)
      PKG_CHECK_MODULES([EVIL], [evil])
      AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if evil package is installed])
      requirement_evas="evil ${requirement_evas}"
   ;;
esac

## strong dependencies

# Eina
PKG_CHECK_MODULES([EINA], [eina >= 1.0.0])
CPPFLAGS_SAVE="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $EINA_CFLAGS"
AC_CHECK_SIZEOF(Eina_Unicode,, [#include <Eina.h>])
CPPFLAGS="$CPPFLAGS_SAVE"

# Freetype
PKG_CHECK_MODULES([FREETYPE], [freetype2 >= 9.3.0])

requirement_evas="freetype2 eina >= 1.0.0 ${requirement_evas}"

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


if test "x${want_fontconfig}" = "xyes" -o "x${want_fontconfig}" = "xauto" ; then
   PKG_CHECK_MODULES([FONTCONFIG],
      [fontconfig],
      [
       have_fontconfig="yes"
       AC_DEFINE(HAVE_FONTCONFIG, 1, [have fontconfig searching capabilities])
       requirement_evas="fontconfig ${requirement_evas}"
      ],
      [
       if test "x${want_fontconfig}" = "xyes" -a "x${use_strict}" = "xyes" ; then
          AC_MSG_ERROR([Fontconfig not found (strict dependencies checking)])
       fi
      ])
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

if test "x${want_fribidi}" = "xyes" -o "x${want_fribidi}" = "xauto" ; then
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
       if test "x$want_fribidi" = "xyes" -a "x$use_strict" = "xyes" ; then
          AC_MSG_ERROR([Fribidi not found (strict dependencies checking)])
       fi
      ])
fi

if test "x${have_fribidi}" = "xno"; then
  HAS_BIDI=0
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
      [harfbuzz >= 0.6.0],
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
   AC_HELP_STRING([--disable-pixman],
      [disable pixman for software rendering. @<:@default=enabled@:>@]),
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

### Checks for header files
AC_HEADER_STDC
AC_CHECK_HEADERS([unistd.h stdint.h sys/param.h netinet/in.h])

EFL_CHECK_PATH_MAX


### Checks for types


### Checks for structures


### Checks for compiler characteristics
AC_C_CONST
AC_C_BIGENDIAN
AC_C___ATTRIBUTE__
AM_PROG_CC_STDC

WIN32_CPPFLAGS=""
case "$host_os" in
   mingw32ce* | cegcc*)
      WIN32_CPPFLAGS="-D_WIN32_WCE=0x0420"
      ;;
   mingw*)
      WIN32_CPPFLAGS="-D_WIN32_WINNT=0x0500"
      ;;
esac
AC_SUBST(WIN32_CPPFLAGS)

WIN32_CFLAGS=""
case "${host_os}" in
   cegcc*)
      WIN32_CFLAGS="-mwin32"
   ;;
esac
AC_SUBST(WIN32_CFLAGS)


### Checks for linker characteristics
lt_enable_auto_import=""
case "${host_os}" in
   mingw* | cegcc*)
      AC_DEFINE(EFL_EVAS_BUILD, 1, [Define to mention that evas is built])
      lt_enable_auto_import="-Wl,--enable-auto-import"
   ;;
esac
AC_SUBST(lt_enable_auto_import)


### Checks for library functions

AC_ISC_POSIX

# alloca
AC_FUNC_ALLOCA

# fnmatch
EFL_CHECK_FNMATCH([], [AC_MSG_ERROR([Cannot find fnmatch()])])

# dlopen
dlopen_libs=""
case "$host_os" in
  mingw32ce* | cegcc*)
# managed by evil
    AC_DEFINE(HAVE_DLADDR)
  ;;
  mingw*)
# nothing on mingw platform
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

SHM_OPEN_LINK=""
AC_MSG_CHECKING([whether shm_open() is present])
LIBS_save=${LIBS}
LIBS="${LIBS} -lrt"
AC_LINK_IFELSE(
   [AC_LANG_PROGRAM(
      [[
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
      ]],
      [[
int fd;
fd = shm_open("/", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
shm_unlink("/");
      ]])],
   [
    have_shm_open="yes"
    AC_DEFINE(HAVE_SHM_OPEN, 1, [Have shm_open() call])
    SHM_OPEN_LINK="-lrt"
   ],
   [have_shm_open="no"])
LIBS=${LIBS_save}
AC_MSG_RESULT([${have_shm_open}])
AC_SUBST(SHM_OPEN_LINK)

# (shm_open (for cache server)
AC_ARG_ENABLE([evas-cserve],
   AC_HELP_STRING([--disable-evas-cserve],
      [disable shared cache server support. @<:@default=enabled@:>@]),
   [
    if test "x${enableval}" = "xyes" ; then
       want_evas_cserve="yes"
    else
       want_evas_cserve="no"
    fi
   ])
AC_MSG_CHECKING(whether to build shared cache server and support)
AC_MSG_RESULT(${want_evas_cserve})

if test "x${want_evas_cserve}" = "xyes" ; then
   AC_MSG_CHECKING([whether shm_open() is present])
   LIBS_save=${LIBS}
   LIBS="${LIBS} -lrt"
   AC_LINK_IFELSE(
      [AC_LANG_PROGRAM(
         [[
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
         ]],
         [[
int fd;
fd = shm_open("/", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
shm_unlink("/");
         ]])],
      [want_evas_cserve="yes"],
      [want_evas_cserve="no"])
   LIBS=${LIBS_save}
   AC_MSG_RESULT([${want_evas_cserve}])
fi

if test "x${want_evas_cserve}" = "xyes" ; then
  AC_DEFINE(EVAS_CSERVE, 1, [Shared caceh server.])
fi

AM_CONDITIONAL([EVAS_CSERVE], [test "x${want_evas_cserve}" = "xyes"])



#######################################
## GL force flavor gles
gl_flavor_gles="no"
AC_MSG_CHECKING(whether to build GLES flavor of GL)
AC_ARG_ENABLE(gl-flavor-gles,
  AC_HELP_STRING([--enable-gl-flavor-gles], [enable gles falvor of gl instead of standard]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_MSG_RESULT(yes)
        gl_flavor_gles="yes"
      else
        AC_MSG_RESULT(no)
        gl_flavor_gles="no"
      fi
  ], [
      AC_MSG_RESULT(no)
      gl_flavor_gles="no"
  ]
)

#######################################
## GLES variety sgx
gles_variety_sgx="no"
AC_MSG_CHECKING(whether to build GLES variety for SGX)
AC_ARG_ENABLE(gles-variety-sgx,
  AC_HELP_STRING([--enable-gles-variety-sgx], [GLES variety SGX 3D unit]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(GLES_VARIETY_SGX, 1, [Imagination SGX GLES2 support])
        gles_variety_sgx="yes"
      else
        AC_MSG_RESULT(no)
        gles_variety_sgx="no"
      fi
  ], [
      AC_MSG_RESULT(no)
      gles_variety_sgx="no"
  ]
)

#######################################
## GLES variety s3c6410
gles_variety_s3c6410="no"
AC_MSG_CHECKING(whether to build GLES variety for s3c6410)
AC_ARG_ENABLE(gles-variety-s3c6410,
  AC_HELP_STRING([--enable-gles-variety-s3c6410], [GLES variety s3c6410 3D unit]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(GLES_VARIETY_S3C6410, 1, [Samsung S3c6410 GLES2 support])
        gles_variety_s3c6410="yes"
      else
        AC_MSG_RESULT(no)
        gles_variety_s3c6410="no"
      fi
  ], [
      AC_MSG_RESULT(no)
      gles_variety_s3c6410="no"
  ]
)

if test "x$gl_flavor_gles" = "xyes"; then
  eng=""
  if test "x$gles_variety_sgx" = "xyes"; then eng="x"$eng; fi
  if test "x$gles_variety_s3c6410" = "xyes"; then eng="x"$eng; fi
  if test "x$eng" = "xx"; then
    eng=""
  else
     AC_MSG_ERROR(
       [Please enable just 1 OpenGL-ES flavor. Choices are:
          --enable-gles-variety-sgx (Most GL-ES2 GPUs with shader compiler)
          --enable-gles-variety-s3c6410 (s3c6410 with offline shader compiler)
        You will also need to enable the OpenGL engine for X11 with:
          --enable-gl-xlib or --enable-gl-xcb
       ])
  fi
fi

#####################################################################
## Engines

EVAS_CHECK_ENGINE([buffer], [${want_evas_engine_buffer}], [yes], [Buffer])

EVAS_CHECK_ENGINE([software-gdi], [${want_evas_engine_software_gdi}], [no], [Software GDI])

EVAS_CHECK_ENGINE([software-ddraw], [${want_evas_engine_software_ddraw}], [no], [Software DirectDraw])

EVAS_CHECK_ENGINE([direct3d], [${want_evas_engine_direct3d}], [no], [Direct3D])

EVAS_CHECK_ENGINE([software-sdl], [${want_evas_engine_software_sdl}], [no], [Software SDL])

EVAS_CHECK_ENGINE([gl-sdl], [${want_evas_engine_gl_sdl}], [no], [OpenGL SDL])

EVAS_CHECK_ENGINE([fb], [${want_evas_engine_fb}], [no], [Framebuffer])

EVAS_CHECK_ENGINE([directfb], [${want_evas_engine_directfb}], [no], [DirectFB])

EVAS_CHECK_ENGINE([software-8-x11], [${want_evas_engine_software_8_x11}], [no], [Software X11 8 bits grayscale])

EVAS_CHECK_ENGINE([software-16-x11], [${want_evas_engine_software_16_x11}], [no], [Software X11 16 bits])

EVAS_CHECK_ENGINE([software-16-ddraw], [${want_evas_engine_software_16_ddraw}], [no], [Software DirectDraw 16 bits])

EVAS_CHECK_ENGINE([software-16-wince], [${want_evas_engine_software_16_wince}], [no], [Software Windows CE 16 bits])

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
want_xcb=
AC_MSG_CHECKING(whether to build Software XCB Engine)
AC_ARG_ENABLE(software-xcb,
  AC_HELP_STRING([--enable-software-xcb], [enable Software XCB Engine. @<:@default==disabled@:>@]), [want_xcb=$enableval]
)
if test "x$want_xcb" = "xyes" ; then
  EVAS_CHECK_ENGINE([software-xcb], [${want_evas_engine_software_xcb}], [no], [Software XCB])
  if test "x$have_evas_engine_software_xcb" != "xno" ; then
    build_software_xcb="yes"
    have_evas_engine_software_x11="yes"
    AC_DEFINE_UNQUOTED(BUILD_ENGINE_SOFTWARE_X11, [1], [Build software X11 engine])
    AC_DEFINE_UNQUOTED(BUILD_ENGINE_SOFTWARE_XCB, [1], [Build software XCB backend])
    if test "x$have_evas_engine_software_xcb" = "xstatic" ; then
      AC_DEFINE_UNQUOTED(EVAS_STATIC_BUILD_SOFTWARE_XCB, [1], [Build software X11 engine as part of libevas])
    fi
  fi
  AC_SUBST(have_evas_engine_software_xcb)
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
    if test "x$have_evas_engine_software_xlib" = "xstatic" ; then
      AC_DEFINE_UNQUOTED(EVAS_STATIC_BUILD_SOFTWARE_XLIB, [1], [Build software X11 engine as part of libevas])
    fi
  fi
else
  have_evas_engine_software_xlib="no"
fi
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
build_gl_xcb=

## XCB Checks ##
want_gl_xcb=
AC_MSG_CHECKING(whether to build OpenGL XCB Engine)
AC_ARG_ENABLE(gl-xcb,
  AC_HELP_STRING([--enable-gl-xcb], [enable OpenGL XCB Engine. @<:@default==disabled@:>@]), [want_gl_xcb=$enableval]
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
      AC_DEFINE_UNQUOTED(EVAS_STATIC_BUILD_GL_XLIB, [1], [Build OpenGL X11 engine as part of libevas])
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
   || test "x$have_evas_engine_gl_sdl" = "xyes"; then
   have_evas_engine_gl_common="yes"
fi
if test "x$have_evas_engine_gl_xlib" = "xstatic" \
   || test "x$have_evas_engine_gl_xcb" = "xstatic" \
   || test "x$have_evas_engine_gl_sdl" = "xstatic"; then
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

# if software 16 x11 is enabled - build software_16 (the generic 16bit
# engine). later enable it fb_16 or other "16" bit engines are enabled.
have_evas_engine_software_16="no"
if test "x$have_evas_engine_software_16_x11" = "xyes" -o "x$have_evas_engine_software_16_x11" = "xstatic"; then
   have_evas_engine_software_16="yes"
fi
if test "x$have_evas_engine_software_sdl" = "xyes" -o "x$have_evas_engine_software_sdl" = "xstatic"; then
   have_evas_engine_software_16="yes"
fi
if test "x$have_evas_engine_software_16_ddraw" = "xyes" -o "x$have_evas_engine_software_16_ddraw" = "xstatic"; then
   have_evas_engine_software_16="yes"
fi
if test "x$have_evas_engine_software_16_wince" = "xyes" -o "x$have_evas_engine_software_16_wince" = "xstatic"; then
   have_evas_engine_software_16="yes"
fi
AM_CONDITIONAL(BUILD_ENGINE_SOFTWARE_16, test "x$have_evas_engine_software_16" = "xyes")

# if software 16 need to be build as part of libevas.so
have_static_software_16="no"
AC_ARG_ENABLE([static-software-16],
   [AC_HELP_STRING([--enable-static-software-16], [Build software 16 engine as part of libevas])],
   [have_static_software_16=${enableval}]
)
AC_MSG_CHECKING([Whether to build software 16 engine as part of libevas])
AC_MSG_RESULT([${have_static_software_16}])

AM_CONDITIONAL(EVAS_STATIC_BUILD_SOFTWARE_16, test "x${have_static_software_16}" = "xyes")
if test "x${have_static_software_16}" = "xyes"; then
   AC_DEFINE(EVAS_STATIC_BUILD_SOFTWARE_16, [1], [Build software 16 engine as part of libevas])
fi

# if software 8 x11 is enabled - build software_8 (the generic 8bit
# engine).
have_evas_engine_software_8="no"
if test "x$have_evas_engine_software_8_x11" = "xyes" -o "x$have_evas_engine_software_8_x11" = "xstatic"; then
   have_evas_engine_software_8="yes"
fi
AM_CONDITIONAL(BUILD_ENGINE_SOFTWARE_8, test "x$have_evas_engine_software_8" = "xyes")

# if software 8 need to be build as part of libevas.so
have_static_software_8="no"
AC_ARG_ENABLE([static-software-8],
   [AC_HELP_STRING([--enable-static-software-8], [Build software 8 engine as part of libevas])],
   [have_static_software_8=${enableval}]
)
AC_MSG_CHECKING([Whether to build software 8 engine as part of libevas])
AC_MSG_RESULT([${have_static_software_8}])

AM_CONDITIONAL(EVAS_STATIC_BUILD_SOFTWARE_8, test "x${have_static_software_8}" = "xyes")
if test "x${have_static_software_8}" = "xyes"; then
   AC_DEFINE(EVAS_STATIC_BUILD_SOFTWARE_8, [1], [Build software 8 engine as part of libevas])
fi

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
         AC_DEFINE(EVAS_BUILD_SAVER_JPEG, [1], [Build JPEG saver])
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

EVAS_CHECK_IMAGE_LOADER([BMP], [${want_evas_image_loader_bmp}])

EVAS_CHECK_IMAGE_LOADER([TGA], [${want_evas_image_loader_tga}])

EVAS_CHECK_IMAGE_LOADER([WBMP], [${want_evas_image_loader_wbmp}])

EVAS_CHECK_IMAGE_LOADER([ICO], [${want_evas_image_loader_ico}])

EVAS_CHECK_IMAGE_LOADER([PSD], [${want_evas_image_loader_psd}])

EVAS_CHECK_IMAGE_LOADER([GENERIC], [${want_evas_image_loader_generic}])

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

#######################################
## Pthread
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
## Pipe Renderer
build_pipe_render="no"

AC_MSG_CHECKING(whether to build Threaded Pipe Rendering support)
AC_ARG_ENABLE(pipe-render,
  AC_HELP_STRING([--enable-pipe-render], [enable threaded pipe rendering support]),
  [ build_pipe_render=$enableval ]
)
AC_MSG_RESULT($build_pipe_render)

AC_MSG_CHECKING(whether we can build Threaded Pipe Rendering support)
if test \( "x$build_pipe_render" = "xyes" -o "x$build_pipe_render" = "xauto" \); then
  AC_MSG_RESULT(yes)
  AC_DEFINE(BUILD_PIPE_RENDER, 1, [Build pipe render support])
  build_pipe_render="yes"
  need_pthreads="yes"
else
  AC_MSG_RESULT(no)
  build_pipe_render="no"
fi

#######################################
## Async Renderer
build_async_render="no"

AC_MSG_CHECKING(whether to build Asynchronously Threaded Pipe Rendering support)
AC_ARG_ENABLE(async-render,
 AC_HELP_STRING([--enable-async-render], [enable asynchronously threaded pipe rendering support]),
 [ build_async_render=$enableval ]
)
AC_MSG_RESULT($build_async_render)

AC_MSG_CHECKING(whether we can build Asynchronously Threaded Pipe Rendering support)
if test \( "x$build_async_render" = "xyes" \); then
  AC_MSG_RESULT(yes)
  AC_DEFINE(EVAS_FRAME_QUEUING, 1, [Build async render support])
  build_async_render="yes"
  AC_DEFINE(BUILD_PIPE_RENDER, 1, [Build pipe render support])
  build_pipe_render="yes"
  need_pthreads="yes"

  PKG_CHECK_MODULES([XEXT],
    [xext < 1.1.1],
    [ build_avoid_libXext_bug=yes ],
    [ build_avoid_libXext_bug=no ]
  )
  if test \( "x$build_avoid_libXext_bug" = "xyes" \); then
    AC_DEFINE(LIBXEXT_VERSION_LOW, 1, [To avoid bug on old libXext version])
  fi
else
  AC_MSG_RESULT(no)
  build_async_render="no"
fi

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
if test "x$build_async_events" = "xyes" || test "x$build_async_events" = "xauto" ; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(BUILD_ASYNC_EVENTS, 1, [Build async events support])
  build_async_events="yes"
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
  amd64)
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
  amd64)
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
## NEON
build_cpu_neon="no"
case $host_cpu in
  arm*)
    build_cpu_neon="yes"
    ;;
esac
AC_MSG_CHECKING(whether to build neon code)
AC_ARG_ENABLE(cpu-neon,
  AC_HELP_STRING([--enable-cpu-neon], [build neon code, the code will be
	  checked at run time to see if the CPU supports it]),
  [
     if test "x$enableval" = "xyes" ; then
       AC_TRY_COMPILE([],
        [asm volatile (".fpu neon\nvqadd.u8 d0, d1, d0\n")],
        [
          AC_MSG_RESULT(yes)
          AC_DEFINE(BUILD_NEON, 1, [Build NEON Code])
          build_cpu_neon="yes"
        ],[
          AC_MSG_RESULT(no)
          build_cpu_neon="no"
        ])
      else
        AC_MSG_RESULT(no)
        build_cpu_neon="no"
      fi
  ],
  [
    if test "x$build_cpu_neon" = "xyes" ; then
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
    fi
  ]
)
     
#######################################
## C
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
  AC_DEFINE(EVAS_MAGIC_DEBUG, 1, [complain when people pass in wrong object types etc.])
fi


#######################################
## Word Caching
want_word_cache="no"
AC_MSG_CHECKING(whether to enable caching of rendered words)
AC_ARG_ENABLE(word-cache,
  AC_HELP_STRING(
    [--enable-word-cache],
    [Enable experimental word caching to speed up rendering [[default=disabled]]]
  ),
  [ want_word_cache="$enableval" ]
)
AC_MSG_RESULT($want_word_cache)

AM_CONDITIONAL(WORD_CACHE, test "x$want_word_cache" = "xyes")
if test "x$want_word_cache" = "xyes"; then
  AC_DEFINE(WORD_CACHE, 1, [Experimental word caching to speed up text rendering.])
fi

#######################################
## Metric Caching
want_metric_cache="no"
AC_MSG_CHECKING(whether to enable caching of rendered metrics)
AC_ARG_ENABLE(metric-cache,
  AC_HELP_STRING(
    [--enable-metric-cache],
    [Enable experimental metric caching to speed up rendering [[default=disabled]]]
  ),
  [ want_metric_cache="$enableval" ]
)
AC_MSG_RESULT($want_metric_cache)

AM_CONDITIONAL(METRIC_CACHE, test "x$want_metric_cache" = "xyes")
if test "x$want_metric_cache" = "xyes"; then
  AC_DEFINE(METRIC_CACHE, 1, [Experimental metric caching to speed up text rendering.])
fi



#####################################################################
## ARGB engine options

#######################################
## Nearest sampling scaler
EVAS_CHECK_SCALER([scale-sample], [sampling scaler], [scaler_sample="yes"], [scaler_sample="no"])
## Smooth super and sub sampling scaler
EVAS_CHECK_SCALER([scale-smooth], [smooth scaler], [scaler_smooth="yes"], [scaler_smooth="no"])

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
EVAS_CHECK_DITHER([small-dither-mask], [small dither mask], [conv_small_dither="yes"], [conv_small_dither="no"])
## Alternate Line dither mask instead of big one (lower quality - but fastest)
EVAS_CHECK_DITHER([line-dither-mask], [line dither mask], [conv_line_dither="yes"], [conv_line_dither="no"])
## No dither mask at all for 16bpp
EVAS_CHECK_DITHER([no-dither-mask], [conversion to 16bpp without dither mask], [conv_no_dither="yes"], [conv_no_dither="no"])

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
## Convert to 24bpp RGB 666 (666 ezx)
conv_24_rgb_ezx="yes"
AC_MSG_CHECKING(whether to build 24bpp 666 (666 ezx) converter code)
AC_ARG_ENABLE(convert-24-rgb-ezx,
  AC_HELP_STRING([--disable-convert-24-rgb-ezx], [disable 24bpp 666 (666 ezx) converter code]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_DEFINE(BUILD_CONVERT_24_RGB_666, 1, [24bpp 666 (666 ezx) Converter Support])
        conv_24_rgb_ezx="yes"
      else
        conv_24_rgb_ezx="no"
      fi
  ], [
      if test "x$conv_24_rgb_ezx" = "xyes" ; then
        AC_DEFINE(BUILD_CONVERT_24_RGB_666, 1, [24bpp 666 (666 ezx) Converter Support])
      fi
  ]
)
AC_MSG_RESULT($conv_24_rgb_ezx)

#######################################
## Convert to 32bpp RGB 666 (666 ezx)
conv_32_rgb_ezx="yes"
AC_MSG_CHECKING(whether to build 32bpp 666 (666 ezx) converter code)
AC_ARG_ENABLE(convert-32-rgb-ezx,
  AC_HELP_STRING([--disable-convert-32-rgb-ezx], [disable 32bpp 666 (666 ezx) converter code]),
  [
      if test "x$enableval" = "xyes" ; then
        AC_DEFINE(BUILD_CONVERT_32_RGB_666, 1, [32bpp 666 (666 ezx) Converter Support])
        conv_32_rgb_ezx="yes"
      else
        conv_32_rgb_ezx="no"
      fi
  ], [
      if test "x$conv_32_rgb_ezx" = "xyes" ; then
        AC_DEFINE(BUILD_CONVERT_32_RGB_666, 1, [32bpp 666 (666 ezx) Converter Support])
      fi
  ]
)
AC_MSG_RESULT($conv_32_rgb_ezx)

#######################################
## Convert to 32bpp RGB with rotation of 0
EVAS_CONVERT_ROT(32, RGB, 0)
## Convert to 32bpp RGB with rotation of 180
EVAS_CONVERT_ROT(32, RGB, 180)
## Convert to 32bpp RGB with rotation of 270
EVAS_CONVERT_ROT(32, RGB, 270)
## Convert to 32bpp RGB with rotation of 90
EVAS_CONVERT_ROT(32, RGB, 90)

#######################################
## Convert to 8bpp grayscale with 256 value, no palette
EVAS_CONVERT_COLOR(8, GRY, 1)
## Convert to 8bpp grayscale with 16 value, no palette
EVAS_CONVERT_COLOR(8, GRY, 16)

#######################################
## Convert to 8bpp grayscale, 64-palette
conv_8_grayscale_64="yes"
AC_MSG_CHECKING(whether to build 8bpp grayscale 64-palette converter code)
AC_ARG_ENABLE(convert-8-grayscale-64,
  AC_HELP_STRING([--disable-convert-8-grayscale-64], [disable 8bpp grayscale 64-palette converter code]),
  [
     if test "x$enableval" = "xyes"; then
       AC_DEFINE(BUILD_CONVERT_8_GRAYSCALE_64, 1, [8bpp Grayscale 64-palette Converter Support])
       conv_8_grayscale_64="yes"
     else
       conv_8_grayscale_64="no"
     fi
  ], [
     if test "x$conv_8_grayscale_64" = "xyes"; then
       AC_DEFINE(BUILD_CONVERT_8_GRAYSCALE_64, 1, [32bpp Grayscale 64-palette Converter Support])
     fi
  ]
)
AC_MSG_RESULT($conv_8_grayscale_64)

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
        [edje_cc=$(pkg-config --variable=prefix edje)/bin/edje_cc],
        [build_examples="no"])
       AC_SUBST(edje_cc)
    else
       build_examples="no"
    fi
   ],
   [build_examples="no"])
AM_CONDITIONAL([BUILD_EXAMPLES], [test "x${build_examples}" = "xyes"])


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
evas-software-buffer.pc
evas-software-x11.pc
evas-software-8-x11.pc
evas-software-16-x11.pc
evas-software-gdi.pc
evas-software-ddraw.pc
evas-software-16-ddraw.pc
evas-direct3d.pc
evas-software-16-wince.pc
evas-software-sdl.pc
evas.pc
doc/evas.dox
doc/Makefile
doc/Doxyfile
src/Makefile
src/bin/Makefile
src/lib/Makefile
src/lib/canvas/Makefile
src/lib/file/Makefile
src/lib/cache/Makefile
src/lib/cserve/Makefile
src/lib/engines/Makefile
src/lib/engines/common/Makefile
src/lib/engines/common/evas_op_add/Makefile
src/lib/engines/common/evas_op_blend/Makefile
src/lib/engines/common/evas_op_copy/Makefile
src/lib/engines/common/evas_op_mask/Makefile
src/lib/engines/common/evas_op_mul/Makefile
src/lib/engines/common/evas_op_sub/Makefile
src/lib/engines/common_8/Makefile
src/lib/engines/common_16/Makefile
src/modules/Makefile
src/modules/engines/Makefile
src/modules/engines/software_generic/Makefile
src/modules/engines/software_gdi/Makefile
src/modules/engines/software_ddraw/Makefile
src/modules/engines/direct3d/Makefile
src/modules/engines/software_16_wince/Makefile
src/modules/engines/software_x11/Makefile
src/modules/engines/fb/Makefile
src/modules/engines/buffer/Makefile
src/modules/engines/directfb/Makefile
src/modules/engines/gl_common/Makefile
src/modules/engines/gl_x11/Makefile
src/modules/engines/gl_sdl/Makefile
src/modules/engines/software_sdl/Makefile
src/modules/engines/software_8/Makefile
src/modules/engines/software_8_x11/Makefile
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
src/modules/loaders/bmp/Makefile
src/modules/loaders/ico/Makefile
src/modules/loaders/tga/Makefile
src/modules/loaders/svg/Makefile
src/modules/loaders/pmaps/Makefile
src/modules/loaders/wbmp/Makefile
src/modules/loaders/psd/Makefile
src/modules/loaders/generic/Makefile
src/modules/savers/Makefile
src/modules/savers/edb/Makefile
src/modules/savers/eet/Makefile
src/modules/savers/jpeg/Makefile
src/modules/savers/png/Makefile
src/modules/savers/tiff/Makefile
src/static_deps/Makefile
src/static_deps/liblinebreak/Makefile
src/lib/include/Makefile
src/examples/Makefile
README
evas.spec
])

#####################################################################
## Sanity Checks

if test "x$build_pthreads" = "xno" ; then
  if test "x$build_async_render" = "xyes" ; then
    echo "ERROR: PThreads off, but async rendering on. Async rendering"
    echo "  needs thread support."
    exit 1
  fi
  if test "x$build_pipe_render" = "xyes" ; then
    echo "ERROR: PThreads off, but pipe rendering on. Pipe rendering"
    echo "  needs thread support."
    exit 1
  fi
fi

if test "x${has_pthreads}" = "xno" ; then
  if test "x$build_async_preload" = "xyes" ; then
    echo "ERROR: PThreads off, but Async Preload on. Async Preload"
    echo "  needs thread support."
    exit 1
  fi
fi

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
  echo "(GLES: $gl_flavor_gles) (SGX: $gles_variety_sgx) (s3c6410: $gles_variety_s3c6410)"
else
  echo
fi
echo "  Software GDI...............: $have_evas_engine_software_gdi"
echo "  Software DirectDraw........: $have_evas_engine_software_ddraw"
echo "  Direct3d...................: $have_evas_engine_direct3d"
echo "  Software SDL...............: $have_evas_engine_software_sdl (primitive: $sdl_primitive)"
echo $ECHO_N "  OpenGL SDL.................: $have_evas_engine_gl_sdl $ECHO_C"
if test "x$have_evas_engine_gl_sdl" = "xyes"; then
  echo "(GLES: $gl_flavor_gles) (SGX: $gles_variety_sgx) (s3c6410: $gles_variety_s3c6410)"
else
  echo
fi
echo "  Software Framebuffer.......: $have_evas_engine_fb"
echo "  DirectFB...................: $have_evas_engine_directfb"
echo "  Software 8bit grayscale....: $have_evas_engine_software_8"
# FIXME: kill software 16bit
echo "  Software 16bit ............: $have_evas_engine_software_16"
echo "  Software 16bit X11.........: $have_evas_engine_software_16_x11"
echo "  Software 16bit Directdraw..: $have_evas_engine_software_16_ddraw"
echo "  Software 16bit WinCE.......: $have_evas_engine_software_16_wince"
echo "  Software 16bit SDL.........: $have_evas_engine_software_sdl (primitive: $sdl_primitive)"
echo
echo "Image Loaders:"
echo "  BMP.....................: $have_evas_image_loader_bmp"
echo "  EDB.....................: $have_evas_image_loader_edb"
echo "  EET.....................: $have_evas_image_loader_eet"
echo "  GENERIC.................: $have_evas_image_loader_generic"
echo "  GIF.....................: $have_evas_image_loader_gif"
echo "  ICO.....................: $have_evas_image_loader_ico"
echo "  JPEG....................: $have_evas_image_loader_jpeg (region: $have_jpeg_region)"
echo "  PMAPS...................: $have_evas_image_loader_pmaps"
echo "  PNG.....................: $have_evas_image_loader_png"
echo "  PSD.....................: $have_evas_image_loader_psd"
echo "  SVG.....................: $have_evas_image_loader_svg"
echo "  TGA.....................: $have_evas_image_loader_tga"
echo "  TIFF....................: $have_evas_image_loader_tiff"
echo "  WBMP....................: $have_evas_image_loader_wbmp"
echo "  XPM.....................: $have_evas_image_loader_xpm"
echo
echo "Font Sourcing Systems:"
echo "  EET.....................: $have_evas_font_loader_eet"
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
echo "  Fallback C Code.........: $build_cpu_c"
echo "  MMX.....................: $build_cpu_mmx"
echo "  SSE.....................: $build_cpu_sse"
echo "  ALTIVEC.................: $build_cpu_altivec"
echo "  NEON....................: $build_cpu_neon"
echo "  Thread Support..........: $build_pthreads"
echo
echo "Features:"
echo "  MAGIC_DEBUG.............: $want_evas_magic_debug"
echo "  Cache Server............: $want_evas_cserve"
echo
dnl
dnl ... DISABLED! some testing has shown that this seems to have some
dnl nasty bugs on both x86 and arm (tegra2 dual core tested), so just
dnl disabling this for now until it can be fixed
dnl 
echo "  Threaded Pipe Rendering.: $build_pipe_render"
echo "  Async Pipe Rendering....: $build_async_render"
echo "  Async Events............: $build_async_events"
echo "  Async Image Preload.....: $build_async_preload"
echo
echo "  Word Cache..............: $want_word_cache"
echo "  Metric Cache............: $want_metric_cache"
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
echo "  8bpp Grayscale (256)....: $conv_8_gry_1"
echo "  8bpp Grayscale (16).....: $conv_8_gry_16"
echo "  8bpp Grayscale 64-pal...: $conv_8_grayscale_64"
# FIXME: add grayscale and B&W support to standard x converters
echo "  16bpp RGB 565...........: $conv_16_rgb_565"
echo "  16bpp BGR 565...........: $conv_16_bgr_565"
echo "  16bpp RGB 555...........: $conv_16_rgb_555"
echo "  16bpp RGB 444...........: $conv_16_rgb_444"
echo "  16bpp RGB 565 (444 ipaq): $conv_16_rgb_ipq"
# FIXME: add 555 (444 ipaq) support
# FIXME: add 30bpp support
# FIXME: add palletted support (ugh!)
# FIXME: add 8bpp and below rotation
echo "  16bpp Rotation 0........: $conv_16_rgb_rot_0"
echo "  16bpp Rotation 90.......: $conv_16_rgb_rot_90"
echo "  16bpp Rotation 180......: $conv_16_rgb_rot_180"
echo "  16bpp Rotation 270......: $conv_16_rgb_rot_270"
echo "  24bpp RGB 888...........: $conv_24_rgb_888"
echo "  24bpp BGR 888...........: $conv_24_bgr_888"
echo "  24bpp RGB 666 (666 ezx).: $conv_24_rgb_ezx"
# FIXME: add 24bpp rotation
echo "  32bpp RGB 8888..........: $conv_32_rgb_8888"
echo "  32bpp RGBX 8888.........: $conv_32_rgbx_8888"
echo "  32bpp BGR 8888..........: $conv_32_bgr_8888"
echo "  32bpp BGRX 8888.........: $conv_32_bgrx_8888"
echo "  32bpp RGB 666 (666 ezx).: $conv_32_rgb_ezx"
echo "  32bpp Rotation 0........: $conv_32_rgb_rot_0"
echo "  32bpp Rotation 90.......: $conv_32_rgb_rot_90"
echo "  32bpp Rotation 180......: $conv_32_rgb_rot_180"
echo "  32bpp Rotation 270......: $conv_32_rgb_rot_270"
echo
echo "Documentation.............: ${build_doc}"
echo "Examples..................: install:${install_examples} build:${build_examples}"
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

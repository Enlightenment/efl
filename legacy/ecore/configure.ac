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

AC_INIT([ecore], [v_ver], [enlightenment-devel@lists.sourceforge.net])
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
AC_DEFINE_UNQUOTED(VERS_MAJ, [v_maj], [Major version])
AC_DEFINE_UNQUOTED(VERS_MIN, [v_min], [Minor version])
AC_DEFINE_UNQUOTED(VERS_MIC, [v_mic], [Micro version])
AC_DEFINE_UNQUOTED(VERS_REV, [v_rev], [Revison])
version_info="lt_cur:lt_rev:lt_age"
release_info="v_rel"
AC_SUBST(version_info)
AC_SUBST(release_info)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
VMAJ=v_maj
AC_SUBST(VMAJ)

m4_ifdef([AM_GNU_GETTEXT_VERSION], [
AM_GNU_GETTEXT_VERSION([0.14])
])

m4_ifdef([AM_GNU_GETTEXT], [
AM_GNU_GETTEXT([external])
po_makefile_in=po/Makefile.in
AM_CONDITIONAL([HAVE_PO], [true])
],[
AM_CONDITIONAL([HAVE_PO], [false])
])
AC_SUBST(LTLIBINTL)

with_max_log_level="<unset>"
AC_ARG_WITH(internal-maximum-log-level,
   [AC_HELP_STRING([--with-internal-maximum-log-level=NUMBER],
                   [limit ecore internal log level to the given number, any call to EINA_LOG() with values greater than this will be compiled out, ignoring runtime settings, but saving function calls.])],
   [
    if test "x${withval}" != "xno"; then
       if echo "${withval}" | grep '^[[0-9]]\+$' >/dev/null 2>/dev/null; then
          AC_MSG_NOTICE([ignoring any EINA_LOG() with level greater than ${withval}])
          AC_DEFINE_UNQUOTED(EINA_LOG_LEVEL_MAXIMUM, ${withval}, [if set, logging is limited to this amount.])
          with_max_log_level="${withval}"
       else
          AC_MSG_ERROR([--with-internal-maximum-log-level takes a decimal number, got "${withval}" instead.])
       fi
    fi
    ], [:])


### Default options with respect to host

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

# dependencies and options
want_curl="no"
want_abstract_sockets="no"
want_gnutls="no"
want_openssl="no"
want_cares="no"
want_cipher="no"
want_signature="no"
want_poll="yes"
want_inotify="no"
want_notify_win32="no"
want_tslib="no"
want_glib="no"

# core modules
want_ecore_con="yes"
want_ecore_ipc="yes"
want_ecore_file="yes"
#want_ecore_config="no"
want_ecore_imf="no"
want_ecore_input="yes"

# graphic system modules
want_evas_simple_x11="no"
want_ecore_x_xcb="no"
want_ecore_x="no"
want_ecore_win32="no"
want_ecore_cocoa="no"
want_ecore_sdl="no"
want_ecore_fb="no"
want_ecore_directfb="no"
want_ecore_wince="no"

# ecore_x options (both xlib and xcb)
want_ecore_x_composite="yes"
want_ecore_x_damage="yes"
want_ecore_x_dpms="yes"
want_ecore_x_randr="yes"
want_ecore_x_render="yes"
want_ecore_x_screensaver="yes"
want_ecore_x_shape="yes"
want_ecore_x_sync="yes"
want_ecore_x_xfixes="yes"
want_ecore_x_xinerama="yes"
want_ecore_x_xprint="yes"
want_ecore_x_xtest="yes"
want_ecore_x_cursor="yes"
want_ecore_x_input="yes"

# ecore_evas modules
want_ecore_evas="yes"
want_ecore_evas_software_buffer="yes"
want_ecore_evas_software_x11="no"
want_ecore_evas_opengl_x11="no"
want_ecore_evas_software_16_x11="no"
want_ecore_evas_software_8_x11="no"
want_ecore_evas_software_xcb="no"
want_ecore_evas_software_gdi="no"
want_ecore_evas_software_ddraw="no"
want_ecore_evas_direct3d="no"
want_ecore_evas_opengl_glew="no"
want_ecore_evas_software_16_ddraw="no"
want_ecore_evas_software_sdl="no"
want_ecore_evas_gl_sdl="no"
want_ecore_evas_directfb="no"
want_ecore_evas_fb="no"
want_ecore_evas_software_16_wince="no"

case "$host_os" in
   mingw32ce* | cegcc*)
      want_ecore_con="no"
      want_ecore_ipc="no"
      want_ecore_wince="yes"
      want_ecore_evas_software_16_wince="yes"
      ;;
   mingw*)
      want_notify_win32="yes"
      want_curl="yes"
      want_glib="auto"
      want_ecore_imf="yes"
      want_ecore_win32="yes"
      want_ecore_evas_software_gdi="yes"
      want_ecore_evas_software_ddraw="yes"
      want_ecore_evas_direct3d="yes"
      want_ecore_evas_opengl_glew="yes"
      want_ecore_evas_software_16_ddraw="auto"
      want_ecore_evas_software_sdl="yes"
      want_ecore_evas_gl_sdl="yes"
      ;;
   darwin*)
      want_curl="yes"
      want_glib="auto"
      want_gnutls="auto"
      want_openssl="auto"
      want_ecore_imf="yes"
      want_ecore_cocoa="yes"
      want_ecore_evas_software_sdl="yes"
      want_ecore_evas_gl_sdl="yes"
      ;;
   *)
      want_curl="yes"
      want_glib="auto"
      want_abstract_sockets="yes"
      want_gnutls="auto"
      want_openssl="auto"
      want_cipher="yes"
      want_signature="yes"
      want_inotify="yes"
      want_tslib="yes"
      want_ecore_fb="yes"
      want_ecore_imf="yes"
      want_ecore_x="yes"
      want_ecore_evas_software_x11="yes"
      want_ecore_evas_opengl_x11="yes"
      want_ecore_evas_software_16_x11="yes"
      want_ecore_evas_software_8_x11="yes"
      want_ecore_evas_software_xcb="yes"
      want_ecore_evas_software_sdl="yes"
      want_ecore_evas_gl_sdl="yes"
      want_ecore_evas_directfb="yes"
      want_ecore_evas_fb="yes"
      ;;
esac

requirements_ecore=""
requirements_ecore_con=""
#requirements_ecore_config=""
requirements_ecore_directfb=""
requirements_ecore_evas=""
requirements_ecore_fb=""
requirements_ecore_file=""
requirements_ecore_imf=""
requirements_ecore_imf_evas=""
requirements_ecore_input=""
requirements_ecore_input_evas=""
requirements_ecore_ipc=""
requirements_ecore_cocoa=""
requirements_ecore_sdl=""
requirements_ecore_x=""
requirements_ecore_win32=""
requirements_ecore_wince=""

### Additional options to configure

want_glib_integration_always=no
AC_ARG_ENABLE(glib-integration-always,
  AC_HELP_STRING([--enable-glib-integration-always], [enable glib integration when ecore_init() is called always]),
  [want_glib_integration_always=$enableval])

if test "x${want_glib_integration_always}" = "xyes" ; then
  AC_DEFINE([GLIB_INTEGRATION_ALWAYS], [1], [Always integrate glib if support compiled])
  want_glib=yes
fi

want_g_main_loop=no
AC_ARG_ENABLE(g-main-loop,
  AC_HELP_STRING([--enable-g-main-loop], [ecore_main_loop based on g_main_loop]),
  [want_g_main_loop=$enableval])

if test "x${want_g_main_loop}" = "xyes" ; then
  AC_DEFINE([USE_G_MAIN_LOOP], [1], [Use g_main_loop in ecore])
  want_glib=yes
fi

if test "x${want_glib_integration_always}" = "xyes" -a "x${want_g_main_loop}" = "xyes"; then
  AC_MSG_ERROR([--enable-glib-integration-always and --enable-glib-main-loop are mutually exclusive])
fi

# abstract sockets (ecore_con.c)
AC_ARG_ENABLE([abstract-sockets],
   [AC_HELP_STRING([--disable-abstract-sockets], [disable abstract sockets.])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_ecore_con_abstract_sockets="yes"
    else
       want_ecore_con_abstract_sockets="no"
    fi
   ],
   [want_ecore_con_abstract_sockets=${want_abstract_sockets}])

if test "x${want_ecore_con_abstract_sockets}" = "xyes" ; then
   AC_DEFINE([HAVE_ABSTRACT_SOCKETS], [1], [Have abstract sockets namespace])
fi

# Simple X11 build/link

AC_ARG_ENABLE(simple-x11,
  AC_HELP_STRING([--enable-simple-x11], [enable simple x11 linking]),
  [want_evas_simple_x11=$enableval])

# XIM
AC_ARG_ENABLE([xim],
   [AC_HELP_STRING([--disable-xim], [disable X Input Method.])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_xim="yes"
    else
       want_xim="no"
    fi
   ],
   [want_xim="yes"])

if test "x${want_xim}" = "xyes" ; then
   AC_DEFINE([ENABLE_XIM], [1], [Enable X Input Method])
fi

# Unit tests

EFL_CHECK_TESTS([enable_tests="yes"], [enable_tests="no"])

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
m4_ifndef([am__fastdepOBJC], [
    AM_CONDITIONAL([am__fastdepOBJC], [false])
    AC_SUBST([cocoa_ldflags])
])

AC_PROG_CXX
AC_PROG_CC

have_gnu_objc=${ac_cv_objc_compiler_gnu}

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])

m4_ifdef([v_mic],
   [
    EFL_COMPILER_FLAG([-Wall])
    EFL_COMPILER_FLAG([-W])
   ])

# The first call to PKG_CHECK_MODULES is done conditionally,
# so we should include this here:
PKG_PROG_PKG_CONFIG

# Check whether pkg-config supports Requires.private
if $PKG_CONFIG --atleast-pkgconfig-version 0.22; then
   pkgconfig_requires_private="Requires.private"
else
   pkgconfig_requires_private="Requires"
fi
AC_SUBST(pkgconfig_requires_private)

### Checks for some build time option
have_backtrace="no"
AC_CHECK_FUNCS([backtrace], [have_backtrace="yes"], [])

want_ecore_timer_dump="yes"

AC_ARG_ENABLE([ecore-timer-dump],
   [AC_HELP_STRING([--disable-ecore-timer-dump], [disable tracking of timer allocation. @<:@default=enable@:>@])],
   [want_ecore_timer_dump=$enableval], [])

if test "x$want_ecore_timer_dump" = "xyes" -a "x$have_backtrace" = "xyes"; then
   AC_DEFINE(WANT_ECORE_TIMER_DUMP, [1], [Want Ecore_Timer dump infrastructure])
fi

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
      EFL_ECORE_BUILD="-DEFL_ECORE_BUILD"
      EFL_ECORE_CON_BUILD="-DEFL_ECORE_CON_BUILD"
      EFL_ECORE_EVAS_BUILD="-DEFL_ECORE_EVAS_BUILD"
      EFL_ECORE_FILE_BUILD="-DEFL_ECORE_FILE_BUILD"
      EFL_ECORE_IMF_BUILD="-DEFL_ECORE_IMF_BUILD"
      EFL_ECORE_IMF_EVAS_BUILD="-DEFL_ECORE_IMF_EVAS_BUILD"
      EFL_ECORE_INPUT_BUILD="-DEFL_ECORE_INPUT_BUILD"
      EFL_ECORE_INPUT_EVAS_BUILD="-DEFL_ECORE_INPUT_EVAS_BUILD"
      EFL_ECORE_IPC_BUILD="-DEFL_ECORE_IPC_BUILD"
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
AC_SUBST(EFL_ECORE_CON_BUILD)
AC_SUBST(EFL_ECORE_EVAS_BUILD)
AC_SUBST(EFL_ECORE_FILE_BUILD)
AC_SUBST(EFL_ECORE_IMF_BUILD)
AC_SUBST(EFL_ECORE_IMF_EVAS_BUILD)
AC_SUBST(EFL_ECORE_INPUT_BUILD)
AC_SUBST(EFL_ECORE_INPUT_EVAS_BUILD)
AC_SUBST(EFL_ECORE_IPC_BUILD)
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
      AC_CHECK_LIB([c], [dlopen],
         [],
         [AC_CHECK_LIB([dl], [dlopen],
             [dlopen_libs=-ldl])
         ])
      AC_CHECK_LIB([c], [clock_gettime],
         [AC_DEFINE(HAVE_CLOCK_GETTIME, [1], [Have clock_gettime()])],
         [AC_CHECK_LIB([rt], [clock_gettime],
             [
              rt_libs=-lrt
              AC_DEFINE(HAVE_CLOCK_GETTIME, [1], [Have clock_gettime()])
             ])
         ])
      ;;
esac
AC_SUBST(WIN32_LIBS)
AC_SUBST(dlopen_libs)
AC_SUBST(rt_libs)

# Eina library

PKG_CHECK_MODULES(EINA, [eina >= 1.0.0])
#FIXME check all the requirements when the eina move will be finished
requirements_ecore="eina >= 1.0.0 ${requirements_ecore}"
requirements_ecore_con="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_con}"
#requirements_ecore_config="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_config}"
requirements_ecore_directfb="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_directfb}"
requirements_ecore_evas="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_evas}"
requirements_ecore_fb="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_fb}"
requirements_ecore_file="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_file}"
requirements_ecore_imf="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_imf}"
requirements_ecore_imf_evas="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_imf_evas}"
requirements_ecore_input="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_input}"
requirements_ecore_input_evas="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_input_evas}"
requirements_ecore_ipc="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_ipc}"
requirements_ecore_cocoa="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_cocoa}"
requirements_ecore_sdl="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_sdl}"
requirements_ecore_win32="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_win32}"
requirements_ecore_wince="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_wince}"
requirements_ecore_x="ecore >= 1.0.0 eina >= 1.0.0 ${requirements_ecore_x}"


# glib support (main loop integration)
AC_ARG_ENABLE([glib],
   [AC_HELP_STRING([--disable-glib], [disable glib support. @<:@default=detect@:>@])],
   [want_glib=$enableval], [])

if test "x$want_glib" != "xno"; then
   PKG_CHECK_MODULES([GLIB], [glib-2.0], [have_glib="yes"], [have_glib="no"])
else
   have_glib="no"
fi
if test "x$want_glib" = "xyes" -a "x$have_glib" = "xno"; then
   AC_MSG_ERROR([GLib support requested, but no GLib found by pkg-config.])
elif test "x$have_glib" = "xyes"; then
   AC_DEFINE(HAVE_GLIB, [1], [Have GLib])
   requirements_ecore="glib-2.0 ${requirements_ecore}"
fi


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

if test -n "$SDL_CONFIG" ; then
   SDL_CFLAGS=`$SDL_CONFIG --cflags`
   SDL_LIBS=`$SDL_CONFIG --libs`
   AC_SUBST(SDL_CFLAGS)
   AC_SUBST(SDL_LIBS)
   have_sdl="yes"
else
   PKG_CHECK_MODULES([SDL], [sdl >= 1.2.0], [have_sdl="yes"], [have_sdl="no"])
fi

if test "x${have_sdl}" = "xyes" ; then
   PKG_CHECK_EXISTS([sdl >= 1.3.0],
      [AC_DEFINE(BUILD_ECORE_EVAS_SDL_130, 1, [Support for SVN SDL])])
fi


# DirectFB library (ecore_directfb)

PKG_CHECK_MODULES([DIRECTFB],
   [directfb >= 0.9.16],
   [have_directfb="yes"],
   [have_directfb="no"])


# Eet library (ecore_config)

#PKG_CHECK_MODULES([EET],
#   [eet >= 1.4.0],
#   [have_eet="yes"],
#   [have_eet="no"])


# Xlib and XCB (ecore_x)

have_x="no"
have_ecore_x="no"
have_ecore_x_xlib="no"
have_ecore_x_xcb="no"

x_dir="";
x_includes="";
x_cflags="";
x_libs="";

ecore_x_libs_private=""

AC_ARG_ENABLE(ecore-x-composite,
   [AC_HELP_STRING([--disable-ecore-x-composite],
       [disable the ecore_x support for Xcomposite extension.
        @<:@default=detect@:>@])],
   [want_ecore_x_composite=$enableval])

AC_ARG_ENABLE(ecore-x-damage,
   [AC_HELP_STRING([--disable-ecore-x-damage],
       [disable the ecore_x support for Xdamage extension.
        @<:@default=detect@:>@])],
   [want_ecore_x_damage=$enableval])

AC_ARG_ENABLE(ecore-x-dpms,
   [AC_HELP_STRING([--disable-ecore-x-dpms],
       [disable the ecore_x support for Xdpms extension.
        @<:@default=detect@:>@])],
   [want_ecore_x_dpms=$enableval])

AC_ARG_ENABLE(ecore-x-randr,
   [AC_HELP_STRING([--disable-ecore-x-randr],
       [disable the ecore_x support for Xrandr extension.
        @<:@default=detect@:>@])],
   [want_ecore_x_randr=$enableval])

AC_ARG_ENABLE(ecore-x-render,
   [AC_HELP_STRING([--disable-ecore-x-render],
       [disable the ecore_x support for Xrender extension.
        @<:@default=detect@:>@])],
   [want_ecore_x_render=$enableval])

AC_ARG_ENABLE(ecore-x-screensaver,
   [AC_HELP_STRING([--disable-ecore-x-screensaver],
       [disable the ecore_x support for Xscreensaver extension.
        @<:@default=detect@:>@])],
   [want_ecore_x_screensaver=$enableval])

AC_ARG_ENABLE(ecore-x-shape,
   [AC_HELP_STRING([--disable-ecore-x-shape],
       [disable the ecore_x support for Xshape extension.
        @<:@default=detect@:>@])],
   [want_ecore_x_shape=$enableval])

AC_ARG_ENABLE(ecore-x-sync,
   [AC_HELP_STRING([--disable-ecore-x-sync],
       [disable the ecore_x support for Xsync extension.
        @<:@default=detect@:>@])],
   [want_ecore_x_sync=$enableval])

AC_ARG_ENABLE(ecore-x-xfixes,
   [AC_HELP_STRING([--disable-ecore-x-xfixes],
       [disable the ecore_x support for Xfixes extension.
        @<:@default=detect@:>@])],
   [want_ecore_x_xfixes=$enableval])

AC_ARG_ENABLE(ecore-x-xinerama,
   [AC_HELP_STRING([--disable-ecore-x-xinerama],
       [disable the ecore_x support for Xinerama extension.
        @<:@default=detect@:>@])],
   [want_ecore_x_xinerama=$enableval])

AC_ARG_ENABLE(ecore-x-xprint,
   [AC_HELP_STRING([--disable-ecore-x-xprint],
       [disable the ecore_x support for Xprint extension.
        @<:@default=detect@:>@])],
   [want_ecore_x_xprint=$enableval])

AC_ARG_ENABLE(ecore-x-xtest,
   [AC_HELP_STRING([--disable-ecore-x-xtest],
       [disable the ecore_x support for Xtest extension.
        @<:@default=detect@:>@])],
   [want_ecore_x_xtest=$enableval])

AC_ARG_ENABLE(ecore-x-cursor,
   [AC_HELP_STRING([--disable-ecore-x-cursor],
       [disable the ecore_x support for Xcursor extension.
        @<:@default=detect@:>@])],
   [want_ecore_x_cursor=$enableval])

AC_ARG_ENABLE(ecore-x-input,
   [AC_HELP_STRING([--disable-ecore-x-input],
       [disable the ecore_x support for Xinput/Xinput2 extension.
        @<:@default=detect@:>@])],
   [want_ecore_x_input=$enableval])


AC_ARG_ENABLE(ecore-x-xcb,
   [AC_HELP_STRING([--enable-ecore-x-xcb],
       [enable the ecore_x module with XCB backend. @<:@default=disabled@:>@])],
   [want_ecore_x_xcb=$enableval],
   [want_ecore_x_xcb="no"])

AC_MSG_CHECKING(whether ecore_x with XCB backend is to be built)
AC_MSG_RESULT($want_ecore_x_xcb)

if test "x$want_ecore_x_xcb" = "xyes" ; then
  PKG_CHECK_MODULES(XCB, xcb xcb-icccm xcb-image xcb-keysyms pixman-1,
    [ have_ecore_x_xcb="yes"
      requirements_ecore_x="xcb xcb-icccm xcb-image xcb-keysyms pixman-1 ${requirements_ecore_x}" ],
    [ have_ecore_x_xcb="no" ])

  if test "x$have_ecore_x_xcb" = "xyes" ; then
    if test "x$want_ecore_x_composite" != "xno"; then
      PKG_CHECK_MODULES(XCB_COMPOSITE, xcb-composite,
        [ have_ecore_x_xcb_composite="yes"
          requirements_ecore_x="xcb-composite ${requirements_ecore_x}"
          AC_DEFINE(ECORE_XCB_COMPOSITE, 1, [Build support for XCB composite]) ],
        [ have_ecore_x_xcb_composite="no" ])
    else
      have_ecore_x_xcb_composite="no"
      AC_MSG_NOTICE("composite extension explicitly disabled")
    fi

    if test "x$want_ecore_x_damage" != "xno"; then
      PKG_CHECK_MODULES(XCB_DAMAGE, xcb-damage,
        [ have_ecore_x_xcb_damage="yes"
          requirements_ecore_x="xcb-damage ${requirements_ecore_x}"
          AC_DEFINE(ECORE_XCB_DAMAGE, 1, [Build support for XCB damage]) ],
        [ have_ecore_x_xcb_damage="no" ])
    else
      have_ecore_x_xcb_damage="no"
      AC_MSG_NOTICE("damage extension explicitly disabled")
    fi

    if test "x$want_ecore_x_dpms" != "xno"; then
      PKG_CHECK_MODULES(XCB_DPMS, xcb-dpms,
        [ have_ecore_x_xcb_dpms="yes"
          requirements_ecore_x="xcb-dpms ${requirements_ecore_x}"
          AC_DEFINE(ECORE_XCB_DPMS, 1, [Build support for XCB dpms]) ],
        [ have_ecore_x_xcb_dpms="no" ])
    else
      have_ecore_x_xcb_dpms="no"
      AC_MSG_NOTICE("dpms extension explicitly disabled")
    fi

    if test "x$want_ecore_x_randr" != "xno"; then
      PKG_CHECK_MODULES(XCB_RANDR, xcb-randr,
        [ have_ecore_x_xcb_randr="yes"
          requirements_ecore_x="xcb-randr ${requirements_ecore_x}"
          AC_DEFINE(ECORE_XCB_RANDR, 1, [Build support for XCB randr]) ],
        [ have_ecore_x_xcb_randr="no" ])
    else
      have_ecore_x_xcb_randr="no"
      AC_MSG_NOTICE("randr extension explicitly disabled")
    fi

    if test "x$want_ecore_x_render" != "xno"; then
      PKG_CHECK_MODULES(XCB_RENDER, xcb-render,
        [ have_ecore_x_xcb_render="yes"
          requirements_ecore_x="xcb-render ${requirements_ecore_x}"
          AC_DEFINE(ECORE_XCB_RENDER, 1, [Build support for XCB render]) ],
        [ have_ecore_x_xcb_render="no" ])
    else
      have_ecore_x_xcb_render="no"
      AC_MSG_NOTICE("render extension explicitly disabled")
    fi

    if test "x$want_ecore_x_screensaver" != "xno"; then
      PKG_CHECK_MODULES(XCB_SCREENSAVER, xcb-screensaver,
        [ have_ecore_x_xcb_screensaver="yes"
          requirements_ecore_x="xcb-screensaver ${requirements_ecore_x}"
          AC_DEFINE(ECORE_XCB_SCREENSAVER, 1, [Build support for XCB screensaver]) ],
        [ have_ecore_x_xcb_screensaver="no" ])
    else
      have_ecore_x_xcb_screensaver="no"
      AC_MSG_NOTICE("screensaver extension explicitly disabled")
    fi

    if test "x$want_ecore_x_shape" != "xno"; then
      PKG_CHECK_MODULES(XCB_SHAPE, xcb-shape,
        [ have_ecore_x_xcb_shape="yes"
          requirements_ecore_x="xcb-shape ${requirements_ecore_x}"
          AC_DEFINE(ECORE_XCB_SHAPE, 1, [Build support for XCB shape]) ],
        [ have_ecore_x_xcb_shape="no" ])
    else
      have_ecore_x_xcb_shape="no"
      AC_MSG_NOTICE("shape extension explicitly disabled")
    fi

    if test "x$want_ecore_x_sync" != "xno"; then
      PKG_CHECK_MODULES(XCB_SYNC, xcb-sync,
        [ have_ecore_x_xcb_sync="yes"
          requirements_ecore_x="xcb-sync ${requirements_ecore_x}"
          AC_DEFINE(ECORE_XCB_SYNC, 1, [Build support for XCB sync]) ],
        [ have_ecore_x_xcb_sync="no" ])
    else
      have_ecore_x_xcb_sync="no"
      AC_MSG_NOTICE("sync extension explicitly disabled")
    fi

    if test "x$want_ecore_x_xfixes" != "xno"; then
      PKG_CHECK_MODULES(XCB_XFIXES, xcb-xfixes,
        [ have_ecore_x_xcb_xfixes="yes"
          requirements_ecore_x="xcb-xfixes ${requirements_ecore_x}"
          AC_DEFINE(ECORE_XCB_FIXES, 1, [Build support for XCB xfixes]) ],
        [ have_ecore_x_xcb_xfixes="no" ])
    else
      have_ecore_x_xcb_xfixes="no"
      AC_MSG_NOTICE("xfixes extension explicitly disabled")
    fi

    if test "x$want_ecore_x_xinerama" != "xno"; then
      PKG_CHECK_MODULES(XCB_XINERAMA, xcb-xinerama,
        [ have_ecore_x_xcb_xinerama="yes"
          requirements_ecore_x="xcb-xinerama ${requirements_ecore_x}"
          AC_DEFINE(ECORE_XCB_XINERAMA, 1, [Build support for XCB xinerama]) ],
        [ have_ecore_x_xcb_xinerama="no" ])
    else
      have_ecore_x_xcb_xinerama="no"
      AC_MSG_NOTICE("xinerama extension explicitly disabled")
    fi

    if test "x$want_ecore_x_xprint" != "xno"; then
      PKG_CHECK_MODULES(XCB_XPRINT, xcb-xprint,
        [ have_ecore_x_xcb_xprint="yes"
          requirements_ecore_x="xcb-xprint ${requirements_ecore_x}"
          AC_DEFINE(ECORE_XCB_XPRINT, 1, [Build support for XCB xprint]) ],
        [ have_ecore_x_xcb_xprint="no" ])
    else
      have_ecore_x_xcb_xprint="no"
      AC_MSG_NOTICE("xprint extension explicitly disabled")
    fi

    if test "x$want_ecore_x_xtest" != "xno"; then
      PKG_CHECK_MODULES(XCB_XTEST, xcb-xtest,
        [ have_ecore_x_xcb_xtest="yes"
          requirements_ecore_x="xcb-xtest ${requirements_ecore_x}"
          AC_DEFINE(ECORE_XCB_XTEST, 1, [Build support for XCB xtest]) ],
        [ have_ecore_x_xcb_xtest="no" ])
    else
      have_ecore_x_xcb_xtest="no"
      AC_MSG_NOTICE("xtest extension explicitly disabled")
    fi

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

    if test "x$want_ecore_x_cursor" = "xyes"; then
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
    else
      Xcursor_cflags=""
      Xcursor_libs=""
      use_Xcursor="no"
      AC_MSG_NOTICE("Xcursor explicitly disabled")
    fi

    AC_SUBST(Xcursor_cflags)
    AC_SUBST(Xcursor_libs)

    ECORE_CHECK_X_EXTENSION([Xkb], [XKB.h], [X11], [XkbSetDetectableAutoRepeat], [$want_ecore_x_xkb])
    ECORE_CHECK_X_EXTENSION([Xcomposite], [Xcomposite.h], [Xcomposite], [XCompositeQueryExtension], [$want_ecore_x_composite])
    ECORE_CHECK_X_EXTENSION([Xdamage], [Xdamage.h], [Xdamage], [XDamageSubtract], [$want_ecore_x_damage])
    ECORE_CHECK_X_EXTENSION([Xdpms], [dpms.h], [Xdpms], [DPMSQueryExtension], [$want_ecore_x_dpms])
    if test "x$use_xdpms" = "xno" ; then
      ECORE_CHECK_X_EXTENSION([Xdpms], [dpms.h], [Xext], [DPMSQueryExtension], [$want_ecore_x_dpms])
    fi
    ECORE_CHECK_X_EXTENSION([Xfixes], [Xfixes.h], [Xfixes], [XFixesExpandRegion], [$want_ecore_x_xfixes])
    ECORE_CHECK_X_EXTENSION([Xinerama], [Xinerama.h], [Xinerama], [XineramaQueryScreens], [$want_ecore_x_xinerama])
    ECORE_CHECK_X_EXTENSION([Xprint], [Print.h], [Xp], [XpQueryScreens], [$want_ecore_x_xprint])
    ECORE_CHECK_X_EXTENSION([Xrandr], [Xrandr.h], [Xrandr], [XRRGetScreenResourcesCurrent], [$want_ecore_x_randr])
    ECORE_CHECK_X_EXTENSION([Xrender], [Xrender.h], [Xrender], [XRenderFindVisualFormat], [$want_ecore_x_render])
    ECORE_CHECK_X_EXTENSION([Xtest], [XTest.h], [Xtst], [XTestFakeKeyEvent], [$want_ecore_x_xtest])
    ECORE_CHECK_X_EXTENSION([Xss], [scrnsaver.h], [Xss], [XScreenSaverSelectInput], [$want_ecore_x_screensaver])
    ECORE_CHECK_X_EXTENSION([Xi2], [XInput2.h], [Xi], [XIQueryDevice], [$want_ecore_x_input])

    ecore_x_libs_private="${Xcursor_libs} ${XKB_LIBS} ${XCOMPOSITE_LIBS} ${XDAMAGE_LIBS} ${XDPMS_LIBS} ${XFIXES_LIBS} ${XINERAMA_LIBS} ${XPRINT_LIBS} ${XRANDR_LIBS} ${XRENDER_LIBS} ${XTEST_LIBS} ${XSS_LIBS} ${XI2_LIBS}"

    AC_DEFINE(HAVE_ECORE_X_XLIB, 1, [Defined to 1 if Xlib is enabled.])
    have_x="yes"

    have_ecore_x_xlib="yes"
  fi
fi

AC_SUBST(x_cflags)
AC_SUBST(x_includes)
AC_SUBST(x_libs)
AC_SUBST(ecore_x_libs_private)

AM_CONDITIONAL(BUILD_ECORE_X_XLIB, test $have_ecore_x_xlib = yes)
AM_CONDITIONAL(BUILD_ECORE_X_XCB, test $have_ecore_x_xcb = yes)


# Evas library (ecore_config, ecore_input_evas, ecore_imf_evas and ecore_evas)

PKG_CHECK_MODULES([EVAS], [evas >= 1.0.0],
   [have_evas="yes"],
   [have_evas="no"])


### Checks for header files

EFL_CHECK_PATH_MAX

AC_HEADER_SYS_WAIT
AC_SYS_LARGEFILE

have_addrinfo="no"
case "$host_os" in
   mingw* | cegcc*)
      AC_DEFINE(HAVE_DLFCN_H, 1, [Define to 1 if you have the <dlfcn.h> header file.])
      AC_DEFINE(HAVE_SYS_MMAN_H, 1, [Define to 1 if you have the <sys/mman.h> header file.])
      AC_DEFINE(HAVE_SYS_TIME_H, 1, [Define to 1 if you have the <sys/time.h> header file.])
      have_addrinfo="yes"
      ;;
   *)
      AC_CHECK_HEADERS([dlfcn.h features.h langinfo.h locale.h sys/time.h sys/mman.h signal.h sys/resource.h])
      ;;
esac

# ecore_con

AC_CHECK_HEADERS([arpa/inet.h arpa/nameser.h netinet/tcp.h netinet/in.h sys/socket.h sys/un.h ws2tcpip.h netdb.h])

if test "x${ac_cv_header_netdb_h}" = "xyes" ; then
   have_addrinfo="yes"
fi

# Framebuffer (ecore_fb)
have_fb="no"
AC_CHECK_HEADER([linux/fb.h],
   [AC_CHECK_HEADER([linux/input.h], [have_fb="yes"])])

# Cocoa header files (ecore_cocoa)

cocoa_ldflags="";
have_cocoa="no"
m4_ifdef([AC_PROG_OBJC], [
    if test "x${have_gnu_objc}" = "xyes" ; then
       AC_LANG_PUSH([Objective C])
       AC_CHECK_HEADER([Cocoa/Cocoa.h],
          [
           have_cocoa="yes"
           cocoa_ldflags="-framework Cocoa"
          ])
       AC_LANG_POP([Objective C])
    fi
])
AC_SUBST(cocoa_ldflags)

want_epoll=yes
AC_ARG_ENABLE(epoll,
  AC_HELP_STRING([--enable-epoll], [enable or disable epoll support]),
  [want_epoll=$enableval])

if test "x${want_epoll}" = "xyes" ; then
  # check for epoll support
  AC_CHECK_HEADERS([sys/epoll.h])
fi

# timerfd_create
AC_CHECK_HEADERS([sys/timerfd.h])
AC_CHECK_FUNCS(timerfd_create)

# thread support

EFL_CHECK_THREADS(
   [
    if test "x${_efl_have_posix_threads}" = "xyes" ; then
       have_threads="POSIX"
    else
       if test "x${_efl_have_win32_threads}" = "xyes" ; then
          have_threads="Win32"
       else
          have_threads="no"
       fi
    fi
   ],
   [have_threads="no"])

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
      WIN32_CPPFLAGS="-D_WIN32_WINNT=0x0501"
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
AC_ISC_POSIX
AC_FUNC_ALLOCA
AC_CHECK_FUNCS(gettimeofday strlcpy)

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
       [[
#include <math.h>
       ]],
       [[
int i = isfinite(0);
       ]])],
   [
    AC_DEFINE(HAVE_ISFINITE, 1, [Define to 1 if you have `isfinite', as a function or macro.])
    have_isfinite="yes"
   ],
   [have_isfinite="no"])

AC_MSG_CHECKING([for isfinite])
AC_MSG_RESULT([${have_isfinite}])

have_atfile_source=auto
AC_ARG_ENABLE([atfile-source],
   [AC_HELP_STRING([--disable-atfile-source],
       [disable use of atfile source functions as openat and mkdirat @<:@default=detect@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_atfile_source="yes"
    else
       have_atfile_source="no"
    fi],
   [have_atfile_source=auto])

if ! test "x${have_atfile_source}" = "xno" ; then
   AC_CHECK_FUNCS(mkdirat,
      [
       have_atfile_source="yes"
       AC_DEFINE(HAVE_ATFILE_SOURCE, 1, [mkdirat exists])
      ],
      [
       if test "x${have_atfile_source}" = "xyes" ; then
          AC_MSG_ERROR([required atfile-source but no mkdirat()])
       fi
       have_atfile_source="no"
      ])
fi

### Checks for optionnal feature
AC_CHECK_FUNC([mallinfo],
   [
    have_mallinfo="yes"
    AC_DEFINE(HAVE_MALLINFO, 1, [Gather memory statistic])
   ],
   [have_mallinfo="no"])

### Ecore modules

## Core modules

# ecore_con
ECORE_CHECK_MODULE([con], [${want_ecore_con}], [Con], [${have_addrinfo}])

have_curl="no"
have_gnutls="no"
have_openssl="no"
have_cares="no"
if test "x${have_ecore_con}" = "xyes" ; then

   ECORE_CHECK_CURL([${want_curl}],
      [
       have_curl="yes"
       requirements_ecore_con="libcurl ${requirements_ecore_con}"
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

   ECORE_CHECK_CARES([${want_cares}],
      [
       have_cares="yes"
       requirements_ecore_con="libcares ${requirements_ecore_con}"
      ],
      [have_cares="no"])

fi

AM_CONDITIONAL([HAVE_CARES], [test "x${have_cares}" = "xyes"])

# ecore_ipc
ECORE_CHECK_MODULE([ipc], [${want_ecore_ipc}], [Ipc], [${have_ecore_con}],
   [requirements_ecore_ipc="ecore-con >= 1.0.0 ${requirements_ecore_ipc}"])

# ecore_file
ECORE_CHECK_MODULE([file], [${want_ecore_file}], [File])

have_poll="no"
have_inotify="no"
have_notify_win32="no"
if test "x${have_ecore_file}" = "xyes" ; then
   ECORE_CHECK_POLL([${want_poll}], [have_poll="yes"], [have_poll="no"])
   ECORE_CHECK_INOTIFY([${want_inotify}], [have_inotify="yes"], [have_inotify="no"])
   ECORE_CHECK_NOTIFY_WIN32([${want_notify_win32}], [have_notify_win32="yes"], [have_notify_win32="no"])

   if test "x${have_ecore_con}" = "xyes" ; then
      requirements_ecore_file="ecore-con >= 1.0.0 ${requirements_ecore_file}"
   fi
fi

# ecore_config
#ecore_config_deps="no"
#if test "x${have_eet}" = "xyes" -a "x${have_evas}"  -a "x${have_ecore_ipc}" ; then
#   ecore_config_deps="yes"
#fi

#ECORE_CHECK_MODULE([config], [${want_ecore_config}], [Config], [${ecore_config_deps}],
#   [requirements_ecore_config="ecore-ipc >= 1.0.0 evas >= 1.0.0 eet >= 1.4.0 ${requirements_ecore_config}"])

AM_CONDITIONAL(BUILD_ECORE_CONFIG, false)

# ecore_imf

ECORE_CHECK_MODULE([imf], [${want_ecore_imf}], [Imf])

# ecore_imf_evas

ecore_imf_evas_deps="no"
if test "x${have_ecore_imf}" = "xyes" -a "x${have_evas}" = "xyes" ; then
   ecore_imf_evas_deps="yes"
fi

ECORE_CHECK_MODULE([imf-evas], [${want_ecore_imf}], [Imf_Evas], [${ecore_imf_evas_deps}],
   [requirements_ecore_imf_evas="ecore-imf >= 1.0.0 evas >= 1.0.0 ${requirements_ecore_imf_evas}"])

# ecore_input{_evas}
ECORE_CHECK_MODULE([input], [${want_ecore_input}], [Input])
ECORE_CHECK_MODULE([input-evas], [${want_ecore_input}], [Input_Evas], [${have_evas}],
   [requirements_ecore_input_evas="ecore-input >= 1.0.0 evas >= 1.0.0 ${requirements_ecore_input}"])

## Graphic systems

# ecore_x{cb}

ecore_x_deps="no"
if test "x${have_x}" = "xyes" -a \
        "x${have_ecore_input}" = "xyes" ; then
   ecore_x_deps="yes"
fi

ECORE_CHECK_MODULE([x], [${want_ecore_x}], [X], [${ecore_x_deps}],
  [
   ecore_x_libs="$ecore_x_libs $x_libs"
   requirements_ecore_x="ecore-input >= 1.0.0 ${requirements_ecore_x}"
  ])

# ecore_win32

ECORE_CHECK_MODULE([win32], [${want_ecore_win32}], [Win32], [${have_ecore_input}],
   [
    ecore_win32_libs="-lole32 -lgdi32"
    requirements_ecore_win32="ecore-input >= 1.0.0 ${requirements_ecore_win32}"
   ])
AC_SUBST(ecore_win32_libs)

# ecore_cocoa

ecore_cocoa_deps="no"
if test "x${have_ecore_input}" = "xyes" -a "x${have_cocoa}" = "xyes" ; then
   ecore_cocoa_deps="yes"
fi

ECORE_CHECK_MODULE([cocoa], [${want_ecore_cocoa}], [Cocoa], [${ecore_cocoa_deps}],
   [requirements_ecore_cocoa="ecore-input >= 1.0.0 ${requirements_ecore_cocoa}"])

# ecore_sdl

ecore_sdl_deps="no"
if test "x${have_sdl}" = "xyes" -a "x${have_ecore_input}" = "xyes" ; then
   ecore_sdl_deps="yes"
fi

ECORE_CHECK_MODULE([sdl], [${want_ecore_sdl}], [Sdl], [${ecore_sdl_deps}],
   [requirements_ecore_sdl="ecore-input >= 1.0.0 ${requirements_ecore_sdl}"])

# ecore_fb
ECORE_CHECK_MODULE([fb], [${want_ecore_fb}], [FB], [$have_fb])

if test "x${have_ecore_fb}" = "xyes" ; then
   ECORE_CHECK_TSLIB([${want_tslib}],
      [
       have_tslib="yes"
       requirements_ecore_fb="${_tslib_requirement} ${requirements_ecore_fb}"
      ],
      [have_tslib="no"])
fi

# ecore_directfb

ECORE_CHECK_MODULE([directfb], [${want_ecore_directfb}], [DirectFB], [${have_directfb}],
   [requirements_ecore_directfb="directfb ${requirements_ecore_directfb}"])

# ecore_wince

ECORE_CHECK_MODULE([wince], [${want_ecore_wince}], [WinCE], [${have_ecore_input}],
   [requirements_ecore_win32="ecore-input >= 1.0.0 ${requirements_ecore_win32}"])

## Ecore Evas

# ecore_evas

ecore_evas_deps="no"
if test "x${have_evas}" = "xyes" -a "x${have_ecore_input}" = "xyes" -a "x${have_ecore_input_evas}" = "xyes" ; then
   ecore_evas_deps="yes"
fi

ECORE_CHECK_MODULE([evas], [${want_ecore_evas}], [Evas], [${ecore_evas_deps}],
  [requirements_ecore_evas="ecore-input >= 1.0.0 ecore-input-evas >= 1.0.0 evas >= 1.0.0 ${requirements_ecore_evas}"])

# ecore_evas_buffer

ECORE_EVAS_CHECK_MODULE([software-buffer],
   [${want_ecore_evas_software_buffer}],
   [Software Buffer],
   [yes])

# ecore_evas_x11

# ecore_evas_software_x11

ECORE_EVAS_CHECK_MODULE([software-x11],
   [${want_ecore_evas_software_x11}],
   [Software X11],
   [${have_ecore_x}])

have_ecore_evas_software_xlib="no"
have_ecore_evas_software_xcb="no"
if test "x$have_ecore_evas_software_x11" = "xyes" ; then
   have_ecore_evas_software_xlib=`${PKG_CONFIG} --variable=Xlib evas-software-x11`
   if test "x${have_ecore_evas_software_xlib}" = "xyes" -a "x${have_ecore_x_xlib}" = "xyes" ; then
      AC_DEFINE(BUILD_ECORE_EVAS_SOFTWARE_XLIB, 1, [Evas Software Xlib Engine Support])
   fi
   have_ecore_evas_software_xcb=`${PKG_CONFIG} --variable=XCB evas-software-x11`
   if test "x$have_ecore_evas_software_xcb" = "xyes" -a "x${have_ecore_x_xcb}" = "xyes" ; then
      AC_DEFINE(BUILD_ECORE_EVAS_SOFTWARE_XCB, 1, [Evas Software XCB Engine Support])
   fi
fi

# ecore_evas_opengl_x11

ECORE_EVAS_CHECK_MODULE([opengl-x11],
   [${want_ecore_evas_opengl_x11}],
   [OpenGL Xlib],
   [${have_ecore_x_xlib}])

# ecore_evas_software_x11 16 bits

ECORE_EVAS_CHECK_MODULE([software-16-x11],
   [${want_ecore_evas_software_16_x11}],
   [Software Xlib 16 bits],
   [${have_ecore_x_xlib}])

ECORE_EVAS_CHECK_MODULE([software-8-x11],
    [$want_ecore_evas_software_8_x11],
    [Software 8bit X11],
    $have_ecore_x_xcb)


if test "x$have_ecore_evas_software_x11" = "xyes" -o \
        "x$have_ecore_evas_opengl_x11" = "xyes" -o \
        "x$have_ecore_evas_software_8_x11" = "xyes" -o \
        "x$have_ecore_evas_software_16_x11" = "xyes" -o \
        "x$have_ecore_evas_software_xcb" = "xyes"; then
   AC_DEFINE(BUILD_ECORE_EVAS_X11, 1, [Support for X Window Engines in Ecore_Evas])
   requirements_ecore_evas="ecore-x >= 1.0.0 ${requirements_ecore_evas}"
fi

# ecore_evas_win32

ECORE_EVAS_CHECK_MODULE([software-gdi],
   [${want_ecore_evas_software_gdi}],
   [Software GDI],
   [${have_ecore_win32}])

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

if test "x${have_ecore_evas_software_gdi}" = "xyes" -o \
        "x${have_ecore_evas_software_ddraw}" = "xyes" -o \
        "x${have_ecore_evas_direct3d}" = "xyes" -o \
        "x${have_ecore_evas_opengl_glew}" = "xyes" -o \
        "x${have_ecore_evas_software_16_ddraw}" = "xyes" ; then
   AC_DEFINE(BUILD_ECORE_EVAS_WIN32, 1, [Support for Win32 Engine in Ecore_Evas])
   requirements_ecore_evas="ecore-win32 >= 1.0.0 ${requirements_ecore_evas}"
fi

# ecore_evas_software_sdl

ECORE_EVAS_CHECK_MODULE([software-sdl],
   [${want_ecore_evas_software_sdl}],
   [Software SDL],
   [${have_ecore_sdl}],
   [requirements_ecore_evas="ecore-sdl >= 1.0.0 ${requirements_ecore_evas}"])

# ecore_evas_gl_sdl

ECORE_EVAS_CHECK_MODULE([opengl-sdl],
   [${want_ecore_evas_gl_sdl}],
   [OpenGL SDL],
   [${have_ecore_sdl}],
   [requirements_ecore_evas="ecore-sdl >= 1.0.0 ${requirements_ecore_evas}"])

# ecore_evas_directfb

ECORE_EVAS_CHECK_MODULE([directfb],
   [${want_ecore_evas_directfb}],
   [DirectFB],
   [${have_ecore_directfb}],
   [requirements_ecore_evas="ecore-directfb >= 1.0.0 ${requirements_ecore_evas}"])

# ecore_evas_fb

ECORE_EVAS_CHECK_MODULE([fb],
   [${want_ecore_evas_fb}],
   [Linux Framebuffer],
   [${have_ecore_fb}],
   [requirements_ecore_evas="ecore-fb >= 1.0.0 ${requirements_ecore_evas}"])

# ecore_evas_wince

ECORE_EVAS_CHECK_MODULE([software-16-wince],
   [${want_ecore_evas_software_16_wince}],
   [16 bpp Software Windows CE],
   [${have_ecore_wince}],
   [requirements_ecore_evas="ecore-wince >= 1.0.0 ${requirements_ecore_evas}"])

### install and build examples

EFL_CHECK_BUILD_EXAMPLES([enable_build_examples="yes"], [enable_build_examples="no"])
EFL_CHECK_INSTALL_EXAMPLES([enable_install_examples="yes"], [enable_install_examples="no"])

### requirements

AC_SUBST(requirements_ecore)
AC_SUBST(requirements_ecore_con)
#AC_SUBST(requirements_ecore_config)
AC_SUBST(requirements_ecore_directfb)
AC_SUBST(requirements_ecore_evas)
AC_SUBST(requirements_ecore_fb)
AC_SUBST(requirements_ecore_file)
AC_SUBST(requirements_ecore_imf)
AC_SUBST(requirements_ecore_imf_evas)
AC_SUBST(requirements_ecore_input)
AC_SUBST(requirements_ecore_input_evas)
AC_SUBST(requirements_ecore_ipc)
AC_SUBST(requirements_ecore_cocoa)
AC_SUBST(requirements_ecore_sdl)
AC_SUBST(requirements_ecore_x)
AC_SUBST(requirements_ecore_win32)
AC_SUBST(requirements_ecore_wince)

AC_CONFIG_FILES([
Makefile
ecore-con.pc
ecore-config.pc
ecore-directfb.pc
ecore-evas.pc
ecore-fb.pc
ecore-file.pc
ecore-cocoa.pc
ecore-imf.pc
ecore-imf-evas.pc
ecore-ipc.pc
ecore-x.pc
ecore-input.pc
ecore-input-evas.pc
ecore-win32.pc
ecore-sdl.pc
ecore-wince.pc
ecore.pc
doc/ecore.dox
doc/Makefile
doc/Doxyfile
src/Makefile
src/bin/Makefile
src/lib/Makefile
src/lib/ecore/Makefile
src/lib/ecore_con/Makefile
src/lib/ecore_config/Makefile
src/lib/ecore_directfb/Makefile
src/lib/ecore_evas/Makefile
src/lib/ecore_fb/Makefile
src/lib/ecore_file/Makefile
src/lib/ecore_cocoa/Makefile
src/lib/ecore_sdl/Makefile
src/lib/ecore_imf/Makefile
src/lib/ecore_imf_evas/Makefile
src/lib/ecore_input/Makefile
src/lib/ecore_input_evas/Makefile
src/lib/ecore_ipc/Makefile
src/lib/ecore_win32/Makefile
src/lib/ecore_wince/Makefile
src/lib/ecore_x/Makefile
src/lib/ecore_x/xlib/Makefile
src/lib/ecore_x/xcb/Makefile
src/examples/Makefile
src/tests/Makefile
README
ecore.spec
$po_makefile_in
])

AC_OUTPUT

echo
echo "$PACKAGE $VERSION"
echo
echo "Optional Modules:"
echo
echo " Core:"
echo
echo "  Ecore........................: always"
echo "    Thread support.............: $have_threads"
echo "    GLib support...............: $have_glib"
echo "    Always integrate GLib......: $want_glib_integration_always"
echo "    Use g_main_loop............: $want_g_main_loop"
echo "    Gathering memory statistic.: $have_mallinfo"
echo "    Gathering timer allocation.: $want_ecore_timer_dump"
echo "  Ecore_Con....................: $have_ecore_con"
if test "x$have_ecore_con" = "xyes" ; then
  echo $ECHO_N "    OpenSSL....................: $have_openssl $ECHO_C"
if test "x$have_gnutls" = "xyes" ; then
  echo " (disabled)"
else
  echo
fi
  echo "    GnuTLS.....................: $have_gnutls"
  echo "    CURL.......................: $have_curl"
  echo "    Abstract Sockets...........: $want_abstract_sockets"
  echo "    c-ares.....................: $have_cares"
fi
echo "  Ecore_Ipc....................: $have_ecore_ipc"
if test "x$have_ecore_ipc" = "xyes" ; then
  echo $ECHO_N "    OpenSSL....................: $have_openssl $ECHO_C"
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
  echo "    Windows notification.......: $have_notify_win32"
  echo "    Poll.......................: $have_poll"
  echo "    CURL.......................: $have_curl"
fi
#echo "  Ecore_Config.................: $have_ecore_config (deprecated)"
echo "  Ecore_IMF....................: $have_ecore_imf"
echo "  Ecore_IMF_Evas...............: $have_ecore_imf_evas"
echo "  Ecore_Input..................: $have_ecore_input"
echo "  Ecore_Input_Evas.............: $have_ecore_input_evas"

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
    echo "    XIM........................: $want_xim"
    echo "    Xi2........................: $use_xi2"
  fi
else
  echo "  Ecore_X......................: $have_ecore_x"
fi
echo "  Ecore_Win32..................: $have_ecore_win32"
echo "  Ecore_Cocoa..................: $have_ecore_cocoa"
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
  if test "x${have_ecore_evas_software_x11}" = "xyes" ; then
    echo "    Software X11...............: $have_ecore_evas_software_x11 (Xlib=${have_ecore_evas_software_xlib}) (XCB=${have_ecore_evas_software_xcb})"
  else
    echo "    Software X11...............: $have_ecore_evas_software_x11"
  fi
  echo "    OpenGL X11.................: $have_ecore_evas_opengl_x11"
  echo "    Software GDI...............: $have_ecore_evas_software_gdi"
  echo "    Software DirectDraw........: $have_ecore_evas_software_ddraw"
  echo "    Direct3D...................: $have_ecore_evas_direct3d"
  echo "    OpenGL Glew................: $have_ecore_evas_opengl_glew"
  echo "    Software SDL...............: $have_ecore_evas_software_sdl"
  echo "    OpenGL SDL.................: $have_ecore_evas_opengl_sdl"
  echo "    DirectFB...................: $have_ecore_evas_directfb"
  echo "    Software Framebuffer.......: $have_ecore_evas_fb"
  echo "    Software 8bit grayscale XCB: $have_ecore_evas_software_8_x11"
  echo "    Software 16bit X11.........: $have_ecore_evas_software_16_x11"
  echo "    Software 16bit DirectDraw..: $have_ecore_evas_software_16_ddraw"
  echo "    Software 16bit WinCE.......: $have_ecore_evas_software_16_wince"
fi
echo
echo "  Tests................: ${enable_tests}"
echo "  Maximum log level....: ${with_max_log_level}"
echo "Documentation..........: ${build_doc}"
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

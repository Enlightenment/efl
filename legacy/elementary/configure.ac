EFL_VERSION([1], [10], [99], [dev])
AC_INIT([elementary], [efl_version], [enlightenment-devel@lists.sourceforge.net])

AC_PREREQ(2.60)
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_CONFIG_HEADERS([elementary_config.h])
AH_TOP([
#ifndef EFL_CONFIG_H__
#define EFL_CONFIG_H__
])
AH_BOTTOM([
#endif /* EFL_CONFIG_H__ */
])

AC_USE_SYSTEM_EXTENSIONS

AM_INIT_AUTOMAKE([1.6 dist-bzip2 dist-xz])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# pkg-config
PKG_PROG_PKG_CONFIG

MODULE_EDJE=`pkg-config --variable=module_arch edje`
AC_SUBST(MODULE_EDJE)
AC_DEFINE_UNQUOTED(MODULE_EDJE, "$MODULE_EDJE", "Edje module architecture")

# Needs to be moved into a macro, and also, needs a way to automatically fetch
# from all the dependencies using the Requires.
DEPS_EOLIAN_FLAGS=`pkg-config --variable=eolian_flags eo evas edje ecore`
AC_SUBST(DEPS_EOLIAN_FLAGS)

ELM_UNIX_DEF="#undef"
ELM_WIN32_DEF="#undef"
have_windows="no"
have_dlopen="no"
case "$host_os" in
   mingw*)
      PKG_CHECK_MODULES([EVIL], [evil])
      AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if evil package is installed.])
      ELM_WIN32_DEF="#define"
      have_windows="yes"
dnl managed by evil
      AC_DEFINE(HAVE_DLADDR)
      have_dlopen="yes"
      dlopen_libs=-ldl
      requirement_elm="evil"
      have_socket="no"
      default_engine=software_gdi
      ;;
   *solaris*)
      ELM_UNIX_DEF="#define"
      have_socket="yes"
      have_dlopen="yes"
      AC_CHECK_LIB([socket], [connect], [], [have_socket="no"])
      default_engine=software_x11
      ;;
   darwin*)
      ELM_UNIX_DEF="#define"
      have_socket="yes"
      have_dlopen="yes"
      AC_CHECK_HEADERS([crt_externs.h])
      AC_DEFINE([environ], [(*_NSGetEnviron())], ["apple doesn't follow POSIX in this case."])
      default_engine=opengl_cocoa
      ;;
   *)
      ELM_UNIX_DEF="#define"
      AC_CHECK_FUNCS(dlopen, res=yes, res=no)
      if test "x$res" = "xyes"; then
        AC_CHECK_FUNCS(dladdr, AC_DEFINE(HAVE_DLADDR))
        have_dlopen="yes"
      else
        AC_CHECK_LIB(dl, dlopen, res=yes, res=no)
        AC_CHECK_LIB(m, sincos)
        if test "x$res" = "xyes"; then
          AC_CHECK_LIB(dl, dladdr, AC_DEFINE(HAVE_DLADDR))
          dlopen_libs=-ldl
          have_dlopen="yes"
        fi
      fi
      have_socket="yes"
      default_engine=software_x11
      ;;
esac

AM_CONDITIONAL([BUILD_RUN], [test "x$have_socket" = "xyes"])
AM_CONDITIONAL([BUILD_MODULES], [test "x$have_dlopen" = "xyes"])

AC_SUBST([default_engine])
AC_DEFINE_UNQUOTED([DEFAULT_ENGINE], ["$default_engine"], ["Default engine according to host"])

m4_ifdef([v_mic],
   [
    EFL_COMPILER_FLAG([-Wshadow])
    EFL_COMPILER_FLAG([-Wall])
    EFL_COMPILER_FLAG([-W])
   ])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT([win32-dll disable-static pic-only])
EFL_INIT

AC_GNU_SOURCE

AC_C_BIGENDIAN
AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST
AC_FUNC_ALLOCA

ELM_VERSION_MAJOR="v_maj"
ELM_VERSION_MINOR="v_min"
AC_SUBST(ELM_VERSION_MAJOR)
AC_SUBST(ELM_VERSION_MINOR)

MODULE_ARCH="v-v_maj.v_min.v_mic"
AC_SUBST(MODULE_ARCH)
AC_DEFINE_UNQUOTED(MODULE_ARCH, "$MODULE_ARCH", "Module architecture")

EFL_SHARED_EXTENSION="${shrext_cmds}"
AC_DEFINE_UNQUOTED(EFL_SHARED_EXTENSION, "${EFL_SHARED_EXTENSION}", "Shared extension")

requirement_elm=""

# Check whether pkg-config supports Requires.private
if $PKG_CONFIG --atleast-pkgconfig-version 0.22; then
   pkgconfig_requires_private="Requires.private"
else
   pkgconfig_requires_private="Requires"
fi
AC_SUBST(pkgconfig_requires_private)

#================================================
if test "x${prefix}" = "xNONE"; then
  LOCALE_DIR="${ac_default_prefix}/share/locale"
else
  LOCALE_DIR="${prefix}/share/locale"
fi
AC_SUBST(LOCALE_DIR)
#================================================
elementary_icon_dir="${datadir}/icons"
AC_SUBST(elementary_icon_dir)

have_fork="no"
want_quicklaunch="auto"
AC_ARG_ENABLE([quick-launch],
        [AC_HELP_STRING([--disable-quick-launch], [disable quick-launch support, @<:@default=detect@:>@])],
        [want_quicklaunch=$enableval], [])

# Check if we can build binary with quicklaunch support
ELM_QUICKLAUNCH

if test "x$want_quicklaunch" != "xno"; then
   AC_CHECK_FUNCS(fork, [
        have_fork="yes"
        AC_DEFINE(HAVE_FORK)
   ])
   AC_CHECK_FUNCS([clearenv])
fi
AM_CONDITIONAL([BUILD_QUICKLAUNCH], [test "x$have_fork" = "xyes" && test "x$have_dlopen" = "xyes"])

AC_CHECK_FUNCS([\
geteuid \
getuid \
])

AC_SUBST(dlopen_libs)
AC_SUBST(ELM_UNIX_DEF)
AC_SUBST(ELM_WIN32_DEF)

AM_CONDITIONAL([ELEMENTARY_WINDOWS_BUILD], [test "x${have_windows}" = "xyes"])

PKG_CHECK_MODULES([ELEMENTARY],
   [
    eina >= efl_version
    eet >= efl_version
    evas >= efl_version
    ecore >= efl_version
    ecore-evas >= efl_version
    ecore-file >= efl_version
    ecore-input >= efl_version
    edje >= efl_version
    eo >= efl_version
    ethumb_client >= efl_version
    emotion >= efl_version
    ecore-imf >= efl_version
    ecore-con >= efl_version
    eio >= efl_version
    eldbus >= efl_version
    efreet >= efl_version
    efreet-mime >= efl_version
    efreet-trash >= efl_version
   ]
)

EFL_ENABLE_EO_API_SUPPORT
EFL_ENABLE_BETA_API_SUPPORT

requirement_elm="\
    eina >= efl_version \
    eet >= efl_version \
    evas >= efl_version \
    ecore >= efl_version \
    ecore-evas >= efl_version \
    ecore-file >= efl_version \
    ecore-input >= efl_version \
    edje >= efl_version \
    eo >= efl_version \
    ethumb_client >= efl_version \
    emotion >= efl_version \
    ecore-imf >= efl_version \
    ecore-con >= efl_version \
    eio >= efl_version \
    eldbus >= efl_version \
    efreet >= efl_version \
    efreet-mime >= efl_version \
    efreet-trash >= efl_version \
    "

have_elementary_x="no"
want_elementary_x="auto"
AC_ARG_ENABLE([ecore-x],
   [AC_HELP_STRING([--disable-ecore-x], [disable ecore-x support. @<:@default=detect@:>@])],
   [want_elementary_x=$enableval], [])

if test "x$want_elementary_x" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_X],
      [ecore-x >= efl_version],
      [
       AC_DEFINE(HAVE_ELEMENTARY_X, 1, [X11 support for Elementary])
       have_elementary_x="yes"
       requirement_elm="ecore-x >= efl_version ${requirement_elm}"
      ],
      [have_elementary_x="no"]
   )
else
    have_elementary_x="no"
fi
if test "x$want_elementary_x" = "xyes" -a "x$have_elementary_x" = "xno"; then
    AC_MSG_ERROR([ecore-x support requested, but ecore-x was not found by pkg-config.])
fi


have_elementary_fb="no"
want_elementary_fb="auto"
AC_ARG_ENABLE([ecore-fb],
   [AC_HELP_STRING([--disable-ecore-fb], [disable ecore-fb support. @<:@default=detect@:>@])],
   [want_elementary_fb=$enableval], [])

if test "x$want_elementary_fb" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_FB],
      [ecore-fb >= efl_version],
      [
       AC_DEFINE(HAVE_ELEMENTARY_FB, 1, [FB support for Elementary])
       have_elementary_fb="yes"
       requirement_elm="ecore-fb >= efl_version ${requirement_elm}"
      ],
      [have_elementary_fb="no"]
   )
else
    have_elementary_fb="no"
fi
if test "x$want_elementary_fb" = "xyes" -a "x$have_elementary_fb" = "xno"; then
    AC_MSG_ERROR([ecore-fb support requested, but ecore-fb was not found by pkg-config.])
fi

have_elementary_psl1ght="no"
want_elementary_psl1ght="auto"
AC_ARG_ENABLE([ecore-psl1ght],
   [AC_HELP_STRING([--disable-ecore-psl1ght], [disable ecore-psl1ght support. @<:@default=detect@:>@])],
   [want_elementary_psl1ght=$enableval], [])

if test "x$want_elementary_psl1ght" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_PSL1GHT],
      [ecore-psl1ght >= efl_version],
      [
       AC_DEFINE(HAVE_ELEMENTARY_PSL1GHT, 1, [PSL1GHT support for Elementary])
       have_elementary_psl1ght="yes"
       requirement_elm="ecore-psl1ght >= efl_version ${requirement_elm}"
      ],
      [have_elementary_psl1ght="no"]
   )
else
    have_elementary_psl1ght="no"
fi
if test "x$want_elementary_psl1ght" = "xyes" -a "x$have_elementary_psl1ght" = "xno"; then
    AC_MSG_ERROR([ecore-psl1ght support requested, but ecore-psl1ght was not found by pkg-config.])
fi

have_elementary_sdl="no"
want_elementary_sdl="auto"
AC_ARG_ENABLE([ecore-sdl],
   [AC_HELP_STRING([--disable-ecore-sdl], [disable ecore-sdl support. @<:@default=detect@:>@])],
   [want_elementary_sdl=$enableval], [])

if test "x$want_elementary_sdl" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_SDL],
      [ecore-sdl >= efl_version],
      [
       AC_DEFINE(HAVE_ELEMENTARY_SDL, 1, [SDL support for Elementary])
       have_elementary_sdl="yes"
       requirement_elm="ecore-sdl >= efl_version ${requirement_elm}"
      ],
      [have_elementary_sdl="no"]
   )
else
    have_elementary_sdl="no"
fi
if test "x$want_elementary_sdl" = "xyes" -a "x$have_elementary_sdl" = "xno"; then
    AC_MSG_ERROR([ecore-sdl support requested, but ecore-sdl was not found by pkg-config.])
fi

have_elementary_cocoa="no"
want_elementary_cocoa="auto"
AC_ARG_ENABLE([ecore-cocoa],
   [AC_HELP_STRING([--disable-ecore-cocoa], [disable ecore-cocoa support. @<:@default=detect@:>@])],
   [want_elementary_cocoa=$enableval], [])

if test "x$want_elementary_cocoa" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_COCOA],
      [ecore-cocoa >= efl_version],
      [
       AC_DEFINE(HAVE_ELEMENTARY_COCOA, 1, [COCOA support for Elementary])
       have_elementary_cocoa="yes"
       requirement_elm="ecore-cocoa >= efl_version ${requirement_elm}"
      ],
      [have_elementary_cocoa="no"]
   )
else
    have_elementary_cocoa="no"
fi
if test "x$want_elementary_cocoa" = "xyes" -a "x$have_elementary_cocoa" = "xno"; then
    AC_MSG_ERROR([ecore-cocoa support requested, but ecore-cocoa was not found by pkg-config.])
fi

have_elementary_win32="no"
want_elementary_win32="auto"
AC_ARG_ENABLE([ecore-win32],
   [AC_HELP_STRING([--disable-ecore-win32], [disable ecore-win32 support. @<:@default=detect@:>@])],
   [want_elementary_win32=$enableval], [])

if test "x$want_elementary_win32" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_WIN32],
      [ecore-win32 >= efl_version],
      [
       AC_DEFINE(HAVE_ELEMENTARY_WIN32, 1, [Windows XP support for Elementary])
       have_elementary_win32="yes"
       requirement_elm="ecore-win32 >= efl_version ${requirement_elm}"
      ],
      [have_elementary_win32="no"]
   )
else
    have_elementary_win32="no"
fi
if test "x$want_elementary_win32" = "xyes" -a "x$have_elementary_win32" = "xno"; then
    AC_MSG_ERROR([ecore-win32 support requested, but ecore-win32 was not found by pkg-config.])
fi

have_elementary_wayland="no"
want_elementary_wayland="auto"
AC_ARG_ENABLE([ecore-wayland],
   [AC_HELP_STRING([--disable-ecore-wayland], [disable ecore-wayland support. @<:@default=detect@:>@])],
   [want_elementary_wayland=$enableval], [])

if test "x$want_elementary_wayland" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_WAYLAND],
      [ecore-wayland],
      [
       AC_DEFINE(HAVE_ELEMENTARY_WAYLAND, 1, [Wayland support for Elementary])
       have_elementary_wayland="yes"
       requirement_elm="ecore-wayland ${requirement_elm}"
      ],
      [have_elementary_wayland="no"]
   )
else
    have_elementary_wayland="no"
fi
if test "x$want_elementary_wayland" = "xyes" -a "x$have_elementary_wayland" = "xno"; then
    AC_MSG_ERROR([ecore-wayland support requested, but ecore-wayland was not found by pkg-config.])
fi

have_elementary_drm="no"
want_elementary_drm="auto"
AC_ARG_ENABLE([ecore-drm],
   [AC_HELP_STRING([--disable-ecore-drm], [disable ecore-drm support. @<:@default=detect@:>@])],
   [want_elementary_drm=$enableval], [])

if test "x$want_elementary_drm" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_DRM],
      [ecore-drm >= efl_version],
      [
       AC_DEFINE(HAVE_ELEMENTARY_DRM, 1, [DRM support for Elementary])
       have_elementary_drm="yes"
       requirement_elm="ecore-drm >= efl_version ${requirement_elm}"
      ],
      [have_elementary_drm="no"]
   )
else
    have_elementary_drm="no"
fi
if test "x$want_elementary_drm" = "xyes" -a "x$have_elementary_drm" = "xno"; then
    AC_MSG_ERROR([ecore-drm support requested, but ecore-drm was not found by pkg-config.])
fi


ELM_ELOCATION_DEF="#undef"
have_elementary_elocation="no"
want_elementary_elocation="auto"
AC_ARG_ENABLE([elocation],
   [AC_HELP_STRING([--disable-elocation], [disable elocation support. @<:@default=detect@:>@])],
   [want_elementary_elocation=$enableval], [])

if test "x$want_elementary_elocation" != "xno"; then
    PKG_CHECK_MODULES([ELEMENTARY_ELOCATION],
       [
        elocation >= 0.1.0
       ],
       [
        AC_DEFINE(HAVE_ELEMENTARY_ELOCATION, 1, [Elocation support for Elementary])
        have_elementary_elocation="yes"
        ELM_ELOCATION_DEF="#define"
        requirement_elm="elocation >= 0.1.0 ${requirement_elm}"
       ],
       [have_elementary_elocation="no"]
    )
else
    have_elementary_elocation="no"
fi
if test "x$want_elementary_elocation" = "xyes" -a "x$have_elementary_elocation" = "xno"; then
    AC_MSG_ERROR([elocation support requested, but elocation was not found by pkg-config.])
fi
AC_SUBST(ELM_ELOCATION_DEF)

ELM_EWEATHER_DEF="#undef"
have_elementary_eweather="no"
want_elementary_eweather="auto"
AC_ARG_ENABLE([eweather],
   [AC_HELP_STRING([--disable-eweather], [disable eweather support. @<:@default=detect@:>@])],
   [want_elementary_eweather=$enableval], [])

if test "x$want_elementary_eweather" != "xno"; then
    PKG_CHECK_MODULES([ELEMENTARY_EWEATHER],
       [
        eweather
       ],
       [
        AC_DEFINE(HAVE_ELEMENTARY_EWEATHER, 1, [EWeather support for Elementary])
        have_elementary_eweather="yes"
        ELM_EWEATHER_DEF="#define"
        requirement_elm="eweather ${requirement_elm}"
       ],
       [have_elementary_eweather="no"]
    )
else
    have_elementary_eweather="no"
fi
if test "x$want_elementary_eweather" = "xyes" -a "x$have_elementary_eweather" = "xno"; then
    AC_MSG_ERROR([Eweather support requested, but eweather was not found by pkg-config.])
fi
AC_SUBST(ELM_EWEATHER_DEF)

ELM_EMAP_DEF="#undef"
have_elementary_emap="no"
want_elementary_emap="auto"
AC_ARG_ENABLE([emap],
   [AC_HELP_STRING([--disable-emap], [disable emap support. @<:@default=detect@:>@])],
   [want_elementary_emap=$enableval], [])

if test "x$want_elementary_emap" != "xno"; then
    PKG_CHECK_MODULES([ELEMENTARY_EMAP],
       [
        emap
       ],
       [
        AC_DEFINE(HAVE_ELEMENTARY_EMAP, 1, [EMap support for Elementary])
        have_elementary_emap="yes"
        ELM_EMAP_DEF="#define"
        requirement_elm="emap ${requirement_elm}"
       ],
       [have_elementary_emap="no"]
    )
else
    have_elementary_emap="no"
fi
if test "x$want_elementary_emap" = "xyes" -a "x$have_elementary_emap" = "xno"; then
    AC_MSG_ERROR([Emap support requested, but emap was not found by pkg-config.])
fi
AC_SUBST(ELM_EMAP_DEF)

ELM_WEB_DEF="#undef"
ELM_WEB2_DEF="#undef"

elementary_web_backend="detect"
elementary_web="no"
want_elementary_web=""
AC_ARG_WITH([elementary-web-backend],
   [AC_HELP_STRING([--with-elementary-web-backend=@<:@ewebkit/ewebkit2/detect/none@:>@], [Select the WebKit backend for Elementary to support web @<:@default=detect@:>@])],
   [
    elementary_web_backend=${withval}
   ],
   [elementary_web_backend="detect"])

case "$elementary_web_backend" in
    ewebkit|ewebkit2|detect|none) ;;
    *) AC_MSG_ERROR([Invalid web backend: must be ewebkit, ewebkit2, detect or none]) ;;
esac

if test "x$elementary_web_backend" = "xdetect"; then
    # FIXME : detect option should check both ewebkit and ewebkit2.
    want_elementary_web="ewebkit"
else
    want_elementary_web=$elementary_web_backend
fi

if test "x$want_elementary_web" != "xnone"; then
    PKG_CHECK_MODULES([ELEMENTARY_WEB],
       [$want_elementary_web],
       [
        AC_DEFINE(HAVE_ELEMENTARY_WEB, 1, [Web support for Elementary])
        webkit_datadir=$($PKG_CONFIG $want_elementary_web --variable=datadir)
        AC_DEFINE_UNQUOTED(WEBKIT_DATADIR,
                           "$webkit_datadir",
                           "WebKit's data dir")
        if test "x$elementary_web_backend" = "xewebkit2"; then
            AC_DEFINE(USE_WEBKIT2, 1, [Use ewebkit2 as Elementary web backend])
            ELM_WEB2_DEF="#define"
        else
            ELM_WEB_DEF="#define"
        fi

        elementary_web=$want_elementary_web
        requirement_elm="${want_elementary_web} ${requirement_elm}"
       ],
       [elementary_web="no"]
    )
else
    elementary_web="no"
fi

if test "x$elementary_web_backend" = "xewebkit" -o "x$elementary_web_backend" = "xewebkit2"; then
    if test "x$elementary_web" = "xnone"; then
        AC_MSG_ERROR([Web support requested, but no $elementary_web_backend found by pkg-config.])
    fi
fi
AC_SUBST(ELM_WEB_DEF)
AC_SUBST(ELM_WEB2_DEF)

ELM_DEBUG_DEF="#undef"
want_elementary_debug="no"
AC_ARG_ENABLE([debug],
   [AC_HELP_STRING([--enable-debug], [enable elementary debug support. @<:@default=disabled@:>@])],
   [want_elementary_debug=$enableval], [])

if test "x$want_elementary_debug" = "xyes"; then
        AC_DEFINE(HAVE_ELEMENTARY_DEBUG, 1, [Elementary debug.])
        ELM_DEBUG_DEF="#define"
fi
AC_SUBST(ELM_DEBUG_DEF)

AC_ARG_WITH([tests],
   [AC_HELP_STRING([--with-tests=none|regular],
                   [choose elementary testing method: regular or none.(coverage will be supported)@<:@default=none@:>@])],
   [build_tests=${withval}],
   [build_tests=none])

if test "${build_tests}" = "regular"; then
   PKG_CHECK_MODULES([CHECK], [check >= 0.9.5])
fi
AM_CONDITIONAL([ENABLE_ELEMENTARY_TESTS], [test "${build_tests}" = "regular"])

ELM_ALLOCA_H_DEF="#undef"
AC_CHECK_HEADER(alloca.h, [ELM_ALLOCA_H_DEF="#define"])
AC_SUBST(ELM_ALLOCA_H_DEF)

ELM_LIBINTL_H_DEF="#undef"
AC_CHECK_HEADER(libintl.h, [ELM_LIBINTL_H_DEF="#define"])
AC_SUBST(ELM_LIBINTL_H_DEF)

ELM_DIRENT_H_DEF="#undef"
AC_CHECK_HEADER(dirent.h, [ELM_DIRENT_H_DEF="#define"])
AC_SUBST(ELM_DIRENT_H_DEF)

AC_CHECK_HEADER(sys/mman.h, [have_mman="yes"], [have_mman="no"])
if test "x${have_mman}" = "xyes"; then
   AC_DEFINE(HAVE_MMAN_H, 1, [Have sys/mman.h header file])
fi

AC_CHECK_HEADERS([locale.h langinfo.h sys/times.h])

my_libs="-lm"
AC_SUBST(my_libs)
AC_SUBST(requirement_elm)

EFL_WITH_BIN([eet], [eet-eet], [eet])
EFL_WITH_BIN([edje], [edje-cc], [edje_cc])
EFL_WITH_BIN([elementary], [elementary-codegen], [elementary_codegen])
EFL_WITH_BIN([elementary], [elm-prefs-cc], [elm_prefs_cc])
EFL_WITH_BIN([eolian], [eolian-gen], [eolian_gen])
# Force the helper to try external eolian-gen
AM_CONDITIONAL(HAVE_EOLIAN_GEN, [true])

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])

EFL_ENABLE_BIN([elementary-test])
EFL_ENABLE_BIN([elementary-codegen])
EFL_ENABLE_BIN([elm-prefs-cc])
EFL_ENABLE_BIN([elementary-config])

m4_ifdef([AM_GNU_GETTEXT_VERSION], [
AM_GNU_GETTEXT_VERSION([0.17])
])

m4_ifdef([AM_GNU_GETTEXT], [
AM_GNU_GETTEXT([external])
po_makefile_in=po/Makefile.in
have_po="yes"
],[
have_po="no"
])
AC_SUBST(LTLIBINTL)

if test "x${POSUB}" = "x" ; then
   have_po="no"
fi

AM_CONDITIONAL([HAVE_PO], [test "x${have_po}" = "xyes"])

EFL_CHECK_PATH_MAX

elementary_base_dir=".elementary"
AC_ARG_WITH([elementary-base-dir],
   [AC_HELP_STRING([--with-elementary-base-dir=PATH], [specify the subdirectory for all elementary data @<:@default=${elementary_base_dir}@:>@])],
   [elementary_base_dir=${withval}],
   [elementary_base_dir=".elementary"])

AC_MSG_NOTICE([ELEMENTARY_BASE_DIR set to ${elementary_base_dir}])
AC_DEFINE_UNQUOTED(ELEMENTARY_BASE_DIR, "${elementary_base_dir}", "subdirectory for all elementary data")

### Build and install examples
EFL_CHECK_BUILD_EXAMPLES([enable_build_examples="yes"], [enable_build_examples="no"])
EFL_CHECK_INSTALL_EXAMPLES([enable_install_examples="yes"], [enable_install_examples="no"])

AC_TRY_COMPILE([
# define _GNU_SOURCE 1
#include <unistd.h>
], [
extern char **environ;
], [
  AC_DEFINE(HAVE_ENVIRON, 1, [extern environ exists])
])

AC_CONFIG_FILES([
Makefile
elementary.spec
elementary.pc
doc/Makefile
doc/Doxyfile
doc/widgets/Makefile
src/Makefile
src/lib/Makefile
src/lib/Elementary.h
src/lib/elm_intro.h
src/bin/Makefile
src/modules/Makefile
src/modules/prefs/Makefile
src/modules/access_output/Makefile
src/modules/datetime_input_ctxpopup/Makefile
src/modules/test_entry/Makefile
src/modules/test_map/Makefile
src/edje_externals/Makefile
src/examples/Makefile
src/tests/Makefile
src/imported/Makefile
src/imported/atspi/Makefile
data/Makefile
data/themes/Makefile
data/images/Makefile
data/objects/Makefile
data/desktop/Makefile
data/edje_externals/Makefile
config/Makefile
config/default/Makefile
config/default/base.src
config/standard/Makefile
config/standard/base.src
config/mobile/Makefile
config/mobile/base.src
$po_makefile_in
cmakeconfig/ElementaryConfig.cmake
cmakeconfig/ElementaryConfigVersion.cmake
])

AC_OUTPUT

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
echo "  Engines:"
echo "    X11....................: ${have_elementary_x}"
echo "    Framebuffer............: ${have_elementary_fb}"
echo "    DRM....................: ${have_elementary_drm}"
echo "    PSL1GHT................: ${have_elementary_psl1ght}"
echo "    SDL....................: ${have_elementary_sdl}"
echo "    Cocoa..................: ${have_elementary_cocoa}"
echo "    Windows XP.............: ${have_elementary_win32}"
echo "    Wayland................: ${have_elementary_wayland}"
echo
echo "  Features:"
echo "    EWeather...............: ${have_elementary_eweather}"
echo "    EMap...................: ${have_elementary_emap}"
echo "    Web (WebKit)...........: ${elementary_web}"
echo "    Quick Launch...........: ${have_fork}"
echo "    Elocation..............: ${have_elementary_elocation}"
echo
echo "  eet......................: ${eet_eet}"
echo "  edje_cc..................: ${edje_cc}"
echo "  eolian_gen...............: ${eolian_gen}"
echo
echo "  Build elementary_test....: ${have_elementary_test}"
echo "  Build elementary_codegen.: ${have_elementary_codegen}"
echo "  Build elm_prefs_cc.......: ${have_elm_prefs_cc}"
echo "  Examples.................: ${enable_build_examples}"
echo "  Examples installed.......: ${enable_install_examples}"
echo "  Build elementary_config..: ${have_elementary_config}"
echo "  Enable debug.............: ${want_elementary_debug}"
echo
echo "Compilation................: make (or gmake)"
echo "  CPPFLAGS.................: $CPPFLAGS"
echo "  CFLAGS...................: $CFLAGS"
echo "  LDFLAGS..................: $LDFLAGS"
echo
if test "${build_tests}" = "none"; then
echo "Tests......................: no"
else
echo "Tests......................: make check"
fi
if test "${build_doc}" = "no"; then
echo "Docs.......................: no (doxygen not found)"
else
echo "Docs.......................: make doc"
fi
echo
echo "Installation...............: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...................: $prefix"
echo

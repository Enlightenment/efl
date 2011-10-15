=-------##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_define([v_maj], [0])
m4_define([v_min], [7])
m4_define([v_mic], [0])
m4_define([v_rev], m4_esyscmd([(svnversion "${SVN_REPO_PATH:-.}" | grep -v export || echo 0) | awk -F : '{printf("%s\n", $1);}' | tr -d ' :MSP\n']))
m4_if(v_rev, [0], [m4_define([v_rev], m4_esyscmd([git log 2> /dev/null | (grep -m1 git-svn-id || echo 0) | sed -e 's/.*@\([0-9]*\).*/\1/' | tr -d '\n']))])
##--   When released, remove the dnl on the below line
dnl m4_undefine([v_rev])
##--   When doing snapshots - change soname. remove dnl on below line
m4_define([relname], [ver-pre-svn-09])
m4_define([v_rel], [-release relname])
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], [m4_define([v_ver], [v_maj.v_min.v_mic.v_rev])],
[m4_define([v_ver], [v_maj.v_min.v_mic])])
m4_define([lt_rev], m4_eval(v_maj + v_min))
m4_define([lt_cur], v_mic)
m4_define([lt_age], v_min)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##

AC_INIT([elementary], [v_ver], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ(2.52)
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

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_GNU_SOURCE

AC_C_BIGENDIAN
AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST
AC_C___ATTRIBUTE__
AC_FUNC_ALLOCA

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
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
version_info="lt_rev:lt_cur:lt_age"
release_info="v_rel"
AC_SUBST(version_info)
AC_SUBST(release_info)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
VMAJ=v_maj
VMIN=v_min
AC_SUBST(VMAJ)
AC_SUBST(VMIN)

# pkg-config
PKG_PROG_PKG_CONFIG
EDJE_VERSION=`$PKG_CONFIG edje --modversion | awk -F . '{printf("%s.0.0", $1);}'`

case "$host_os" in
  mingw32ce* | cegcc*)
    MODULE_ARCH="$host_os-$host_cpu"
    MODULE_EDJE="$host_os-$host_cpu"
    ;;
  *)
    MODULE_ARCH="$host_os-$host_cpu-v_maj.v_min.v_mic"
    MODULE_EDJE="$host_os-$host_cpu-${EDJE_VERSION}"
    ;;
esac
AC_SUBST(release_info)
AC_SUBST(MODULE_ARCH)
AC_DEFINE_UNQUOTED(MODULE_ARCH, "$MODULE_ARCH", "Module architecture")
AC_SUBST(MODULE_EDJE)
AC_DEFINE_UNQUOTED(MODULE_EDJE, "$MODULE_EDJE", "Edje module architecture")

EFL_SHARED_EXTENSION="${shrext_cmds}"
AC_DEFINE_UNQUOTED(EFL_SHARED_EXTENSION, "${EFL_SHARED_EXTENSION}", "Shared extension")

requirement_elm=""

PKG_PROG_PKG_CONFIG

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

lt_enable_auto_import=""
ELM_UNIX_DEF="#undef"
ELM_WIN32_DEF="#undef"
ELM_WINCE_DEF="#undef"
have_windows="no"
case "$host_os" in
   mingw32ce* | cegcc*)
      PKG_CHECK_MODULES([EVIL], [evil])
      AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if evil package is installed.])
      lt_enable_auto_import="-Wl,--enable-auto-import"
      ELM_WINCE_DEF="#define"
      have_windows="yes"
dnl managed by evil
      AC_DEFINE(HAVE_DLADDR)
      dlopen_libs=-ldl
      requirement_elm="evil"
      have_socket="no"
    ;;
   mingw*)
      PKG_CHECK_MODULES([EVIL], [evil])
      AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if evil package is installed.])
      lt_enable_auto_import="-Wl,--enable-auto-import"
      ELM_WIN32_DEF="#define"
      have_windows="yes"
dnl managed by evil
      AC_DEFINE(HAVE_DLADDR)
      dlopen_libs=-ldl
      requirement_elm="evil"
      have_socket="no"
      ;;
   *solaris*)
      ELM_UNIX_DEF="#define"
      have_socket="yes"
      AC_CHECK_LIB([socket], [connect], [], [have_socket="no"])
      ;;
   darwin*)
      ELM_UNIX_DEF="#define"
      have_socket="yes"
      AC_CHECK_HEADERS([crt_externs.h])
      AC_DEFINE([environ], [(*_NSGetEnviron())], ["apple doesn't follow POSIX in this case."])
      ;;
   *)
      ELM_UNIX_DEF="#define"
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
      have_socket="yes"
      ;;
esac
AM_CONDITIONAL([BUILD_RUN], [test "x$have_socket" = "xyes"])

m4_ifdef([v_mic],
   [
    EFL_COMPILER_FLAG([-Wall])
    EFL_COMPILER_FLAG([-W])
   ])

have_fork="no"
want_quicklaunch="auto"
AC_ARG_ENABLE([quick-launch],
	[AC_HELP_STRING([--disable-quick-launch], [disable quick-launch support, @<:@default=detect@:>@])],
	[want_quicklaunch=$enableval], [])

if test "x$want_quicklaunch" != "xno"; then
   AC_CHECK_FUNCS(fork, [
   	have_fork="yes"
  	AC_DEFINE(HAVE_FORK)
   ])
fi
AM_CONDITIONAL([BUILD_QUICKLAUNCH], [test "x$have_fork" = "xyes"])

AC_SUBST(dlopen_libs)
AC_SUBST(lt_enable_auto_import)
AC_SUBST(ELM_UNIX_DEF)
AC_SUBST(ELM_WIN32_DEF)
AC_SUBST(ELM_WINCE_DEF)

AM_CONDITIONAL([ELEMENTARY_WINDOWS_BUILD], [test "x${have_windows}" = "xyes"])

PKG_CHECK_MODULES([ELEMENTARY],
   [
    eina >= 1.0.999
    eet >= 1.4.0
    evas >= 1.0.999
    ecore >= 1.0.999
    ecore-evas >= 1.0.999
    ecore-file >= 1.0.999
    ecore-imf >= 1.0.999
    ecore-con >= 1.0.999
    edje >= 1.0.999
   ]
)

PKG_CHECK_MODULES([EIO],
	[eio],
	[
	 have_eio="yes"
	 AC_DEFINE(HAVE_EIO, 1, [Use EIO for asynchronous file access])
	 requirement_elm="eio ${requirement_elm}"
	],
	[have_eio="no"])

AM_CONDITIONAL([HAVE_EIO], [test "x${have_eio}" = "xyes"])

have_emotion="no"
want_elementary_emotion="yes"
AC_ARG_ENABLE([emotion],
   [AC_HELP_STRING([--disable-emotion], [disable emotion support. @<:@default=enabled@:>@])],
   [want_elementary_emotion=$enableval], [])

if test "x$want_elementary_emotion" != "xno"; then
   PKG_CHECK_MODULES([EMOTION],
	[emotion],
	[
	 have_emotion="yes"
	 AC_DEFINE(HAVE_EMOTION, 1, [Use Emotion for asynchronous file access])
	 requirement_elm="emotion ${requirement_elm}"
	],
	[have_emotion="no"])
fi

AM_CONDITIONAL([HAVE_EMOTION], [test "x${have_emotion}" = "xyes"])

requirement_elm="edje >= 1.0.0 ecore-file >= 1.0.0 ecore-evas >= 1.0.0 ecore >= 1.0.0 evas >= 1.0.0 eet >= 1.4.0 eina >= 1.0.0 ${requirement_elm}"

have_elementary_x="no"
want_elementary_x="auto"
AC_ARG_ENABLE([ecore-x],
   [AC_HELP_STRING([--disable-ecore-x], [disable ecore-x support. @<:@default=detect@:>@])],
   [want_elementary_x=$enableval], [])

if test "x$want_elementary_x" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_X],
      [ecore-x >= 1.0.0],
      [
       AC_DEFINE(HAVE_ELEMENTARY_X, 1, [X11 support for Elementary])
       have_elementary_x="yes"
       requirement_elm="ecore-x >= 1.0.0 ${requirement_elm}"
      ],
      [have_elementary_x="no"]
   )
else
    have_elementary_x="no"
fi
if test "x$want_elementary_x" = "xyes" -a "x$have_elementary_x" = "xno"; then
    AC_MSG_ERROR([ecore-x support requested, but not found by pkg-config.])
fi


have_elementary_fb="no"
want_elementary_fb="auto"
AC_ARG_ENABLE([ecore-fb],
   [AC_HELP_STRING([--disable-ecore-fb], [disable ecore-fb support. @<:@default=detect@:>@])],
   [want_elementary_fb=$enableval], [])

if test "x$want_elementary_fb" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_FB],
      [ecore-fb >= 1.0.0],
      [
       AC_DEFINE(HAVE_ELEMENTARY_FB, 1, [FB support for Elementary])
       have_elementary_fb="yes"
       requirement_elm="ecore-fb >= 1.0.0 ${requirement_elm}"
      ],
      [have_elementary_fb="no"]
   )
else
    have_elementary_fb="no"
fi
if test "x$want_elementary_fb" = "xyes" -a "x$have_elementary_fb" = "xno"; then
    AC_MSG_ERROR([ecore-fb support requested, but not found by pkg-config.])
fi

have_elementary_sdl="no"
want_elementary_sdl="auto"
AC_ARG_ENABLE([ecore-sdl],
   [AC_HELP_STRING([--disable-ecore-sdl], [disable ecore-sdl support. @<:@default=detect@:>@])],
   [want_elementary_sdl=$enableval], [])

if test "x$want_elementary_sdl" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_SDL],
      [ecore-sdl >= 1.0.0],
      [
       AC_DEFINE(HAVE_ELEMENTARY_SDL, 1, [SDL support for Elementary])
       have_elementary_sdl="yes"
       requirement_elm="ecore-sdl >= 1.0.0 ${requirement_elm}"
      ],
      [have_elementary_sdl="no"]
   )
else
    have_elementary_sdl="no"
fi
if test "x$want_elementary_sdl" = "xyes" -a "x$have_elementary_sdl" = "xno"; then
    AC_MSG_ERROR([ecore-sdl support requested, but not found by pkg-config.])
fi

have_elementary_win32="no"
want_elementary_win32="auto"
AC_ARG_ENABLE([ecore-win32],
   [AC_HELP_STRING([--disable-ecore-win32], [disable ecore-win32 support. @<:@default=detect@:>@])],
   [want_elementary_win32=$enableval], [])

if test "x$want_elementary_win32" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_WIN32],
      [ecore-win32 >= 1.0.0],
      [
       AC_DEFINE(HAVE_ELEMENTARY_WIN32, 1, [Windows XP support for Elementary])
       have_elementary_win32="yes"
       requirement_elm="ecore-win32 >= 1.0.0 ${requirement_elm}"
      ],
      [have_elementary_win32="no"]
   )
else
    have_elementary_win32="no"
fi
if test "x$want_elementary_win32" = "xyes" -a "x$have_elementary_win32" = "xno"; then
    AC_MSG_ERROR([ecore-win32 support requested, but not found by pkg-config.])
fi

have_elementary_wince="no"
want_elementary_wince="auto"
AC_ARG_ENABLE([ecore-wince],
   [AC_HELP_STRING([--disable-ecore-wince], [disable ecore-wince support. @<:@default=detect@:>@])],
   [want_elementary_wince=$enableval], [])

if test "x$want_elementary_wince" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_WINCE],
      [ecore-wince >= 1.0.0],
      [
       AC_DEFINE(HAVE_ELEMENTARY_WINCE, 1, [Windows CE support for Elementary])
       have_elementary_wince="yes"
       requirement_elm="ecore-wince >= 1.0.0 ${requirement_elm}"
      ],
      [have_elementary_wince="no"]
   )
else
    have_elementary_wince="no"
fi
if test "x$want_elementary_wince" = "xyes" -a "x$have_elementary_wince" = "xno"; then
    AC_MSG_ERROR([ecore-wince support requested, but not found by pkg-config.])
fi

ELM_EDBUS_DEF="#undef"
have_elementary_edbus="no"
want_elementary_edbus="auto"
AC_ARG_ENABLE([edbus],
   [AC_HELP_STRING([--disable-edbus], [disable edbus support. @<:@default=detect@:>@])],
   [want_elementary_edbus=$enableval], [])

if test "x$want_elementary_edbus" != "xno"; then
    PKG_CHECK_MODULES([ELEMENTARY_EDBUS],
       [
        edbus >= 1.0.0
       ],
       [
        AC_DEFINE(HAVE_ELEMENTARY_EDBUS, 1, [EDBus support for Elementary])
        have_elementary_edbus="yes"
        ELM_EDBUS_DEF="#define"
        requirement_elm="edbus >= 1.0.0 ${requirement_elm}"
       ],
       [have_elementary_edbus="no"]
    )
else
    have_elementary_edbus="no"
fi
if test "x$want_elementary_edbus" = "xyes" -a "x$have_elementary_edbus" = "xno"; then
    AC_MSG_ERROR([E_DBus support requested, but no e_dbus found by pkg-config.])
fi
AC_SUBST(ELM_EDBUS_DEF)

ELM_EFREET_DEF="#undef"
have_elementary_efreet="no"
want_elementary_efreet="auto"
AC_ARG_ENABLE([efreet],
   [AC_HELP_STRING([--disable-efreet], [disable efreet support. @<:@default=detect@:>@])],
   [want_elementary_efreet=$enableval], [])

if test "x$want_elementary_efreet" != "xno"; then
    PKG_CHECK_MODULES([ELEMENTARY_EFREET],
       [
        efreet >= 1.0.0
        efreet-mime >= 1.0.0
        efreet-trash >= 1.0.0
       ],
       [
        AC_DEFINE(HAVE_ELEMENTARY_EFREET, 1, [Efreet support for Elementary])
        have_elementary_efreet="yes"
        ELM_EFREET_DEF="#define"
        requirement_elm="efreet >= 1.0.0 efreet-mime >= 1.0.0 efreet-trash >= 1.0.0 ${requirement_elm}"
       ],
       [have_elementary_efreet="no"]
    )
else
    have_elementary_efreet="no"
fi
if test "x$want_elementary_efreet" = "xyes" -a "x$have_elementary_efreet" = "xno"; then
    AC_MSG_ERROR([Efreet support requested, but no efreet/efreet-mime/efreet-trash found by pkg-config.])
fi
AC_SUBST(ELM_EFREET_DEF)

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
    AC_MSG_ERROR([Eweather support requested, but no eweather found by pkg-config.])
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
    AC_MSG_ERROR([Emap support requested, but no emap found by pkg-config.])
fi
AC_SUBST(ELM_EMAP_DEF)



ELM_ETHUMB_DEF="#undef"
have_elementary_ethumb="no"
want_elementary_ethumb="auto"
AC_ARG_ENABLE([ethumb],
   [AC_HELP_STRING([--disable-ethumb], [disable ethumb support. @<:@default=detect@:>@])],
   [want_elementary_ethumb=$enableval], [])

if test "x$want_elementary_ethumb" != "xno"; then
    PKG_CHECK_MODULES([ELEMENTARY_ETHUMB],
       [
        ethumb_client
       ],
       [
        AC_DEFINE(HAVE_ELEMENTARY_ETHUMB, 1, [Ethumb support for Elementary])
        have_elementary_ethumb="yes"
        ELM_ETHUMB_DEF="#define"
        requirement_elm="ethumb_client ${requirement_elm}"
       ],
       [have_elementary_ethumb="no"]
    )
else
    have_elementary_ethumb="no"
fi
if test "x$want_elementary_ethumb" = "xyes" -a "x$have_elementary_ethumb" = "xno"; then
    AC_MSG_ERROR([Ethumb support requested, but no ethumb found by pkg-config.])
fi
AC_SUBST(ELM_ETHUMB_DEF)

ELM_WEB_DEF="#undef"
have_elementary_web="no"
want_elementary_web="auto"
AC_ARG_ENABLE([web],
   [AC_HELP_STRING([--disable-web], [disable web (WebKit) support. @<:@default=detect@:>@])],
   [want_elementary_web=$enableval], [])

if test "x$want_elementary_web" != "xno"; then
    PKG_CHECK_MODULES([ELEMENTARY_WEB],
       [
        ewebkit
       ],
       [
        AC_DEFINE(HAVE_ELEMENTARY_WEB, 1, [Web support for Elementary])
        webkit_datadir=$($PKG_CONFIG ewebkit --variable=datadir)
        AC_DEFINE_UNQUOTED(WEBKIT_DATADIR,
                           "$webkit_datadir",
                           "WebKit's data dir")
        have_elementary_web="yes"
        ELM_WEB_DEF="#define"
## web_client? how about ewebkit? at least in current upstreak webkit
## there is no web_client.pc        
#        requirement_elm="web_client ${requirement_elm}"
        requirement_elm="ewebkit ${requirement_elm}"
       ],
       [have_elementary_web="no"]
    )
else
    have_elementary_web="no"
fi
if test "x$want_elementary_web" = "xyes" -a "x$have_elementary_web" = "xno"; then
    AC_MSG_ERROR([Web support requested, but no ewebkit found by pkg-config.])
fi
AC_SUBST(ELM_WEB_DEF)

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

ELM_ALLOCA_H_DEF="#undef"
AC_CHECK_HEADER(alloca.h, [ELM_ALLOCA_H_DEF="#define"])
AC_SUBST(ELM_ALLOCA_H_DEF)

ELM_LIBINTL_H_DEF="#undef"
AC_CHECK_HEADER(libintl.h, [ELM_LIBINTL_H_DEF="#define"])
AC_SUBST(ELM_LIBINTL_H_DEF)

EFL_CHECK_THREADS([
  TH=1
] , [
  AC_MSG_ERROR(no thread support found. required.)
  exit 1
])
AC_SUBST(EFL_PTHREAD_CFLAGS)
AC_SUBST(EFL_PTHREAD_LIBS)

my_libs="-lm"
AC_SUBST(my_libs)
AC_SUBST(requirement_elm)

EFL_WITH_BIN([eet], [eet-eet], [eet])
EFL_WITH_BIN([edje], [edje-cc], [edje_cc])

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])

EFL_ENABLE_BIN([elementary-test])
EFL_ENABLE_BIN([elementary-config])

m4_ifdef([AM_GNU_GETTEXT_VERSION], [
AM_GNU_GETTEXT_VERSION([0.17])
])

m4_ifdef([AM_GNU_GETTEXT], [
AM_GNU_GETTEXT([external])
po_makefile_in=po/Makefile.in
AM_CONDITIONAL([HAVE_PO], [true])
],[
AM_CONDITIONAL([HAVE_PO], [false])
])
AC_SUBST(LTLIBINTL)
EFL_CHECK_PATH_MAX


### Build and install examples
EFL_CHECK_BUILD_EXAMPLES([enable_build_examples="yes"], [enable_build_examples="no"])
EFL_CHECK_INSTALL_EXAMPLES([enable_install_examples="yes"], [enable_install_examples="no"])


AC_OUTPUT([
Makefile
elementary.spec
elementary.pc
doc/Makefile
doc/Doxyfile
doc/widgets/Makefile
src/Makefile
src/lib/Makefile
src/lib/Elementary.h
src/bin/Makefile
src/modules/Makefile
src/modules/access_output/Makefile
src/modules/test_entry/Makefile
src/modules/test_map/Makefile
src/edje_externals/Makefile
src/examples/Makefile
data/Makefile
data/themes/Makefile
data/images/Makefile
data/objects/Makefile
data/desktop/Makefile
data/edje_externals/Makefile
config/Makefile
config/default/Makefile
config/standard/Makefile
config/illume/Makefile
$po_makefile_in
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
echo "  Engines:"
echo "    X11....................: ${have_elementary_x}"
echo "    Framebuffer............: ${have_elementary_fb}"
echo "    SDL....................: ${have_elementary_sdl}"
echo "    Windows XP.............: ${have_elementary_win32}"
echo "    Windows CE.............: ${have_elementary_wince}"
echo
echo "  Features:"
echo "    EDBus..................: ${have_elementary_edbus}"
echo "    EFreet.................: ${have_elementary_efreet}"
echo "    EWeather...............: ${have_elementary_eweather}"
echo "    EMap...................: ${have_elementary_emap}"
echo "    Ethumb.................: ${have_elementary_ethumb}"
echo "    Web (WebKit)...........: ${have_elementary_web}"
echo "    Quick Launch...........: ${have_fork}"
echo "    EIO....................: ${have_eio}"
echo "    Emotion................: ${have_emotion}"
echo
echo "  eet......................: ${eet_eet}"
echo "  edje_cc..................: ${edje_cc}"
echo
echo "  Build elementary_test....: ${have_elementary_test}"
echo "  Examples.................: ${enable_build_examples}"
echo "  Examples installed.......: ${enable_install_examples}"
echo "  Build elementary_config..: ${have_elementary_config}"
echo
echo "Compilation................: make (or gmake)"
echo "  CPPFLAGS.................: $CPPFLAGS"
echo "  CFLAGS...................: $CFLAGS"
echo "  LDFLAGS..................: $LDFLAGS"
echo
echo "Installation...............: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...................: $prefix"
echo

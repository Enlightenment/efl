# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT([elementary], [0.5.1.0], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
AM_CONFIG_HEADER([elementary_config.h])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_C_BIGENDIAN
AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST
AC_FUNC_ALLOCA

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

VMAJ=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $1);}'`
VMIN=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $2);}'`
VMIC=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $3);}'`
SNAP=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $4);}'`
version_info=`expr $VMAJ + $VMIN`":$VMIC:$VMIN"
AC_SUBST(version_info)

release="ver-pre-svn-04"
case "$host_os" in
  mingw32ce* | cegcc*)
    MODULE_ARCH="$host_os-$host_cpu"
    ;;
  *)
    release_info="-release $release"
    MODULE_ARCH="$host_os-$host_cpu-$release"
    ;;
esac
AC_SUBST(release_info)
AC_SUBST(MODULE_ARCH)
AC_DEFINE_UNQUOTED(MODULE_ARCH, "$MODULE_ARCH", "Module architecture")

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
      requirement_elm="evil"
      have_socket="no"
    ;;
   mingw*)
      PKG_CHECK_MODULES([EVIL], [evil])
      AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if evil package is installed.])
      lt_enable_auto_import="-Wl,--enable-auto-import"
      ELM_WIN32_DEF="#define"
      have_windows="yes"
      requirement_elm="evil"
      have_socket="no"
      ;;
   *solaris*)
      ELM_UNIX_DEF="#define"
      have_socket="yes"
      AC_CHECK_LIB([socket], [connect], [], [have_socket="no"])
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

have_fork="no"
AC_CHECK_FUNCS(fork, [
  have_fork="yes"
  AC_DEFINE(HAVE_FORK)
])
AM_CONDITIONAL([BUILD_QUICKLAUNCH], [test "x$have_fork" = "xyes"])

AC_SUBST(dlopen_libs)
AC_SUBST(lt_enable_auto_import)
AC_SUBST(ELM_UNIX_DEF)
AC_SUBST(ELM_WIN32_DEF)
AC_SUBST(ELM_WINCE_DEF)

AM_CONDITIONAL([ELEMENTARY_WINDOWS_BUILD], [test "x${have_windows}" = "xyes"])

PKG_CHECK_MODULES([ELEMENTARY],
   [
    eina-0 >= 0.0.1
    eet >= 1.1.0
    evas >= 0.9.9
    ecore >= 0.9.9
    ecore-evas >= 0.9.9
    ecore-job >= 0.9.9
    ecore-txt >= 0.9.9
    ecore-file >= 0.9.9
    edje >= 0.9.91
   ]
)

requirement_elm="edje ecore-file ecore-txt ecore-job ecore-evas ecore evas eet eina-0 ${requirement_elm}"

have_elementary_x="no"
PKG_CHECK_MODULES([ELEMENTARY_X],
   [ecore-x],
   [
    AC_DEFINE(HAVE_ELEMENTARY_X, 1, [X11 support for Elementary])
    have_elementary_x="yes"
    requirement_elm="ecore-x ${requirement_elm}"
   ],
   [have_elementary_x="no"]
)

have_elementary_fb="no"
PKG_CHECK_MODULES([ELEMENTARY_FB],
   [ecore-fb],
   [
    AC_DEFINE(HAVE_ELEMENTARY_FB, 1, [FB support for Elementary])
    have_elementary_fb="yes"
    requirement_elm="ecore-fb ${requirement_elm}"
   ],
   [have_elementary_fb="no"]
)

have_elementary_win32="no"
PKG_CHECK_MODULES([ELEMENTARY_WIN32],
   [ecore-win32],
   [
    AC_DEFINE(HAVE_ELEMENTARY_WIN32, 1, [Windows XP support for Elementary])
    have_elementary_win32="yes"
    requirement_elm="ecore-win32 ${requirement_elm}"
   ],
   [have_elementary_win32="no"]
)

have_elementary_wince="no"
PKG_CHECK_MODULES([ELEMENTARY_WINCE],
   [ecore-wince],
   [
    AC_DEFINE(HAVE_ELEMENTARY_WINCE, 1, [Windows CE support for Elementary])
    have_elementary_wince="yes"
    requirement_elm="ecore-wince ${requirement_elm}"
   ],
   [have_elementary_wince="no"]
)

ELM_EDBUS_DEF="#undef"
have_elementary_edbus="no"
PKG_CHECK_MODULES([ELEMENTARY_EDBUS],
   [
    edbus
    ehal
   ],
   [
    AC_DEFINE(HAVE_ELEMENTARY_EDBUS, 1, [EDBus support for Elementary])
    have_elementary_edbus="yes"
    ELM_EDBUS_DEF="#define"
    requirement_elm="ehal edbus ${requirement_elm}"
   ],
   [have_elementary_edbus="no"]
)
AC_SUBST(ELM_EDBUS_DEF)

ELM_EFREET_DEF="#undef"
have_elementary_efreet="no"
PKG_CHECK_MODULES([ELEMENTARY_EFREET],
   [
    efreet
    efreet-mime
    efreet-trash
   ],
   [
    AC_DEFINE(HAVE_ELEMENTARY_EFREET, 1, [Efreet support for Elementary])
    have_elementary_efreet="yes"
    ELM_EFREET_DEF="#define"
    requirement_elm="efreet efreet-mime efreet-trash ${requirement_elm}"
   ],
   [have_elementary_efreet="no"]
)
AC_SUBST(ELM_EFREET_DEF)

ELM_EWEATHER_DEF="#undef"
have_elementary_eweather="no"
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
AC_SUBST(ELM_EWEATHER_DEF)

ELM_ALLOCA_H_DEF="#undef"
AC_CHECK_HEADER(alloca.h, [ELM_ALLOCA_H_DEF="#define"])
AC_SUBST(ELM_ALLOCA_H_DEF)

ELM_LIBINTL_H_DEF="#undef"
AC_CHECK_HEADER(libintl.h, [ELM_LIBINTL_H_DEF="#define"])
AC_SUBST(ELM_LIBINTL_H_DEF)

AC_ARG_WITH(edje-cc,
[  --with-edje-cc=PATH              specify a specific path to edje_cc],
[
  v=$withval;
  edje_cc=$v
  echo "  Enlightenment edje_cc explicitly set to "$edje_cc;
],[
  edje_cc=$(pkg-config --variable=prefix edje)/bin/edje_cc
])
AC_SUBST(edje_cc)

my_libs="-lm"
AC_SUBST(my_libs)
AC_SUBST(requirement_elm)

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])

AC_OUTPUT([
Makefile
elementary.pc
doc/elementary.dox
doc/Makefile
src/Makefile
src/lib/Makefile
src/lib/Elementary.h
src/bin/Makefile
data/Makefile
data/themes/Makefile
data/images/Makefile
data/objects/Makefile
data/desktop/Makefile
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
echo "  X11...........: ${have_elementary_x}"
echo "  Framebuffer...: ${have_elementary_fb}"
echo "  Windows XP....: ${have_elementary_win32}"
echo "  Windows CE....: ${have_elementary_wince}"
echo
echo "Features:"
echo "  EDBus.........: ${have_elementary_edbus}"
echo "  EFreet........: ${have_elementary_efreet}"
echo "  EWeather......: ${have_elementary_eweather}"
echo
echo "edje_cc.........: ${edje_cc}"
echo
echo "------------------------------------------------------------------------"
echo
echo "Now type 'make' ('gmake' on some systems) to compile $PACKAGE,"
echo "and then afterwards as root (or the user who will install this), type"
echo "'make install'. Change users with 'su' or 'sudo' appropriately."
echo

##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_define([v_maj], [1])
m4_define([v_min], [9])
m4_define([v_mic], [0])
dnl m4_define([v_rev], m4_esyscmd([(git rev-list --count HEAD 2>/dev/null || echo 0) | tr -d '\n']))
##--   When released, remove the dnl on the below line
dnl m4_undefine([v_rev])
##--   When doing snapshots - change soname. remove dnl on below line
m4_define([relname], [beta1])
dnl m4_define([v_rel], [-release relname])
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], [m4_define([v_ver], [v_maj.v_min.v_mic.v_rev])],
[m4_define([v_ver], [v_maj.v_min.v_mic])])
m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], v_mic)
m4_define([lt_age], v_min)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##

AC_INIT([emotion_generic_players], [v_ver-beta1], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.59])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])

AC_USE_SYSTEM_EXTENSIONS
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_LANG([C])
AC_PROG_CC_C99
AM_PROG_CC_C_O

PKG_PROG_PKG_CONFIG
if test "x${PKG_CONFIG}" = "x" ; then
   AC_MSG_ERROR([pkg-config tool not found. Install it or set PKG_CONFIG environment variable to that path tool. Exiting...])
fi

#### Additional options to configure

AC_ARG_WITH([profile],
   [AC_HELP_STRING([--with-profile=PROFILE],
                   [use the predefined build profile, one of: dev, debug and release.
                    @<:@default=dev@:>@])],
   [build_profile=${withval}],
   [build_profile=dev])

case "${build_profile}" in
   dev|debug|release)
     ;;
   *)
     AC_MSG_ERROR([Unknown build profile --with-profile=${build_profile}])
     ;;
esac

with_max_log_level=""
case "${build_profile}" in
   release)
     with_max_log_level="3"
     ;;
esac

if test -n "${with_max_log_level}"; then
   AC_MSG_NOTICE([ignoring any EINA_LOG() with level greater than ${with_max_log_level}])
   AC_DEFINE_UNQUOTED([EINA_LOG_LEVEL_MAXIMUM], [${with_max_log_level}], [if set, logging is limited to this amount.])
fi


#### Checks for compiler characteristics

EFL_CHECK_COMPILER_FLAGS([EFL], [-Wall -Wextra -Wpointer-arith -Wno-missing-field-initializers -fvisibility=hidden -fdata-sections -ffunction-sections])
EFL_CHECK_LINKER_FLAGS([EFL], [-fvisibility=hidden -fdata-sections -ffunction-sections -Wl,--gc-sections -fno-strict-aliasing -Wl,--as-needed])
case "${build_profile}" in
   dev)
     EFL_CHECK_COMPILER_FLAGS([EFL], [-Wshadow])
     ;;
esac


#### Main dependency: emotion

PKG_CHECK_MODULES([EMOTION], [emotion >= v_maj.v_min])

MODULE_ARCH=`pkg-config --variable=module_arch emotion`
if test -z "${MODULE_ARCH}"; then
   EMOTION_VERSION=`pkg-config --modversion emotion | cut -d. -f1-2`
   case "$host_os" in
      mingw32ce*)
         MODULE_ARCH="v"
      ;;
      *)
         MODULE_ARCH="v-${EMOTION_VERSION}"
      ;;
   esac
fi

EMOTION_GENERIC_PLAYERS_DIR=`pkg-config --variable=generic_players emotion`
if test -z "${EMOTION_GENERIC_PLAYERS_DIR}"; then
   EMOTION_GENERIC_PLAYERS_DIR="`pkg-config --variable=libdir emotion`/emotion/generic_players"
fi
EMOTION_GENERIC_PLAYERS_DIR="${EMOTION_GENERIC_PLAYERS_DIR}/${MODULE_ARCH}"

AC_MSG_CHECKING([where to install generic players])
AC_MSG_RESULT([${EMOTION_GENERIC_PLAYERS_DIR}])
AC_SUBST(EMOTION_GENERIC_PLAYERS_DIR)
AC_SUBST(MODULE_ARCH)

#### Players
EMOTION_GENERIC_PLAYER([VLC], [yes], [libvlc >= 2.0 eina >= 1.1.99 ecore >= 1.7.99])


AC_CONFIG_FILES([
Makefile
pkgbuild/PKGBUILD
])

AC_OUTPUT

echo
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE_NAME $PACKAGE_VERSION"
echo "------------------------------------------------------------------------"
echo
echo "Configuration Options Summary:"
echo "  Build Profile..........: ${build_profile}"
echo
echo "Players:"
echo "  VLC....................: ${with_vlc}"
echo
echo "Compilation..............: make (or gmake)"
echo "  CPPFLAGS...............: $CPPFLAGS"
echo "  CFLAGS.................: $CFLAGS"
echo "  CXXFLAGS...............: $CXXFLAGS"
echo "  LDFLAGS................: $LDFLAGS"
echo
echo "Installation.............: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix.................: $prefix"
echo "  emotion generic players: ${EMOTION_GENERIC_PLAYERS_DIR}"
echo

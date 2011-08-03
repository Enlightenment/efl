##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_define([v_maj], [0])
m4_define([v_min], [1])
m4_define([v_mic], [1])
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

AC_INIT([ethumb], [v_ver], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.52])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_ISC_POSIX

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
_XTERM_COLORS

##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], , [m4_define([v_rev], [0])])
m4_ifdef([v_rel], , [m4_define([v_rel], [])])
AC_DEFINE_UNQUOTED(VERS_MAJ, [v_maj], [Major version])
AC_DEFINE_UNQUOTED(VERS_MIN, [v_min], [Minor version])
AC_DEFINE_UNQUOTED(VERS_MIC, [v_mic], [Micro version])
AC_DEFINE_UNQUOTED(VERS_REV, [v_rev], [Revison])
version_info="lt_rev:lt_cur:lt_age"
release_info="v_rel"
AC_SUBST(version_info)
AC_SUBST(release_info)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
VMAJ=v_maj
AC_SUBST(VMAJ)

dnl we just have set the version info, then:
AC_SUBST(version_info)

AC_PROG_CC
AM_PROG_CC_STDC
AM_PROG_CC_C_O
AC_HEADER_STDC
AC_C___ATTRIBUTE__
dnl AC_C_CONST is bogus, its tests fails on -Wall -O1 ( uninitialized values)
dnl AC_C_CONST

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])
define([AC_LIBTOOL_LANG_GCJ_CONFIG], [:])
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])
AC_PROG_LIBTOOL

VMAJ=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $1);}'`
VMIN=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $2);}'`
version_info=`expr $VMAJ + $VMIN`
AC_SUBST(version_info)
AC_SUBST(VMAJ)

#themesdir="${datadir}/ethumb/data/frames"
#AC_SUBST(themesdir)
#AS_AC_EXPAND(THEMESDIR, $themesdir)
#AC_DEFINE_UNQUOTED(THEMESDIR, ["$THEMESDIR"], [Where frame theme files are installed.]
#)

#AS_AC_EXPAND(SYSCONFDIR, $sysconfdir)
#AC_DEFINE_UNQUOTED(SYSCONFDIR, ["$SYSCONFDIR"], [Where system configuration is stored])

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])

with_max_log_level="<unset>"
AC_ARG_WITH(internal-maximum-log-level,
   [AC_HELP_STRING([--with-internal-maximum-log-level=NUMBER],
                   [limit eina internal log level to the given number, any call to EINA_LOG() with values greater than this will be compiled out, ignoring runtime settings, but saving function calls.])],
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

EFL_ETHUMB_BUILD=""
EFL_ETHUMB_CLIENT_BUILD=""
lt_enable_auto_import=""
case "$host_os" in
   mingw* | cegcc*)
      EFL_ETHUMB_BUILD="-DEFL_ETHUMB_BUILD"
      EFL_ETHUMB_CLIENT_BUILD="-DEFL_ETHUMB_CLIENT_BUILD"
      lt_enable_auto_import="-Wl,--enable-auto-import"
   ;;
esac
AC_SUBST(EFL_ETHUMB_BUILD)
AC_SUBST(EFL_ETHUMB_CLIENT_BUILD)
AC_SUBST(lt_enable_auto_import)

PKG_CHECK_MODULES(EINA, [eina >= 1.0.0])
PKG_CHECK_MODULES(EVAS, [evas >= 1.0.0])
PKG_CHECK_MODULES(ECORE, [ecore >= 1.0.0])
PKG_CHECK_MODULES(ECORE_EVAS, [ecore-evas >= 1.0.0])
PKG_CHECK_MODULES(ECORE_FILE, [ecore-file >= 1.0.0 ])
PKG_CHECK_MODULES(EDJE, [edje >= 1.0.0])
EFL_WITH_BIN([edje], [edje-cc], [edje_cc])

requirement_ethumb="eina >= 1.0.0 evas >= 1.0.0 ecore >= 1.0.0 ecore-evas >= 1.0.0 ecore-file >= 1.0.0 edje >= 1.0.0"
requirement_ethumb_client="ethumb edbus >= 1.0.0"

AM_CONDITIONAL(HAVE_EMOTION, false)
define([CHECK_MODULE_EMOTION],
[
        AC_ETH_CHECK_PKG(EMOTION, emotion, [], [EMOTION=false])
])

AC_ETH_OPTIONAL_MODULE([emotion], true, [CHECK_MODULE_EMOTION])

if $USE_MODULE_EMOTION ; then
        requirement_ethumb="$requirement_ethumb emotion"
fi

AM_CONDITIONAL(HAVE_EPDF, false)
define([CHECK_MODULE_EPDF],
[
        AC_ETH_CHECK_PKG(EPDF, epdf, [], [EPDF=false])
])

AC_ETH_OPTIONAL_MODULE([epdf], true, [CHECK_MODULE_EPDF])
if $USE_MODULE_EPDF ; then
        requirement_ethumb="$requirement_ethumb epdf"
fi

AM_CONDITIONAL(HAVE_EDBUS, false)
define([CHECK_MODULE_ETHUMBD],
[
        AC_ETH_CHECK_PKG(EDBUS, edbus, [], [ETHUMBD=false])
])

AC_ETH_OPTIONAL_MODULE([ethumbd], true, [CHECK_MODULE_ETHUMBD])
if $USE_MODULE_ETHUMBD ; then
        dbusservicedir=${datadir}/dbus-1/services
        AC_ARG_WITH([dbus-services],
                    AC_HELP_STRING([--with-dbus-services=DBUS_SERVICES], [specify a directory to store dbus service files.]),
                    [
                      dbusservicedir=$withval
                    ]
                   )
fi


AM_CONDITIONAL(HAVE_LIBEXIF, false)
define([CHECK_MODULE_LIBEXIF],
[
        AC_ETH_CHECK_PKG(LIBEXIF, libexif, [], [LIBEXIF=false])
])

AC_ETH_OPTIONAL_MODULE([libexif], true, [CHECK_MODULE_LIBEXIF])
if $USE_MODULE_LIBEXIF ; then
        requirement_ethumb="$requirement_ethumb libexif"
fi

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
       [[
#include <stdlib.h>
#include <sys/types.h>
#include <sys/xattr.h>
       ]],
       [[
size_t tmp = listxattr("/", NULL, 0);
tmp = getxattr("/", "user.ethumb.md5", NULL, 0);
setxattr("/", "user.ethumb.md5", NULL, 0, 0);
       ]])],
    [
      AC_DEFINE(HAVE_XATTR, 1, [Define to 1 if you have 'listxattr', 'setxattr' and 'getxattr'])
      have_xattr="yes"
    ],
    [have_xattr="no"])

AC_MSG_CHECKING([for Xattr])
AC_MSG_RESULT([${have_xattr}])

AC_SUBST(requirement_ethumb)
AC_SUBST(requirement_ethumb_client)
AC_SUBST(dbusservicedir)

AC_CONFIG_FILES([
ethumb.pc
ethumb.spec
ethumb_client.pc
org.enlightenment.Ethumb.service
Makefile
src/Makefile
src/bin/Makefile
src/lib/Makefile
src/lib/client/Makefile
src/plugins/Makefile
src/plugins/emotion/Makefile
src/plugins/epdf/Makefile
src/tests/Makefile
data/Makefile
data/frames/Makefile
m4/Makefile
doc/Makefile
doc/Doxyfile
])

AC_OUTPUT

txt_strip() {
        echo "[$]@" | sed -e 's/^[[ \t]]*\([[^ \t]]*\)[[ \t]]*$/\1/g'
}

MODS=""
for mod in $OPTIONAL_MODULES; do
        MODS="$MODS ${COLOR_HGREEN}+$mod${COLOR_END}"
done
MODS=$(txt_strip $MODS)

UNUSED_MODS=""
for mod in $UNUSED_OPTIONAL_MODULES; do
        UNUSED_MODS="$UNUSED_MODS ${COLOR_HRED}-$mod${COLOR_END}"
done
UNUSED_MODS=$(txt_strip $UNUSED_MODS)

cat <<SUMMARY_EOF

Summary:
 * project..........: $PACKAGE $VERSION
 * prefix...........: $(txt_strip $prefix)
 * DBus services dir: $(txt_strip $dbusservicedir)
 * CFLAGS...........: $(txt_strip $CFLAGS)
 * LDFLAGS..........: $(txt_strip $LDFLAGS)

Configuration Options Summary:

 * use xattr........: ${have_xattr}
 * maximum log level: ${with_max_log_level}
 * documentation....: ${build_doc}

SUMMARY_EOF

if test "x$MODS" != "x"; then
   echo -e " * enabled modules..: $MODS"
fi

if test "x$UNUSED_MODS" != "x"; then
   echo -e " * disabled modules.: $UNUSED_MODS"
fi

cat << HINT_EOF

now type:

Now type 'make' ('gmake' on some systems) to compile $PACKAGE, and
then afterwards as root (or the user who will install this), type
'make install'. Change users with 'su' or 'sudo' appropriately.

HINT_EOF

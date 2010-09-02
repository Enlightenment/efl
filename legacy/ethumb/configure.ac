AC_INIT([ethumb], [0.1.0], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.52])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_ISC_POSIX

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
AM_CONFIG_HEADER([config.h])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
_XTERM_COLORS

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

AC_SUBST(requirement_ethumb)
AC_SUBST(requirement_ethumb_client)
AC_SUBST(dbusservicedir)

AC_OUTPUT([
ethumb.pc
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
])

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

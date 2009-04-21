AC_INIT(ethumb, 0.1, antognolli@profusion.mobi)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(configure.ac)
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_ISC_POSIX

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)
AC_SUBST(ACLOCAL_AMFLAGS, "-I m4")
_XTERM_COLORS

AC_PROG_CC
AM_PROG_CC_STDC
AM_PROG_CC_C_O
AC_HEADER_STDC
dnl AC_C_CONST is bogus, its tests fails on -Wall -O1 ( uninitialized values)
dnl AC_C_CONST

define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

VMAJ=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $1);}'`
VMIN=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $2);}'`
version_info=`expr $VMAJ + $VMIN`
AC_SUBST(version_info)

themesdir="${datadir}/ethumb/data/frames"
AC_SUBST(themesdir)
AS_AC_EXPAND(THEMESDIR, $themesdir)
AC_DEFINE_UNQUOTED(THEMESDIR, ["$THEMESDIR"], [Where frame theme files are installed.]
)

AS_AC_EXPAND(SYSCONFDIR, $sysconfdir)
AC_DEFINE_UNQUOTED(SYSCONFDIR, ["$SYSCONFDIR"], [Where system configuration is stored])

pluginsdir="${libdir}/ethumb/plugins"
AC_SUBST(pluginsdir)
AS_AC_EXPAND(PLUGINSDIR, $pluginsdir)
AC_DEFINE_UNQUOTED(PLUGINSDIR, ["$PLUGINSDIR"], [Where plugins are installed.])

PKG_CHECK_MODULES(EINA, [eina-0])
PKG_CHECK_MODULES(EVAS, [evas])
PKG_CHECK_MODULES(ECORE, [ecore])
PKG_CHECK_MODULES(ECORE_EVAS, [ecore-evas])
PKG_CHECK_MODULES(ECORE_FILE, [ecore-file])
PKG_CHECK_MODULES(EDJE, [edje])

requirement_ethumb="eina-0 evas ecore ecore-evas ecore-file edje"

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

AC_SUBST(requirement_ethumb)

AC_OUTPUT([
ethumb.pc
Makefile
src/Makefile
src/bin/Makefile
src/lib/Makefile
src/plugins/Makefile
src/plugins/emotion/Makefile
src/plugins/epdf/Makefile
data/Makefile
data/frames/Makefile
m4/Makefile
])

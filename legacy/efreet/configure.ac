rm -f config.cache

AC_INIT([efreet], [0.5.0.050], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.52])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST
AC_C___ATTRIBUTE__

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

AC_FUNC_ALLOCA

PCFLAGS=$CFLAGS

PKG_CHECK_MODULES(ECORE, [ecore >= 0.9.9 ecore-file >= 0.9.9])
PKG_CHECK_MODULES([EINA], [eina-0])

CFLAGS=$ECORE_CFLAGS" "$CFLAGS

requirements="ecore ecore-file"

AC_ARG_ENABLE(strict-spec,
	[AC_HELP_STRING([--enable-strict-spec],[Enable strict spec compliance])],
	[enable_strict_spec=$enableval], [enable_strict_spec="auto"])
if test "x$enable_strict_spec" = "xyes" ; then
    AC_DEFINE(STRICT_SPEC, 1, [Strict Spec Compliance])
fi

AC_ARG_ENABLE(sloppy-spec,
	[AC_HELP_STRING([--disable-sloppy-spec],[Enable sloppy spec compliance])],
	[enable_sloppy_spec=$enableval], [enable_sloppy_spec="yes"])
if test "x$enable_sloppy_spec" = "xyes" ; then
    AC_DEFINE(SLOPPY_SPEC, 1, [Sloppy Spec Compliance])
fi

CFLAGS=$PCFLAGS

AC_ARG_ENABLE(hidden-visibility,
	[AC_HELP_STRING([--enable-hidden-visibility],[Enable hidden visibility])],
	[enable_hidden_visibility=$enableval], [enable_hidden_visibility="auto"])
if test "x$enable_hidden_visibility" = "xyes" ; then
    CPPFLAGS="$CPPFLAGS -fvisibility=hidden"
else
    AC_DEFINE(DEFAULT_VISIBLITY, 1, [Default visibility])
fi
AM_CONDITIONAL(DEFAULT_VISIBILITY, test "x$enable_hidden_visibility" != "xyes")
EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])

AC_SUBST(requirements)
AC_OUTPUT([
efreet.spec
efreet.pc
efreet-mime.pc
efreet-trash.pc
Makefile
doc/Makefile
doc/Doxyfile
src/Makefile
src/lib/Makefile
src/bin/Makefile
src/bin/data/Makefile
src/bin/data/sub/Makefile
src/bin/compare/Makefile
], []
)

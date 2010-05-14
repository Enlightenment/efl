# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT([eeze], [0.0.1], [enlightenment-devel@lists.sourceforge.net])
release="ver-pre-svn-05"
AC_PREREQ([2.52])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
AM_CONFIG_HEADER([config.h])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC
AC_HEADER_STDC
AM_PROG_CC_C_O

define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
AC_PROG_LIBTOOL

VMAJ=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $1);}'`
VMIN=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $2);}'`
VMIC=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $3);}'`
SNAP=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $4);}'`
version_info=`expr $VMAJ + $VMIN`":$VMIC:$VMIN"
eudev_release_info="-release $release"
AC_SUBST(version_info)
AC_SUBST(eudev_release_info)

modules=""
dnl Check enabled modules to build
AC_ARG_ENABLE([eudev],[AC_HELP_STRING([--disable-eudev], [Disable eudev build])])
AS_IF(
   [test "x$enableval" != "xno"],
      modules+="udev"
      [enable_eudev="yes"],
   [enable_eudev=no],
)

if test -z $modules;then
  AC_MSG_ERROR(you have not selected any modules to build!)
fi
# pkg-config
PKG_PROG_PKG_CONFIG

# Check whether pkg-config supports Requires.private
AS_IF(
   [$PKG_CONFIG --atleast-pkgconfig-version 0.22],
      [pkgconfig_requires_private="Requires.private"],
   [pkgconfig_requires_private="Requires"]
)
AC_SUBST(pkgconfig_requires_private)

PKG_CHECK_MODULES([ECORE], [ecore])

AS_IF(
   [test "x${enable_eudev}" = "xyes"],
     [PKG_CHECK_MODULES(UDEV, libudev, [], [enable_eudev="no"])]
)

AM_CONDITIONAL([BUILD_EUDEV],     [test "x${enable_eudev}"     = "xyes"])

EFL_EDBUS_BUILD=""
lt_enable_auto_import=""
AS_CASE(
   ["$host_os"],
      [mingw* | cegcc*],
        AC_MSG_ERROR([THIS WILL NOT RUN ON WINDOWS!])
)

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])

AC_OUTPUT([
Makefile
doc/eeze.dox
doc/Makefile
src/Makefile
src/lib/Makefile
src/lib/udev/Makefile
src/bin/Makefile
eudev.pc
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
echo "  Modules:"
echo
echo "    EUdev..............: $enable_eudev"
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo

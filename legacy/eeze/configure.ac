# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT([eeze], [0.5.0], [enlightenment-devel@lists.sourceforge.net])
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
eeze_release_info="-release $release"
AC_SUBST(version_info)
AC_SUBST(eeze_release_info)

PKG_PROG_PKG_CONFIG

# Check whether pkg-config supports Requires.private
AS_IF(
   [$PKG_CONFIG --atleast-pkgconfig-version 0.22],
      [pkgconfig_requires_private="Requires.private"],
   [pkgconfig_requires_private="Requires"]
)
AC_SUBST(pkgconfig_requires_private)

PKG_CHECK_MODULES([ECORE], [ecore])
PKG_CHECK_MODULES(UDEV, libudev)

udev_version=$(pkg-config libudev --modversion)

if test $udev_version -lt 142;then
  AC_MSG_WARN([Old udev version detected, enabling compat code])
  AC_DEFINE([OLD_UDEV_RRRRRRRRRRRRRR],[1],[compat functionality for udev < 142])
fi

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
src/bin/Makefile
eeze.pc
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
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo

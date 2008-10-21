# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT([embryo], [0.9.9.050], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.52])
AC_CONFIG_SRCDIR([configure.ac])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_ISC_POSIX

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

AC_C_BIGENDIAN
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST
AM_PROG_CC_C_O
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

PKG_CHECK_MODULES([EINA], [eina-0])

WIN32_CFLAGS=""
lt_enable_auto_import=""
case "$host_os" in
	mingw*|cegcc)
		PKG_CHECK_MODULES([EVIL], evil)
		AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if evil package is installed])
		dnl needed for correct definition of EAPI
		AC_DEFINE(EFL_EMBRYO_BUILD, 1, [Define to mention that embryo is built])
		AC_DEFINE(HAVE_GETTIMEOFDAY, 1, [Defined to 1 (MinGW / CeGCC platform)])
		if test "x$host_os" = "xcegcc" ; then
			WIN32_CFLAGS="-mwin32"
			lt_enable_auto_import="-Wl,--enable-auto-import"
		fi
		;;
	*)
		AC_CHECK_FUNCS(gettimeofday)
esac
AC_SUBST(WIN32_CFLAGS)
AC_SUBST(lt_enable_auto_import)

AC_FUNC_ALLOCA

AC_CHECK_HEADER([fnmatch.h],
   [dummy="yes"],
   [AC_MSG_ERROR([Cannot find fnmatch.h. Make sure your CFLAGS environment variable contains include lines for the location of this file. MinGW users: see the INSTALL file])])

AC_CHECK_FUNCS([fnmatch], [res="yes"], [res="no"])
if test "x$res" = "xno"; then
   AC_SEARCH_LIBS([fnmatch],
      [fnmatch iberty evil],
      [res="yes"],
      [res="no"])
   if test "x$res" = "xno"; then
      AC_MSG_ERROR([Cannot find fnmatch() in neither libc nor libfnmatch, nor libiberty, nor libevil])
	fi
fi

embryoincludedir='${datadir}/include'
AC_SUBST(embryoincludedir)

AC_OUTPUT([
Makefile
embryo.pc
include/Makefile
src/Makefile
src/lib/Makefile
src/bin/Makefile
README
embryo.spec
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
echo "  Compilation..........: make"
echo
echo "  Installation.........: make install"
echo
echo "    prefix.............: $prefix"
echo

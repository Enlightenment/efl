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


### Default options with respect to host

requirement_embryo=""
embryoincludedir="${datadir}/include"


### Additional options to configure


### Checks for libraries

PKG_PROG_PKG_CONFIG

# Evil library for compilation on Windows

EFL_EMBRYO_BUILD=""
case "$host_os" in
   mingw* | cegcc*)
   PKG_CHECK_MODULES([EVIL], [evil])
   AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if Evil library is installed])
   requirement_embryo="evil ${requirement_embryo}"
   EFL_EMBRYO_BUILD="-DEFL_EMBRYO_BUILD"
   ;;
esac
AC_SUBST(EFL_EMBRYO_BUILD)


### Checks for header files

AC_CHECK_HEADER([fnmatch.h],
   [dummy="yes"],
   [AC_MSG_ERROR([Cannot find fnmatch.h. Make sure your CFLAGS environment variable contains include lines for the location of this file. MinGW users: see the INSTALL file])])


### Checks for programs
AC_PROG_CC

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])


### Checks for types


### Checks for structures


### Checks for compiler characteristics
AC_C_BIGENDIAN
AM_PROG_CC_C_O
AC_C_CONST
AC_C_INLINE
AC_PROG_CC_STDC
AC_HEADER_STDC
AC_C___ATTRIBUTE__

EMBRYO_CPPFLAGS=""
EMBRYO_CFLAGS=""
case "$host_os" in
   mingw32ce*)
      EMBRYO_CPPFLAGS="-D_WIN32_WCE=0x0420"
      ;;
   cegcc*)
      EMBRYO_CPPFLAGS="-D_WIN32_WCE=0x0420"
      EMBRYO_CFLAGS="-mwin32"
      ;;
esac
AC_SUBST(EMBRYO_CPPFLAGS)
AC_SUBST(EMBRYO_CFLAGS)


### Checks for linker characteristics

lt_enable_auto_import=""
case "$host_os" in
   mingw* | cegcc*)
      lt_enable_auto_import="-Wl,--enable-auto-import"
      ;;
esac
AC_SUBST(lt_enable_auto_import)


### Checks for library functions
AC_FUNC_ALLOCA

case "$host_os" in
   mingw* | cegcc*)
      AC_DEFINE(HAVE_GETTIMEOFDAY, 1, [Defined to 1 (MinGW / CeGCC platform)])
      ;;
   *)
      AC_CHECK_FUNCS(gettimeofday)
      ;;
esac

fnmatch_libs=""
AC_CHECK_FUNCS([fnmatch], [res="yes"], [res="no"])
if test "x$res" = "xno"; then
   AC_SEARCH_LIBS([fnmatch],
      [fnmatch iberty evil],
      [res="yes"],
      [res="no"])
   if test "x$res" = "xno"; then
      AC_MSG_ERROR([Cannot find fnmatch() in neither libc nor libfnmatch, nor libiberty, nor libevil])
   fi
   fnmatch_libs="${ac_cv_search_fnmatch}"
fi
AC_SUBST(fnmatch_libs)


AC_SUBST(requirement_embryo)
AC_SUBST(embryoincludedir)

AC_OUTPUT([
Makefile
doc/Makefile
doc/embryo.dox
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

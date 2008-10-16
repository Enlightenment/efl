
AC_INIT([evil], [0.0.1], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.52])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST

dnl If the host is not Windows, we exit
case "$host_os" in
  mingw* | cegcc*)
    ;;
  *)
    AC_MSG_ERROR([OS must be Windows. Exiting...])
    ;;
esac

AM_INIT_AUTOMAKE(1.6 dist-bzip2)

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


### Checks for libraries

win32_libs=""
case "$host_os" in
  mingw | mingw32 | mingw32msvc)
    win32_libs="-lole32 -luuid -lws2_32 -lsecur32"
    ;;
  cegcc*)
    win32_libs="-lws2"
    ;;
  mingw32ce*)
    win32_libs="-lws2"
    ;;
esac
AC_SUBST(win32_libs)

EFL_CHECK_MPATROL([have_mpatrol="yes"], [have_mpatrol="no"])


### Checks for header files
AC_CHECK_HEADERS_ONCE([errno.h])


### Checks for programs
AM_PROG_AS
AC_LANG(C)
AC_PROG_CPP
AC_PROG_CC

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])


### Checks for types


### Checks for structures


### Checks for compiler characteristics
AC_C_CONST
AC_C___ATTRIBUTE__

win32_cflags=""
have_wince="no"
have_mingw32ce="no"
case "$host_os" in
  cegcc*)
    have_wince="yes"
    win32_cflags="-mwin32"
    ;;
  mingw32ce*)
    have_wince="yes"
    have_mingw32ce="yes"
    ;;
  *)
    have_wince="no"
    ;;
esac
AC_SUBST(win32_cflags)

if test "x${have_wince}" = "xyes" ; then
  AC_DEFINE(EVIL_HAVE_WINCE, 1, [Define to mention that Windows CE is the target])
fi

AM_CONDITIONAL(EVIL_HAVE_WINCE, test "x${have_wince}" = "xyes")
AM_CONDITIONAL(EVIL_HAVE_MINGW32CE, test "x${have_mingw32ce}" = "xyes")


### Checks for linker characteristics


### Checks for library functions


AC_CONFIG_FILES([
Makefile
evil.pc
doc/Makefile
src/Makefile
src/bin/Makefile
src/lib/Makefile
src/lib/dlfcn/Makefile
])

AC_OUTPUT


#####################################################################
## Info

echo
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE_NAME $PACKAGE_VERSION $PACKAGE_TARNAME"
echo "------------------------------------------------------------------------"
echo
echo "Configuration Options Summary:"
echo
echo "  OS...................: ${host_os}"
echo
echo "  Documentation........: ${build_doc}"
echo "  Mpatrol..............: ${have_mpatrol}"
echo
echo "  Compilation..........: make"
echo
echo "  Installation.........: make install"
echo
echo "    prefix.............: $prefix"
echo

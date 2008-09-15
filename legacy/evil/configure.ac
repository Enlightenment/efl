
AC_INIT(evil, 0.0.1, enlightenment-devel@lists.sourceforge.net)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST

dnl If the host is not windows, we exit,
dnl otherwise, we set the correct flags
dnl for each platform.
win32_libs=""
win32_cflags=""
case "$host_os" in
  mingw | mingw32 | mingw32msvc)
    win32_libs="-lole32 -luuid -lws2_32"
    ;;
  cegcc)
    win32_cflags="-mwin32"
    win32_libs="-lws2"
    ;;
  mingw32ce)
    win32_libs="-lws2"
    ;;
  *)
    AC_MSG_ERROR([OS must be Windows. Exiting...])
    ;;
esac
AC_SUBST(win32_cflags)
AC_SUBST(win32_libs)

dnl needed for correct definition of EAPI
AC_DEFINE(EFL_EVIL_BUILD, 1, [Define to mention that evil is built])
AC_DEFINE(EFL_EVIL_DLFCN_BUILD, 1, [Define to mention that evil dlfcn is built])

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

AM_PROG_AS
AC_LANG(C)
AC_PROG_CPP
AC_PROG_CC
AC_C_CONST

AC_MSG_CHECKING([for __attribute__])
AC_CACHE_VAL(_cv_have___attribute__,
  [
    AC_TRY_COMPILE([#include <stdlib.h>],
      [int func(int x); int foo(int x __attribute__ ((unused))) { exit(1); }],
      [_cv_have___attribute__="yes"],
      [_cv_have___attribute__="no"])
  ]
)

if test "x${_cv_have___attribute__}" = "xyes" ; then
   AC_DEFINE(HAVE___ATTRIBUTE__, 1, [Define to 1 if your compiler has __attribute__])
fi
AC_MSG_RESULT(${_cv_have___attribute__})


AC_CONFIG_FILES([
Makefile
evil.pc
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
echo "  Compilation..........: make"
echo
echo "  Installation.........: make install"
echo
echo "    prefix.............: $prefix"
echo

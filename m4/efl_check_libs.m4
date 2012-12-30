dnl Copyright (C) 2012 Vincent Torri <vincent dot torri at gmail dot com>
dnl This code is public domain and can be freely used or copied.

dnl Macro that check dependencies libraries for the EFL:

dnl libjpeg
dnl zlib

dnl _EFL_CHECK_LIB_LIBJPEG is for internal use
dnl _EFL_CHECK_LIB_LIBJPEG(EFL)
dnl it will abort (AC_MSG_ERROR) if libjpeg is not found.

AC_DEFUN([_EFL_CHECK_LIB_LIBJPEG],
[dnl
EFL_CHECK_LIB_CODE([$1], [-ljpeg], [have_fct], [[
#include <stdio.h>
#include <jpeglib.h>
]], [[
struct jpeg_error_mgr er; void *error = jpeg_std_error(&er);
]])

if test "${have_fct}" = "no"; then
  AC_MSG_ERROR([Cannot find libjpeg. Make sure your CFLAGS and LDFLAGS environment variable are set properly.])
fi
])

dnl _EFL_CHECK_LIB_ZLIB is for internal use
dnl _EFL_CHECK_LIB_ZLIB(EFL)
dnl it will abort (AC_MSG_ERROR) if zlib is not found.

AC_DEFUN([_EFL_CHECK_LIB_ZLIB],
[dnl
m4_pushdef([DOWNEFL], m4_translit([$1], [-A-Z], [_a-z]))dnl

PKG_CHECK_EXISTS([zlib >= 1.2.3], [_efl_have_lib="yes"], [_efl_have_lib="no"])

if test "${_efl_have_lib}" = "yes"; then
   requirements_pc_[]m4_defn([DOWNEFL])="${requirements_pc_[]m4_defn([DOWNEFL])} zlib >= 1.2.3"
   requirements_pc_deps_[]m4_defn([DOWNEFL])="${requirements_pc_deps_[]m4_defn([DOWNEFL])} zlib >= 1.2.3"
else
   EFL_CHECK_LIB_CODE([$1], [-lz], [have_fct], [[
#include <zlib.h>
]], [[const char *v = zlibVersion();]])

  if test "${have_fct}" = "no"; then
    AC_MSG_ERROR([Cannot find zlib. Make sure your CFLAGS and LDFLAGS environment variable are set properly.])
  fi
fi
m4_popdef([DOWNEFL])dnl
])

dnl Macro that checks for a library
dnl
dnl EFL_CHECK_LIB(EFL, LIBRARY)
dnl it will abort if library is not found

AC_DEFUN([EFL_CHECK_LIB],
[dnl
m4_pushdef([UP], m4_translit([$2], [-a-z], [_A-Z]))dnl
m4_default([_EFL_CHECK_LIB_]m4_defn([UP]))($1)
AC_MSG_CHECKING([for $2])
AC_MSG_RESULT([yes])
m4_popdef([UP])dnl
])

dnl Macro that iterates over a sequence of white separated libraries
dnl and that calls EFL_CHECK_LIB() for each of these libraries
dnl
dnl EFL_CHECK_LIBS(EFL, LIBRARIES)
dnl it will abort if libraries are not found

AC_DEFUN([EFL_CHECK_LIBS],
[m4_foreach_w([lib], [$2], [EFL_CHECK_LIB($1, m4_defn([lib]))])])

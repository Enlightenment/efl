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
m4_pushdef([UPEFL], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWNEFL], m4_translit([$1], [-A-Z], [_a-z]))dnl

AC_CHECK_HEADER([jpeglib.h], [],
   [AC_MSG_ERROR([Cannot find jpeglib.h. Make sure your CFLAGS environment variable contains include lines for the location of this file])])

AC_CHECK_LIB([jpeg], [jpeg_std_error],
      [requirements_libs_[]m4_defn([DOWNEFL])="${requirements_libs_[]m4_defn([DOWNEFL])} -ljpeg"],
      [AC_MSG_ERROR([Cannot find libjpeg library. Make sure your LDFLAGS environment variable contains include lines for the location of this file])])

m4_popdef([DOWNEFL])dnl
m4_popdef([UPEFL])dnl
])

dnl _EFL_CHECK_LIB_ZLIB is for internal use
dnl _EFL_CHECK_LIB_ZLIB(EFL)
dnl it will abort (AC_MSG_ERROR) if zlib is not found.

AC_DEFUN([_EFL_CHECK_LIB_ZLIB],
[dnl
m4_pushdef([UPEFL], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWNEFL], m4_translit([$1], [-A-Z], [_a-z]))dnl

PKG_CHECK_EXISTS([zlib >= 1.2.3], [_efl_have_lib="yes"], [_efl_have_lib="no"])

if test "x${_efl_have_lib}" = "xyes" ; then
   requirements_pc_[]m4_defn([DOWNEFL])="${requirements_pc_[]m4_defn([DOWNEFL])} zlib >= 1.2.3"
   requirements_pc_deps_[]m4_defn([DOWNEFL])="${requirements_pc_deps_[]m4_defn([DOWNEFL])} zlib >= 1.2.3"
else
   AC_CHECK_HEADER([zlib.h], [],
      [AC_MSG_ERROR([Cannot find zlib.h. Make sure your CFLAGS environment variable contains include lines for the location of this file])])

   AC_CHECK_LIB([z], [zlibVersion],
      [requirements_libs_[]m4_defn([DOWNEFL])="${requirements_libs_[]m4_defn([DOWNEFL])} -lz"],
      [AC_MSG_ERROR([Cannot find libjpeg library. Make sure your LDFLAGS environment variable contains include lines for the location of this file])])
fi

m4_popdef([DOWNEFL])dnl
m4_popdef([UPEFL])dnl
])

dnl Macro that checks for a library
dnl
dnl EFL_CHECK_LIB(EFL, LIBRARY)
dnl it will abort if library is not found

AC_DEFUN([EFL_CHECK_LIB],
[dnl
m4_pushdef([UP], m4_translit([$2], [-a-z], [_A-Z]))dnl

AC_MSG_CHECKING([for $2])
m4_default([_EFL_CHECK_LIB_]m4_defn([UP]))($1)
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

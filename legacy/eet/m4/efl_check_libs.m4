dnl Copyright (C) 2012 Vincent Torri <vincent dot torri at gmail dot com>
dnl This code is public domain and can be freely used or copied.

dnl Macro that check dependencies libraries for the EFL:

dnl libjpeg
dnl zlib

dnl _EFL_CHECK_LIB_LIBJPEG is for internal use
dnl _EFL_CHECK_LIB_LIBJPEG(EFL, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)

AC_DEFUN([_EFL_CHECK_LIB_LIBJPEG],
[
m4_pushdef([UPEFL], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWNEFL], m4_translit([$1], [-A-Z], [_a-z]))dnl

AC_CHECK_HEADER([jpeglib.h],
   [have_dep="yes"],
   [
    AC_MSG_ERROR(["Cannot find jpeglib.h. Make sure your CFLAGS environment variable contains include lines for the location of this file"])
    have_dep="no"
   ])

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([jpeg], [jpeg_std_error],
      [
       have_dep="yes"
       requirements_libs_[]m4_defn([DOWNEFL])="${requirements_libs_[]m4_defn([DOWNEFL])} -ljpeg"
      ],
      [
       AC_MSG_ERROR("Cannot find libjpeg library. Make sure your LDFLAGS environment variable contains include lines for the location of this file")
       have_dep="no"
      ])
fi

AS_IF([test "x${have_dep}" = "xyes"], [$2], [$3])

m4_popdef([DOWNEFL])
m4_popdef([UPEFL])
])

dnl _EFL_CHECK_LIB_ZLIB is for internal use
dnl _EFL_CHECK_LIB_ZLIB(EFL, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)

AC_DEFUN([_EFL_CHECK_LIB_ZLIB],
[
m4_pushdef([UPEFL], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWNEFL], m4_translit([$1], [-A-Z], [_a-z]))dnl

PKG_CHECK_EXISTS([zlib],
   [
    _efl_have_lib="yes"
    requirements_pc_[]m4_defn([DOWNEFL])="${requirements_pc_[]m4_defn([DOWNEFL])} zlib"
   ],
   [
    _efl_have_lib="no"
   ])

if test "x${_efl_have_lib}" = "xno" ; then
   AC_CHECK_HEADER([zlib.h],
      [_efl_have_lib="yes"],
      [
       AC_MSG_ERROR(["Cannot find zlib.h. Make sure your CFLAGS environment variable contains include lines for the location of this file"])
       _efl_have_lib="no"
      ])

   if test "x${_efl_have_lib}" = "xyes" ; then
      AC_CHECK_LIB([z], [zlibVersion],
         [
          _efl_have_lib="yes"
          requirements_libs_[]m4_defn([DOWNEFL])="${requirements_libs_[]m4_defn([DOWNEFL])} -lz"
         ],
         [
          AC_MSG_ERROR(["Cannot find libjpeg library. Make sure your LDFLAGS environment variable contains include lines for the location of this file"])
          _efl_have_lib="no"
         ])
   fi
fi

AS_IF([test "x${_efl_have_lib}" = "xyes"], [$2], [$3])

m4_popdef([DOWNEFL])
m4_popdef([UPEFL])
])

dnl Macro that checks for a library
dnl
dnl EFL_CHECK_LIB(EFL, LIBRARY)
dnl AC_DEFINE : EFL_HAVE_LIBRARY (LIBRARY being replaced by its value)

AC_DEFUN([EFL_CHECK_LIB],
[
m4_pushdef([UP], m4_translit([$2], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$2], [-A-Z], [_a-z]))dnl

m4_default([_EFL_CHECK_LIB_]m4_defn([UP]))($1, [have_lib="yes"], [have_lib="no"])

AC_MSG_CHECKING([for ]m4_defn([DOWN]))
AC_MSG_RESULT([${have_lib}])

if test "x${have_lib}" = "xyes" ; then
   AC_DEFINE([HAVE_]m4_defn([UP]), [1], [Define to 1 if the `]m4_defn([DOWN])[' library is installed.])
fi

efl_lib_[]m4_defn([DOWN])="${have_lib}"

m4_popdef([DOWN])
m4_popdef([UP])
])

dnl Macro that iterates over a sequence of white separated libraries
dnl and that calls EFL_CHECK_LIB() for each of these libraries
dnl
dnl EFL_CHECK_LIBS(EFL, LIBRARIES)

AC_DEFUN([EFL_CHECK_LIBS],
[
m4_foreach_w([lib], [$2], [EFL_CHECK_LIB($1, m4_defn([lib]))])
])

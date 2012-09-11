dnl Copyright (C) 2012 Vincent Torri <vincent dot torri at gmail dot com>
dnl This code is public domain and can be freely used or copied.

dnl Macro that check dependencies libraries for the EFL

dnl _EFL_CHECK_LIB_ICONV is for internal use
dnl _EFL_CHECK_LIB_ICONV(libname, lib, ACTION-IF-FOUND ,ACTION-IF-NOT-FOUND)

AC_DEFUN([_EFL_CHECK_LIB_ICONV],
[
AC_MSG_CHECKING([whether iconv() is in $1])

LIBS_save="${LIBS}"
LIBS="$2 ${LIBS}"
AC_LINK_IFELSE([AC_LANG_PROGRAM(
   [[
#include <stdlib.h>
#include <iconv.h>
   ]],
   [[
iconv_t ic;
size_t count = iconv(ic, NULL, NULL, NULL, NULL);
   ]])],
   [
    have_dep="yes"
    ICONV_LIBS=$2
   ],
   [
    have_dep="no"
   ])
LIBS=${LIBS_save}

AC_MSG_RESULT([${have_dep}])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])
])

dnl Macro that checks for iconv library in libc, libiconv and libiconv_plug
dnl
dnl EFL_CHECK_LIB_ICONV(EFL[, ACTION-IF-FOUND[ ,ACTION-IF-NOT-FOUND]])
dnl update requirements

AC_DEFUN([EFL_CHECK_LIB_ICONV],
[
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

AC_ARG_WITH([iconv-link],
   AC_HELP_STRING([--with-iconv-link=ICONV_LINK], [explicitly specify an iconv link option]),
   [
    have_dep="yes"
    ICONV_LIBS=${withval}
   ],
   [have_dep="no"])

AC_MSG_CHECKING([for explicit iconv link options])
if test "x${ICONV_LIBS}" = "x" ; then
   AC_MSG_RESULT([no explicit iconv link option])
else
   AC_MSG_RESULT([${ICONV_LIBS}])
fi

if test "x${have_dep}" = "xno" ; then
   AC_CHECK_HEADERS([iconv.h], [have_dep="yes"])

   if test "x${have_dep}" = "xyes" ; then
      _EFL_CHECK_LIB_ICONV([libc], [], [have_dep="yes"], [have_dep="no"])
   fi

   if test "x${have_dep}" = "xno" ; then
      _EFL_CHECK_LIB_ICONV([libiconv], [-liconv],
         [
          have_dep="yes"
          m4_defn([DOWN])[]_requirements_libs="$m4_defn([DOWN])[]_requirements_libs -liconv"
         ],
         [have_dep="no"])
   fi

   if test "x${have_dep}" = "xno" ; then
      _EFL_CHECK_LIB_ICONV([libiconv_plug],  [-liconv_plug],
         [
          have_dep="yes"
          m4_defn([DOWN])[]_requirements_libs="$m4_defn([DOWN])[]_requirements_libs -liconv_plug"
         ],
         [have_dep="no"])
   fi
fi

AS_IF([test "x${have_dep}" = "xyes"], [$2], [$3])

m4_popdef([DOWN])
])

dnl Macro that checks for zlib
dnl
dnl EFL_CHECK_LIB_ZLIB(EFL[, ACTION-IF-FOUND[ ,ACTION-IF-NOT-FOUND]])
dnl update requirements

AC_DEFUN([EFL_CHECK_LIB_ZLIB],
[
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

PKG_CHECK_MODULES([ZLIB], [zlib],
   [
    have_dep="yes"
    m4_defn([DOWN])[]_requirements_pc="$m4_defn([DOWN])[]_requirements_pc zlib"
   ],
   [have_dep="no"])

if test "x${have_dep}" = "xno" ; then
   AC_CHECK_HEADER([zlib.h], [have_dep="yes"], [have_dep="no"])

   if test "x${have_dep}" = "xyes" ; then
      AC_CHECK_LIB([z], [zlibVersion],
         [
          have_dep="yes"
          ZLIB_CFLAGS=
          ZLIB_LIBS="-lz"
          m4_defn([DOWN])[]_requirements_libs="$m4_defn([DOWN])[]_requirements_libs -lz"
         ],
         [have_dep="no"])
   fi
fi

AS_IF([test "x${have_dep}" = "xyes"], [$2], [$3])

m4_popdef([DOWN])
])

dnl Macro that checks for libjpeg
dnl
dnl EFL_CHECK_LIB_LIBJPEG(EFL[, ACTION-IF-FOUND[ ,ACTION-IF-NOT-FOUND]])
dnl update requirements

AC_DEFUN([EFL_CHECK_LIB_LIBJPEG],
[
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

AC_CHECK_HEADER([jpeglib.h], [have_dep="yes"], [have_dep="no"])

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([jpeg], [jpeg_std_error],
      [
       have_dep="yes"
       LIBJPEG_CFLAGS=
       LIBJPEG_LIBS="-ljpeg"
       m4_defn([DOWN])[]_requirements_libs="$m4_defn([DOWN])[]_requirements_libs -ljpeg"
      ],
      [have_dep="no"])
fi

AS_IF([test "x${have_dep}" = "xyes"], [$2], [$3])

m4_popdef([DOWN])
])

dnl Macro that checks for a library
dnl
dnl EFL_CHECK_LIB(EFL, LIBRARY-FILE[, ACTION-IF-FOUND[ ,ACTION-IF-NOT-FOUND]])
dnl AC_SUBST : EFL_CFLAGS and EFL_LIBS (EFL being replaced by its value)
dnl AM_CONDITIONAL : EFL_HAVE_EFL (the 2nd EFL being replaced by its value)

AC_DEFUN([EFL_CHECK_LIB],
[
m4_pushdef([UPEFL], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([UP], m4_translit([$2], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$2], [-A-Z], [_a-z]))dnl

m4_default([EFL_CHECK_LIB_]m4_defn([UP]))($1, [have_lib="yes"], [have_lib="no"])

AC_ARG_VAR(UP[_CFLAGS], [preprocessor flags for $2])
AC_SUBST(UP[_CFLAGS])
AC_ARG_VAR(UP[_LIBS], [linker flags for $2])
AC_SUBST(UP[_LIBS])

AM_CONDITIONAL([EFL_HAVE_]UPEFL, [test "x${have_lib}" = "xyes"])

m4_popdef([DOWN])
m4_popdef([UP])
m4_popdef([UPEFL])
])

dnl Macro that iterates over a sequence of white separated libraries
dnl and that call EFL_CHECK_LIB() for each of these libraries
dnl
dnl EFL_CHECK_LIBS(EFL, LIBRARY-FILE)

AC_DEFUN([EFL_CHECK_LIBS],
[
m4_foreach_w([lib], [$2], [EFL_CHECK_LIB($1, m4_defn([lib]))])
])
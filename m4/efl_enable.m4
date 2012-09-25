dnl Copyright (C) 2012 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that allows an EFL to not be built

dnl Usage: EFL_ENABLE_LIB(lib, want)
dnl Call AC_SUBST(BINARY_PRG) (BINARY is the uppercase of binary, - being transformed into _)
dnl Define have_lib (- is transformed into _)
dnl Define conditional EFL_BUILD_LIB (LIB is the uppercase of lib, - being transformed into _)

AC_DEFUN([EFL_ENABLE_LIB],
[

m4_pushdef([UP], m4_translit([[$1]], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([[$1]], [-A-Z], [_a-z]))dnl

dnl configure option

AC_ARG_ENABLE([build-$1],
   [AC_HELP_STRING([--disable-build-$1], [disable building of ]DOWN)],
   [
    if test "x${enableval}" = "xyes" ; then
       efl_want_build_[]m4_defn([DOWN])="yes"
    else
       efl_want_build_[]m4_defn([DOWN])="no"
    fi
   ],
   [efl_want_build_[]m4_defn([DOWN])=$2])

AC_MSG_CHECKING([whether to build ]DOWN[ binary])
AC_MSG_RESULT([${efl_want_build_[]m4_defn([DOWN])}])

AM_CONDITIONAL(EFL_BUILD_[]UP, test "x${efl_want_build_[]m4_defn([DOWN])}" = "xyes")
])

dnl Copyright (C) 2010 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if a binary is built or not

dnl Usage: EFL_ENABLE_BIN(binary)
dnl Call AC_SUBST(BINARY_PRG) (BINARY is the uppercase of binary, - being tranformed into _)
dnl Define have_binary (- is tranformed into _)
dnl Define conditional BUILD_BINARY (BINARY is the uppercase of binary, - being tranformed into _)

AC_DEFUN([EFL_ENABLE_BIN],
[

m4_pushdef([UP], m4_translit([[$1]], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([[$1]], [-A-Z], [_a-z]))dnl

have_[]m4_defn([DOWN])="yes"

dnl configure option

AC_ARG_ENABLE([$1],
   [AC_HELP_STRING([--disable-$1], [disable building of ]DOWN)],
   [
    if test "x${enableval}" = "xyes" ; then
       have_[]m4_defn([DOWN])="yes"
    else
       have_[]m4_defn([DOWN])="no"
    fi
   ])

AC_MSG_CHECKING([whether to build ]DOWN[ binary])
AC_MSG_RESULT([$have_[]m4_defn([DOWN])])

if test "x$have_[]m4_defn([DOWN])" = "xyes"; then
   UP[]_PRG=DOWN[${EXEEXT}]
fi

AC_SUBST(UP[]_PRG)

AM_CONDITIONAL(BUILD_[]UP, test "x$have_[]m4_defn([DOWN])" = "xyes")

AS_IF([test "x$have_[]m4_defn([DOWN])" = "xyes"], [$2], [$3])

])

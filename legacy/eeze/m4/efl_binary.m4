dnl Copyright (C) 2010 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that checks if a binary is built or not

dnl Usage: EFL_ENABLE_BIN(binary, dep[, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Call AC_SUBST(BINARY_PRG) (BINARY is the uppercase of binary, - being transformed into _)
dnl Define have_binary (- is transformed into _)
dnl Define conditional BUILD_BINARY (BINARY is the uppercase of binary, - being transformed into _)

AC_DEFUN([EFL_ENABLE_BIN],
[

m4_pushdef([UP], m4_translit([[$1]], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([[$1]], [-A-Z], [_a-z]))dnl

dnl configure option

AC_ARG_ENABLE([$1],
   [AC_HELP_STRING([--disable-$1], [disable building of ]DOWN)],
   [
    if test "x${enableval}" = "xyes" ; then
       have_[]m4_defn([DOWN])="yes"
    else
       have_[]m4_defn([DOWN])="no"
    fi
   ],
   [have_[]m4_defn([DOWN])=$2])

AC_MSG_CHECKING([whether to build ]DOWN[ binary])
AC_MSG_RESULT([$have_[]m4_defn([DOWN])])

if test "x$have_[]m4_defn([DOWN])" = "xyes"; then
   UP[]_PRG=DOWN[${EXEEXT}]
fi

AC_SUBST(UP[]_PRG)

AM_CONDITIONAL(BUILD_[]UP, test "x$have_[]m4_defn([DOWN])" = "xyes")

AS_IF([test "x$have_[]m4_defn([DOWN])" = "xyes"], [$3], [$4])

])

dnl Macro that specifies the binary to be used

dnl Usage: EFL_WITH_BIN(binary, package, msg)
dnl Call AC_SUBST(BINARY_PRG) (BINARY is the uppercase of binary, - being transformed into _)
dnl Define with_binary (- is transformed into _)
dnl Define conditional BUILD_BINARY (BINARY is the uppercase of binary, - being transformed into _)

AC_DEFUN([EFL_WITH_BIN],
[

m4_pushdef([UP], m4_translit([[$1]], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([[$1]], [-A-Z], [_a-z]))dnl

AC_REQUIRE([PKG_PROG_PKG_CONFIG])
AC_MSG_NOTICE([$PKG_CONFIG])

with_[]m4_defn([DOWN])=m4_esyscmd($PKG_CONFIG --variable=prefix $2)/bin/m4_defn([DOWN])

dnl configure option

AC_ARG_WITH([$1],
   [AC_HELP_STRING([--with-$1-bin=PATH], [specify a specific path to ]DOWN)],
   [
    with_[]m4_defn([DOWN])=$withval
    _efl_msg="( explicitely set)"
   ])

AC_MSG_NOTICE([$msg: ]m4_defn([DOWN])[$_efl_msg])

AC_SUBST(with_[]m4_defn([DOWN]))

AS_IF([test "x$have_[]m4_defn([DOWN])" = "xyes"], [$4], [$5])

])

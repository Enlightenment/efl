dnl Usage: EFL_WITH_BIN_SUFFIX(package, binary, suffix)
dnl Call AC_SUBST(_binary) (_binary is the lowercase of binary, - being transformed into _ by default, or the value set by the user)

AC_DEFUN([EFL_WITH_BIN_SUFFIX],
[

m4_pushdef([DOWN], m4_translit([[$2]], [-A-Z], [_a-z]))dnl
m4_pushdef([UP], m4_translit([[$2]], [-a-z], [_A-Z]))dnl
dnl configure option

_efl_with_binary=""
_efl_binary_define="no"

AC_ARG_WITH([$2],
   [AC_HELP_STRING([--with-$2=PATH], [specify a specific path to ]DOWN[ @<:@default=]DOWN[@:>@])],
   [
    _efl_with_binary=${withval}
    _efl_binary_define="yes"
   ], [])

AC_ARG_WITH([bin-$2],
   [AC_HELP_STRING([--with-bin-$2=PATH], [specify a specific path to ]DOWN[ @<:@default=]DOWN[@:>@ DEPRECATED])],
   [
    _efl_with_binary=${withval}
    _efl_binary_define="yes"
    efl_deprecated_option="yes"
   ], [])

DOWN=${_efl_with_binary}
AC_MSG_NOTICE(DOWN[ set to ${_efl_with_binary}])

with_binary_[]m4_defn([DOWN])[]$3=${_efl_with_binary}

AM_CONDITIONAL(HAVE_[]UP[]m4_translit([[$3]], [a-z], [A-Z]), [test "x${_efl_binary_define}" = "xyes"])
AC_SUBST(DOWN)

])

dnl Usage: EFL_WITH_BIN(package, binary)
dnl Call AC_SUBST(_binary) (_binary is the lowercase of binary, - being transformed into _ by default, or the value set by the user)

AC_DEFUN([EFL_WITH_BIN], [EFL_WITH_BIN_SUFFIX([$1], [$2], [])])

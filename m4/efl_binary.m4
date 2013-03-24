

dnl Usage: EFL_WITH_BIN(package, binary)
dnl Call AC_SUBST(_binary) (_binary is the lowercase of binary, - being transformed into _ by default, or the value set by the user)

AC_DEFUN([EFL_WITH_BIN],
[

m4_pushdef([DOWN], m4_translit([[$2]], [-A-Z], [_a-z]))dnl

dnl configure option

AC_ARG_WITH([bin-$2],
   [AC_HELP_STRING([--with-bin-$2=PATH], [specify a specific path to ]DOWN[ @<:@default=]DOWN[@:>@])],
   [_efl_with_binary=${withval}],
   [_efl_with_binary="EFL_RUN_IN_TREE=1 $(top_builddir)/bin/$1/]DOWN")

DOWN=${_efl_with_binary}
AC_MSG_NOTICE(DOWN[ set to ${_efl_with_binary}])

with_binary_[]m4_defn([DOWN])=${_efl_with_binary}

AC_SUBST(DOWN)

])

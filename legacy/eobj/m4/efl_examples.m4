dnl Copyright (C) 2008 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if building examples is wanted.

dnl Usage: EFL_CHECK_BUILD_EXAMPLES([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Defines the automake conditionnal EFL_ENABLE_BUILD_EXAMPLES

AC_DEFUN([EFL_CHECK_BUILD_EXAMPLES],
[

dnl configure option

AC_ARG_ENABLE([build-examples],
   [AC_HELP_STRING([--enable-build-examples], [enable building examples @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       _efl_enable_build_examples="yes"
    else
       _efl_enable_build_examples="no"
    fi
   ],
   [_efl_enable_build_examples="no"])

AC_MSG_CHECKING([whether examples are built])
AC_MSG_RESULT([${_efl_enable_build_examples}])

AM_CONDITIONAL(EFL_BUILD_EXAMPLES, test "x${_efl_enable_build_examples}" = "xyes")

AS_IF([test "x$_efl_enable_build_examples" = "xyes"], [$1], [$2])
])


dnl Macro that check if installing examples is wanted.

dnl Usage: EFL_CHECK_INSTALL_EXAMPLES([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Defines the automake conditionnal EFL_ENABLE_INSTALL_EXAMPLES

AC_DEFUN([EFL_CHECK_INSTALL_EXAMPLES],
[

dnl configure option

AC_ARG_ENABLE([install-examples],
   [AC_HELP_STRING([--enable-install-examples], [enable installing example source files @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       _efl_enable_install_examples="yes"
    else
       _efl_enable_install_examples="no"
    fi
   ],
   [_efl_enable_install_examples="no"])

AC_MSG_CHECKING([whether examples are installed])
AC_MSG_RESULT([${_efl_enable_install_examples}])

AM_CONDITIONAL(EFL_INSTALL_EXAMPLES, test "x${_efl_enable_install_examples}" = "xyes")

AS_IF([test "x$_efl_enable_install_examples" = "xyes"], [$1], [$2])
])

dnl End of efl_examples.m4

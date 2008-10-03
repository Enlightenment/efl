dnl Copyright (C) 2008 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if benchmark support is wanted.

dnl Usage: EFL_CHECK_BENCHMARK([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Defines the automake conditionnal EFL_ENABLE_BENCHMARK

AC_DEFUN([EFL_CHECK_BENCHMARK],
[

dnl configure option

AC_ARG_ENABLE([benchmark],
   [AC_HELP_STRING([--enable-benchmark], [enable tests @<:@default=no@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       _efl_enable_benchmark="yes"
    else
       _efl_enable_benchmark="no"
    fi
   ],
   [_efl_enable_benchmark="no"]
)
AC_MSG_CHECKING([whether benchmark are built])
AC_MSG_RESULT([${_efl_enable_benchmark}])

AM_CONDITIONAL(EFL_ENABLE_BENCHMARK, test "x${_efl_enable_benchmark}" = "xyes")

if test "x${_efl_enable_benchmark}" = "xyes" ; then
   ifelse([$1], , :, [$1])
else
   ifelse([$2], , :, [$2])
fi
])

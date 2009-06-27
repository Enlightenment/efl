dnl Copyright (C) 2009 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that enables scaler support is wanted.

dnl Usage: EVAS_CHECK_SCALER(scaler, description [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl The parameter 'scaler' is the type of scaler.
dnl The parameter 'description' is the description of the scaler.
dnl Defines BUILD_[TYPEOFSCALER]

AC_DEFUN([EVAS_CHECK_SCALER],
[
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

AC_ARG_ENABLE([$1],
   AC_HELP_STRING([--disable-$1], [disable $2 code @<:@default=enabled@:>@]),
   [
    if test "x${enableval}" = "xyes" ; then
       _efl_enable_scaler_option_[]DOWN="yes"
    else
       _efl_enable_scaler_option_[]DOWN="no"
    fi
   ],
   [_efl_enable_scaler_option_[]DOWN="yes"])

AC_MSG_CHECKING(whether to build $2)
AC_MSG_RESULT([${_efl_enable_scaler_option_[]DOWN}])

if test "x${_efl_enable_scaler_option_[]DOWN}" = "xyes" ; then
   AC_DEFINE([BUILD_]UP[], [1], [define to 1 if you have the $2 support])
fi

if test "x${_efl_enable_scaler_option_[]DOWN}" = "xyes" ; then
   m4_default([$3], [:])
else
   m4_default([$4], [:])
fi

m4_popdef([UP])
m4_popdef([DOWN])
])

dnl End of evas_scaler.m4

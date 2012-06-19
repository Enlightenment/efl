dnl Copyright (C) 2012 Vincent Torri <vincent dot torri at gmail dot com>
dnl This code is public domain and can be freely used or copied.

dnl Macro that check if compiler of linker flags are available


dnl Macro that checks for a compiler flag availability
dnl
dnl EFL_CHECK_COMPILER_FLAG(EFL, FLAG[, ACTION-IF-FOUND[ ,ACTION-IF-NOT-FOUND]])
dnl AC_SUBST : EFL_CFLAGS (EFL being replaced by its value)
dnl AM_CONDITIONAL : EFL_HAVE_FLAG (FLAG being replaced by its value)

AC_DEFUN([EFL_CHECK_COMPILER_FLAG],
[
m4_pushdef([UPEFL], m4_translit([[$1]], [-a-z], [_A-Z]))
m4_pushdef([UP], m4_translit([[$2]], [-a-z], [_A-Z]))

dnl store in options -Wfoo if -Wno-foo is passed
option=m4_bpatsubst([[$2]], [-Wno-], [-W])

CFLAGS_save="${CFLAGS}"
CFLAGS="${CFLAGS} ${option}"

AC_LANG_PUSH([C])
AC_MSG_CHECKING([whether the compiler supports $2])

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM([[]])],
   [have_flag="yes"],
   [have_flag="no"])
AC_MSG_RESULT([${have_flag}])

CFLAGS="${CFLAGS_save}"
AC_LANG_POP([C])

if test "x${have_flag}" = "xyes" ; then
   UPEFL[_CFLAGS]="${UPEFL[_CFLAGS]} [$2]"
fi
AC_ARG_VAR(UPEFL[_CFLAGS], [preprocessor flags for $2])
AC_SUBST(UPEFL[_CFLAGS])

AM_CONDITIONAL([EFL_HAVE]UP, [test "x${have_flag}" = "xyes"])

m4_popdef([UP])
m4_popdef([UPEFL])
])

dnl Macro that iterates over a sequence of white separated flags
dnl and that call EFL_CHECK_COMPILER_FLAG() for each of these flags
dnl
dnl EFL_CHECK_COMPILER_FLAGS(EFL, FLAGS)

AC_DEFUN([EFL_CHECK_COMPILER_FLAGS],
[
m4_foreach_w([flag], [$2], [EFL_CHECK_COMPILER_FLAG([$1], m4_defn([flag]))])
])

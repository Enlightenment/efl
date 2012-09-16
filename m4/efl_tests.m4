dnl Copyright (C) 2008-2012 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if tests programs are wanted and if yes, if
dnl the Check library is available.
dnl the lcov program is available.

dnl Usage: EFL_CHECK_TESTS(EFL[, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Define the automake conditionnal EFL_ENABLE_TESTS

AC_DEFUN([EFL_CHECK_TESTS],
[
m4_pushdef([UPEFL], m4_translit([$1], [-a-z], [_A-Z]))dnl

dnl configure option

AC_REQUIRE([PKG_PROG_PKG_CONFIG])

PKG_CHECK_MODULES([CHECK],
   [check >= 0.9.5],
   [_efl_enable_tests="yes"],
   [_efl_enable_tests="no"])

AM_CONDITIONAL([EFL_ENABLE_TESTS], [test "x${_efl_enable_tests}" = "xyes"])

_efl_enable_coverage="no"
AC_CHECK_PROG(have_lcov, [lcov], [yes], [no])
if test "x$have_lcov" = "xyes" ; then
   m4_defn([UPEFL])[]_CFLAGS="${m4_defn([UPEFL])[]_CFLAGS} -fprofile-arcs -ftest-coverage"
   m4_defn([UPEFL])[]_LIBS="${m4_defn([UPEFL])[]_LIBS} -lgcov"
# remove any optimisation flag and force debug symbols
   if test "x${prefer_assert}" = "xno"; then
      m4_defn([UPEFL])[]_CFLAGS="${m4_defn([UPEFL])[]_CFLAGS} -DNDEBUG"
   else
      m4_defn([UPEFL])[]_CFLAGS="${m4_defn([UPEFL])[]_CFLAGS} -g -O0 -DDEBUG"
   fi
   _efl_enable_coverage="yes"
else
   AC_MSG_WARN([lcov is not found, disable profiling instrumentation])
fi

AM_CONDITIONAL([EFL_ENABLE_COVERAGE], [test "x${_efl_enable_coverage}" = "xyes"])

AS_IF([test "x$_efl_enable_tests" = "xyes"], [$2], [$3])

m4_popdef([UPEFL])
])

dnl End of efl_tests.m4

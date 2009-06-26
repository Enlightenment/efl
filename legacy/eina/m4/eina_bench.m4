dnl use: EINA_BENCH_MODULE(foo-bar, have_bench, pkg_file [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
AC_DEFUN([EINA_BENCH_MODULE],
[
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

want_module="$2"
have_module="no"

AC_ARG_ENABLE([$1],
   [AC_HELP_STRING([--disable-benchmark-$1], [disable benchmarking against ]DOWN[ @<:@default=yes@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_module="yes"
    else
       enable_module="no"
    fi
   ],
   [enable_module="yes"])

if test "x${enable_module}" = "xyes" -a "x${want_module}" = "xyes"; then
   PKG_CHECK_MODULES([]UP[], [$3], [have_module="yes"], [have_module="no"])
fi

AM_CONDITIONAL([EINA_HAVE_]UP[], [test "x${have_module}" = "xyes"])

if test "x${have_module}" = "xyes" ; then
   ifelse([$4], , :, [$4])
else
   ifelse([$5], , :, [$5])
fi

m4_popdef([UP])
m4_popdef([DOWN])
])

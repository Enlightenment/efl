dnl
dnl EFL_CHECK_DEFINE(symbol, header_file)
dnl
dnl NOTE: EFL_CHECK_DEFINE is strongly inspired by
dnl       APR_CHECK_DEFINE which can be found in the
dnl       sources of Apache's APR (under Apache Licence)
dnl
AC_DEFUN([EFL_CHECK_DEFINE], [
  AC_CACHE_CHECK([for $1 in $2],ac_cv_define_$1,[
    AC_EGREP_CPP(YES_IS_DEFINED, [
#include <$2>
#ifdef $1
YES_IS_DEFINED
#endif
    ], ac_cv_define_$1=yes, ac_cv_define_$1=no)
  ])
  if test "$ac_cv_define_$1" = "yes"; then
    AC_DEFINE(HAVE_$1, 1, [Define if $1 is defined in $2])
  fi
])

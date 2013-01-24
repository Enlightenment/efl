AC_DEFUN([EFL_CHECK_GCC_BUILTIN],
[efl_check_gcc_builtin_save_libs=$LIBS
LIBS="-lm $LIBS"
AC_LINK_IFELSE(
[AC_LANG_PROGRAM(
[[#ifndef __GNUC__
choke me
#else
#undef $1
/* Declare this function with same prototype as __builtin_$1.
  This removes warning about conflicting type with builtin */
__typeof__(__builtin_$1) $1;

__typeof__(__builtin_$1) *f = $1;
#endif
]], [[return f != $1;]]
)],
[AC_DEFINE([$2], [123], [GCC builtin $1 exists])])
LIBS=$efl_check_gcc_builtin_save_libs])


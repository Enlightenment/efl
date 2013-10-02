AC_DEFUN([EFL_CHECK_GCC_BUILTIN],
[efl_check_gcc_builtin_save_libs=$LIBS
EINA_CONFIGURE_$2=""
LIBS="-lm $LIBS"
AC_LINK_IFELSE(
[AC_LANG_PROGRAM(
[[]], [[return __builtin_$1(42);]]
)],
[EINA_CONFIGURE_$2="#define EINA_$2"
AC_DEFINE([$2], [1], [GCC builtin $1 exists])])
AC_SUBST(EINA_CONFIGURE_$2)
LIBS=$efl_check_gcc_builtin_save_libs])


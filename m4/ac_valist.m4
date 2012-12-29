dnl That code is public domain and can be freely used or copied.
dnl Originally snatched from somewhere...

dnl Macro for checking if va_list is an array

dnl Usage: AC_C_VA_LIST_AS_ARRAY
dnl call AC_DEFINE for HAVE_VA_LIST_AS_ARRAY
dnl if for this architecture va_list is defined as an array

AC_DEFUN([AC_C_VA_LIST_AS_ARRAY],
[

AC_MSG_CHECKING([whether va_list is defined as an array])

AC_CACHE_VAL([ac_cv_valistasarray],
   [AC_TRY_RUN(
       [
#include <stdlib.h>
#include <stdarg.h>
void foo(int i, ...)
{
	va_list ap1, ap2;
	va_start(ap1, i);
	ap2 = ap1;
	if (va_arg(ap2, int) != 123 || va_arg(ap1, int) != 123)
		exit(1);
	va_end(ap1);
}
int main(void)
{
	foo(0, 123);
	return(0);
}
       ],
       [ac_cv_valistasarray="no"],
       [ac_cv_valistasarray="yes"],
       [ac_cv_valistasarray="no"]
    )])

AC_MSG_RESULT($ac_cv_valistasarray)

if test "x${ac_cv_valistasarray}" = "xyes" ; then
   AC_DEFINE([HAVE_VA_LIST_AS_ARRAY], [1], [Define to 1 if va_list is an array])
fi

])

dnl End of ac_valist.m4

dnl Copyright (C)  2013  ProFUSION embedded systems
dnl This code is public domain and can be freely used or copied.

dnl Macro for checking if va_list is an array

dnl Usage: AC_C_VA_LIST_AS_ARRAY
dnl call AC_DEFINE for HAVE_VA_LIST_AS_ARRAY if for this architecture
dnl va_list is defined as an array

AC_DEFUN([AC_C_VA_LIST_AS_ARRAY],
[
AC_MSG_CHECKING([whether va_list is defined as an array])
AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
      [[
#include <stdlib.h>
#include <stdarg.h>

#define BUILD_ASSERT(cond) \
      do { (void) sizeof(char [1 - 2*!(cond)]); } while(0)
      ]],
      [[
va_list ap;
BUILD_ASSERT(__builtin_types_compatible_p(typeof(ap),
            typeof(&(ap)[0])));
return 0;
      ]])],
       [have_va_list_as_array="no"],
       [have_va_list_as_array="yes"])

AC_MSG_RESULT([${have_va_list_as_array}])

if test "x${have_va_list_as_array}" = "xyes" ; then
   AC_DEFINE([HAVE_VA_LIST_AS_ARRAY], [1], [Define to 1 if va_list is an array])
fi

])

dnl End of ac_valist.m4

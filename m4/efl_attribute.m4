dnl Copyright (C) 2011 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macros for checking if the compiler supports some __attribute__ uses

dnl Usage: EFL_ATTRIBUTE_VECTOR
dnl call AC_DEFINE for HAVE_GCC_ATTRIBUTE_VECTOR if __attribute__((vector)) is available

AC_DEFUN([EFL_ATTRIBUTE_VECTOR],
[
AC_MSG_CHECKING([for __attribute__ ((vector))])
AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
       [[
typedef int v4si __attribute__ ((vector_size (16)));
       ]],
       [[
if (sizeof(v4si) == 16)
  return 0;
else
  return -1;
       ]])],
   [have_attribute_vector="yes"],
   [have_attribute_vector="no"])
AC_MSG_RESULT([${have_attribute_vector}])

if test "x${have_attribute_vector}" = "xyes" ; then
   AC_DEFINE([HAVE_GCC_ATTRIBUTE_VECTOR], [1], [Define to 1 if your compiler supports __attribute__ ((vector)).])
fi
])

dnl End of efl_attribute.m4

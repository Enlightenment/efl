dnl Copyright (C) 2011 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macros for checking if the compiler supports some __attribute__ uses

dnl Usage: EFL_ATTRIBUTE_UNUSED
dnl call AC_DEFINE for __UNUSED__ if __attribute__((unused)) is available

AC_DEFUN([EFL_ATTRIBUTE_UNUSED],
[
AC_MSG_CHECKING([for __attribute__ ((unused))])
AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
       [[
void foo(int x __attribute__ ((unused))) {}
       ]],
       [[
       ]])],
   [have_attribute_unused="yes"],
   [have_attribute_unused="no"])
AC_MSG_RESULT([${have_attribute_unused}])

if test "x${have_attribute_unused}" = "xyes" ; then
   AC_DEFINE([__UNUSED__], [__attribute__ ((unused))], [Macro declaring a function argument to be unused.])
else
   AC_DEFINE([__UNUSED__], [], [__attribute__ ((unused)) is not supported.])
fi
])

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

dnl Usage: EFL_ATTRIBUTE_ALWAYS_INLINE
dnl call AC_DEFINE for alway_inline if __attribute__((always_inline)) is available

AC_DEFUN([EFL_ATTRIBUTE_ALWAYS_INLINE],
[

have_attribute_forceinline="no"

AC_MSG_CHECKING([for __forceinline])

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
       [[
#include <windows.h>
__forceinline void foo(void) {}
       ]],
       [[
       ]])],
   [
    have_attribute_always_inline="yes"
    have_attribute_forceinline="yes"
   ],
   [have_attribute_always_inline="no"])

AC_MSG_RESULT([${have_attribute_always_inline}])

if test "x${have_attribute_always_inline}" = "xno" ; then
   AC_MSG_CHECKING([for __attribute__ ((always_inline))])
   AC_COMPILE_IFELSE(
      [AC_LANG_PROGRAM(
          [[
__attribute__((always_inline)) inline void foo(void) {}
          ]],
          [[
          ]])],
      [have_attribute_always_inline="yes"],
      [have_attribute_always_inline="no"])
   AC_MSG_RESULT([${have_attribute_always_inline}])
fi

if test "x${have_attribute_always_inline}" = "xyes" ; then
   if test "x${have_attribute_forceinline}" = "xyes" ; then
      AC_DEFINE([EFL_ALWAYS_INLINE], [__forceinline], [Macro declaring a function to always be inlined.])
   else
      AC_DEFINE([EFL_ALWAYS_INLINE], [__attribute__ ((always_inline)) inline], [Macro declaring a function to always be inlined.])
   fi
else
   AC_DEFINE([EFL_ALWAYS_INLINE], [static inline], [Macro declaring a function to always be inlined.])
fi
])

dnl End of efl_attribute.m4

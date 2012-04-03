dnl Copyright (C) 2012 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if dlopen and dladdr functions are available or not.

dnl Usage: EFL_CHECK_DLOPEN([, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Call AC_SUBST(EFL_DLOPEN_LIBS)
dnl Define HAVE_DLOPEN to 1if dlopen is available

AC_DEFUN([EFL_CHECK_DLOPEN],
[
case "$host_os" in
   mingw*)
      _efl_have_dlopen="yes"
      EFL_DLOPEN_LIBS="-ldl"
      ;;
   *)
   _efl_have_dlopen="no"

dnl Check is dlopen is in libc

   AC_MSG_CHECKING([for dlopen in libc])
   AC_LINK_IFELSE(
      [AC_LANG_PROGRAM(
          [[
#include <dlfcn.h>
          ]],
          [[
void *h = dlopen(0, 0);
          ]])],
      [_efl_have_dlopen="yes"],
      [_efl_have_dlopen="no"])

   AC_MSG_RESULT([${_efl_have_dlopen}])

   if test "x${_efl_have_dlopen}" = "xno" ; then
      AC_MSG_CHECKING([for dlopen in libdl])

      LIBS_save="${LIBS}"
      LIBS="${LIBS} -ldl"
      AC_LINK_IFELSE(
         [AC_LANG_PROGRAM(
             [[
#include <dlfcn.h>
             ]],
             [[
void *h = dlopen(0, 0);
             ]])],
         [
          EFL_DLOPEN_LIBS="-ldl"
          _efl_have_dlopen="yes"
         ],
         [_efl_have_dlopen="no"])

      LIBS="${LIBS_save}"

      AC_MSG_RESULT([${_efl_have_dlopen}])
   fi
   ;;
esac

AC_SUBST([EFL_DLOPEN_LIBS])

if test "x${_efl_have_dlopen}" = "xyes" ; then
   AC_DEFINE([HAVE_DLOPEN], [1], [Define to 1 if you have the `dlopen' function.])
fi

AS_IF([test "x${_efl_have_dlopen}" = "xyes"], [$1], [$2])
])

dnl Usage: EFL_CHECK_DLADDR([, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Call AC_SUBST(EFL_DLADDR_LIBS)
dnl Define HAVE_DLADDR to 1if dladdr is available

AC_DEFUN([EFL_CHECK_DLADDR],
[
case "$host_os" in
   mingw*)
      _efl_have_dladdr="yes"
      EFL_DLADDR_LIBS="-ldl"
      ;;
   *)
   _efl_have_dladdr="no"

dnl Check is dladdr is in libc

   AC_MSG_CHECKING([for dladdr in libc])
   AC_LINK_IFELSE(
      [AC_LANG_PROGRAM(
          [[
#define _GNU_SOURCE
#include <dlfcn.h>
          ]],
          [[
int res = dladdr(0, 0);
          ]])],
      [_efl_have_dladdr="yes"],
      [_efl_have_dladdr="no"])

   AC_MSG_RESULT([${_efl_have_dladdr}])

   if test "x${_efl_have_dladdr}" = "xno" ; then
      AC_MSG_CHECKING([for dladdr in libdl])

      LIBS_save="${LIBS}"
      LIBS="${LIBS} -ldl"
      AC_LINK_IFELSE(
         [AC_LANG_PROGRAM(
             [[
#define _GNU_SOURCE
#include <dlfcn.h>
             ]],
             [[
int res = dladdr(0, 0);
             ]])],
         [
          EFL_DLADDR_LIBS="-ldl"
          _efl_have_dladdr="yes"
         ],
         [_efl_have_dladdr="no"])

      LIBS="${LIBS_save}"

      AC_MSG_RESULT([${_efl_have_dladdr}])
   fi
   ;;
esac

AC_SUBST([EFL_DLADDR_LIBS])

if test "x${_efl_have_dladdr}" = "xyes" ; then
   AC_DEFINE([HAVE_DLADDR], [1], [Define to 1 if you have the `dladdr' function.])
fi

AS_IF([test "x${_efl_have_dladdr}" = "xyes"], [$1], [$2])
])

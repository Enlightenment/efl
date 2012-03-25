dnl Copyright (C) 2010 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if shm_open function is available or not.

dnl Usage: EFL_CHECK_SHM_OPEN([, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Call AC_SUBST(EFL_SHM_OPEN_LIBS)
dnl Define HAVE_SHM_OPEN to 1if shm_open is available

AC_DEFUN([EFL_CHECK_SHM_OPEN],
[
_efl_have_shm_open="no"

dnl Check is shm_open is in libc

AC_MSG_CHECKING([for shm_open in libc])
AC_LINK_IFELSE(
   [AC_LANG_PROGRAM(
       [[
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
       ]],
       [[
int fd;

fd = shm_open("/dev/null", O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
       ]])],
   [_efl_have_shm_open="yes"],
   [_efl_have_shm_open="no"])

AC_MSG_RESULT([${_efl_have_shm_open}])

if test "x${_efl_have_shm_open}" = "xno" ; then
   AC_MSG_CHECKING([for shm_open in librt])

   LIBS_save="${LIBS}"
   LIBS="${LIBS} -lrt"
   AC_LINK_IFELSE(
      [AC_LANG_PROGRAM(
          [[
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
          ]],
          [[
int fd;

fd = shm_open("/dev/null", O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
          ]])],
      [
       EFL_SHM_OPEN_LIBS="-lrt"
       _efl_have_shm_open="yes"
      ],
      [_efl_have_shm_open="no"])

   LIBS="${LIBS_save}"

   AC_MSG_RESULT([${_efl_have_shm_open}])
fi

AC_SUBST([EFL_SHM_OPEN_LIBS])

if test "x${_efl_have_shm_open}" = "xyes" ; then
   AC_DEFINE([HAVE_SHM_OPEN], [1], [Define to 1 if you have the `shm_open' function.])
fi

AS_IF([test "x${_efl_have_shm_open}" = "xyes"], [$1], [$2])

])

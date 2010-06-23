dnl Copyright (C) 2010 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if several pthread library is available or not.

dnl Usage: EFL_CHECK_PTHREAD(want_pthread_spin[, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Call AC_SUBST(EFL_PTHREAD_CFLAGS)
dnl Call AC_SUBST(EFL_PTHREAD_LIBS)
dnl Define EFL_HAVE_PTHREAD
dnl Define EFL_HAVE_PTHREAD_SPINLOCK

AC_DEFUN([EFL_CHECK_PTHREAD],
[

dnl configure option

AC_ARG_ENABLE([pthread],
   [AC_HELP_STRING([--disable-pthread], [enable POSIX threads code @<:@default=auto@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       _efl_enable_pthread="yes"
    else
       _efl_enable_pthread="no"
    fi
   ],
   [_efl_enable_pthread="auto"])

AC_MSG_CHECKING([whether to build POSIX threads code])
AC_MSG_RESULT([${_efl_enable_pthread}])

dnl check if the compiler supports pthreads

case "$host_os" in
   mingw*)
      _efl_pthread_cflags=""
      _efl_pthread_libs="-lpthreadGC2"
      ;;
   solaris*)
      _efl_pthread_cflags="-mt"
      _efl_pthread_libs="-mt"
      ;;
   *)
      _efl_pthread_cflags="-pthread"
      _efl_pthread_libs="-pthread"
      ;;
esac

_efl_have_pthread="no"

if test "x${_efl_enable_pthread}" = "xyes" || test "x${_efl_enable_pthread}" = "xauto" ; then

   SAVE_CFLAGS=${CFLAGS}
   CFLAGS="${CFLAGS} ${_efl_pthread_cflags}"
   SAVE_LIBS=${LIBS}
   LIBS="${LIBS} ${_efl_pthread_libs}"
   AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([[
#include <pthread.h>
                       ]],
                       [[
pthread_t id;
id = pthread_self();
                       ]])],
      [_efl_have_pthread="yes"],
      [_efl_have_pthread="no"])
   CFLAGS=${SAVE_CFLAGS}
   LIBS=${SAVE_LIBS}

fi

AC_MSG_CHECKING([whether system support POSIX threads])
AC_MSG_RESULT([${_efl_have_pthread}])
if test "$x{_efl_enable_pthread}" = "xyes" && test "x${_efl_have_pthread}" = "xno"; then
   AC_MSG_ERROR([pthread support requested but not found.])
fi

EFL_PTHREAD_CFLAGS=""
EFL_PTHREAD_LIBS=""
if test "x${_efl_have_pthread}" = "xyes" ; then
   EFL_PTHREAD_CFLAGS=${_efl_pthread_cflags}
   EFL_PTHREAD_LIBS=${_efl_pthread_libs}
fi

AC_SUBST(EFL_PTHREAD_CFLAGS)
AC_SUBST(EFL_PTHREAD_LIBS)

if test "x${_efl_have_pthread}" = "xyes" ; then
   AC_DEFINE(EFL_HAVE_PTHREAD, 1, [Define to mention that POSIX threads are supported])
fi

dnl check if the compiler supports pthreads spinlock

_efl_have_pthread_spinlock="no"

if test "x${_efl_have_pthread}" = "xyes" && test "x$1" = "xyes" ; then

   SAVE_CFLAGS=${CFLAGS}
   CFLAGS="${CFLAGS} ${EFL_PTHREAD_CFLAGS}"
   SAVE_LIBS=${LIBS}
   LIBS="${LIBS} ${EFL_PTHREAD_LIBS}"
   AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([[
#include <pthread.h>
                       ]],
                       [[
pthread_spinlock_t lock;
int res;
res = pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE);
                       ]])],
      [_efl_have_pthread_spinlock="yes"],
      [_efl_have_pthread_spinlock="no"])
   CFLAGS=${SAVE_CFLAGS}
   LIBS=${SAVE_LIBS}

fi

AC_MSG_CHECKING([whether to build POSIX threads spinlock code])
AC_MSG_RESULT([${_efl_have_pthread_spinlock}])
if test "x${_efl_enable_pthread}" = "xyes" && test "x${_efl_have_pthread_spinlock}" = "xno" && test "x$1" = "xyes" ; then
   AC_MSG_WARN([pthread support requested but spinlocks are not supported])
fi

if test "x${_efl_have_pthread_spinlock}" = "xyes" ; then
   AC_DEFINE(EFL_HAVE_PTHREAD_SPINLOCK, 1, [Define to mention that POSIX threads spinlocks are supported])
fi

AS_IF([test "x$_efl_have_pthread" = "xyes"], [$2], [$3])
AS_IF([test "x$_efl_have_pthread_spinlock" = "xyes"], [$4], [$5])

])

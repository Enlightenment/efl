dnl Copyright (C) 2010 Vincent Torri <vtorri at univ-evry dot fr>
dnl rwlock code added by Mike Blumenkrantz <mike at zentific dot com>
dnl This code is public domain and can be freely used or copied.

dnl Macro that check if POSIX or Win32 threads library is available or not.

dnl Usage: EFL_CHECK_THREADS(ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND])
dnl Call AC_SUBST(EFL_PTHREAD_CFLAGS)
dnl Call AC_SUBST(EFL_PTHREAD_LIBS)
dnl Defines EFL_HAVE_POSIX_THREADS or EFL_HAVE_WIN32_THREADS, and EFL_HAVE_THREADS

AC_DEFUN([EFL_CHECK_THREADS],
[

dnl Generic thread detection

EFL_PTHREAD_CFLAGS=""
EFL_PTHREAD_LIBS=""

_efl_enable_posix_threads="no"
_efl_have_posix_threads="no"
_efl_have_win32_threads="no"

case "$host_os" in
   mingw*)
      _efl_have_win32_threads="yes"
      AC_DEFINE([EFL_HAVE_WIN32_THREADS], [1], [Define to mention that Win32 threads are supported])
      AC_DEFINE([EFL_HAVE_THREADS], [1], [Define to mention that POSIX or Win32 threads are supported])
      ;;
   solaris*)
      _efl_enable_posix_threads="yes"
      _efl_threads_cflags="-mt"
      _efl_threads_libs="-mt"
      ;;
   *)
      _efl_enable_posix_threads="yes"
      _efl_threads_cflags="-pthread"
      _efl_threads_libs="-pthread"
      ;;
esac

dnl check if the compiler supports POSIX threads


if test "x${_efl_enable_posix_threads}" = "xyes" ; then

   SAVE_CFLAGS=${CFLAGS}
   CFLAGS="${CFLAGS} ${_efl_threads_cflags}"
   SAVE_LIBS=${LIBS}
   LIBS="${LIBS} ${_efl_threads_libs}"
   AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([[
#include <pthread.h>
                       ]],
                       [[
pthread_t id;
id = pthread_self();
                       ]])],
      [
       _efl_have_posix_threads="yes"
       AC_DEFINE([EFL_HAVE_POSIX_THREADS], [1], [Define to mention that POSIX threads are supported])
       AC_DEFINE([EFL_HAVE_THREADS], [1], [Define to mention that POSIX or Win32 threads are supported])
       EFL_PTHREAD_CFLAGS=${_efl_threads_cflags}
       EFL_PTHREAD_LIBS=${_efl_threads_libs}
      ],
      [_efl_have_posix_threads="no"])
   CFLAGS=${SAVE_CFLAGS}
   LIBS=${SAVE_LIBS}

fi

AC_MSG_CHECKING([which threads API is used])
if test "x${_efl_have_posix_threads}" = "xyes" ; then
   efl_have_threads="POSIX"
else
   if test "x${_efl_have_win32_threads}" = "xyes" ; then
      efl_have_threads="Windows"
   else
      efl_have_threads="no"
   fi
fi
AC_MSG_RESULT([${efl_have_threads}])

AC_SUBST(EFL_PTHREAD_CFLAGS)
AC_SUBST(EFL_PTHREAD_LIBS)

dnl check if the compiler supports pthreads spinlock

efl_have_posix_threads_spinlock="no"

if test "x${_efl_have_posix_threads}" = "xyes" ; then

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
      [efl_have_posix_threads_spinlock="yes"],
      [efl_have_posix_threads_spinlock="no"])
   CFLAGS=${SAVE_CFLAGS}
   LIBS=${SAVE_LIBS}

fi

AC_MSG_CHECKING([whether to build POSIX threads spinlock code])
AC_MSG_RESULT([${efl_have_posix_threads_spinlock}])

if test "x${efl_have_posix_threads_spinlock}" = "xyes" ; then
   AC_DEFINE([EFL_HAVE_POSIX_THREADS_SPINLOCK], [1], [Define to mention that POSIX threads spinlocks are supported])
fi

dnl Check ON-OFF threads

_efl_enable_on_off_threads="no"
AC_ARG_ENABLE([on-off-threads],
   [AC_HELP_STRING([--enable-on-off-threads], [only turn this on if you know what you are doing, and do not complain if the world freeze])],
   [_efl_enable_on_off_threads="${enableval}"])

efl_have_on_off_threads="no"
if test "x${_efl_have_posix_threads}" = "xyes" && test "x${_efl_enable_on_off_threads}" = "xyes"; then
   efl_have_on_off_threads="yes"
   AC_DEFINE([EFL_ON_OFF_THREADS], [1], [make it possible to disable all locks])
fi
AC_MSG_CHECKING([whether to turn on/off threads lock on demand])
AC_MSG_RESULT([${efl_have_on_off_threads}])

dnl Check debug threads

_efl_enable_debug_threads="no"
AC_ARG_ENABLE([debug-threads],
   [AC_HELP_STRING([--enable-debug-threads], [disable assert when you forgot to call eina_threads_init])],
   [_efl_enable_debug_threads="${enableval}"])

efl_have_debug_threads="no"
if test "x${_efl_have_posix_threads}" = "xyes" && test "x${_efl_enable_debug_threads}" = "xyes"; then
   efl_have_debug_threads="yes"
   AC_DEFINE([EFL_DEBUG_THREADS], [1], [Assert when forgot to call eina_threads_init])
fi

AS_IF([test "x$_efl_have_posix_threads" = "xyes" || test "x$_efl_have_win32_threads" = "xyes"], [$1], [$2])
])

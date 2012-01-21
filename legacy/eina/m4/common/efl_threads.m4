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

dnl configure option

AC_ARG_ENABLE([posix-threads],
   [AC_HELP_STRING([--disable-posix-threads], [enable POSIX threads code @<:@default=auto@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       _efl_enable_posix_threads="yes"
    else
       _efl_enable_posix_threads="no"
    fi
   ],
   [_efl_enable_posix_threads="auto"])

AC_MSG_CHECKING([whether to build POSIX threads code])
AC_MSG_RESULT([${_efl_enable_posix_threads}])

AC_ARG_ENABLE([win32-threads],
   [AC_HELP_STRING([--disable-win32-threads], [enable Win32 threads code @<:@default=no@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       _efl_enable_win32_threads="yes"
    else
       _efl_enable_win32_threads="no"
    fi
   ],
   [_efl_enable_win32_threads="no"])

AC_MSG_CHECKING([whether to build Windows threads code])
AC_MSG_RESULT([${_efl_enable_win32_threads}])

dnl
dnl * no  + no
dnl * yes + no  : win32: error,    other : pthread
dnl * yes + yes : win32 : wthread, other : pthread
dnl * no  + yes : win32 : wthread, other : error

if  test "x${_efl_enable_posix_threads}" = "xyes" && test "x${_efl_enable_win32_threads}" = "xyes" ; then
   case "$host_os" in
      mingw*)
         _efl_enable_posix_threads=no
         ;;
      *)
         _efl_enable_win32_threads=no
         ;;
   esac
fi

if  test "x${_efl_enable_win32_threads}" = "xyes" ; then
   case "$host_os" in
      mingw*)
         ;;
      *)
         AC_MSG_ERROR([Win32 threads support requested but non Windows system found.])
         ;;
   esac
fi

if  test "x${_efl_enable_posix_threads}" = "xyes" ; then
   case "$host_os" in
      mingw*)
         AC_MSG_ERROR([POSIX threads support requested but Windows system found.])
         ;;
      *)
         ;;
   esac
fi

dnl check if the compiler supports POSIX threads

case "$host_os" in
   mingw*)
      ;;
   solaris*)
      _efl_threads_cflags="-mt"
      _efl_threads_libs="-mt"
      ;;
   *)
      _efl_threads_cflags="-pthread"
      _efl_threads_libs="-pthread"
      ;;
esac

_efl_have_posix_threads="no"
_efl_have_win32_threads="no"

if test "x${_efl_enable_posix_threads}" = "xyes" || test "x${_efl_enable_posix_threads}" = "xauto" ; then

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
      [_efl_have_posix_threads="yes"],
      [_efl_have_posix_threads="no"])
   CFLAGS=${SAVE_CFLAGS}
   LIBS=${SAVE_LIBS}

fi

AC_MSG_CHECKING([whether system support POSIX threads])
AC_MSG_RESULT([${_efl_have_posix_threads}])
if test "$x{_efl_enable_posix_threads}" = "xyes" && test "x${_efl_have_posix_threads}" = "xno"; then
   AC_MSG_ERROR([POSIX threads support requested but not found.])
fi

EFL_PTHREAD_CFLAGS=""
EFL_PTHREAD_LIBS=""
if test "x${_efl_have_posix_threads}" = "xyes" ; then
   EFL_PTHREAD_CFLAGS=${_efl_threads_cflags}
   EFL_PTHREAD_LIBS=${_efl_threads_libs}
fi

AC_SUBST(EFL_PTHREAD_CFLAGS)
AC_SUBST(EFL_PTHREAD_LIBS)

_efl_enable_on_off_threads="no"
AC_ARG_ENABLE([on-off-threads],
   [AC_HELP_STRING([--enable-on-off-threads], [only turn this on if you know what you are doing, and don't complain if the world freeze])],
   [_efl_enable_on_off_threads="${enableval}"])

have_on_off_threads="no"
if test "x${_efl_enable_on_off_threads}" = "xyes"; then
   have_on_off_threads="yes"
   AC_DEFINE([EFL_ON_OFF_THREADS], [1], [make it possible to disable all locks])
fi
AC_MSG_CHECKING([whether to turn on/off threads lock on demand])
AC_MSG_RESULT([${_efl_enable_on_off_threads}])

_efl_enable_debug_threads="no"
AC_ARG_ENABLE([debug-threads],
   [AC_HELP_STRING([--enable-debug-threads], [disable assert when you forgot to call eina_threads_init])],
   [_efl_enable_debug_threads="${enableval}"])

have_debug_threads="no"
if test "x${_efl_have_posix_threads}" = "xyes" -a "x${_efl_enable_debug_threads}" = "xyes"; then
   have_debug_threads="yes"
   AC_DEFINE([EFL_DEBUG_THREADS], [1], [Assert when forgot to call eina_threads_init])
fi

if test "x${_efl_have_posix_threads}" = "xyes" ; then
   AC_DEFINE([EFL_HAVE_POSIX_THREADS], [1], [Define to mention that POSIX threads are supported])
fi

if test "x${_efl_enable_win32_threads}" = "xyes" ; then
   _efl_have_win32_threads="yes"
   AC_DEFINE([EFL_HAVE_WIN32_THREADS], [1], [Define to mention that Win32 threads are supported])
fi

if test "x${_efl_have_posix_threads}" = "xyes" || test "x${_efl_have_win32_threads}" = "xyes" ; then
   AC_DEFINE([EFL_HAVE_THREADS], [1], [Define to mention that POSIX or Win32 threads are supported])
fi

AS_IF([test "x$_efl_have_posix_threads" = "xyes" || test "x$_efl_have_win32_threads" = "xyes"], [$1], [$2])
])

dnl Usage: EFL_CHECK_SPINLOCK(ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND])
dnl Defines EFL_HAVE_POSIX_THREADS_SPINLOCK
AC_DEFUN([EFL_CHECK_SPINLOCK],
[

dnl check if the compiler supports pthreads spinlock

_efl_have_posix_threads_spinlock="no"

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
      [_efl_have_posix_threads_spinlock="yes"],
      [_efl_have_posix_threads_spinlock="no"])
   CFLAGS=${SAVE_CFLAGS}
   LIBS=${SAVE_LIBS}

fi

AC_MSG_CHECKING([whether to build POSIX threads spinlock code])
AC_MSG_RESULT([${_efl_have_posix_threads_spinlock}])
if test "x${_efl_enable_posix_threads}" = "xyes" && test "x${_efl_have_posix_threads_spinlock}" = "xno" ; then
   AC_MSG_WARN([POSIX threads support requested but spinlocks are not supported])
fi

if test "x${_efl_have_posix_threads_spinlock}" = "xyes" ; then
   AC_DEFINE([EFL_HAVE_POSIX_THREADS_SPINLOCK], [1], [Define to mention that POSIX threads spinlocks are supported])
fi
AS_IF([test "x$_efl_have_posix_threads_spinlock" = "xyes"], [$1], [$2])
])


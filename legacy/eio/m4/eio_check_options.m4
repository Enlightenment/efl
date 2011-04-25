dnl use: EIO_CHECK_INOTIFY(default-enabled[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([EIO_CHECK_INOTIFY],
[
_eio_want_inotify=$1
_eio_have_inotify="no"

dnl We need to check if the right inotify version is accessible
_eio_want_inotify="yes"
AC_ARG_ENABLE(inotify,
   [AC_HELP_STRING([--disable-inotify], [disable inotify])],
   [
    if test "x${enableval}" = "xyes" ; then
       _eio_want_inotify="yes"
    else
       _eio_want_inotify="no"
    fi
   ])

AC_MSG_CHECKING(whether inotify is to be used for filemonitoring)
AC_MSG_RESULT($_eio_want_inotify)

dnl It is hard to find a good test on how to check the correct
dnl inotify version. They changed the headers a lot.
dnl in kernel 2.6.13 __NR_inotify_init was added to the defined syscalls
dnl in asm/unistd.h and IN_MOVE_SELF was added to linux/inotify.h
dnl so with this check you need a very new kernel and kernel-headers!

if test "x${_eio_want_inotify}" = "xyes" ; then
   AC_CHECK_LIB([c], [inotify_init],
      [
       AC_DEFINE(HAVE_INOTIFY, 1, [ File monitoring with Inotify ])
       AC_DEFINE(HAVE_SYS_INOTIFY, 1, [ File monitoring with Inotify - sys/inotify.h ])
       _eio_have_inotify="yes"
      ],
      [
       AC_TRY_COMPILE(
          [
           #include <asm/unistd.h>
           #include <linux/inotify.h>
          ],
          [int a = __NR_inotify_init; int b = IN_MOVE_SELF;],
          [
           AC_DEFINE([HAVE_INOTIFY], [1], [ File monitoring with Inotify ])
           _eio_have_inotify="yes"
          ],
          [_eio_have_inotify="no"])
      ])
fi

if test "x$_eio_have_inotify" = "xyes" ; then
   m4_default([$2], [:])
else
   m4_default([$3], [:])
fi
])
 
dnl use: EIO_CHECK_NOTIFY_WIN32(default-enabled[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([EIO_CHECK_NOTIFY_WIN32],
[
_eio_want_notify_win32=$1
_eio_have_notify_win32="no"

AC_ARG_ENABLE(notify-win32,
   [AC_HELP_STRING([--disable-notify-win32], [disable Windows notification])],
   [
    if test "x${enableval}" = "xyes" ; then
       _eio_want_notify_win32="yes"
    else
       _eio_want_notify_win32="no"
    fi
   ])

AC_MSG_CHECKING(whether Windows notification is to be used for filemonitoring)
AC_MSG_RESULT(${_eio_want_notify_win32})

if test "x${_eio_want_notify_win32}" = "xyes" ; then
   AC_DEFINE([HAVE_NOTIFY_WIN32], [1], [ File monitoring with Windows notification ])
   _eio_have_notify_win32="yes"
fi

if test "x${_eio_have_notify_win32}" = "xyes" ; then
   m4_default([$2], [:])
else
   m4_default([$3], [:])
fi
])

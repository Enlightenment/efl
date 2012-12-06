dnl use: EIO_CHECK_NOTIFY_WIN32([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([EIO_CHECK_NOTIFY_WIN32],
[
if test "x${have_win32}" = "xyes" ; then
   AC_DEFINE([HAVE_NOTIFY_WIN32], [1], [ File monitoring with Windows notification ])
fi

AC_MSG_CHECKING([whether Windows notification is to be used for filemonitoring])
AC_MSG_RESULT([${have_win32}])

AS_IF([test "x${have_win32}" = "xyes"], [$1], [$2])
])

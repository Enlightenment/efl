dnl file with extensions to pkg-config module
dnl
dnl EFL_PKG_CHECK_STRICT(MODULE, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl similar to PKG_CHECK_EXISTS() that will AC_MSG_ERROR() if not found
AC_DEFUN([EFL_PKG_CHECK_STRICT],
[
   PKG_CHECK_EXISTS([$1], [$2],
      [m4_ifval([$3], [$3], [AC_MSG_ERROR([pkg-config missing $1])])]
      )
])

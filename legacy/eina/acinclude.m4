m4_ifndef([AC_HEADER_ASSERT], [
# AC_HEADER_ASSERT
# ----------------
# Check whether to enable assertions.
AC_DEFUN([AC_HEADER_ASSERT],
[
  AC_MSG_CHECKING([whether to enable assertions])
  AC_ARG_ENABLE([assert],
    [  --disable-assert        turn off assertions],
    [AC_MSG_RESULT([no])
     AC_DEFINE(NDEBUG, 1, [Define to 1 if assertions should be disabled.])],
    [AC_MSG_RESULT(yes)])
])
])

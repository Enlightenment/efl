dnl use: AC_DEFINE_IF(id, testcond, val, comment)
AC_DEFUN([AC_DEFINE_IF],
[
if $2; then
   AC_DEFINE($1, $3, $4)
fi
])

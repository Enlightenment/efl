dnl use: EINA_CONFIG(configsuffix, testcond)
AC_DEFUN([EINA_CONFIG],
[
if $2; then
   EINA_CONFIGURE_$1="#define EINA_$1"
fi
AC_SUBST([EINA_CONFIGURE_$1])
])

dnl use: ECORE_CHECK_MODULE(Foo, default-enabled[, dependancy[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([ECORE_CHECK_MODULE],
[
pushdef([UP], translit([$1], [a-z], [A-Z]))dnl
pushdef([DOWN], translit([$1], [A-Z], [a-z]))dnl

have_ecore_[]DOWN="no"
ecore_[]DOWN[]_cflags=""
ecore_[]DOWN[]_libs=""

ifelse("x$2", "xno",
[
  AC_ARG_ENABLE(ecore-[]DOWN,
    AC_HELP_STRING(
      [--enable-ecore-[]DOWN],
      [enable the ecore_[]DOWN module. [[default=disabled]]]
    ),
    [ want_ecore_[]DOWN=$enableval ],
    [ want_ecore_[]DOWN=no ])
],
[
  AC_ARG_ENABLE(ecore-[]DOWN,
    AC_HELP_STRING(
      [--disable-ecore-[]DOWN],
      [disable the ecore_[]DOWN module. [[default=enabled]]]
    ),
    [ want_ecore_[]DOWN=$enableval ],
    [ want_ecore_[]DOWN=yes ])
])

AC_MSG_CHECKING(whether ecore_[]DOWN module is to be built)

if test "x$want_ecore_[]DOWN" = "xyes" ; then
  if test "x$3" = "x" -o "x$3" = "xyes" ; then
    AC_DEFINE(BUILD_ECORE_[]UP, 1, [Build Ecore_$1 Module])
    have_ecore_[]DOWN="yes"
    ecore_[]DOWN[]_libs="-lecore_[]DOWN"
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no (dependancy failed)])
  fi
fi

AM_CONDITIONAL(BUILD_ECORE_[]UP, test $have_ecore_[]DOWN = yes)

if test "x$have_ecore_[]DOWN" = "xyes" ; then
  ifelse([$4], , :, [$4])
else
  ifelse([$5], , :, [$5])
fi

AC_SUBST(ecore_[]DOWN[]_cflags)
AC_SUBST(ecore_[]DOWN[]_libs)

popdef([UP])
popdef([DOWN])
])

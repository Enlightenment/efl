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
else
  AC_MSG_RESULT([no])
fi

AM_CONDITIONAL(BUILD_ECORE_[]UP, test "x$have_ecore_[]DOWN" = "xyes")

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

dnl use: ECORE_EVAS_CHECK_MODULE(foo-bar, want, description, backend[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([ECORE_EVAS_CHECK_MODULE],
[
pushdef([UP], translit([$1], [-a-z], [_A-Z]))dnl
pushdef([DOWN], translit([$1], [-A-Z], [_a-z]))dnl

have_ecore_evas_[]DOWN="no"
want_module="$2"

AC_ARG_ENABLE(ecore-$1,
  AC_HELP_STRING(
    [--enable-ecore-evas-$1],
    [enable $3 support in the ecore_evas module.]
  ),
  [ want_module=$enableval ]
)
AC_MSG_CHECKING(whether ecore_evas $3 support is to be built)
AC_MSG_RESULT($want_module)

if test "x$4" = "xyes" -a \
        "x$have_ecore_evas" = "xyes" -a \
        "x$want_module" = "xyes" ; then
  PKG_CHECK_MODULES(EVAS_[]UP, evas-$1,
    [
     AC_DEFINE(BUILD_ECORE_EVAS_[]UP, 1, [Support for $3 Engine in Ecore_Evas])
     have_ecore_evas_[]DOWN="yes";
    ]
  )
fi

if test "x$have_ecore_evas_[]DOWN" = "xyes" ; then
  ifelse([$5], , :, [$5])
else
  ifelse([$6], , :, [$6])
fi

popdef([UP])
popdef([DOWN])
])

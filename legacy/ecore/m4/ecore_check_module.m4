dnl use: ECORE_CHECK_MODULE(Foo, default-enabled, description[, dependency[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([ECORE_CHECK_MODULE],
[
m4_pushdef([UP], m4_translit([[$1]], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([[$1]], [-A-Z], [_a-z]))dnl

have_ecore_[]m4_defn([DOWN])=no
ecore_[]m4_defn([DOWN])[]_cflags=
ecore_[]m4_defn([DOWN])[]_libs=
want_module=$2

AC_ARG_ENABLE(ecore-$1,
   [AC_HELP_STRING(
       [--enable-ecore-$1],
       [enable the ecore_]m4_defn([DOWN])[ module])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_module="yes"
    else
       want_module="no"
    fi
   ],
   [])

AC_MSG_CHECKING([whether Ecore_$3 module is to be built])

if test "x${want_module}" = "xyes" ; then
   if test "x$4" = "x" || test "x$4" = "xyes" ; then
      AC_DEFINE([BUILD_ECORE_]m4_defn([UP]), [1], [Build Ecore_$3 Module])
      have_ecore_[]m4_defn([DOWN])="yes"
      ecore_[]m4_defn([DOWN])[]_libs="-lecore_[]m4_defn([DOWN])"
      AC_MSG_RESULT([yes])
   else
      AC_MSG_RESULT([no (dependency failed)])
   fi
else
   AC_MSG_RESULT([no])
fi

AM_CONDITIONAL([BUILD_ECORE_]UP, [test "x$have_ecore_]DOWN[" = "xyes"])

AS_IF([test "x$have_ecore_[]m4_defn([DOWN])" = "xyes"], [$5], [$6])

AC_SUBST(ecore_[]m4_defn([DOWN])[]_cflags)
AC_SUBST(ecore_[]m4_defn([DOWN])[]_libs)

m4_popdef([UP])
m4_popdef([DOWN])
])

dnl use: ECORE_EVAS_CHECK_MODULE_FULL(foo-bar, evas-module, want, description, backend[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([ECORE_EVAS_CHECK_MODULE_FULL],
[
m4_pushdef([UP], m4_translit([[$1]], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([[$1]], [-A-Z], [_a-z]))dnl

have_ecore_evas_[]m4_defn([DOWN])="no"
want_module="$3"

AC_ARG_ENABLE(ecore-evas-$1,
   [AC_HELP_STRING(
       [--enable-ecore-evas-$1],
       [enable $4 support in the ecore_evas module.])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_module="yes"
    else
       want_module="no"
    fi
   ],
   [])

AC_MSG_CHECKING([whether ecore_evas $4 support is to be built])
AC_MSG_RESULT([${want_module}])

if test "x$5" = "xyes" -a \
        "x$have_ecore_evas" = "xyes" -a \
        "x$want_module" = "xyes" ; then
   PKG_CHECK_EXISTS([evas-$2],
      [
       AC_DEFINE([BUILD_ECORE_EVAS_]m4_defn([UP]), [1], [Support for $4 Engine in Ecore_Evas])
       have_ecore_evas_[]m4_defn([DOWN])="yes"
      ])
fi

AC_MSG_CHECKING([whether ecore_evas $4 support is built])
AC_MSG_RESULT([$have_ecore_evas_]m4_defn([DOWN]))

AS_IF([test "x$have_ecore_evas_[]m4_defn([DOWN])" = "xyes"], [$6], [$7])

m4_popdef([UP])
m4_popdef([DOWN])
])

dnl use: ECORE_EVAS_CHECK_MODULE(foo-bar, want, description, backend[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([ECORE_EVAS_CHECK_MODULE],
[ECORE_EVAS_CHECK_MODULE_FULL([$1], [$1], [$2], [$3], [$4], [$5], [$6])])

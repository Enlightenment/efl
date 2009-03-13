dnl use: ECORE_CHECK_MODULE(Foo, default-enabled[, dependancy[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([ECORE_CHECK_MODULE],
[
m4_pushdef([UP], m4_toupper([[$1]]))dnl
m4_pushdef([DOWN], m4_tolower([[$1]]))dnl

have_ecore_[]m4_defn([DOWN])=no
ecore_[]m4_defn([DOWN])[]_cflags=
ecore_[]m4_defn([DOWN])[]_libs=
want_module=$2

if test "x${want_module}" = "xno" ; then
   AC_ARG_ENABLE(ecore-[]m4_defn([DOWN]),
      [AC_HELP_STRING(
          [--enable-ecore-[]m4_defn([DOWN])],
          [enable the ecore_]m4_defn([DOWN])[ module. [[default=disabled]]])],
      [want_module=$enableval],
      [want_module=no])
else
   AC_ARG_ENABLE(ecore-[]m4_defn([DOWN]),
      [AC_HELP_STRING(
          [--disable-ecore-[]m4_defn([DOWN])],
          [disable the ecore_]m4_defn([DOWN])[ module. [[default=enabled]]])],
      [want_module=$enableval],
      [want_module=yes])
fi

AC_MSG_CHECKING([whether ecore_]m4_defn([DOWN])[ module is to be built])

if test "x${want_module}" = "xyes" ; then
   if test "x$3" = "x" -o "x$3" = "xyes" ; then
      AC_DEFINE([BUILD_ECORE_]m4_defn([UP]), [1], [Build Ecore_$1 Module])
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

if test "x$have_ecore_[]m4_defn([DOWN])" = "xyes" ; then
   m4_default([$4], [:])
else
   m4_default([$5], [:])
fi

AC_SUBST(ecore_[]m4_defn([DOWN])[]_cflags)
AC_SUBST(ecore_[]m4_defn([DOWN])[]_libs)

m4_popdef([UP])
m4_popdef([DOWN])
])

dnl use: ECORE_EVAS_CHECK_MODULE(foo-bar, want, description, backend[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([ECORE_EVAS_CHECK_MODULE],
[
m4_pushdef([UP], m4_translit([[$1]], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([[$1]], [-A-Z], [_a-z]))dnl

have_ecore_evas_[]m4_defn([DOWN])="no"
want_module="$2"

if test "x${want_module}" = "xno" ; then
   AC_ARG_ENABLE(ecore-evas-$1,
      [AC_HELP_STRING(
          [--enable-ecore-evas-$1],
          [enable $3 support in the ecore_evas module.])],
      [want_module=$enableval])
else
   AC_ARG_ENABLE(ecore-evas-$1,
      [AC_HELP_STRING(
          [--disable-ecore-evas-$1],
          [disable $3 support in the ecore_evas module.])],
      [want_module=$enableval])
fi

AC_MSG_CHECKING([whether ecore_evas $3 support is to be built])
AC_MSG_RESULT([${want_module}])

if test "x$4" = "xyes" -a \
        "x$have_ecore_evas" = "xyes" -a \
        "x$want_module" = "xyes" ; then
   PKG_CHECK_EXISTS([evas-$1],
      [
       AC_DEFINE([BUILD_ECORE_EVAS_]m4_defn([UP]), [1], [Support for $3 Engine in Ecore_Evas])
       have_ecore_evas_[]m4_defn([DOWN])="yes"
      ])
fi

AC_MSG_CHECKING([whether ecore_evas $3 support is built])
AC_MSG_RESULT([$have_ecore_evas_]m4_defn([DOWN]))

if test "x$have_ecore_evas_[]m4_defn([DOWN])" = "xyes" ; then
   m4_default([$5], [:])
else
   m4_default([$6], [:])
fi

m4_popdef([UP])
m4_popdef([DOWN])
])

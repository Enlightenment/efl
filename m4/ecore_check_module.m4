dnl use: ECORE_EVAS_MODULE(name, want, [DEPENDENCY-CHECK-CODE])
AC_DEFUN([ECORE_EVAS_MODULE],
[dnl
m4_pushdef([UP], m4_translit([[$1]], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([[$1]], [-A-Z], [_a-z]))dnl

have_ecore_evas_[]m4_defn([DOWN])="no"
want_module="$2"

ecore_evas_engines_[]m4_defn([DOWN])[]_cflags=""
ecore_evas_engines_[]m4_defn([DOWN])[]_libs=""

if test "x$want_module" = "xyes" || test "x$want_module" = "xstatic"; then
   $3

   AC_DEFINE([BUILD_ECORE_EVAS_]m4_defn([UP]), [1], [Support for $1 Engine in Ecore_Evas])
   have_ecore_evas_[]m4_defn([DOWN])="yes"

   if test "x$1" = "xgl-drm"; then
      PKG_CHECK_MODULES([GBM], [gbm])
      ecore_evas_engines_[]m4_defn([DOWN])[]_cflags="${GBM_CFLAGS}"
      ecore_evas_engines_[]m4_defn([DOWN])[]_libs="${GBM_LIBS}"
   fi
fi

AC_SUBST([ecore_evas_engines_]m4_defn([DOWN])[_cflags])
AC_SUBST([ecore_evas_engines_]m4_defn([DOWN])[_libs])

EFL_ADD_FEATURE([ECORE_EVAS], [$1], [${want_module}])dnl
AM_CONDITIONAL([BUILD_ECORE_EVAS_]UP, [test "x$have_ecore_evas_]m4_defn([DOWN])[" = "xyes"])dnl
m4_popdef([UP])dnl
m4_popdef([DOWN])dnl
])

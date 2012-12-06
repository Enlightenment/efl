dnl file to manage modules in efl

dnl EFL_DEPEND_PKG(EFL, NAME, PACKAGE, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Adds a pkg-config dependency to an efl, AC_DEFINE() HAVE_NAME,
dnl and inserts dependencies in proper variables
AC_DEFUN([EFL_DEPEND_PKG],
[
m4_pushdef([UPEFL], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWNEFL], m4_translit([$1], [-A-Z], [_a-z]))dnl
m4_pushdef([UPNAME], m4_translit([$2], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWNNAME], m4_translit([$2], [-A-Z], [_a-z]))dnl

   have_[]m4_defn([DOWNNAME])="no"

   EFL_PKG_CHECK_STRICT([$3], [
      AC_DEFINE([HAVE_]m4_defn([UPNAME]), [1], [Have `]m4_defn([DOWNNAME])[' pkg-config installed.])
      requirements_pc_[]m4_defn([DOWNEFL])="$3 ${requirements_pc_[][]m4_defn([DOWNEFL])}"
      requirements_pc_deps_[]m4_defn([DOWNEFL])="$3 ${requirements_pc_deps_[]m4_defn([DOWNEFL])}"
      have_[]m4_defn([DOWNNAME])="yes"

      $4

      ], [$5])

m4_popdef([DOWNNAME])
m4_popdef([UPNAME])
m4_popdef([DOWNEFL])
m4_popdef([UPEFL])
])

dnl EFL_OPTIONAL_DEPEND_PKG(EFL, VARIABLE, NAME, PACKAGE, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
AC_DEFUN([EFL_OPTIONAL_DEPEND_PKG],
[
m4_pushdef([DOWN], m4_translit([$3], [-A-Z], [_a-z]))dnl

   have_[]m4_defn([DOWN])="no"
   if test "x$2" = "xyes"; then
      EFL_DEPEND_PKG([$1], [$3], [$4], [$5], [$6])
   fi

m4_popdef([DOWN])
])

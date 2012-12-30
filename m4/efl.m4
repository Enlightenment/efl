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

dnl EFL_LIB_START(PKG)
dnl start the setup of an EFL library, defines variables and prints a notice
AC_DEFUN([EFL_LIB_START],
[
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

requirements_libs_[]m4_defn([DOWN])=""
requirements_pc_[]m4_defn([DOWN])=""
requirements_pc_deps_[]m4_defn([DOWN])=""

AC_SUBST([requirements_libs_]m4_defn([DOWN]))
AC_SUBST([requirements_pc_]m4_defn([DOWN]))

AC_MSG_NOTICE([Start $1 checks])

m4_popdef([DOWN])
])

dnl EFL_LIB_END(PKG)
dnl finishes the setup of an EFL library
AC_DEFUN([EFL_LIB_END],
[
AC_MSG_NOTICE([Finished $1 checks])
])

dnl EFL_LIB_START_OPTIONAL(PKG, TEST)
dnl test if library should be build and then EFL_LIB_START()
dnl must call EFL_LIB_END_OPTIONAL() to close it.
AC_DEFUN([EFL_LIB_START_OPTIONAL],
[
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl

if $2; then
   efl_lib_optional_[]m4_defn([DOWN])="yes"
else
   efl_lib_optional_[]m4_defn([DOWN])="no"
   AC_MSG_NOTICE([Skipping $1 checks (disabled)])
fi

if test "$efl_lib_optional_[]m4_defn([DOWN])" = "yes"; then
   EFL_LIB_START([$1])
   AC_DEFINE([HAVE_]m4_defn([UP]), [1], [optional EFL $1 is enabled])

dnl closed at EFL_LIB_END_OPTIONAL()

m4_popdef([UP])
m4_popdef([DOWN])
])

dnl EFL_LIB_END_OPTIONAL(PKG)
dnl closes block started by EFL_LIB_START_OPTIONAL() and then
dnl defines AM_CONDITIONAL([HAVE_PKG]) and AC_DEFINE([HAVE_PKG])
AC_DEFUN([EFL_LIB_END_OPTIONAL],
[
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl

dnl close if started at EFL_LIB_START_OPTIONAL()
   EFL_LIB_END([$1])
fi

AM_CONDITIONAL([HAVE_]m4_defn([UP]), [test "$efl_lib_optional_[]m4_defn([DOWN])" = "yes"])

m4_popdef([UP])
m4_popdef([DOWN])
])

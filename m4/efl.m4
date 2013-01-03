dnl file to manage modules in efl

dnl EFL_EVAL_PKGS(EFL)
dnl does PKG_CHECK_MODULES() for given EFL
AC_DEFUN([EFL_EVAL_PKGS],
[dnl
m4_pushdef([DOWNEFL], m4_translit([$1], [-A-Z], [_a-z]))dnl
if test "x${requirements_pc_deps_[]m4_defn([DOWNEFL])}" != "x"; then
   PKG_CHECK_MODULES([$1], [${requirements_pc_deps_[]m4_defn([DOWNEFL])}])
fi
m4_popdef([DOWNEFL])dnl
])

dnl EFL_INTERNAL_DEPEND_PKG(EFL, OTHEREFL)
dnl Adds a pkg-config dependency on another EFL.
AC_DEFUN([EFL_INTERNAL_DEPEND_PKG],
[dnl
m4_pushdef([DOWNEFL], m4_translit([$1], [-A-Z], [_a-z]))dnl
requirements_pc_[]m4_defn([DOWNEFL])="$2 >= ${PACKAGE_VERSION} ${requirements_pc_[][]m4_defn([DOWNEFL])}"
m4_popdef([DOWNEFL])dnl
])

dnl EFL_PLATFORM_DEPEND(EFL, PLATFORM)
dnl PLATFORM is one of: all, evil, escape, exotic
AC_DEFUN([EFL_PLATFORM_DEPEND],
[dnl
m4_pushdef([DOWNEFL], m4_translit([$1], [-A-Z], [_a-z]))dnl
case "$2" in
  all)
    requirements_pc_[]m4_defn([DOWNEFL])="${platform_pc} ${requirements_pc_[][]m4_defn([DOWNEFL])}"
    requirements_libs_[]m4_defn([DOWNEFL])="${platform_libs} ${requirements_libs_[][]m4_defn([DOWNEFL])}"
    requirements_cflags_[]m4_defn([DOWNEFL])="${platform_cflags} ${requirements_cflags_[][]m4_defn([DOWNEFL])}"
    ;;
  evil)
    requirements_pc_[]m4_defn([DOWNEFL])="${platform_pc_evil} ${requirements_pc_[][]m4_defn([DOWNEFL])}"
    requirements_libs_[]m4_defn([DOWNEFL])="${platform_libs_evil} ${requirements_libs_[][]m4_defn([DOWNEFL])}"
    requirements_cflags_[]m4_defn([DOWNEFL])="${platform_cflags_evil} ${requirements_cflags_[][]m4_defn([DOWNEFL])}"
    ;;
  escape)
    requirements_pc_[]m4_defn([DOWNEFL])="${platform_pc_escape} ${requirements_pc_[][]m4_defn([DOWNEFL])}"
    requirements_libs_[]m4_defn([DOWNEFL])="${platform_libs_escape} ${requirements_libs_[][]m4_defn([DOWNEFL])}"
    requirements_cflags_[]m4_defn([DOWNEFL])="${platform_cflags_escape} ${requirements_cflags_[][]m4_defn([DOWNEFL])}"
    ;;
  exotic)
    requirements_pc_[]m4_defn([DOWNEFL])="${platform_pc_exotic} ${requirements_pc_[][]m4_defn([DOWNEFL])}"
    requirements_libs_[]m4_defn([DOWNEFL])="${platform_libs_exotic} ${requirements_libs_[][]m4_defn([DOWNEFL])}"
    requirements_cflags_[]m4_defn([DOWNEFL])="${platform_cflags_exotic} ${requirements_cflags_[][]m4_defn([DOWNEFL])}"
    ;;
  *)
    AC_MSG_ERROR([Unknown platform: $2])
esac
m4_popdef([DOWNEFL])dnl
])

dnl EFL_CRYPTO_DEPEND(EFL)
dnl the given EFL will use/depend on system crypto settings
AC_DEFUN([EFL_CRYPTO_DEPEND],
[dnl
m4_pushdef([DOWNEFL], m4_translit([$1], [-A-Z], [_a-z]))dnl
requirements_pc_[]m4_defn([DOWNEFL])="${requirements_pc_crypto} ${requirements_pc_[][]m4_defn([DOWNEFL])}"
requirements_pc_deps_[]m4_defn([DOWNEFL])="${requirements_pc_deps_crypto} ${requirements_pc_deps_[][]m4_defn([DOWNEFL])}"
requirements_libs_[]m4_defn([DOWNEFL])="${requirements_libs_crypto} ${requirements_libs_[][]m4_defn([DOWNEFL])}"
requirements_cflags_[]m4_defn([DOWNEFL])="${requirements_cflags_crypto} ${requirements_cflags_[][]m4_defn([DOWNEFL])}"
m4_popdef([DOWNEFL])dnl
])

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

dnl EFL_OPTIONAL_INTERNAL_DEPEND_PKG(EFL, VARIABLE, NAME)
AC_DEFUN([EFL_OPTIONAL_INTERNAL_DEPEND_PKG],
[
m4_pushdef([DOWN], m4_translit([$3], [-A-Z], [_a-z]))dnl

   have_[]m4_defn([DOWN])="no"
   if test "x$2" = "xyes"; then
      EFL_INTERNAL_DEPEND_PKG([$1], [$3])
   fi

m4_popdef([DOWN])
])

dnl EFL_ADD_LIBS(PKG, LIBS)
dnl Add libraries that the EFL library will depend on
dnl See EFL_DEPEND_PKG() for pkg-config version.
AC_DEFUN([EFL_ADD_LIBS],
[dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl
requirements_libs_[]m4_defn([DOWN])="${requirements_libs_[]m4_defn([DOWN])} $2"
m4_popdef([DOWN])dnl
])

dnl EFL_ADD_CFLAGS(PKG, CFLAGS)
dnl Add CFLAGS that the EFL library will use
dnl See EFL_DEPEND_PKG() for pkg-config version.
AC_DEFUN([EFL_ADD_CFLAGS],
[dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl
requirements_cflags_[]m4_defn([DOWN])="${requirements_cflags_[]m4_defn([DOWN])} $2"
m4_popdef([DOWN])dnl
])

dnl EFL_LIB_START(PKG)
dnl start the setup of an EFL library, defines variables and prints a notice
AC_DEFUN([EFL_LIB_START],
[
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl

requirements_libs_[]m4_defn([DOWN])=""
requirements_cflags_[]m4_defn([DOWN])=""
requirements_pc_[]m4_defn([DOWN])=""
requirements_pc_deps_[]m4_defn([DOWN])=""

m4_defn([UP])_LIBS="${m4_defn([UP])_LIBS}"
m4_defn([UP])_LDFLAGS="${m4_defn([UP])_LDFLAGS}"
m4_defn([UP])_CFLAGS="${m4_defn([UP])_CFLAGS}"

AC_SUBST([requirements_libs_]m4_defn([DOWN]))
AC_SUBST([requirements_cflags_]m4_defn([DOWN]))
AC_SUBST([requirements_pc_]m4_defn([DOWN]))
AC_SUBST(m4_defn([UP])[_LIBS])
AC_SUBST(m4_defn([UP])[_LDFLAGS])
AC_SUBST(m4_defn([UP])[_CFLAGS])

AC_MSG_NOTICE([Start $1 checks])

m4_popdef([UP])
m4_popdef([DOWN])
])

dnl EFL_LIB_END(PKG)
dnl finishes the setup of an EFL library
AC_DEFUN([EFL_LIB_END],
[
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl

m4_defn([UP])_LDFLAGS="${m4_defn([UP])_LDFLAGS}  ${EFL_LDFLAGS}"
m4_defn([UP])_LIBS="${m4_defn([UP])_LIBS} ${m4_defn([UP])_LDFLAGS} ${EFL_LIBS} ${requirements_libs_[]m4_defn([DOWN])} ${requirements_libs_efl} "
m4_defn([UP])_CFLAGS="${m4_defn([UP])_CFLAGS} ${EFL_CFLAGS} ${requirements_cflags_[]m4_defn([DOWN])} ${requirements_cflags_efl}"
requirements_pc_[]m4_defn([DOWN])="${requirements_pc_[]m4_defn([DOWN])} ${requirements_pc_efl}"
requirements_pc_deps_[]m4_defn([DOWN])="${requirements_pc_deps_[]m4_defn([DOWN])} ${requirements_pc_deps_efl}"

AC_MSG_NOTICE([Finished $1 checks])
m4_popdef([UP])
m4_popdef([DOWN])
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

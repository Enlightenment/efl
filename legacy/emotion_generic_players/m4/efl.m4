dnl file to manage modules in efl

dnl EFL_VERSION(major, minor, micro, release)
dnl This setup EFL version information and should be called BEFORE AC_INIT().
dnl
dnl release parameter is 'dev' to use from SVN or libtool -release field.
dnl It may be empty if not dev (svn/live build) and no -release is to be used.
dnl
dnl Examples:
dnl     EFL_VERSION(1, 7, 99, dev)
dnl     EFL_VERSION(1, 7, 99, ver-1234)
dnl This will define couple of m4 symbols:
dnl     v_maj = given major number (first parameter)
dnl     v_min = given minor number (second parameter)
dnl     v_mic = given micro number (third parameter)
dnl     v_rev = if release, it's 0, otherwise it's dev_version.
dnl     v_rel = if release, it's -release followed by fourth parameter,
dnl             otherwise it's empty. (mostly for libtool)
dnl     efl_version = if release, it's major.minor.micro, otherwise it's
dnl             major.minor.micro.dev_version
dnl     dev_version = development version (svn revision).
dnl     def_build_profile = dev or release based on 'dev' release parameter.
AC_DEFUN([EFL_VERSION],
[dnl
m4_define([v_maj], [$1])dnl
m4_define([v_min], [$2])dnl
m4_define([v_mic], [$3])dnl
m4_define([dev_version], m4_esyscmd([(git rev-list --count HEAD 2>/dev/null || echo 0) | tr -d '\n']))dnl
m4_define([v_rev], m4_if($4, dev, [dev_version], [0]))dnl
m4_define([v_rel], [])dnl
m4_define([def_build_profile], m4_if($4, dev, [dev], [release]))dnl
m4_define([efl_version], m4_if($4, dev, [v_maj.v_min.v_mic.v_rev], [v_maj.v_min.v_mic]))dnl
dnl m4_define([efl_version], [v_maj.v_min.v_mic])dnl
])

dnl EFL_COLOR
dnl will check if terminal supports color and if color is wanted by user.
dnl
dnl Used Variables:
dnl     WANT_COLOR: if no, forces no color output.
dnl     TERM: used to check if color should be enabled.
dnl
dnl Defined Variables:
dnl     COLOR_YES: to be used in positive/yes conditions
dnl     COLOR_NO: to be used in negative/no conditions
dnl     COLOR_OTHER: to be used to highlight some other condition
dnl     COLOR_RESET: to reset color
dnl     want_color: yes or no
AC_DEFUN([EFL_COLOR],
[dnl
case "$TERM" in
   xterm|xterm-color|xterm-256color|Eterm|aterm|kterm|rxvt*|screen|gnome|interix)
      want_color="${WANT_COLOR:-yes}"
      ;;
   *)
      want_color="no"
      ;;
esac

### echo compatibility

## the BSD echo does not have the -e option (it is the default behaviour)
echo_e=
if test "`echo -e x`" = "x"; then
   echo_e=-e
fi
AC_SUBST([ECHO_E], [${echo_e}])

if test "${want_color}" = "yes"; then
   COLOR_YES=`echo $echo_e "\033@<:@1;32m"`
   COLOR_NO=`echo $echo_e "\033@<:@1;31m"`
   COLOR_OTHER=`echo $echo_e "\033@<:@1;36m"`
   COLOR_RESET=`echo $echo_e "\033@<:@0m"`
else
   COLOR_YES=""
   COLOR_NO=""
   COLOR_OTHER=""
   COLOR_RESET=""
fi
])

dnl EFL_INIT()
dnl Will AC_DEFINE() the following:
dnl     VMAJ = v_maj
dnl     VMIN = v_min
dnl     VMIC = v_mic
dnl     VREV = v_rev
dnl Will AC_SUBST() the following:
dnl     VMAJ = v_maj
dnl     VMIN = v_min
dnl     VMIC = v_mic
dnl     EFL_LTLIBRARY_FLAGS="-no-undefined -version-info ..."
dnl     EFL_LTMODULE_FLAGS="-no-undefined -avoid-version"
dnl Will define the following m4:
dnl     lt_cur = libtool 'current' field of libtool's -version-info
dnl     lt_rev = libtool 'revision' field of libtool's -version-info
dnl     lt_age = libtool 'age' field of libtool's -version-info
AC_DEFUN([EFL_INIT],
[dnl
AC_REQUIRE([EFL_COLOR])dnl
AC_DEFINE_UNQUOTED([VMAJ], [v_maj], [Major version])dnl
AC_DEFINE_UNQUOTED([VMIN], [v_min], [Minor version])dnl
AC_DEFINE_UNQUOTED([VMIC], [v_mic], [Micro version])dnl
AC_DEFINE_UNQUOTED([VREV], [v_rev], [Revison])dnl
VMAJ=v_maj
VMIN=v_min
VMIC=v_mic
AC_SUBST([VMAJ])dnl
AC_SUBST([VMIN])dnl
AC_SUBST([VMIC])dnl
dnl
dnl TODO: warning - lt_cur:
dnl the previous code assumed v_maj + v_min, but this will be a problem when
dnl we bump v_maj and reset v_min. 1 + 7 == 7 + 1, so if v_maj is bumped
dnl we multiply it by 100.
m4_define([lt_cur], m4_if(m4_cmp(v_maj, 1), 0, m4_eval(v_maj + v_min), m4_eval(v_maj * 100 + v_min)))dnl
m4_define([lt_rev], v_mic)dnl
m4_define([lt_age], v_min)dnl
dnl
EFL_LTLIBRARY_FLAGS="-no-undefined -version-info lt_cur:lt_rev:lt_age v_rel"
AC_SUBST(EFL_LTLIBRARY_FLAGS)dnl
EFL_LTMODULE_FLAGS="-no-undefined -avoid-version"
AC_SUBST([EFL_LTMODULE_FLAGS])dnl
AC_MSG_NOTICE([Initialized AC_PACKAGE_NAME (AC_PACKAGE_VERSION) development=dev_version v_rel])
])

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
m4_pushdef([DOWNOTHER], m4_translit([$2], [-A-Z], [_a-z]))dnl
dnl TODO: we need to fix the package config names for 2.0
dnl TODO: and make them uniform in scheme.
depname="$2"
libdirname="m4_defn([DOWNOTHER])"
libname="m4_defn([DOWNOTHER])"
case "m4_defn([DOWNOTHER])" in
   ethumb_client)
      depname="ethumb_client"
      ;;
esac
requirements_pc_[]m4_defn([DOWNEFL])="${depname} >= ${PACKAGE_VERSION} ${requirements_pc_[][]m4_defn([DOWNEFL])}"
requirements_cflags_[]m4_defn([DOWNEFL])="-I\$(top_srcdir)/src/lib/${libdirname} -I\$(top_builddir)/src/lib/${libdirname} ${requirements_cflags_[][]m4_defn([DOWNEFL])}"
requirements_internal_libs_[]m4_defn([DOWNEFL])="lib/${libdirname}/lib${libname}.la ${requirements_internal_libs_[][]m4_defn([DOWNEFL])}"
requirements_internal_deps_libs_[]m4_defn([DOWNEFL])="${requirements_public_libs_[]m4_defn([DOWNOTHER])} ${requirements_internal_deps_libs_[][]m4_defn([DOWNEFL])}"
m4_popdef([DOWNOTHER])dnl
m4_popdef([DOWNEFL])dnl
])

dnl EFL_PLATFORM_DEPEND(EFL, PLATFORM)
dnl PLATFORM is one of: all, evil, escape, exotic
AC_DEFUN([EFL_PLATFORM_DEPEND],
[dnl
m4_pushdef([DOWNOTHER], m4_translit([$2], [-A-Z], [_a-z]))dnl
case "m4_defn([DOWNOTHER])" in
  all)
    if test "x${efl_lib_optional_evil}" = "xyes"; then
       EFL_INTERNAL_DEPEND_PKG([$1], [evil])
    elif test "x${efl_lib_optional_escape}" = "xyes"; then
       EFL_INTERNAL_DEPEND_PKG([$1], [escape])
    elif test "x${efl_lib_optional_exotic}" = "xyes"; then
       EFL_INTERNAL_DEPEND_PKG([$1], [exotic])
    fi
    ;;
  *)
    if test "x${efl_lib_optional_[]m4_defn([DOWNOTHER])}" = "xyes"; then
       EFL_INTERNAL_DEPEND_PKG([$1], [$2])
    fi
    ;;
esac
m4_popdef([DOWNOTHER])dnl
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
[dnl
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
[dnl
m4_pushdef([DOWN], m4_translit([$3], [-A-Z], [_a-z]))dnl

   have_[]m4_defn([DOWN])="no"
   if test "x$2" = "xyes"; then
      EFL_DEPEND_PKG([$1], [$3], [$4], [$5], [$6])
   fi
m4_popdef([DOWN])dnl
])

dnl EFL_OPTIONAL_INTERNAL_DEPEND_PKG(EFL, VARIABLE, NAME)
AC_DEFUN([EFL_OPTIONAL_INTERNAL_DEPEND_PKG],
[dnl
   if test "x$2" = "xyes"; then
      EFL_INTERNAL_DEPEND_PKG([$1], [$3])
   fi
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

dnl EFL_ADD_PUBLIC_LIBS(PKG, PUBLIC_LIBS)
dnl Add libraries that the EFL library will depend on when used.
dnl
dnl Unlike EFL_ADD_LIBS(), that is only used when generating PKG,
dnl this one is used when linking PKG to other libraries or applications.
dnl
dnl For instance if you use some other library in your header that user
dnl inclues.
AC_DEFUN([EFL_ADD_PUBLIC_LIBS],
[dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl
requirements_public_libs_[]m4_defn([DOWN])="${requirements_public_libs_[]m4_defn([DOWN])} $2"
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
dnl
dnl Exports (AC_SUBST)
dnl     PKG_CFLAGS: what to use for CFLAGS
dnl
dnl     PKG_LDFLAGS: what to use for LDFLAGS
dnl
dnl     PKG_LIBS: what to use in automake's _LIBADD or _LDADD. Includes
dnl             everything else.
dnl
dnl     PKG_INTERNAL_LIBS: all other EFL as lib/name/libname.la that this
dnl             package depend. Used in automake's _DEPENDENCIES.
dnl
dnl     USE_PKG_LIBS: what to use in automake's _LIBADD or _LDADD when using
dnl             this PKG (PKG_LIBS + libpkg.la)
dnl
dnl     USE_PKG_INTERNAL_LIBS: extends PKG_INTERNAL_LIBS with lib/pkg/libpkg.la
dnl
dnl     requirements_pc_pkg: all pkg-config (pc) files used by this pkg,
dnl             includes internal EFL (used in 'Requires.private' in pkg.pc)
dnl
dnl     requirements_libs_pkg: external libraries this package needs when
dnl             linking (used in 'Libs.private' in pkg.pc)
dnl
dnl     requirements_public_libs_pkg: external libraries other packages need
dnl             when using this (used in 'Libs' in pkg.pc)
dnl
dnl     requirements_cflags_pkg: what to use for CFLAGS (same as PKG_CFLAGS).
dnl
dnl Variables:
dnl     requirements_pc_deps_pkg: external pkg-config (pc) files used by this
dnl             pkg (used in EFL_EVAL_PKGS())
dnl
dnl     requirements_internal_libs_pkg: all other EFL as lib/name/libname.la
dnl             that this package depend.
dnl
dnl     requirements_internal_deps_libs_pkg: external libraries that are public
dnl             dependencies (due internal libs).
dnl
AC_DEFUN([EFL_LIB_START],
[dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl

requirements_internal_libs_[]m4_defn([DOWN])=""
requirements_internal_deps_libs_[]m4_defn([DOWN])=""
requirements_libs_[]m4_defn([DOWN])=""
requirements_public_libs_[]m4_defn([DOWN])=""
requirements_cflags_[]m4_defn([DOWN])=""
requirements_pc_[]m4_defn([DOWN])=""
requirements_pc_deps_[]m4_defn([DOWN])=""

m4_defn([UP])_LIBS="${m4_defn([UP])_LIBS}"
m4_defn([UP])_INTERNAL_LIBS="${m4_defn([UP])_INTERNAL_LIBS}"
USE_[]m4_defn([UP])_LIBS="${USE_[]m4_defn([UP])_LIBS}"
USE_[]m4_defn([UP])_INTERNAL_LIBS="${USE_[]m4_defn([UP])_INTERNAL_LIBS}"
m4_defn([UP])_LDFLAGS="${m4_defn([UP])_LDFLAGS}"
m4_defn([UP])_CFLAGS="${m4_defn([UP])_CFLAGS}"

AC_SUBST([requirements_libs_]m4_defn([DOWN]))dnl
AC_SUBST([requirements_public_libs_]m4_defn([DOWN]))dnl
AC_SUBST([requirements_cflags_]m4_defn([DOWN]))dnl
AC_SUBST([requirements_pc_]m4_defn([DOWN]))dnl
AC_SUBST(m4_defn([UP])[_LIBS])dnl
AC_SUBST(m4_defn([UP])[_INTERNAL_LIBS])dnl
AC_SUBST([USE_]m4_defn([UP])[_LIBS])dnl
AC_SUBST([USE_]m4_defn([UP])[_INTERNAL_LIBS])dnl
AC_SUBST(m4_defn([UP])[_LDFLAGS])dnl
AC_SUBST(m4_defn([UP])[_CFLAGS])dnl
AC_MSG_NOTICE([Start $1 checks])dnl
m4_popdef([UP])dnl
m4_popdef([DOWN])dnl
])

dnl EFL_LIB_END(PKG)
dnl finishes the setup of an EFL library
AC_DEFUN([EFL_LIB_END],
[dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl

libdirname="m4_defn([DOWN])"
libname="m4_defn([DOWN])"

m4_defn([UP])_LDFLAGS="${EFLALL_COV_LDFLAGS} ${EFLALL_LDFLAGS} ${m4_defn([UP])_LDFLAGS}"
m4_defn([UP])_LIBS=" ${m4_defn([UP])_LDFLAGS} ${EFLALL_COV_LIBS} ${EFLALL_LIBS} ${m4_defn([UP])_LIBS} ${requirements_internal_libs_[]m4_defn([DOWN])} ${requirements_internal_deps_libs_[]m4_defn([DOWN])} ${requirements_public_libs_[]m4_defn([DOWN])} ${requirements_libs_[]m4_defn([DOWN])} ${requirements_libs_eflall} "
m4_defn([UP])_INTERNAL_LIBS="${m4_defn([UP])_INTERNAL_LIBS} ${requirements_internal_libs_[]m4_defn([DOWN])}"
USE_[]m4_defn([UP])_LIBS="${m4_defn([UP])_LIBS} lib/${libdirname}/lib${libname}.la"
USE_[]m4_defn([UP])_INTERNAL_LIBS="${m4_defn([UP])_INTERNAL_LIBS} lib/${libdirname}/lib${libname}.la"
m4_defn([UP])_CFLAGS="${EFLALL_COV_CFLAGS} ${EFLALL_CFLAGS} ${m4_defn([UP])_CFLAGS} -I\$(top_srcdir)/src/lib/${libdirname} -I\$(top_builddir)/src/lib/${libdirname} -I\$(top_srcdir)/src/bindings/${libdirname} -I\$(top_builddir)/src/bindings/${libdirname} ${requirements_cflags_[]m4_defn([DOWN])} ${requirements_cflags_eflall} -DEFL_[]m4_defn([UP])_BUILD=1"
requirements_pc_[]m4_defn([DOWN])="${requirements_pc_[]m4_defn([DOWN])} ${requirements_pc_eflall}"
requirements_pc_deps_[]m4_defn([DOWN])="${requirements_pc_deps_[]m4_defn([DOWN])} ${requirements_pc_deps_eflall}"

AC_MSG_NOTICE([Finished $1 checks])dnl
m4_popdef([UP])dnl
m4_popdef([DOWN])dnl
])

dnl EFL_LIB_START_OPTIONAL(PKG, TEST)
dnl test if library should be build and then EFL_LIB_START()
dnl must call EFL_LIB_END_OPTIONAL() to close it.
AC_DEFUN([EFL_LIB_START_OPTIONAL],
[dnl
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
m4_popdef([UP])dnl
m4_popdef([DOWN])dnl
])

dnl EFL_LIB_END_OPTIONAL(PKG)
dnl closes block started by EFL_LIB_START_OPTIONAL() and then
dnl defines AM_CONDITIONAL([HAVE_PKG]) and AC_DEFINE([HAVE_PKG])
AC_DEFUN([EFL_LIB_END_OPTIONAL],
[dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl

dnl close if started at EFL_LIB_START_OPTIONAL()
   EFL_LIB_END([$1])
fi

AM_CONDITIONAL([HAVE_]m4_defn([UP]), [test "$efl_lib_optional_[]m4_defn([DOWN])" = "yes"])dnl
m4_popdef([UP])dnl
m4_popdef([DOWN])dnl
])

dnl EFL_ADD_FEATURE(PKG, NAME, [VALUE])
dnl if VALUE is not specified, will use ${have_name} instead.
dnl
dnl Defined Variables:
dnl     features_pkg
AC_DEFUN([EFL_ADD_FEATURE],
[dnl
m4_pushdef([DOWNPKG], m4_translit([$1], [-A-Z], [_a-z]))dnl
m4_pushdef([DOWNNAME], m4_translit([$2], [-A-Z], [_a-z]))dnl

value="m4_if($3, , [${have_]m4_defn([DOWNNAME])[:-${want_]m4_defn([DOWNNAME])[}}], [$3])"
case "${value}" in
   yes)
      tmp="${COLOR_YES}+$2${COLOR_RESET}"
      ;;
   no)
      tmp="${COLOR_NO}-$2${COLOR_RESET}"
      ;;
   *)
      tmp="${COLOR_OTHER}$2=${value}${COLOR_RESET}"
      ;;
esac
if test -z "${features_[]m4_defn([DOWNPKG])}"; then
   features_[]m4_defn([DOWNPKG])="${tmp}"
else
   features_[]m4_defn([DOWNPKG])="${features_[]m4_defn([DOWNPKG])} ${tmp}"
fi
m4_popdef([DOWNNAME])dnl
m4_popdef([DOWNPKG])dnl
])

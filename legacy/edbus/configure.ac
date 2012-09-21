##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_define([v_maj], [1])
m4_define([v_min], [7])
m4_define([v_mic], [99])
m4_define([v_rev], m4_esyscmd([(svnversion "${SVN_REPO_PATH:-.}" | grep -v '\(export\|Unversioned directory\)' || echo 0) | awk -F : '{printf("%s\n", $1);}' | tr -d ' :MSP\n']))
m4_if(v_rev, [0], [m4_define([v_rev], m4_esyscmd([git log 2> /dev/null | (grep -m1 git-svn-id || echo 0) | sed -e 's/.*@\([0-9]*\).*/\1/' | tr -d '\n']))])
##--   When released, remove the dnl on the below line
dnl m4_undefine([v_rev])
##--   When doing snapshots - change soname. remove dnl on below line
dnl m4_define([relname], [ver-pre-svn-07])
dnl m4_define([v_rel], [-release relname])
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], [m4_define([v_ver], [v_maj.v_min.v_mic.v_rev])],
[m4_define([v_ver], [v_maj.v_min.v_mic])])
m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], v_mic)
m4_define([lt_age], v_min)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##

AC_INIT([edbus], [v_ver], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.60])

AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

PKG_PROG_PKG_CONFIG

EFL_COMPILER_FLAG([-Wall])
EFL_COMPILER_FLAG([-Wextra])
EFL_COMPILER_FLAG([-Wshadow])
EFL_COMPILER_FLAG([-Wno-unused-parameter])
EFL_COMPILER_FLAG([-Wvla])
EFL_COMPILER_FLAG([-Wundef])
EFL_COMPILER_FLAG([-Wformat=2])
EFL_COMPILER_FLAG([-Wlogical-op])
EFL_COMPILER_FLAG([-Wsign-compare])
EFL_COMPILER_FLAG([-Wformat-security])
EFL_COMPILER_FLAG([-Wmissing-include-dirs])
EFL_COMPILER_FLAG([-Wformat-nonliteral])
EFL_COMPILER_FLAG([-Wold-style-definition])
EFL_COMPILER_FLAG([-Wpointer-arith])
EFL_COMPILER_FLAG([-Winit-self])
EFL_COMPILER_FLAG([-Wdeclaration-after-statement])
EFL_COMPILER_FLAG([-Wmissing-declarations])
EFL_COMPILER_FLAG([-Wmissing-noreturn])
EFL_COMPILER_FLAG([-Wendif-labels])
EFL_COMPILER_FLAG([-Wstrict-aliasing=2])
EFL_COMPILER_FLAG([-Wwrite-strings])
EFL_COMPILER_FLAG([-Wno-long-long])
EFL_COMPILER_FLAG([-Wno-overlength-strings])
EFL_COMPILER_FLAG([-Wno-missing-field-initializers])
EFL_COMPILER_FLAG([-Wno-nested-externs])
EFL_COMPILER_FLAG([-Wchar-subscripts])
EFL_COMPILER_FLAG([-Wtype-limits])
EFL_COMPILER_FLAG([-Wuninitialized])

AC_LANG_C

AC_PROG_CC
AC_PROG_MKDIR_P
AM_PROG_CC_C_O
AC_C___ATTRIBUTE__
AC_C_VA_LIST_AS_ARRAY

AC_DISABLE_STATIC
define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

# doxygen program for documentation building
EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])

PKG_CHECK_MODULES([EINA], [eina >= 1.7.0])
PKG_CHECK_MODULES([ECORE], [ecore])
PKG_CHECK_MODULES([DBUS], [dbus-1])

### Build and install examples
EFL_CHECK_BUILD_EXAMPLES([enable_build_examples="yes"], [enable_build_examples="no"])

with_max_log_level="EINA_LOG_LEVEL_DBG"
AC_ARG_WITH(maximum-log-level,
   [AC_HELP_STRING([--with-maximum-log-level=NUMBER],
                   [limit log level, any call to EINA_LOG() with values greater than this will be compiled out, ignoring runtime settings, but saving function calls.])],
   [with_max_log_level="${withval}"], [:])
AC_DEFINE_UNQUOTED(EINA_LOG_LEVEL_MAXIMUM, ${with_max_log_level}, [if set, logging is limited to this amount.])

##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], , [m4_define([v_rev], [0])])
m4_ifdef([v_rel], , [m4_define([v_rel], [])])
AC_DEFINE_UNQUOTED(VMAJ, [v_maj], [Major version])
AC_DEFINE_UNQUOTED(VMIN, [v_min], [Minor version])
AC_DEFINE_UNQUOTED(VMIC, [v_mic], [Micro version])
AC_DEFINE_UNQUOTED(VREV, [v_rev], [Revison])
version_info="lt_cur:lt_rev:lt_age"
release_info="v_rel"
AC_SUBST(version_info)
AC_SUBST(release_info)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
VMAJ=v_maj
VMIN=v_min
AC_SUBST(VMAJ)
AC_SUBST(VMIN)

AC_CONFIG_FILES([
Makefile
doc/Makefile
doc/Doxyfile
])

AC_OUTPUT

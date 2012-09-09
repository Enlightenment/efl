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
m4_ifdef([v_rev], [m4_define([v_ver], [v_maj.v_min.v_mic.v_rev])], [m4_define([v_ver], [v_maj.v_min.v_mic])])
m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], v_mic)
m4_define([lt_age], v_min)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##

AC_INIT([eo], [v_ver], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.59])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])
AH_TOP([
#ifndef EFL_CONFIG_H__
#define EFL_CONFIG_H__
])
AH_BOTTOM([
#endif /* EFL_CONFIG_H__ */
])

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], , [m4_define([v_rev], [0])])
m4_ifdef([v_rel], , [m4_define([v_rel], [])])
AC_DEFINE_UNQUOTED([VMAJ], [v_maj], [Major version])
AC_DEFINE_UNQUOTED([VMIN], [v_min], [Minor version])
AC_DEFINE_UNQUOTED([VMIC], [v_mic], [Micro version])
AC_DEFINE_UNQUOTED([VREV], [v_rev], [Revison])
version_info="lt_cur:lt_rev:lt_age"
release_info="v_rel"
AC_SUBST([version_info])
AC_SUBST([release_info])
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
VMAJ=v_maj
AC_SUBST([VMAJ])


### Needed information

AC_CANONICAL_HOST

EFL_EO_BUILD=""
case "$host_os" in
   mingw*)
      EFL_EO_BUILD="-DEFL_EO_BUILD"
   ;;
esac
AC_SUBST([EFL_EO_BUILD])

requirements_pc_eo="eina"


### Additional options to configure

AC_ARG_ENABLE([coverage],
   [AC_HELP_STRING([--enable-coverage], [enable coverage profiling instrumentation @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       _efl_enable_coverage="yes"
    else
       _efl_enable_coverage="no"
    fi
   ],
   [_efl_enable_coverage="no"])

AC_MSG_CHECKING([whether to use profiling instrumentation])
AC_MSG_RESULT([${_efl_enable_coverage}])


### Checks for programs

AC_PROG_CC

# pkg-config
PKG_PROG_PKG_CONFIG

# Check whether pkg-config supports Requires.private
if $PKG_CONFIG --atleast-pkgconfig-version 0.22; then
   pkgconfig_requires_private="Requires.private"
else
   pkgconfig_requires_private="Requires"
fi
AC_SUBST(pkgconfig_requires_private)

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])

# lcov

if test "x$_efl_enable_coverage" = "xyes" ; then
   AC_CHECK_PROG(have_lcov, [lcov], [yes], [no])
   if test "x$have_lcov" = "xno" ; then
      _efl_enable_coverage="no"
   fi
fi

AM_CONDITIONAL([EFL_ENABLE_COVERAGE], [test "x${_efl_enable_coverage}" = "xyes"])


### Checks for libraries

PKG_CHECK_MODULES([EO], [${requirements_pc_eo}])

PKG_CHECK_MODULES([ELM], [elementary >= 1.7.0], [have_elm="yes"], [have_elm="no"])

AM_CONDITIONAL([EO_BUILD_EXAMPLE_EVAS], [test "x${have_elm}" = "xyes"])


### Checks for header files

AC_CHECK_HEADERS([stdarg.h])


### Checks for types


### Checks for structures


### Checks for compiler characteristics

if test "x$_efl_enable_coverage" = "xyes" ; then
   EO_CFLAGS="${EO_CFLAGS} -fprofile-arcs -ftest-coverage"
   EO_LIBS="${EO_LIBS} -lgcov"
fi


### Checks for linker characteristics

lt_enable_auto_import=""
case "${host_os}" in
   mingw*)
      lt_enable_auto_import="-Wl,--enable-auto-import"
   ;;
esac
AC_SUBST([lt_enable_auto_import])

### Checks for library functions

### Make the debug preprocessor configurable

### Unit tests, coverage and benchmarking

EFL_CHECK_BENCHMARK([enable_benchmark="yes"], [enable_benchmark="no"])

AC_SUBST([requirements_pc_eo])


### Build and install examples
EFL_CHECK_BUILD_EXAMPLES([enable_build_examples="yes"], [enable_build_examples="no"])

AC_CONFIG_FILES([
Makefile
eo.pc
doc/Doxyfile
doc/Makefile
src/Makefile
src/benchmarks/Makefile
src/examples/Makefile
src/examples/eo_isa/Makefile
src/examples/evas/Makefile
src/examples/simple/Makefile
src/lib/Makefile
src/tests/Makefile
src/tests/access/Makefile
src/tests/composite_objects/Makefile
src/tests/constructors/Makefile
src/tests/eo_suite/Makefile
src/tests/function_overrides/Makefile
src/tests/interface/Makefile
src/tests/mixin/Makefile
src/tests/signals/Makefile
])

AC_OUTPUT

#####################################################################
## Info

echo
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE $VERSION"
echo "------------------------------------------------------------------------"
echo
echo
echo "Configuration Options Summary:"
echo
echo "  Documentation........: ${build_doc}"
echo "  Unit Tests...........:"
echo "    build..............: make check"
echo "    coverage...........: ${_efl_enable_coverage}"
echo "  Examples.............: ${enable_build_examples}"
echo "  Benchmark............: ${enable_benchmark}"
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo

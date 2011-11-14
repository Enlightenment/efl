##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_define([v_maj], [1])
m4_define([v_min], [0])
m4_define([v_mic], [999])
m4_define([v_rev], m4_esyscmd([(svnversion "${SVN_REPO_PATH:-.}" | grep -v export || echo 0) | awk -F : '{printf("%s\n", $1);}' | tr -d ' :MSP\n']))
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

AC_INIT([efreet], [v_ver], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.52])
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
AC_SUBST(VMAJ)

### Needed information

AC_CANONICAL_BUILD
AC_CANONICAL_HOST


### Additional options to configure

AC_ARG_ENABLE([strict-spec],
   [AC_HELP_STRING([--enable-strict-spec], [Enable strict spec compliance @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_strict_spec="yes"
    else
       enable_strict_spec="no"
    fi
   ],
   [enable_strict_spec="no"])

if test "x${enable_strict_spec}" = "xyes" ; then
   AC_DEFINE([STRICT_SPEC], [1], [Strict Spec Compliance])
fi

AC_ARG_ENABLE([sloppy-spec],
   [AC_HELP_STRING([--disable-sloppy-spec], [Enable sloppy spec compliance @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_sloppy_spec="yes"
    else
       enable_sloppy_spec="no"
    fi
   ],
   [enable_sloppy_spec="yes"])

if test "x${enable_sloppy_spec}" = "xyes" ; then
   AC_DEFINE([SLOPPY_SPEC], [1], [Sloppy Spec Compliance])
fi


#AC_ARG_ENABLE(hidden-visibility,
#	[AC_HELP_STRING([--enable-hidden-visibility],[Enable hidden visibility])],
#	[enable_hidden_visibility=$enableval], [enable_hidden_visibility="auto"])
#if test "x$enable_hidden_visibility" = "xyes" ; then
#    CPPFLAGS="$CPPFLAGS -fvisibility=hidden"
#else
#    AC_DEFINE(DEFAULT_VISIBLITY, 1, [Default visibility])
#fi
#AM_CONDITIONAL(DEFAULT_VISIBILITY, test "x$enable_hidden_visibility" != "xyes")
AM_CONDITIONAL(DEFAULT_VISIBILITY, false)


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


### Checks for libraries

# Evil library for compilation on Windows CE

EFL_EFREET_BUILD=""
EFL_EFREET_MIME_BUILD=""
EFL_EFREET_TRASH_BUILD=""
case "$host_os" in
   mingw*)
      PKG_CHECK_MODULES([EVIL], [evil])
      AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if evil package is installed])
      requirement_efreet="evil"
      EFL_EFREET_BUILD="-DEFL_EFREET_BUILD"
      EFL_EFREET_MIME_BUILD="-DEFL_EFREET_MIME_BUILD"
      EFL_EFREET_TRASH_BUILD="-DEFL_EFREET_TRASH_BUILD"
   ;;
esac
AC_SUBST(EFL_EFREET_BUILD)
AC_SUBST(EFL_EFREET_MIME_BUILD)
AC_SUBST(EFL_EFREET_TRASH_BUILD)

requirement_efreet="ecore-file >= 1.0.0 ecore >= 1.0.0 eet >= 1.4.0 eina >= 1.0.0 ${requirement_efreet}"
PKG_CHECK_MODULES(EFREET, [${requirement_efreet}])
PKG_CHECK_MODULES(EINA, [eina >= 1.0.0])


### Checks for header files

AC_CHECK_HEADERS([netinet/in.h arpa/inet.h])

### Checks for types


### Checks for structures


### Checks for compiler characteristics

AC_C_CONST
AC_C___ATTRIBUTE__
AM_PROG_CC_STDC
AC_HEADER_STDC

case "${host_os}" in
   mingw*)
      EFREET_CFLAGS="${EFREET_CFLAGS} ${EVIL_CFLAGS}"
   ;;
esac

if ! test "x${VMIC}" = "x" ; then
   EFL_COMPILER_FLAG([-Wall])
   EFL_COMPILER_FLAG([-W])
fi

EFL_COMPILER_FLAG([-Wshadow])

EFL_CHECK_PATH_MAX


### Checks for linker characteristics
lt_enable_auto_import=""
WIN32_LIBS=""
case "${host_os}" in
   mingw*)
      WIN32_LIBS="-lws2_32"
      lt_enable_auto_import="-Wl,--enable-auto-import"
   ;;
esac
AC_SUBST(WIN32_LIBS)
AC_SUBST(lt_enable_auto_import)


### Checks for library functions
AC_ISC_POSIX
AC_FUNC_ALLOCA
AC_CHECK_FUNCS(strlcpy clearenv)

### Unit tests, coverage

EFL_CHECK_TESTS([enable_tests="yes"], [enable_tests="no"])

EFL_CHECK_COVERAGE([${enable_tests}], [enable_coverage="yes"], [enable_coverage="no"])
EFREET_CFLAGS="${EFREET_CFLAGS} ${EFL_COVERAGE_CFLAGS}"
EFREET_LIBS="${EFREET_LIBS} ${EFL_COVERAGE_LIBS}"
if test "x$enable_coverage" = "xyes" ; then
   EFREET_CFLAGS="${EFREET_CFLAGS} ${EFL_DEBUG_CFLAGS}"
fi

AC_SUBST(requirement_efreet)

AC_OUTPUT([
efreet.spec
efreet.pc
efreet-mime.pc
efreet-trash.pc
Makefile
doc/Makefile
doc/Doxyfile
src/Makefile
src/lib/Makefile
src/bin/Makefile
src/tests/Makefile
src/tests/data/Makefile
src/tests/data/sub/Makefile
src/tests/compare/Makefile
])


#####################################################################
## Info

echo
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE $VERSION"
echo "------------------------------------------------------------------------"
echo
echo "Configuration Options Summary:"
echo
echo "  Specification compliance:"
echo "    Strict.............: ${enable_strict_spec}"
echo "    Sloppy.............: ${enable_sloppy_spec}"
echo
echo "  Tests................: ${enable_tests}"
echo "  Coverage.............: ${enable_coverage}"
echo
echo "  Documentation........: ${build_doc}"
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo

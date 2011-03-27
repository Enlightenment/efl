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
dnl m4_define([v_rel], [-release ver-pre-svn-07])
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], [m4_define([v_ver], [v_maj.v_min.v_mic.v_rev])], [m4_define([v_ver], [v_maj.v_min.v_mic])])
m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], v_mic)
m4_define([lt_age], v_min)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##

AC_INIT([embryo], [v_ver], [enlightenment-devel@lists.sourceforge.net])
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

### Default options with respect to host

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

requirement_embryo=""
embryoincludedir="${datadir}/include"


### Additional options to configure

EFL_ENABLE_BIN([embryo-cc])


### Checks for programs
AC_PROG_CC

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])

# pkg-config

PKG_PROG_PKG_CONFIG

# Check whether pkg-config supports Requires.private
if $PKG_CONFIG --atleast-pkgconfig-version 0.22; then
   pkgconfig_requires_private="Requires.private"
else
   pkgconfig_requires_private="Requires"
fi
AC_SUBST(pkgconfig_requires_private)


### Checks for libraries

# Evil library for compilation on Windows

EFL_EMBRYO_BUILD=""
case "$host_os" in
   mingw* | cegcc*)
   PKG_CHECK_MODULES([EVIL], [evil])
   AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if Evil library is installed])
   requirement_embryo="evil ${requirement_embryo}"
   EFL_EMBRYO_BUILD="-DEFL_EMBRYO_BUILD"
   ;;
esac
AC_SUBST(EFL_EMBRYO_BUILD)


### Checks for header files

EFL_CHECK_PATH_MAX


### Checks for types


### Checks for structures


### Checks for compiler characteristics
AC_C_BIGENDIAN
AM_PROG_CC_C_O
AC_C_CONST
AC_C_INLINE
AC_PROG_CC_STDC
AC_HEADER_STDC
AC_C___ATTRIBUTE__

EMBRYO_CPPFLAGS=""
EMBRYO_CFLAGS=""
case "$host_os" in
   mingw32ce*)
      EMBRYO_CPPFLAGS="-D_WIN32_WCE=0x0420"
      ;;
   cegcc*)
      EMBRYO_CPPFLAGS="-D_WIN32_WCE=0x0420"
      EMBRYO_CFLAGS="-mwin32"
      ;;
esac
AC_SUBST(EMBRYO_CPPFLAGS)
AC_SUBST(EMBRYO_CFLAGS)


### Checks for linker characteristics

lt_enable_auto_import=""
case "$host_os" in
   mingw* | cegcc*)
      lt_enable_auto_import="-Wl,--enable-auto-import"
      ;;
esac
AC_SUBST(lt_enable_auto_import)


### Checks for library functions

AC_ISC_POSIX

# alloca
AC_FUNC_ALLOCA

# fnmatch
EFL_CHECK_FNMATCH([], [AC_MSG_ERROR([Cannot find fnmatch()])])

case "$host_os" in
   mingw* | cegcc*)
      AC_DEFINE(HAVE_GETTIMEOFDAY, 1, [Defined to 1 (MinGW / CeGCC platform)])
      ;;
   *)
      AC_CHECK_FUNCS(gettimeofday)
      ;;
esac


AC_SUBST(requirement_embryo)
AC_SUBST(embryoincludedir)

AC_OUTPUT([
Makefile
doc/Makefile
doc/embryo.dox
embryo.pc
include/Makefile
src/Makefile
src/lib/Makefile
src/bin/Makefile
README
embryo.spec
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
echo "  Build embryo_cc......: $have_embryo_cc"
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

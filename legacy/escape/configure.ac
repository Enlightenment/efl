
### Version

m4_define([v_maj], [0])
m4_define([v_min], [99])
m4_define([v_mic], [0])

m4_define([escape_version], [v_maj.v_min.v_mic])

m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], v_mic)
m4_define([lt_age], v_min)

AC_INIT([escape], [escape_version], [enlightenment-devel@lists.sourceforge.net])
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


### If the host is not for PS3
AC_PROG_LIBTOOL
AC_CANONICAL_HOST

case "$host_vendor" in
  ps3*)
    ;;
  *)
    AC_MSG_ERROR([OS must be PS3. Exiting...])
    ;;
esac

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

LT_PREREQ([2.2])

VMAJ=v_maj
AC_SUBST(VMAJ)

version_info="lt_cur:lt_rev:lt_age"
AC_SUBST(version_info)


### Checks for programs
AM_PROG_AS
AC_PROG_CXX
AC_LANG(C)
AC_PROG_CPP
AC_PROG_CC

### Checks for libraries

ESCAPE_CFLAGS=""
ESCAPE_CPPFLAGS=""
ESCAPE_LIBS="-llv2"

AC_SUBST(ESCAPE_CPPFLAGS)
AC_SUBST(ESCAPE_CFLAGS)
AC_SUBST(ESCAPE_LIBS)

### Checks for header files
AC_CHECK_HEADERS([errno.h])


### Checks for types
EFL_CHECK_PATH_MAX


### Checks for structures


### Checks for compiler characteristics
AC_C_CONST

### Checks for linker characteristics


### Checks for library functions


AC_CONFIG_FILES([
Makefile
escape.pc
src/Makefile
src/lib/Makefile
])

AC_OUTPUT


#####################################################################
## Info

echo
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE_NAME $PACKAGE_VERSION"
echo "------------------------------------------------------------------------"
echo
echo "Configuration Options Summary:"
echo
echo "  OS...................: ${host_os}"
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  CXXFLAGS.............: $CXXFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo


### Version

m4_define([v_maj], [0])
m4_define([v_min], [99])
m4_define([v_mic], [0])

m4_define([evil_version], [v_maj.v_min.v_mic])

m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], v_mic)
m4_define([lt_age], v_min)

AC_INIT([evil], [evil_version], [enlightenment-devel@lists.sourceforge.net])
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


### If the host is not Windows, or is ceGCC, we exit

AC_CANONICAL_HOST

case "$host_os" in
  cegcc*)
    AC_MSG_ERROR([ceGCC compiler is not supported anymore. Exiting...])
    ;;
  mingw*)
    ;;
  *)
    AC_MSG_ERROR([OS must be Windows. Exiting...])
    ;;
esac

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

LT_PREREQ([2.4])
LT_INIT([win32-dll])

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

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])


### Checks for libraries

win32_libs=""
case "$host_os" in
  mingw32ce*)
    win32_libs="-lws2"
    ;;
  *)
    win32_libs="-lole32 -lws2_32 -lsecur32"
    ;;
esac
AC_SUBST(win32_libs)

EFL_CHECK_MPATROL([have_mpatrol="yes"], [have_mpatrol="no"])


### Checks for header files
AC_CHECK_HEADERS([errno.h])


### Checks for types


### Checks for structures


### Checks for compiler characteristics
AC_C_CONST
AC_C___ATTRIBUTE__

win32_cppflags="-DEFL_EVIL_BUILD"
win32_cflags="-Wall -Wextra -Wshadow -Wdeclaration-after-statement -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls"
win32_cxxflags=""
win32_libs=""
have_wince="no"
case "$host_os" in
  mingw32ce*)
    have_wince="yes"
    win32_cppflags="${win32_cppflags} -D_WIN32_WCE=0x0420"
    win32_libs="-ltoolhelp"
    ;;
  *)
    have_wince="no"
    win32_cxxflags="-fno-rtti -fno-exceptions"
    win32_cppflags="${win32_cppflags} -D_WIN32_WINNT=0x0501 -DSECURITY_WIN32"
    win32_libs="-lpsapi"
    ;;
esac
AC_SUBST([win32_cppflags])
AC_SUBST([win32_cflags])
AC_SUBST([win32_cxxflags])
AC_SUBST([win32_libs])

AM_CONDITIONAL(EVIL_HAVE_WINCE, test "x${have_wince}" = "xyes")


### Checks for linker characteristics


### Checks for library functions


AC_CONFIG_FILES([
Makefile
evil.pc
doc/Makefile
src/Makefile
src/bin/Makefile
src/lib/Makefile
src/lib/dlfcn/Makefile
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
echo "  Documentation........: ${build_doc}"
echo "  Mpatrol..............: ${have_mpatrol}"
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

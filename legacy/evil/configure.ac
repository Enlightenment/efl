
### Version

m4_define([v_maj], [1])
m4_define([v_min], [6])
m4_define([v_mic], [99])

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
dnl    AC_MSG_ERROR([OS must be Windows. Exiting...])
    ;;
esac

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

lt_cv_deplibs_check_method='pass_all'
LT_INIT([win32-dll])

VMAJ=v_maj
AC_SUBST([VMAJ])

version_info="lt_cur:lt_rev:lt_age"
AC_SUBST([version_info])


### Needed information

EFL_SELECT_WINDOWS_VERSION


### Checks for programs
AM_PROG_AS
AC_PROG_CXX
AC_LANG([C])
AC_PROG_CPP
AC_PROG_CC

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])


### Checks for libraries

win32_libs=""
win32_dl_libs=""
case "$host_os" in
  mingw32ce*)
    win32_libs="-lws2"
    win32_dl_libs="-ltoolhelp"
    ;;
  *)
    win32_libs="-lole32 -luuid -lws2_32 -lsecur32"
    win32_dl_libs="-lpsapi"
    ;;
esac
AC_SUBST([win32_libs])
AC_SUBST([win32_dl_libs])


### Checks for header files
AC_CHECK_HEADERS([errno.h])


### Checks for types


### Checks for structures


### Checks for compiler characteristics
EFL_ATTRIBUTE_UNUSED

win32_cppflags=""
win32_cflags="-Wall -Wextra -Wshadow -Wdeclaration-after-statement -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls"
win32_cflags="${win32_cflags} -mms-bitfields"
win32_cxxflags=""
have_wince="no"
case "$host_os" in
  mingw32ce*)
    have_wince="yes"
    win32_cppflags="${win32_cppflags}"
    ;;
  *)
    have_wince="no"
    win32_cxxflags="-fno-rtti -fno-exceptions"
    win32_cppflags="${win32_cppflags} -DSECURITY_WIN32"
    ;;
esac
AC_SUBST([win32_cppflags])
AC_SUBST([win32_cflags])
AC_SUBST([win32_cxxflags])

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
echo "  Windows version......: ${_efl_windows_version}"
echo
echo "  Documentation........: ${build_doc}"
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

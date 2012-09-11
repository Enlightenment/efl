m4_define([v_maj], [1])
m4_define([v_min], [7])
m4_define([v_mic], [99])

m4_define([v_rev], m4_esyscmd([(svnversion "${SVN_REPO_PATH:-.}" | grep -v '\(export\|Unversioned directory\)' || echo 0) | awk -F : '{printf("%s\n", $1);}' | tr -d ' :MSP\n']))
m4_if(v_rev, [0], [m4_define([v_rev], m4_esyscmd([git log 2> /dev/null | (grep -m1 git-svn-id || echo 0) | sed -e 's/.*@\([0-9]*\).*/\1/' | tr -d '\n']))])

#### FIXME: i am sure that we can do some m4 to automagically do the stuff below for release and snapshots

##--   When released, remove the dnl on the below line
dnl m4_undefine([v_rev])

##--   When doing snapshots - change soname. remove dnl on below line
dnl m4_define([relname], [ver-pre-svn-07])
dnl m4_define([v_rel], [-release relname])

m4_ifdef([v_rev], [m4_define([efl_version], [v_maj.v_min.v_mic.v_rev])], [m4_define([efl_version], [v_maj.v_min.v_mic])])

m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], v_mic)
m4_define([lt_age], v_min)

AC_INIT([efl], [efl_version], [enlightenment-devel@lists.sourceforge.net])
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

AC_GNU_SOURCE
AC_SYS_LARGEFILE

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

m4_ifdef([v_rev], , [m4_define([v_rev], [0])])
AC_DEFINE_UNQUOTED([VMAJ], [v_maj], [Major version])
AC_DEFINE_UNQUOTED([VMIN], [v_min], [Minor version])
AC_DEFINE_UNQUOTED([VMIC], [v_mic], [Micro version])
AC_DEFINE_UNQUOTED([VREV], [v_rev], [Revison])
VMAJ=v_maj
AC_SUBST([VMAJ])


#### Default values

want_eina="yes"
want_eet="yes"
want_evas="yes"
want_ecore="yes"
want_embryo="yes"
want_eio="yes"
want_edje="yes"
want_efreet="yes"
want_e_dbus="yes"
want_eeze="yes"
want_emotion="yes"
want_ethumb="yes"
want_elementary="yes"

requirements_libs_evil=""
requirements_libs_eina=""
requirements_libs_eet=""
requirements_libs_evas=""
requirements_libs_ecore=""
requirements_libs_embryo=""
requirements_libs_eio=""
requirements_libs_edje=""
requirements_libs_efreet=""
requirements_libs_e_dbus=""
requirements_libs_eeze=""
requirements_libs_emotion=""
requirements_libs_ethumb=""
requirements_libs_elementary=""

requirements_pc_eina=""
requirements_pc_eet=""
requirements_pc_evas=""
requirements_pc_ecore=""
requirements_pc_embryo=""
requirements_pc_eio=""
requirements_pc_edje=""
requirements_pc_efreet=""
requirements_pc_e_dbus=""
requirements_pc_eeze=""
requirements_pc_emotion=""
requirements_pc_ethumb=""
requirements_pc_elementary=""

AC_SUBST([requirements_libs_evil])
AC_SUBST([requirements_libs_eina])
AC_SUBST([requirements_libs_eet])
AC_SUBST([requirements_libs_evas])
AC_SUBST([requirements_libs_ecore])
AC_SUBST([requirements_libs_embryo])
AC_SUBST([requirements_libs_eio])
AC_SUBST([requirements_libs_edje])
AC_SUBST([requirements_libs_efreet])
AC_SUBST([requirements_libs_e_dbus])
AC_SUBST([requirements_libs_eeze])
AC_SUBST([requirements_libs_emotion])
AC_SUBST([requirements_libs_ethumb])
AC_SUBST([requirements_libs_elementary])

AC_SUBST([requirements_pc_eina])
AC_SUBST([requirements_pc_eet])
AC_SUBST([requirements_pc_evas])
AC_SUBST([requirements_pc_ecore])
AC_SUBST([requirements_pc_embryo])
AC_SUBST([requirements_pc_eio])
AC_SUBST([requirements_pc_edje])
AC_SUBST([requirements_pc_efreet])
AC_SUBST([requirements_pc_e_dbus])
AC_SUBST([requirements_pc_eeze])
AC_SUBST([requirements_pc_emotion])
AC_SUBST([requirements_pc_ethumb])
AC_SUBST([requirements_pc_elementary])


AC_CANONICAL_HOST

have_wince="no"
have_win32="no"
have_windows="no"
case "$host_os" in
   cegcc*)
      AC_MSG_ERROR([ceGCC compiler is not supported anymore. Exiting...])
   ;;
   mingw32ce*)
      have_wince="yes"
      have_windows="yes"
      want_efreet="no"
      want_e_dbus="no"
      want_eeze="no"
      want_emotion="no"
      want_ethumb="no"
   ;;
   mingw*)
      have_win32="yes"
      have_windows="yes"
      want_eeze="no"
   ;;
esac

AM_CONDITIONAL([HAVE_WINCE], [test "x${have_wince}" = "xyes"])
AM_CONDITIONAL([HAVE_WIN32], [test "x${have_win32}" = "xyes"])
AM_CONDITIONAL([HAVE_WINDOWS], [test "x${have_windows}" = "xyes"])


#### Additional options to configure


#### Checks for programs

### libtool

if test "x${have_windows}" = "xyes" ; then
   lt_cv_deplibs_check_method='pass_all'
fi
AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL
version_info="lt_cur:lt_rev:lt_age"
AC_SUBST([version_info])
release_info="v_rel"
AC_SUBST([release_info])

### compilers

AM_PROG_AS
AC_PROG_CXX
AC_LANG(C)
AC_PROG_CPP
AC_PROG_CC

# pkg-config

PKG_PROG_PKG_CONFIG
if test "x${PKGCONFIG}" = "x" ; then
   AC_MSG_ERROR([pkg-config tool not found. Install it or set PKGCONFIG environment variable to that path tool. Exiting...])
fi

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])


#### Checks for libraries


#### Checks for header files


#### Checks for types


#### Checks for structures


#### Checks for compiler characteristics

EFL_ATTRIBUTE_UNUSED
# EFL_CHECK_COMPILER_FLAGS([MY_LIB], [-Wall -Wextra])


#### Checks for linker characteristics

lt_enable_auto_import=""
case "${host_os}" in
   mingw*)
      lt_enable_auto_import="-Wl,--enable-auto-import"
   ;;
esac
AC_SUBST([lt_enable_auto_import])


#### Checks for library functions



######################  EFL  ######################

#### Evil

if test "x${have_windows}" = "xyes" ; then

### Default values

### Additional options to configure
EFL_SELECT_WINDOWS_VERSION

### Checks for programs

### Checks for libraries

EVIL_LIBS=""
EVIL_DLFCN_LIBS=""
case "$host_os" in
  mingw32ce*)
    EVIL_LIBS="-lws2"
    EVIL_DLFCN_LIBS="-ltoolhelp"
    ;;
  *)
    EVIL_LIBS="-lole32 -lws2_32 -lsecur32"
    EVIL_DLFCN_LIBS="-lpsapi"
    ;;
esac
requirements_libs_evil="${EVIL_LIBS}"
AC_SUBST([EVIL_LIBS])
AC_SUBST([EVIL_DLFCN_LIBS])

### Checks for header files

AC_CHECK_HEADERS([errno.h])

### Checks for types

### Checks for structures

### Checks for compiler characteristics

EVIL_CPPFLAGS="-DEFL_EVIL_BUILD"
EVIL_CFLAGS="-Wall -Wextra -Wshadow -Wdeclaration-after-statement -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls -mms-bitfields"
EVIL_CXXFLAGS=""

if test "x${have_win32}" = "xyes" ; then
   EVIL_CXXFLAGS="-fno-rtti -fno-exceptions"
   EVIL_CPPFLAGS="${EVIL_CPPFLAGS} -DSECURITY_WIN32"
fi

AC_SUBST([EVIL_CPPFLAGS])
AC_SUBST([EVIL_CFLAGS])
AC_SUBST([EVIL_CXXFLAGS])

### Checks for linker characteristics

### Checks for library functions

fi

#### End of Evil


#### Eina

### Default values
### Additional options to configure
### Checks for programs
### Checks for libraries
### Checks for header files
### Checks for types
### Checks for structures
### Checks for compiler characteristics
### Checks for linker characteristics
### Checks for library functions

#### End of Eina

#EFL_CHECK_LIBS([eina], [iconv], [have_eina="yes"], [have_eina="no"])
#EFL_CHECK_LIBS([eet], [zlib libjpeg], [have_eet="yes"], [have_eet="no"])

AC_CONFIG_FILES([
Makefile
src/Makefile
src/bin/Makefile
src/bin/evil/Makefile
src/lib/Makefile
src/lib/evil/Makefile
])

AC_OUTPUT


#### Info

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
if test "x${have_windows}" = "xyes" ; then
   echo "  Windows version......: ${_efl_windows_version}"
fi
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


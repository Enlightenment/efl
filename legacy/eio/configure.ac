##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_define([v_maj], [1])
m4_define([v_min], [7])
m4_define([v_mic], [0])
m4_define([v_rev], m4_esyscmd([(svnversion "${SVN_REPO_PATH:-.}" | grep -v '\(export\|Unversioned directory\)' || echo 0) | awk -F : '{printf("%s\n", $1);}' | tr -d ' :MSP\n']))
m4_if(v_rev, [0], [m4_define([v_rev], m4_esyscmd([git log 2> /dev/null | (grep -m1 git-svn-id || echo 0) | sed -e 's/.*@\([0-9]*\).*/\1/' | tr -d '\n']))])
##--   When released, remove the dnl on the below line
m4_undefine([v_rev])
##--   When doing snapshots - change soname. remove dnl on below line
dnl m4_define([relname], [ver-pre-svn-09])
dnl m4_define([v_rel], [-release relname])
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], [m4_define([v_ver], [v_maj.v_min.v_mic.v_rev])],
[m4_define([v_ver], [v_maj.v_min.v_mic])])
m4_define([lt_rev], m4_eval(v_maj + v_min))
m4_define([lt_cur], v_mic)
m4_define([lt_age], v_min)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##

AC_INIT([eio], [v_ver-beta], [enlightenment-devel@lists.sourceforge.net])
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

AC_GNU_SOURCE
AC_SYS_LARGEFILE

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
version_info="lt_rev:lt_cur:lt_age"
release_info="v_rel"
AC_SUBST([version_info])
AC_SUBST([release_info])
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
VMAJ=v_maj
AC_SUBST([VMAJ])


### Needed information

AC_CANONICAL_BUILD
AC_CANONICAL_HOST


### Default options with respect to host

want_inotify="no"
want_notify_win32="no"

case "$host_os" in
   mingw*)
      want_notify_win32="yes"
      ;;
   *)
      want_inotify="yes"
      ;;
esac


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
AC_SUBST([pkgconfig_requires_private])

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])


### Checks for libraries

EFL_EIO_BUILD=""
case "$host_os" in
   mingw*)
      PKG_CHECK_EXISTS([evil >= 1.6.99])
      AC_DEFINE([HAVE_EVIL], [1], [Set to 1 if evil package is installed])
      requirements_eio="${requirements_eio} evil >= 1.6.99"
      EFL_EIO_BUILD="-DEFL_EIO_BUILD"
   ;;
esac
AC_SUBST([EFL_EIO_BUILD])

requirements_eio="${requirements_eio} eet >= 1.6.99 eina >= 1.6.99 ecore >= 1.6.99"
AC_SUBST([requirements_eio])

PKG_CHECK_MODULES([EIO], [${requirements_eio}])


### Checks for header files

AC_HEADER_DIRENT
AC_HEADER_TIME
AC_CHECK_HEADERS([unistd.h libgen.h grp.h features.h pwd.h])

EFL_CHECK_THREADS(
   [
    if test "x${_efl_have_posix_threads}" = "xyes" ; then
       have_threads="POSIX"
    else
       if test "x${_efl_have_win32_threads}" = "xyes" ; then
          have_threads="Win32"
       else
          have_threads="no"
       fi
    fi
   ],
   [have_threads="no"])

if test "x${have_threads}" = "xno" ; then
   AC_MSG_ERROR([Threads not supported. Be sure to have pthread on non Windows OS])
fi

EFL_CHECK_PATH_MAX

### Checks for types

### Checks for structures

### Checks for compiler characteristics
AC_C_BIGENDIAN
AC_C_INLINE
AC_C___ATTRIBUTE__
AM_PROG_CC_C_O

EFL_EIO_BUILD=""
case "$host_os" in
   mingw*)
      EFL_EIO_BUILD="-DEFL_EIO_BUILD"
   ;;
esac
AC_SUBST(EFL_EIO_BUILD)

### Checks for linker characteristics

# use --enable-auto-import on Windows

lt_enable_auto_import=""
case "$host_os" in
   mingw* | cegcc*)
      lt_enable_auto_import="-Wl,--enable-auto-import"
      ;;
esac
AC_SUBST(lt_enable_auto_import)

### Checks for library functions
AC_CHECK_FUNCS([fchmod chown getpwnam getgrnam])

### Check for splice system call

AC_MSG_CHECKING([whether to use splice for file copy])
AC_TRY_LINK(
   [
#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif
#include <fcntl.h>
   ],
   [
long ret = splice(0,0,1,0,400,0);
   ],
   [have_splice="yes"],
   [have_splice="no"])
AC_MSG_RESULT([${have_splice}])

if test "x${have_splice}" = "xyes" ; then
   AC_DEFINE([HAVE_SPLICE], [1], [Define to mention that splice syscall is supported])
fi

# Check for lstat

AC_MSG_CHECKING([whether lstat is available])
AC_TRY_LINK(
   [
#include <sys/types.h>
#include <sys/stat.h>
#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif
   ],
   [
struct stat st;
lstat("/tmp", &st);
   ],
   [have_lstat="yes"],
   [have_lstat="no"])
AC_MSG_RESULT([${have_lstat}])

if test "x${have_lstat}" = "xyes" ; then
   AC_DEFINE([HAVE_LSTAT], [1], [Define to mention that lstat syscall is supported])
fi

# extended attribute

AC_MSG_CHECKING([for extended attributes])

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
       [[
#include <stdlib.h>
#include <sys/types.h>
#include <sys/xattr.h>
       ]],
       [[
size_t tmp = listxattr("/", NULL, 0);
tmp = getxattr("/", "user.ethumb.md5", NULL, 0);
setxattr("/", "user.ethumb.md5", NULL, 0, 0);
       ]])],
   [
    AC_DEFINE([HAVE_XATTR], [1], [Define to 1 if you have the `listxattr', `setxattr' and `getxattr' functions.])
    have_xattr="yes"
   ],
   [have_xattr="no"])

AC_MSG_RESULT([${have_xattr}])

# Check for inotify specificity
have_inotify="no"
have_notify_win32="no"
EIO_CHECK_INOTIFY([${want_inotify}], [have_inotify="yes"], [have_inotify="no"])
EIO_CHECK_NOTIFY_WIN32([${want_notify_win32}], [have_notify_win32="yes"], [have_notify_win32="no"])

AM_CONDITIONAL([EIO_HAVE_INOTIFY], [test "x$have_inotify" = "xyes"])
AM_CONDITIONAL([EIO_HAVE_WINCHANGE], [test "x$have_notify_win32" = "xyes"])
EFL_CHECK_BUILD_EXAMPLES([enable_build_examples="yes"], [enable_build_examples="no"])
EFL_CHECK_INSTALL_EXAMPLES([enable_install_examples="yes"], [enable_install_examples="no"])


AC_OUTPUT([
eio.pc
eio.spec
Makefile
doc/Makefile
doc/eio.dox
doc/Doxyfile
src/Makefile
src/lib/Makefile
src/examples/Makefile
])

echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE_NAME $PACKAGE_VERSION"
echo "------------------------------------------------------------------------"
echo
echo
echo
echo "Configuration Options Summary:"
echo
echo "  Thread Support.......: ${have_threads}"
echo "  Inotify..............: ${have_inotify}"
echo "  Windows notification.: ${have_notify_win32}"
echo
echo "  Documentation........: ${build_doc}"
echo "  Examples...............: ${enable_build_examples}"
echo "  Examples installed.....: ${enable_install_examples}"
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo

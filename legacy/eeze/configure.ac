##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_define([v_maj], [1])
m4_define([v_min], [0])
m4_define([v_mic], [999])
m4_define([v_rev], m4_esyscmd([(svnversion "${SVN_REPO_PATH:-.}" | grep -v export || echo 0) | awk -F : '{printf("%s\n", $1);}' | tr -d ' :MSP\n']))
m4_if(v_rev, [0], [m4_define([v_rev], m4_esyscmd([git log 2> /dev/null | (grep -m1 git-svn-id || echo 0) | sed -e 's/.*@\([0-9]*\).*/\1/' | tr -d '\n']))])
##--   When released, remove the dnl on the below line
dnl m4_undefine([v_rev])
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], [m4_define([v_ver], [v_maj.v_min.v_mic.v_rev])], [m4_define([v_ver], [v_maj.v_min.v_mic])])
m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], v_mic)
m4_define([lt_age], v_min)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##

AC_INIT([eeze], [v_ver], [enlightenment-devel@lists.sourceforge.net])
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

requirement_eeze="ecore >= 1.0.0 eina >= 1.0.0 libudev"


### Checks for programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_C___ATTRIBUTE__

# pkg-config
PKG_PROG_PKG_CONFIG

# Check whether pkg-config supports Requires.private
AS_IF(
   [$PKG_CONFIG --atleast-pkgconfig-version 0.22],
      [pkgconfig_requires_private="Requires.private"],
   [pkgconfig_requires_private="Requires"]
)
AC_SUBST(pkgconfig_requires_private)

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])


### Checks for libraries

PKG_CHECK_MODULES([EEZE], [${requirement_eeze}])

udev_version=$(pkg-config libudev --modversion)

if test $udev_version -lt 143;then
  AC_MSG_ERROR([udev version is too old!])
elif test $udev_version -lt 148;then
  AC_MSG_WARN([Old udev version detected, enabling compat code])
  AC_DEFINE([OLD_UDEV_RRRRRRRRRRRRRR],[1],[compat functionality for udev < 148])
fi

eeze_mount=
PKG_CHECK_EXISTS([mount >= 2.18.0],
  [
     AC_DEFINE([HAVE_EEZE_MOUNT], [1], [Eeze is mount-capable])
     AM_CONDITIONAL([HAVE_EEZE_MOUNT], [true])
     eeze_mount="yes"
  ],
  AM_CONDITIONAL([HAVE_EEZE_MOUNT], [false])
)

if test "x$eeze_mount" = "xyes";then
  with_mount="detect"
  with_umount="detect"
  with_eject="detect"
  AC_ARG_WITH([mount], AS_HELP_STRING([--with-mount], [specify mount bin @<:@default=detect@:>@]))
  AC_ARG_WITH([umount], AS_HELP_STRING([--with-umount], [specify umount bin @<:@default=detect@:>@]))
  AC_ARG_WITH([eject], AS_HELP_STRING([--with-eject], [specify eject bin @<:@default=detect@:>@]))
  PKG_CHECK_MODULES([LIBMOUNT], [mount >= 2.18.0])
  mount_v=$(pkg-config --modversion mount)
  PKG_CHECK_MODULES([ECORE_FILE], [ecore-file >= 1.0.0])
  PKG_CHECK_MODULES([EET], [eet >= 1.4.0])
  PKG_CHECK_MODULES([ECORE_CON], [ecore-con >= 1.0.0])

  if test "x$with_mount" = "xdetect";then
    AC_PATH_PROG([with_mount], [mount], [no])
    test "x$with_mount" = "xno" && AC_MSG_ERROR([mount binary could not be detected])
  fi
  AC_DEFINE_UNQUOTED([EEZE_MOUNT_BIN], ["$with_mount"], [mount bin to use])

  if test "x$with_umount" = "xdetect";then
    AC_PATH_PROG([with_umount], [umount], [no])
    test "x$with_umount" = "xno" && AC_MSG_ERROR([umount binary could not be detected])
  fi
  AC_DEFINE_UNQUOTED([EEZE_UNMOUNT_BIN], ["$with_umount"], [umount bin to use])

  if test "x$with_eject" = "xdetect";then
    AC_PATH_PROG([with_eject], [eject], [no])
    test "x$with_eject" = "xno" && AC_MSG_ERROR([eject binary could not be detected])
  fi
  AC_DEFINE_UNQUOTED([EEZE_EJECT_BIN], ["$with_eject"], [eject bin to use])
fi
if test -n "$mount_v";then
  AM_CONDITIONAL([OLD_LIBMOUNT], [test "$(echo $mount_v | cut -d'.' -f2)" -lt 19])
else
  AM_CONDITIONAL([OLD_LIBMOUNT], [false])
fi
if test -z "$OLD_LIBMOUNT_TRUE" ; then
  AC_DEFINE_UNQUOTED([OLD_LIBMOUNT], [1], [using first version of libmount])
fi
### Checks for header files


### Checks for types


### Checks for structures


### Checks for compiler characteristics

AC_HEADER_STDC

if ! test "x${VMIC}" = "x" ; then
   EFL_COMPILER_FLAG([-Wall])
   EFL_COMPILER_FLAG([-W])
fi

EFL_COMPILER_FLAG([-Wshadow])


### Binary

EFL_ENABLE_BIN([eeze-udev-test], ["yes"])

AC_SUBST(requirement_eeze)


AC_OUTPUT([
Makefile
doc/eeze.dox
doc/Makefile
doc/Doxyfile
src/Makefile
src/lib/Makefile
src/bin/Makefile
eeze.pc
eeze.spec
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
if test "x$eeze_mount" = "xyes";then
  echo
  echo "Mount..................: ${with_mount}"
  echo "Umount.................: ${with_umount}"
  echo "Eject..................: ${with_eject}"
  echo
fi
echo "Test...................: ${have_eeze_udev_test}"
echo
echo "Documentation..........: ${build_doc}"
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo


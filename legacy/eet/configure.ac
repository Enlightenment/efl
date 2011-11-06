y##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_define([v_maj], [1])
m4_define([v_min], [4])
m4_define([v_mic], [999])
m4_define([v_rev], m4_esyscmd([(svnversion "${SVN_REPO_PATH:-.}" | grep -v export || echo 0) | awk -F : '{printf("%s\n", $1);}' | tr -d ' :MSP\n']))
m4_if(v_rev, [0], [m4_define([v_rev], m4_esyscmd([git log 2> /dev/null | (grep -m1 git-svn-id || echo 0) | sed -e 's/.*@\([0-9]*\).*/\1/' | tr -d '\n']))])
##--   When released, remove the dnl on the below line
dnl m4_undefine([v_rev])
##--   When doing snapshots - change soname. remove dnl on below line
dnl m4_define([relname], [ver-pre-svn-05])
dnl m4_define([v_rel], [-release relname])
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], [m4_define([v_ver], [v_maj.v_min.v_mic.v_rev])], [m4_define([v_ver], [v_maj.v_min.v_mic])])
m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], v_mic)
m4_define([lt_age], v_min)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##

AC_INIT([eet], [v_ver], [enlightenment-devel@lists.sourceforge.net])
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

### Default options with respect to host

want_gnutls="auto"
want_openssl="auto"
want_cipher="yes"
want_signature="yes"

requirement_eet=""


### Additional options to configure

# Amalgamation

AC_ARG_ENABLE([amalgamation],
   [AC_HELP_STRING([--enable-amalgamation], [enable generation of one single file with all source code in it, helps compiler optimizations.])],
   [if test "x${enableval}" = "xyes"; then
       do_amalgamation="yes"
    else
       do_amalgamation="no"
    fi
   ],
   [do_amalgamation="no"]
)
AM_CONDITIONAL(EET_AMALGAMATION, test "x${do_amalgamation}" = "xyes")

EFL_ENABLE_BIN([eet])

# Old eet file format support

old_eet_file_format="yes"
AC_ARG_ENABLE(old-eet-file-format,
   [AC_HELP_STRING(
      [--disable-old-eet-file-format],
      [disable old eet file format support. [[default=enabled]]]
    )],
   [old_eet_file_format=$enableval]
)
AC_MSG_CHECKING([whether to support old eet file format])
AC_MSG_RESULT([${old_eet_file_format}])

if test "x${old_eet_file_format}" = "xyes" ; then
   AC_DEFINE(EET_OLD_EET_FILE_FORMAT, 1, [support old eet file format])
else
   AC_DEFINE(EET_OLD_EET_FILE_FORMAT, 0, [support old eet file format])
fi

# Gnutls support

AC_ARG_ENABLE([gnutls],
   [AC_HELP_STRING([--disable-gnutls], [disable gnutls eet support])],
   [want_gnutls=$enableval]
)
AC_MSG_CHECKING([whether to use Gnutls])
AC_MSG_RESULT([${want_gnutls}])

# Specific GNUTLS improvement

new_gnutls_api="yes"
AC_ARG_ENABLE(new-gnutls-api,
   [AC_HELP_STRING(
      [--disable-new-gnutls-api],
      [enable use of gnutls_x509_crt_verify_hash. [[default=enable]]]
    )],
   [new_gnutls_api=$enableval]
)
AC_MSG_CHECKING([whether to use gnutls_x509_crt_verify_hash])
AC_MSG_RESULT([${new_gnutls_api}])

if test "x${new_gnutls_api}" = "xyes" ; then
   AC_CHECK_LIB(gnutls, gnutls_x509_crt_verify_hash,
                [ new_gnutls_api="yes" ],
		[ new_gnutls_api="no" ]
		)

   if test "x${new_gnutls_api}" = "xyes"; then
      AC_DEFINE(EET_USE_NEW_GNUTLS_API, 1, [use gnutls_x509_crt_verify_hash])
   fi
fi

# Openssl support

AC_ARG_ENABLE([openssl],
   [AC_HELP_STRING([--disable-openssl], [disable openssl eet support])],
   [want_openssl=$enableval]
)
AC_MSG_CHECKING([whether to use OpenSSL])
AC_MSG_RESULT([${want_openssl}])

# Cryptography support

AC_ARG_ENABLE([cipher],
   [AC_HELP_STRING([--disable-cipher], [disable cipher support for eet API])],
   [want_cipher=$enableval]
)
AC_MSG_CHECKING([whether to use cipher])
AC_MSG_RESULT([${want_cipher}])

AC_ARG_ENABLE([signature],
   [AC_HELP_STRING([--disable-signature], [disable signature file support for eet])],
   [want_signature=$enableval]
)
AC_MSG_CHECKING([whether to use signature])
AC_MSG_RESULT([${want_signature}])

# Assert or fail.
prefer_assert="no"
AC_ARG_ENABLE([assert],
   [AC_HELP_STRING([--enable-assert], [enable assert, [[default=disabled]]])],
   [prefer_assert=$enableval]
)

# Examples

AC_ARG_ENABLE([install-examples],
   [AC_HELP_STRING([--disable-install-examples],
       [disable installing examples (compiled or just source). @<:@default==enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       install_examples="yes"
    else
       install_examples="no"
    fi
   ],
   [install_examples="yes"])
AM_CONDITIONAL([INSTALL_EXAMPLES], [test "x${install_examples}" = "xyes"])

AC_ARG_ENABLE([build-examples],
   [AC_HELP_STRING([--enable-build-examples],
       [enable building examples. @<:@default==disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       build_examples="yes"
    else
       build_examples="no"
    fi
   ],
   [build_examples="no"])
AM_CONDITIONAL([BUILD_EXAMPLES], [test "x${build_examples}" = "xyes"])

# Unit tests, coverage and benchmarking

EFL_CHECK_TESTS([enable_tests="yes"], [enable_tests="no"])
EFL_CHECK_COVERAGE([${enable_tests}], [enable_coverage="yes"], [enable_coverage="no"])

if test "x${prefer_assert}" = "xno"; then
   DEBUG_CFLAGS="-DNDEBUG"
else
# use debug symbols and set DEBUG if coverage support is enabled
   DEBUG_CFLAGS="${EFL_DEBUG_CFLAGS}"
fi
AC_SUBST(DEBUG_CFLAGS)


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

# Evil library for compilation on Windows

EFL_EET_BUILD=""
case "$host_os" in
   mingw*)
      PKG_CHECK_MODULES([EVIL], [evil])
      AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if Evil library is installed])
      requirement_eet="evil ${requirement_eet}"
      EFL_EET_BUILD="-DEFL_EET_BUILD"
   ;;
esac
AC_SUBST(EFL_EET_BUILD)

EET_LIBS=""
case "$host_os" in
   mingw32ce*)
      EET_LIBS="-lws2"
      ;;
   mingw*)
      EET_LIBS="-lws2_32"
      ;;
esac
AC_SUBST(EET_LIBS)

# Eina library

PKG_CHECK_MODULES(EINA, [eina >= 1.0.0])
requirement_eet="eina >= 1.0.0 ${requirement_eet}"

# Gnutls library
have_gnutls="no"
if test "x${want_gnutls}" = "xyes" || test "x${want_gnutls}" = "xauto" ; then
   PKG_CHECK_MODULES(GNUTLS, [gnutls >= 1.7.6],
      [
       have_gnutls="yes"
       want_openssl="no"
       AC_DEFINE(HAVE_GNUTLS, 1, [Have Gnutls support])
       requirement_eet="gnutls ${requirement_eet}"
      ],
      [have_gnutls="no"])
   if test "x${have_gnutls}" = "xyes" ; then
      AC_PATH_GENERIC([libgcrypt], [], [have_gnutls="yes"], [have_gnutls="no"])
      if test "x${have_gnutls}" = "xyes" ; then
         GNUTLS_CFLAGS="${GNUTLS_CFLAGS} ${LIBGCRYPT_CFLAGS}"
         GNUTLS_LIBS="${GNUTLS_LIBS} ${LIBGCRYPT_LIBS}"
      fi
   fi
fi

# Openssl library
have_openssl="no"
if test "x${want_openssl}" = "xyes" || test "x${want_openssl}" = "xauto" ; then
   PKG_CHECK_MODULES(OPENSSL, openssl,
      [
       have_openssl="yes"
       AC_DEFINE(HAVE_OPENSSL, 1, [Have Openssl support])
       requirement_eet="openssl ${requirement_eet}"
      ],
      [have_openssl="no"])
fi

if test "x${have_gnutls}" = "xyes" ; then
   secure_layer="GnuTLS"
elif test "x${have_openssl}" = "xyes" ; then
   secure_layer="OpenSSL"
else
   secure_layer="no"
fi

have_cipher="no"
if test "x${have_gnutls}" = "xyes" && test "x${want_cipher}" = "xyes" ; then
   have_cipher="yes"
   AC_DEFINE(HAVE_CIPHER, 1, [Have cipher support built in eet])
elif test "x${have_openssl}" = "xyes" && test "x${want_cipher}" = "xyes" ; then
   have_cipher="yes"
   AC_DEFINE(HAVE_CIPHER, 1, [Have cipher support built in eet])
fi

AC_MSG_CHECKING(whether to activate cipher support in eet)
AC_MSG_RESULT(${have_cipher})

have_signature="no"
if test "x${have_gnutls}" = "xyes" && test "x${want_signature}" = "xyes" ; then
   have_signature="yes"
   AC_DEFINE(HAVE_SIGNATURE, 1, [Have signature support for eet file])
elif test "x${have_openssl}" = "xyes" && test "x${want_signature}" = "xyes" ; then
   have_signature="yes"
   AC_DEFINE(HAVE_SIGNATURE, 1, [Have signature support for eet file])
fi

AC_MSG_CHECKING(whether to activate signature support in eet)
AC_MSG_RESULT(${have_signature})

### Checks for header files

AC_CHECK_HEADER([zlib.h],
   [dummy="yes"],
   [AC_MSG_ERROR("Cannot find zlib.h. Make sure your CFLAGS environment variable contains include lines for the location of this file")])

AC_CHECK_HEADER([jpeglib.h],
   [dummy="yes"],
   [AC_MSG_ERROR("Cannot find jpeglib.h. Make sure your CFLAGS environment variable contains include lines for the location of this file")])

AC_CHECK_HEADERS(netinet/in.h unistd.h)
EFL_CHECK_PATH_MAX

### Checks for types


### Checks for structures


### Checks for compiler characteristics
AM_PROG_CC_C_O
AC_C_CONST
AC_C_INLINE
AC_PROG_CC_STDC
AC_C___ATTRIBUTE__

# Check whether the null pointer is zero on this arch
AC_TRY_RUN(
   [
#include <stdlib.h>
#include <inttypes.h>
int main (int argc, char **argv) {
  void *foo = NULL;
  uintptr_t bar = (uintptr_t)foo;
  return (uintptr_t)foo;
}
   ],
   [have_null="yes"],
   [have_null="no"],
   [
    AC_MSG_WARN([Cannot check when cross-compiling -- assuming null is okay])
    have_null="yes"
   ])
AC_MSG_CHECKING([value of the null pointer])
AC_MSG_RESULT([${have_null}])

if test ! "x${have_null}" = "xyes" ; then
   AC_MSG_WARN([Your system is a bit too funny, eet might not work properly])
fi

# These are needed for fmemopen/open_memstream
AC_DEFINE(_GNU_SOURCE, , [Enable GNU extensions])

EET_CPPFLAGS=""
EET_CFLAGS=""
case "$host_os" in
   mingw32ce*)
      EET_CPPFLAGS="-D_WIN32_WCE=0x0420"
      ;;
   mingw*)
      EET_CPPFLAGS="-D_WIN32_WINNT=0x0501"
      ;;
esac
AC_SUBST(EET_CPPFLAGS)
AC_SUBST(EET_CFLAGS)


### Checks for linker characteristics

# use --enable-auto-import on Windows

lt_enable_auto_import=""
case "$host_os" in
   mingw*)
      lt_enable_auto_import="-Wl,--enable-auto-import"
      ;;
esac
AC_SUBST(lt_enable_auto_import)

EFL_LINKER_FLAG([-Wl,--as-needed])

### Checks for library functions
AC_ISC_POSIX
AC_FUNC_ALLOCA

AC_CHECK_FUNCS(fmemopen open_memstream realpath)

EFL_CHECK_FNMATCH([], [AC_MSG_ERROR([Cannot find fnmatch()])])


AC_SUBST(requirement_eet)

AC_OUTPUT([
Makefile
eet.pc
doc/Makefile
doc/Doxyfile
doc/eet.dox
src/Makefile
src/lib/Makefile
src/bin/Makefile
src/tests/Makefile
src/examples/Makefile
eet.spec
])


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
echo "  Amalgamation.........: ${do_amalgamation}"
echo "  Secure layer.........: ${secure_layer}"
if test "x${have_gnutls}" = "xyes" || test "x${have_openssl}" = "xyes" ; then
   echo "    Cipher support.....: ${have_cipher}"
   echo "    Signature..........: ${have_signature}"
fi
echo
echo "  Old eet file format..: ${old_eet_file_format}"
echo
echo "  Tests................: ${enable_tests}"
echo "  Coverage.............: ${enable_coverage}"
echo
echo "  Build eet............: $have_eet"
echo
echo "  Documentation........: ${build_doc}"
if test "x${build_doc}" = "xyes" ; then
   echo "    Building...........: make doc"
fi
echo "  Examples.............: ${build_examples}"
if test "x${build_examples}" = "xyes" ; then
   echo "    Install............: ${install_examples}"
fi
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo

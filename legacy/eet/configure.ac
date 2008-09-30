# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT(eet, 1.1.0, enlightenment-devel@lists.sourceforge.net)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(configure.ac)
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_ISC_POSIX

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST
AM_PROG_CC_C_O

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

VMAJ=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $1);}'`
VMIN=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $2);}'`
VMIC=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $3);}'`
SNAP=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $4);}'`
version_info=`expr $VMAJ + $VMIN`":$VMIC:$VMIN"
AC_SUBST(version_info)

PKG_PROG_PKG_CONFIG

WIN32_CFLAGS=""
WIN32_LIBS=""
lt_no_undefined=""
lt_enable_auto_import=""
case "$host_os" in
	mingw*|cegcc)
		PKG_CHECK_MODULES([EVIL], [evil])
		AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if evil package is installed])
		dnl needed for correct definition of EAPI
		AC_DEFINE(EFL_EET_BUILD, 1, [Define to mention that eet is built])
		if test "$host_os" = "cegcc" ; then
			WIN32_CFLAGS="-mwin32"
			WIN32_LIBS="-lws2"
			lt_enable_auto_import="-Wl,--enable-auto-import"
		else
			WIN32_LIBS="-lws2_32"
		fi
		lt_no_undefined="-no-undefined"
		;;
esac
AC_SUBST(WIN32_CFLAGS)
AC_SUBST(WIN32_LIBS)
AC_SUBST(lt_no_undefined)
AC_SUBST(lt_enable_auto_import)

dnl Checking for __attribute__ support
AC_MSG_CHECKING([for __attribute__])
AC_CACHE_VAL(_cv_have___attribute__,
  [
    AC_TRY_COMPILE([#include <stdlib.h>],
      [int func(int x); int foo(int x __attribute__ ((unused))) { exit(1); }],
      [_cv_have___attribute__="yes"],
      [_cv_have___attribute__="no"])
  ]
)

if test "x${_cv_have___attribute__}" = "xyes" ; then
   AC_DEFINE(HAVE___ATTRIBUTE__, 1, [Define to 1 if your compiler has __attribute__])
fi
AC_MSG_RESULT(${_cv_have___attribute__})

AC_FUNC_ALLOCA

AC_CHECK_HEADER(zlib.h,, AC_MSG_ERROR("Cannot find zlib.h. Make sure your CFLAGS environment variable contains include lines for the location of this file"))
AC_CHECK_HEADER(jpeglib.h,, AC_MSG_ERROR("Cannot find jpeglib.h. Make sure your CFLAGS environment variable contains include lines for the location of this file"))

AC_CHECK_HEADERS(netinet/in.h)

AC_CHECK_HEADER(fnmatch.h,, AC_MSG_ERROR([Cannot find fnmatch.h. Make sure your CFLAGS environment variable contains include lines for the location of this file. MinGW users: see the INSTALL file]))

fnmatch_libs=""
AC_CHECK_FUNCS(fnmatch, res=yes, res=no)
if test "x$res" = "xno"; then
	AC_CHECK_LIB(fnmatch, fnmatch, res=yes fnmatch_libs="-lfnmatch", res=no)
dnl Test for compilation with MinGW.
dnl fnmatch function is in the libiberty library
	if test "x$res" = "xno"; then
		AC_CHECK_LIB(iberty, fnmatch, res=yes fnmatch_libs="-liberty", res=no)
	fi
	if test "x$res" = "xno"; then
		AC_MSG_ERROR([Cannot find fnmatch() in neither libc nor libfnmatch, nor libiberty])
	fi
fi

AC_SUBST(fnmatch_libs)

dnl These are needed for fmemopen/open_memstream
AC_DEFINE(_GNU_SOURCE, , [Enable GNU extensions])

AC_CHECK_FUNCS(fmemopen open_memstream realpath)

dnl Check whether the null pointer is zero on this arch
AC_MSG_CHECKING([value of the null pointer])
AC_TRY_RUN([
#include <stdlib.h>
int main (int argc, char **argv) {
  void *foo = NULL;
  int bar = (int)foo;
  return (int)foo;
}
], AC_MSG_RESULT([yes]), [
	AC_MSG_RESULT([no])
	AC_MSG_WARN([Your system is a bit too funny, eet might not work properly])
], AC_MSG_WARN([Cannot check when cross-compiling -- assuming null is okay])
)

dnl Unit Tests

AC_ARG_ENABLE(tests,
  [AC_HELP_STRING([--enable-tests], [Enable tests @<:@default=no@:>@])],
  [
   if test "x${enableval}" = "xyes" ; then
     enable_tests="yes"
   else
     enable_tests="no"
   fi
  ],
  [enable_tests="no"]
)
AC_MSG_CHECKING([if tests are built])
AC_MSG_RESULT([${enable_tests}])

if test "x${enable_tests}" = "xyes" ; then
  PKG_CHECK_MODULES([CHECK],
    [check >= 0.9.5],
    [dummy="yes"],
    [enable_tests="no"]
  )
fi

AM_CONDITIONAL(EET_ENABLE_TESTS, test "x${enable_tests}" = "xyes")

dnl Openssl support
want_openssl="auto"
have_openssl="no"
AC_ARG_ENABLE(openssl,
  [AC_HELP_STRING([--disable-openssl], [disable openssl eet support])],
  [ want_openssl=$enableval ]
)
if test "x$want_openssl" = "xyes" -o "x$want_openssl" = "xauto"; then
  PKG_CHECK_MODULES(OPENSSL, openssl,
    [
      have_openssl="yes"
      AC_DEFINE(HAVE_OPENSSL, 1, [Have Openssl support])
    ],
    [
      if test "x$use_strict" = "xyes"; then
        AC_MSG_ERROR([Openssl not found (strict dependencies checking)])
      fi
    ])
fi

dnl Crypto option
want_cypher="yes"
have_cypher="no"
want_signature="yes"
have_signature="no"

AC_MSG_CHECKING(whether to activate cypher support in eet)
AC_ARG_ENABLE(cypher,
  [AC_HELP_STRING([--disable-cypher], [disable cypher support for eet API])],
  [ want_cypher=$enableval ]
)
if test "x$have_openssl" = "xyes" -a "x$want_cypher" = "xyes"; then
  have_cypher="yes"
  AC_DEFINE(HAVE_CYPHER, 1, [Have cypher support built in eet])
fi
AC_MSG_RESULT($have_cypher)

AC_MSG_CHECKING(whether to activate signature support in eet)
AC_ARG_ENABLE(signature,
  [AC_HELP_STRING([--disable-signature], [disable signature file support for eet])],
  [ want_signature=$enableval ]
)
if test "x$have_openssl" = "xyes" -a "x$want_signature" = "xyes"; then
  have_signature="yes"
  AC_DEFINE(HAVE_SIGNATURE, 1, [Have signature support for eet file])
fi
AC_MSG_RESULT($have_signature)

dnl Coverage

AC_ARG_ENABLE(coverage,
  [AC_HELP_STRING([--enable-coverage],
    [compile with coverage profiling instrumentation @<:@default=no@:>@])],
  [
   if test "x${enableval}" = "xyes" ; then
     enable_coverage="yes"
   else
     enable_coverage="no"
   fi],
  [enable_coverage="no"]
)
AC_MSG_CHECKING([whether to use profiling instrumentation])
AC_MSG_RESULT($enable_coverage)

if test "x$enable_tests" = "xno" -a "x$enable_coverage" = "xyes"; then
  enable_coverage="no"
fi

if test "x$enable_coverage" = "xyes"; then
  AC_CHECK_PROG(have_lcov,
    [lcov],
    [yes],
    [no]
  )
  if test "x$have_lcov" = "xyes" ; then
    COVERAGE_CFLAGS="-fprofile-arcs -ftest-coverage"
    COVERAGE_LIBS="-lgcov"
dnl remove any optimisation flag and force debug symbols
    CFLAGS="-g -O0"
  else
    AC_MSG_WARN([lcov is not found, disable profiling instrumentation])
    enable_coverage="no"
  fi
fi
AC_SUBST(COVERAGE_CFLAGS)
AC_SUBST(COVERAGE_LIBS)

AM_CONDITIONAL(EET_ENABLE_COVERAGE, test "x$enable_coverage" = "xyes")

dnl Disable support for old eet file format.
old_eet_file_format="yes"
AC_ARG_ENABLE(old-eet-file-format,
  AC_HELP_STRING(
    [--disable-old-eet-file-format],
    [disable old eet file format support. [[default=enabled]]]
  ),
  [ old_eet_file_format=$enableval ]
)
AM_CONDITIONAL(EET_OLD_EET_FILE_FORMAT, test "x$old_eet_file_format" = "xyes")
if test "x$old_eet_file_format" = "xyes"; then
  AC_DEFINE(EET_OLD_EET_FILE_FORMAT, 1, [support old eet file format])
else
  AC_DEFINE(EET_OLD_EET_FILE_FORMAT, 0, [support old eet file format])
fi

dnl Assert or fail.
prefer_assert="no"
AC_ARG_ENABLE(assert,
  AC_HELP_STRING(
    [--enable-assert],
    [enable assert, [[default=disabled]]]
  ),
  [ prefer_assert=$enableval ]
)
AM_CONDITIONAL(EET_ASSERT, test "x$prefer_assert" = "xyes")
if test "x$prefer_assert" = "xno"; then
  cflags="$cflags -DNDEBUG"
fi

#AM_CHECK_DOXYGEN()

AC_OUTPUT([
Makefile
eet.pc
eet.c
src/Makefile
src/lib/Makefile
src/bin/Makefile
src/tests/Makefile
README
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
echo "  Openssl..............: ${have_openssl}"
echo "    Cypher support.....: ${have_cypher}"
echo "    Signature..........: ${have_signature}"

echo "  Tests................: ${enable_tests}"
echo "  Coverage.............: ${enable_coverage}"
echo
echo "  Old eet file format..: ${old_eet_file_format}"
echo
echo "  Compilation..........: make"
echo
echo "  Installation.........: make install"
echo
echo "    prefix.............: $prefix"
echo

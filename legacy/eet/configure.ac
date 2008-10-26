# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT([eet], [1.1.0], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.52])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_ISC_POSIX

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

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


### Default options with respect to host

case "$host_os" in
   mingw32ce* | cegcc*)
      want_openssl="no"
      want_cypher="no"
      want_signature="no"
      ;;
   *)
      want_openssl="auto"
      want_cypher="yes"
      want_signature="yes"
      ;;
esac

requirement_eet=""


### Additional options to configure

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

# Openssl support

AC_ARG_ENABLE([openssl],
   [AC_HELP_STRING([--disable-openssl], [disable openssl eet support])],
   [want_openssl=$enableval]
)
AC_MSG_CHECKING([whether to use OpenSSL])
AC_MSG_RESULT([${want_openssl}])

# Cryptography support

AC_ARG_ENABLE([cypher],
   [AC_HELP_STRING([--disable-cypher], [disable cypher support for eet API])],
   [want_cypher=$enableval]
)
AC_MSG_CHECKING([whether to use cypher])
AC_MSG_RESULT([${want_cypher}])

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


### Checks for libraries

PKG_PROG_PKG_CONFIG

# Evil library for compilation on Windows

EFL_EET_BUILD=""
case "$host_os" in
   mingw* | cegcc*)
   PKG_CHECK_MODULES([EVIL], [evil])
   AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if Evil library is installed])
   requirement_eet="evil ${requirement_eet}"
   EFL_EET_BUILD="-DEFL_EET_BUILD"
   ;;
esac
AC_SUBST(EFL_EET_BUILD)

# Eina library

PKG_CHECK_MODULES(EINA, [eina-0])
requirement_eet="eina-0 ${requirement_eet}"

# Openssl library
have_openssl="no"
if test "x${want_openssl}" = "xyes" -o "x${want_openssl}" = "xauto" ; then
   PKG_CHECK_MODULES(OPENSSL, openssl,
      [
       have_openssl="yes"
       AC_DEFINE(HAVE_OPENSSL, 1, [Have Openssl support])
       requirement_eet="openssl ${requirement_eet}"
      ])
fi

have_cypher="no"
if test "x${have_openssl}" = "xyes" -a "x${want_cypher}" = "xyes" ; then
   have_cypher="yes"
   AC_DEFINE(HAVE_CYPHER, 1, [Have cypher support built in eet])
fi

AC_MSG_CHECKING(whether to activate cypher support in eet)
AC_MSG_RESULT(${have_cypher})

have_signature="no"
if test "x${have_openssl}" = "xyes" -a "x${want_signature}" = "xyes" ; then
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

AC_CHECK_HEADERS(netinet/in.h)

AC_CHECK_HEADER([fnmatch.h],
   [dummy="yes"],
   [AC_MSG_ERROR([Cannot find fnmatch.h. Make sure your CFLAGS environment variable contains include lines for the location of this file. MinGW users: see the INSTALL file])])


### Checks for programs
AC_PROG_CC

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])


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
int main (int argc, char **argv) {
  void *foo = NULL;
  int bar = (int)foo;
  return (int)foo;
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
   cegcc*)
      EET_CPPFLAGS="-D_WIN32_WCE=0x0420"
      EET_CFLAGS="-mwin32"
      ;;
esac
AC_SUBST(EET_CPPFLAGS)
AC_SUBST(EET_CFLAGS)


### Checks for linker characteristics

EET_LIBS=""
case "$host_os" in
   mingw32ce*)
      EET_LIBS="-lws2"
      ;;
   cegcc*)
      EET_LIBS="-lws2"
      ;;
   mingw*)
      EET_LIBS="-lws2_32"
      ;;
esac
AC_SUBST(EET_LIBS)

lt_enable_auto_import=""
case "$host_os" in
   mingw* | cegcc*)
      lt_enable_auto_import="-Wl,--enable-auto-import"
      ;;
esac
AC_SUBST(lt_enable_auto_import)


### Checks for library functions
AC_FUNC_ALLOCA

AC_CHECK_FUNCS(fmemopen open_memstream realpath)

fnmatch_libs=""
AC_CHECK_FUNCS([fnmatch], [res="yes"], [res="no"])
if test "x$res" = "xno"; then
   AC_SEARCH_LIBS([fnmatch],
      [fnmatch iberty evil],
      [res="yes"],
      [res="no"])
   if test "x$res" = "xno"; then
      AC_MSG_ERROR([Cannot find fnmatch() in neither libc nor libfnmatch, nor libiberty, nor libevil])
   fi
   fnmatch_libs="${ac_cv_search_fnmatch}"
fi
AC_SUBST(fnmatch_libs)


AC_SUBST(requirement_eet)

AC_OUTPUT([
Makefile
eet.pc
doc/Makefile
doc/eet.dox
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
echo
echo "  Documentation........: ${enable_doc}"
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

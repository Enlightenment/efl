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
dnl m4_define([v_rel], [-release relname])
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], [m4_define([v_ver], [v_maj.v_min.v_mic.v_rev])], [m4_define([v_ver], [v_maj.v_min.v_mic])])
m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], v_mic)
m4_define([lt_age], v_min)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##

AC_INIT([eina], [v_ver], [enlightenment-devel@lists.sourceforge.net])
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

requirement_eina=""

case "$host_os" in
   mingw32ce*)
      MODULE_ARCH="$host_os-$host_cpu"
      ;;
   *)
      MODULE_ARCH="$host_os-$host_cpu-v_maj.v_min.v_mic"
      ;;
esac
AC_SUBST(MODULE_ARCH)
AC_DEFINE_UNQUOTED(MODULE_ARCH, "${MODULE_ARCH}", "Module architecture")

case "$host_os" in
   mingw*)
      MODULE_EXT=".dll"
      ;;
   *)
      MODULE_EXT=".so"
      ;;
esac

AC_DEFINE_UNQUOTED(SHARED_LIB_SUFFIX, "${MODULE_EXT}", [Suffix for shared objects])

EFL_CHECK_CPU_MMX([have_mmx="yes"], [have_mmx="no"])
EFL_CHECK_CPU_SSE([have_sse="yes"], [have_sse="no"])
EFL_CHECK_CPU_SSE2([have_sse2="yes"], [have_sse2="no"])
EFL_CHECK_CPU_ALTIVEC([have_altivec="yes"], [have_altivec="no"])

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
EFL_CHECK_SPINLOCK([have_posix_threads_spinlock="yes"], [have_posix_threads_spinlock="no"])

if ! test "x${have_threads}" = "xno" ; then
   EINA_CONFIGURE_HAVE_THREADS="#define EINA_HAVE_THREADS"
fi
AC_SUBST(EINA_CONFIGURE_HAVE_THREADS)
AM_CONDITIONAL([EINA_HAVE_THREADS], [! test "x${have_threads}" = "xno"])

if ! test "x${have_debug_threads}" = "xno"; then
   EINA_CONFIGURE_HAVE_DEBUG_THREADS="#define EINA_HAVE_DEBUG_THREADS"
fi
AC_SUBST(EINA_CONFIGURE_HAVE_DEBUG_THREADS)
AM_CONDITIONAL([EINA_DEBUG_THREADS], [! test "x${have_debug_threads}" = "xno"])

if ! test "x${have_on_off_threads}" = "xno"; then
   EINA_CONFIGURE_HAVE_ON_OFF_THREADS="#define EINA_HAVE_ON_OFF_THREADS"
fi
AC_SUBST(EINA_CONFIGURE_HAVE_ON_OFF_THREADS)
AM_CONDITIONAL([EINA_ON_OFF_THREADS], [! test "x${have_on_off_threads}" = "xno"])

### Additional options to configure

# Magic debug
AC_ARG_ENABLE([magic-debug],
   [AC_HELP_STRING([--disable-magic-debug], [disable magic debug of eina structure @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_magic_debug="yes"
    else
       have_magic_debug="no"
    fi
   ],
   [have_magic_debug="yes"])

AC_MSG_CHECKING([whether magic debug is enable])
AC_MSG_RESULT([${have_magic_debug}])

if test "x${have_magic_debug}" = "xyes" ; then
   EINA_CONFIGURE_MAGIC_DEBUG="#define EINA_MAGIC_DEBUG"
fi
AC_SUBST(EINA_CONFIGURE_MAGIC_DEBUG)

# Safety checks (avoid crashes on wrong api usage)
AC_ARG_ENABLE(safety-checks,
   [AC_HELP_STRING([--disable-safety-checks], [disable safety checks for NULL pointers and like. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_safety_checks="yes"
    else
       have_safety_checks="no"
    fi
   ],
   [have_safety_checks="yes"])

AC_MSG_CHECKING(whether to do safety checking on api parameters)
AC_MSG_RESULT($have_safety_checks)

AM_CONDITIONAL(SAFETY_CHECKS, test "x$have_safety_checks" = "xyes")
if test "x$have_safety_checks" = "xyes"; then
  AC_DEFINE(EINA_SAFETY_CHECKS, 1, [disable safety checks for NULL pointers and like.])
  EINA_CONFIGURE_SAFETY_CHECKS="#define EINA_SAFETY_CHECKS"
fi
AC_SUBST(EINA_CONFIGURE_SAFETY_CHECKS)

with_max_log_level="<unset>"
AC_ARG_WITH(internal-maximum-log-level,
   [AC_HELP_STRING([--with-internal-maximum-log-level=NUMBER],
                   [limit eina internal log level to the given number, any call to EINA_LOG() with values greater than this will be compiled out, ignoring runtime settings, but saving function calls.])],
   [
    if test "x${withval}" != "xno"; then
       if echo "${withval}" | grep '^[[0-9]]\+$' >/dev/null 2>/dev/null; then
          AC_MSG_NOTICE([ignoring any EINA_LOG() with level greater than ${withval}])
          AC_DEFINE_UNQUOTED(EINA_LOG_LEVEL_MAXIMUM, ${withval}, [if set, logging is limited to this amount.])
          with_max_log_level="${withval}"
       else
          AC_MSG_ERROR([--with-internal-maximum-log-level takes a decimal number, got "${withval}" instead.])
       fi
    fi
    ], [:])

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
AM_CONDITIONAL(EINA_AMALGAMATION, test "x${do_amalgamation}" = "xyes")

# Choose best memory pool
AC_ARG_ENABLE([default-mempool],
   [AC_HELP_STRING([--enable-default-mempool], [Default memory allocator could be faster for some computer. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes"; then
       have_default_mempool="yes"
    else
       have_default_mempool="no"
    fi
   ],
   [have_default_mempool="no"]
)
AC_MSG_CHECKING([whether to use default mempool allocator])
AC_MSG_RESULT([${have_default_mempool}])

if test "x${have_default_mempool}" = "xyes" ; then
   EINA_CONFIGURE_DEFAULT_MEMPOOL="#define EINA_DEFAULT_MEMPOOL"
fi
AC_SUBST(EINA_CONFIGURE_DEFAULT_MEMPOOL)

# Report stringshare usage
AC_ARG_ENABLE([stringshare-usage],
   [AC_HELP_STRING([--enable-stringshare-usage], [Report stringshare usage on stringshare shutdown. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes"; then
       have_stringshare_usage="yes"
    else
       have_stringshare_usage="no"
    fi
   ],
   [have_stringshare_usage="no"]
)
AC_MSG_CHECKING([whether to report stringshare usage])
AC_MSG_RESULT([${have_stringshare_usage}])

if test "x${have_stringshare_usage}" = "xyes"; then
   AC_DEFINE(EINA_STRINGSHARE_USAGE, 1, [Report Eina stringshare usage pattern])
fi

# Check if we want to benchmark on real data
enable_benchmark_e17="no"
AC_ARG_ENABLE([e17],
   [AC_HELP_STRING([--enable-e17], [enable heavy benchmark @<:@default=no@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_benchmark_e17="yes"
    else
       enable_benchmark_e17="no"
    fi
   ],
   [enable_benchmark_e17="no"])

AC_MSG_CHECKING([whether e17 real data benchmark are built])
AC_MSG_RESULT([${enable_benchmark_e17}])

AM_CONDITIONAL(EINA_ENABLE_BENCHMARK_E17, test "x${enable_benchmark_e17}" = "xyes")


### Checks for programs
AC_PROG_CC
AC_PROG_CXX

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

# Valgrind
want_valgrind="auto"
have_valgrind="no"

AC_ARG_ENABLE([valgrind],
  [AC_HELP_STRING([--enable-valgrind], [improve valgrind support by hinting it of our memory usages, having it to report proper mempool leaks.])],
  [want_valgrind=$enableval])

AC_MSG_CHECKING([whether to enable build with valgrind])
AC_MSG_RESULT([${want_valgrind}])

if test "x${want_valgrind}" = "xyes" || test "x${want_valgrind}" = "xauto"; then
   PKG_CHECK_MODULES([VALGRIND],
      [valgrind >= 2.4.0],
      [
       requirement_eina="valgrind ${requirement_eina}"
       have_valgrind="yes"
      ],
      [
       AC_DEFINE([NVALGRIND], [1], [Valgrind support disabled])
       if test "x${want_valgrind}" = "xyes"; then
          AC_MSG_ERROR([Valgrind >= 2.4.0 is required])
       fi
      ])
else
    AC_DEFINE([NVALGRIND], [1], [Valgrind support disabled])
fi


# Evil library for compilation on Windows CE

EFL_EINA_BUILD=""
case "$host_os" in
   mingw*)
      PKG_CHECK_MODULES([EVIL], [evil])
      AC_DEFINE([HAVE_EVIL], [1], [Set to 1 if evil package is installed])
      requirement_eina="evil"
      EFL_EINA_BUILD="-DEFL_EINA_BUILD"
   ;;
esac
AC_SUBST([EFL_EINA_BUILD])

# Check ememoa memory pool library

AC_ARG_ENABLE([ememoa],
   [AC_HELP_STRING([--enable-ememoa], [build ememoa memory pool module @<:@default=yes@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_ememoa="yes"
    else
       enable_ememoa="no"
    fi
   ],
   [enable_ememoa="yes"])

AC_MSG_CHECKING([whether to use ememoa for memory pool])
AC_MSG_RESULT([$enable_ememoa])

if test "x${enable_ememoa}" = "xyes" ; then
   PKG_CHECK_MODULES([EMEMOA],
      [ememoa >= 0.0.26 ],
      [enable_ememoa="yes"],
      [enable_ememoa="no"])
fi

PKG_CHECK_MODULES([ECORE_EVAS],
   [ecore-evas ecore evas],
   [build_tiler_example="yes"],
   [build_tiler_example="no"])

AM_CONDITIONAL([BUILD_TILER_EXAMPLE], [test "x${build_tiler_example}" = "xyes"])


### Checks for header files
AC_HEADER_ASSERT
AC_HEADER_DIRENT
AC_HEADER_TIME
EFL_CHECK_PATH_MAX
AC_CHECK_HEADERS([unistd.h libgen.h inttypes.h stdint.h])

if test "x${ac_cv_header_inttypes_h}" = "xyes" ; then
   EINA_CONFIGURE_HAVE_INTTYPES_H="#define EINA_HAVE_INTTYPES_H"
   AC_DEFINE([HAVE_INTTYPES_H], [1], [Define to 1 if you have the <inttypes.h> header file.])
fi
AC_SUBST([EINA_CONFIGURE_HAVE_INTTYPES_H])

if test "x${ac_cv_header_inttypes_h}" = "xyes" ; then
    EINA_CONFIGURE_HAVE_STDINT_H="#define EINA_HAVE_STDINT_H"
    AC_DEFINE([HAVE_STDINT_H], [1], [Define to 1 if you have the <stdint.h> header file.])
fi
AC_SUBST([EINA_CONFIGURE_HAVE_STDINT_H])

### Checks for types

AC_CHECK_SIZEOF([wchar_t])
EINA_SIZEOF_WCHAR_T=$ac_cv_sizeof_wchar_t
AC_SUBST([EINA_SIZEOF_WCHAR_T])

### Checks for structures


### Checks for compiler characteristics
AC_C_CONST
AC_C_BIGENDIAN
AC_C_INLINE
AC_C___ATTRIBUTE__
AC_PROG_CC_STDC

have_wince="no"
have_win32="no"
EINA_CPPFLAGS=""
EINA_CFLAGS=""
case "$host_os" in
   mingw32ce*)
      EINA_CPPFLAGS="-D_WIN32_WCE=0x0420"
      EINA_CFLAGS="${EVIL_CFLAGS}"
      have_win32="yes"
      have_wince="yes"
      ;;
   mingw*)
      EINA_CPPFLAGS="-D_WIN32_WINNT=0x0501"
      EINA_CFLAGS="${EVIL_CFLAGS}"
      have_win32="yes"
      ;;
esac
AM_CONDITIONAL([EINA_HAVE_WINCE], [test "x$have_wince" = "xyes"])
AM_CONDITIONAL([EINA_HAVE_WIN32], [test "x$have_win32" = "xyes"])

m4_ifdef([v_mic],
   [
    EFL_COMPILER_FLAG([-Wall])
    EFL_COMPILER_FLAG([-W])
   ])

EFL_COMPILER_FLAG([-Wshadow])

AC_SUBST([EINA_CPPFLAGS])
AC_SUBST([EINA_CFLAGS])


### Checks for linker characteristics
EINA_LIBS=
lt_enable_auto_import=""
case "${host_os}" in
   mingw*)
      EINA_LIBS="-ldl ${EVIL_LIBS} -lm"
      lt_enable_auto_import="-Wl,--enable-auto-import"
   ;;
   dragonfly*|openbsd*)
      EINA_LIBS="-lm"
   ;;
   freebsd*|netbsd*)
      EINA_LIBS="-lrt -lm"
   ;;
   darwin*)
      EINA_LIBS="-lm"
   ;;
   cygwin*)
      EINA_LIBS="-ldl -lm"
   ;;
   *)
      EINA_LIBS="-ldl -lrt -lm"
   ;;
esac
AC_SUBST([EINA_LIBS])
AC_SUBST([lt_enable_auto_import])

EFL_LINKER_FLAG([-fno-strict-aliasing])


### Checks for library functions
AC_ISC_POSIX
AC_FUNC_ALLOCA
AC_CHECK_FUNCS([strlcpy dirfd openat fstatat fpathconf])

# dlopen and dladdr
dlopen_libs=""
case "$host_os" in
   mingw*)
# managed by evil
      AC_DEFINE([HAVE_DLADDR])
      AC_DEFINE([HAVE_DLOPEN])
      ;;
   *)
      AC_CHECK_FUNCS([dlopen], [res="yes"], [res="no"])
      if test "x${res}" = "xyes" ; then
         AC_CHECK_FUNCS([dladdr], [AC_DEFINE([HAVE_DLADDR])])
         AC_DEFINE([HAVE_DLOPEN])
      else
         AC_CHECK_LIB([dl], [dlopen], [res="yes"], [res="no"])
         if test "x${res}" = "xyes" ; then
            AC_CHECK_LIB([dl], [dladdr], [AC_DEFINE([HAVE_DLADDR])])
            AC_DEFINE(HAVE_DLOPEN)
            dlopen_libs="-ldl"
         fi
      fi
      ;;
esac
AC_SUBST([dlopen_libs])

EFL_CHECK_FNMATCH([], [AC_MSG_ERROR([Cannot find fnmatch()])])

# iconv library
have_iconv="no"
AC_ARG_WITH([iconv-link],
   AC_HELP_STRING([--with-iconv-link=ICONV_LINK], [explicitly specify an iconv link option]),
   [
    LIBS="$withval $LIBS"
    have_iconv="yes"
    iconv_libs=$withval
   ])

AC_MSG_CHECKING(for explicit iconv link options)
if test "x${iconv_libs}" = "x" ; then
   AC_MSG_RESULT([no explicit iconv link option])
else
   AC_MSG_RESULT([$iconv_libs])
fi

if test "x${have_iconv}" = "xno" ; then
   AC_CHECK_HEADERS([iconv.h], [have_iconv="yes"])

   if test "x${have_iconv}" = "xyes" ; then
      AC_MSG_CHECKING([whether iconv() is in libc])

      AC_LINK_IFELSE(
         [AC_LANG_PROGRAM(
             [[
#include <stdlib.h>
#include <iconv.h>
             ]],
             [[
iconv_t ic;
size_t count = iconv(ic, NULL, NULL, NULL, NULL);
             ]])],
         [have_iconv="yes"],
         [have_iconv="no"])

      AC_MSG_RESULT([${have_iconv}])
   fi

   if test "x${have_iconv}" = "xno" ; then
      AC_MSG_CHECKING([whether iconv() is in libiconv.a])

      LIBS_save="${LIBS}"
      LIBS="-liconv $LIBS"
      AC_LINK_IFELSE(
         [AC_LANG_PROGRAM(
             [[
#include <stdlib.h>
#include <iconv.h>
             ]],
             [[
iconv_t ic;
size_t count;
count = iconv(ic, NULL, NULL, NULL, NULL);
             ]])],
         [
          have_iconv="yes"
          iconv_libs="-liconv"
         ],
         [
          have_iconv="no"
          LIBS=${LIBS_save}
         ])

      AC_MSG_RESULT([${have_iconv}])
   fi

   if test "x${have_iconv}" = "xno" ; then
      AC_MSG_CHECKING([whether iconv() is in libiconv_plug.a])

      LIBS_save="${LIBS}"
      LIBS="-liconv_plug $LIBS"
      AC_LINK_IFELSE(
         [AC_LANG_PROGRAM(
             [[
#include <stdlib.h>
#include <iconv.h>
             ]],
             [[
iconv_t ic;
size_t count = iconv(ic, NULL, NULL, NULL, NULL);
             ]])],
         [
          have_iconv="yes"
          iconv_libs="-liconv_plug"
         ],
         [
          have_iconv="no"
          LIBS=${LIBS_save}
         ])

      AC_MSG_RESULT([${have_iconv}])
   fi
fi

AC_SUBST([iconv_libs])

if test "x${have_iconv}" = "xyes" ; then
   AC_DEFINE([HAVE_ICONV], [1], [Set to 1 if iconv library is installed])
fi

# shmopen
have_shmopen=no
AC_MSG_CHECKING([for shm_open availability])

LIBS_save="${LIBS}"
LIBS="${LIBS} -lrt"
AC_LINK_IFELSE(
   [AC_LANG_PROGRAM(
       [[
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
       ]],
       [[
int fd;

fd = shm_open("/dev/null", O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
       ]])],
   [have_shmopen="yes"],
   [have_shmopen="no"])

LIBS="${LIBS_save}"

AC_MSG_RESULT([${have_shmopen}])

if test "x${have_shmopen}" = "xyes"; then
   AC_DEFINE([HAVE_SHMOPEN], [1], [Define to 1 if you have the `shm_open' function.])
   rt_libs="-lrt"
fi

AC_SUBST([rt_libs])

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

### Modules

if test "x${have_default_mempool}" = "xyes" ; then
   enable_chained_pool="no"
   enable_pass_through="static"
else
   enable_chained_pool="static"
   enable_pass_through="no"
fi

enable_one_big="static"

EINA_CHECK_MODULE([chained-pool],   [${enable_chained_pool}], [chained pool])
EINA_CHECK_MODULE([ememoa-fixed],   [${enable_ememoa}],       [ememoa fixed])
EINA_CHECK_MODULE([ememoa-unknown], [${enable_ememoa}],       [ememoa unknown])
EINA_CHECK_MODULE([fixed-bitmap],   [no],                     [fixed bitmap])
EINA_CHECK_MODULE([pass-through],   [${enable_pass_through}], [pass through])
EINA_CHECK_MODULE([buddy],          [no],                     [buddy])
EINA_CHECK_MODULE([one-big],        [${enable_one_big}],      [one big])

### Make the debug preprocessor configurable

### Unit tests, coverage and benchmarking

EFL_CHECK_TESTS([enable_tests="yes"], [enable_tests="no"])

EFL_CHECK_COVERAGE([${enable_tests}], [enable_coverage="yes"], [enable_coverage="no"])
EINA_CFLAGS="${EINA_CFLAGS} ${EFL_COVERAGE_CFLAGS}"
EINA_LIBS="${EINA_LIBS} ${EFL_COVERAGE_LIBS}"
if test "x$enable_coverage" = "xyes" ; then
   EINA_CFLAGS="${EINA_CFLAGS} ${EFL_DEBUG_CFLAGS}"
fi

EFL_CHECK_BENCHMARK([enable_benchmark="yes"], [enable_benchmark="no"])
EINA_BENCH_MODULE([glib], [${enable_benchmark}], [glib-2.0], [enable_benchmark_glib="yes"], [enable_benchmark_glib="no"])
if test -n "$CXX" && test "x$enable_benchmark" = "xyes" ; then
   AC_DEFINE_UNQUOTED([CITYHASH_BENCH], [1], [enable bench tests for google hash method])
fi
AM_CONDITIONAL([CITYHASH_BENCH], [test -n "$CXX" && test "x$enable_benchmark" = "xyes"])

AC_SUBST(requirement_eina)

### Build and install examples
EFL_CHECK_BUILD_EXAMPLES([enable_build_examples="yes"], [enable_build_examples="no"])
EFL_CHECK_INSTALL_EXAMPLES([enable_install_examples="yes"], [enable_install_examples="no"])

AC_CONFIG_FILES([
Makefile
eina.pc
eina.spec
doc/Makefile
doc/Doxyfile
src/Makefile
src/include/Makefile
src/include/eina_config.h
src/lib/Makefile
src/modules/Makefile
src/modules/mp/Makefile
src/modules/mp/chained_pool/Makefile
src/modules/mp/ememoa_fixed/Makefile
src/modules/mp/ememoa_unknown/Makefile
src/modules/mp/pass_through/Makefile
src/modules/mp/fixed_bitmap/Makefile
src/modules/mp/buddy/Makefile
src/modules/mp/one_big/Makefile
src/tests/Makefile
src/examples/Makefile
])

AC_OUTPUT

#####################################################################
## Info

echo
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE $VERSION"
echo "------------------------------------------------------------------------"
echo
echo
echo "Configuration Options Summary:"
echo
echo "  Magic debug..........: ${have_magic_debug}"
echo "  Safety checks........: ${have_safety_checks}"
echo "  Maximum log level....: ${with_max_log_level}"
echo "  Report string usage..: ${have_stringshare_usage}"
echo "  Valgrind support.....: ${have_valgrind}"
echo "  Default mempool......: ${have_default_mempool}"
echo "  Thread Support.......: ${have_threads}"
if test "${have_threads}" = "POSIX" ; then
echo "    spinlock...........: ${have_posix_threads_spinlock}"
echo "    debug usage........: ${have_debug_threads}"
echo "    on/off support.....: ${have_on_off_threads}"
fi
echo "  Amalgamation.........: ${do_amalgamation}"
echo "  Iconv support........: ${have_iconv}"
echo "  File dirfd...........: ${have_dirfd}"
echo "  File xattr...........: ${have_xattr}"
echo
echo "  Documentation........: ${build_doc}"
echo "  Tests................: ${enable_tests}"
echo "  Coverage.............: ${enable_coverage}"
echo "  Examples.............: ${enable_build_examples}"
echo "  Tiler Example........: ${build_tiler_example}"
echo "  Examples installed...: ${enable_install_examples}"
echo "  Benchmark............: ${enable_benchmark}"
if test "x${enable_benchmark}" = "xyes" ; then
echo "    Glib...............: ${enable_benchmark_glib}"
echo "    E17 real data......: ${enable_benchmark_e17}"
fi
echo
echo "  CPU Specific Extensions:"
echo "    MMX................: ${have_mmx}"
echo "    SSE................: ${have_sse}"
echo "    SSE2...............: ${have_sse2}"
echo "    ALTIVEC............: ${have_altivec}"
echo
echo "  Memory pools:"
echo "    Buddy..............: ${enable_buddy}"
echo "    Chained pool.......: ${enable_chained_pool}"
echo "    Ememoa fixed.......: ${enable_ememoa_fixed}"
echo "    Ememoa unknown.....: ${enable_ememoa_unknown}"
echo "    Fixed bitmap.......: ${enable_fixed_bitmap}"
echo "    One big............: ${enable_one_big}"
echo "    Pass through.......: ${enable_pass_through}"
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo
EFL_FORM_VOLTRON

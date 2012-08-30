##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_define([v_maj], [1])
m4_define([v_min], [7])
m4_define([v_mic], [99])
m4_define([v_rev], m4_esyscmd([(svnversion "${SVN_REPO_PATH:-.}" | grep -v '\(export\|Unversioned directory\)' || echo 0) | awk -F : '{printf("%s\n", $1);}' | tr -d ' :MSP\n']))
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

#lt_cv_deplibs_check_method='pass_all'
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

requirements_pc_eina=""

have_wince="no"
have_win32="no"
case "$host_os" in
   mingw32ce*)
      have_win32="yes"
      have_wince="yes"
      MODULE_ARCH="$host_os-$host_cpu"
      MODULE_EXT=".dll"
      ;;
   mingw*)
      have_win32="yes"
      MODULE_ARCH="$host_os-$host_cpu-v_maj.v_min.v_mic"
      MODULE_EXT=".dll"
      ;;
   *)
      MODULE_ARCH="$host_os-$host_cpu-v_maj.v_min.v_mic"
      MODULE_EXT=".so"
      ;;
esac
AM_CONDITIONAL([EINA_HAVE_WINCE], [test "x$have_wince" = "xyes"])
AM_CONDITIONAL([EINA_HAVE_WIN32], [test "x$have_win32" = "xyes"])
AC_SUBST([MODULE_ARCH])
AC_DEFINE_UNQUOTED([MODULE_ARCH], ["${MODULE_ARCH}"], ["Module architecture"])
AC_DEFINE_UNQUOTED([SHARED_LIB_SUFFIX], ["${MODULE_EXT}"], [Suffix for shared objects])

EFL_CHECK_CPU_MMX([have_mmx="yes"], [have_mmx="no"])
EFL_CHECK_CPU_SSE([have_sse="yes"], [have_sse="no"])
EFL_CHECK_CPU_SSE2([have_sse2="yes"], [have_sse2="no"])
EFL_CHECK_CPU_ALTIVEC([have_altivec="yes"], [have_altivec="no"])

EFL_CHECK_THREADS

if ! test "x${efl_have_threads}" = "xno" ; then
   EINA_CONFIGURE_HAVE_THREADS="#define EINA_HAVE_THREADS"
fi
AC_SUBST(EINA_CONFIGURE_HAVE_THREADS)
AM_CONDITIONAL([EINA_HAVE_THREADS], [! test "x${efl_have_threads}" = "xno"])

if test "x${efl_have_debug_threads}" = "xyes"; then
   EINA_CONFIGURE_HAVE_DEBUG_THREADS="#define EINA_HAVE_DEBUG_THREADS"
fi
AC_SUBST(EINA_CONFIGURE_HAVE_DEBUG_THREADS)
AM_CONDITIONAL([EINA_DEBUG_THREADS], [test "x${efl_have_debug_threads}" = "xyes"])

if ! test "x${efl_have_on_off_threads}" = "xno"; then
   EINA_CONFIGURE_HAVE_ON_OFF_THREADS="#define EINA_HAVE_ON_OFF_THREADS"
fi
AC_SUBST(EINA_CONFIGURE_HAVE_ON_OFF_THREADS)
AM_CONDITIONAL([EINA_ON_OFF_THREADS], [! test "x${efl_have_on_off_threads}" = "xno"])

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
       if echo "${withval}" | grep -E '^[[0-9]]+$' >/dev/null 2>/dev/null; then
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

# Assert or fail.

AC_ARG_ENABLE([assert],
   [AC_HELP_STRING([--enable-assert], [enable assert, @<:@default=no@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       prefer_assert="yes"
    else
       prefer_assert="no"
    fi
   ],
   [prefer_assert="no"])

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
#for src/tests...
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

## Compatibility layers

# Evil library for compilation on Windows

EFL_EINA_BUILD=""
case "$host_os" in
   mingw*)
      PKG_CHECK_EXISTS([evil >= 1.6.99])
      AC_DEFINE([HAVE_EVIL], [1], [Set to 1 if Evil package is installed])
      requirements_pc_eina="${requirements_pc_eina} evil"
      EFL_EINA_BUILD="-DEFL_EINA_BUILD"
   ;;
esac
AC_SUBST([EFL_EINA_BUILD])

# Escape library for compilation on Playstation 3

case "$host_vendor" in
  ps3*)
      PKG_CHECK_EXISTS([escape])
      AC_DEFINE([HAVE_ESCAPE], [1], [Set to 1 if Escape package is installed])
      requirements_pc_eina="${requirements_pc_eina} escape"
    ;;
esac

# Exotic library for copilation on Coyote

PKG_CHECK_EXISTS([exotic],
   [
    enable_exotic="yes"
    AC_DEFINE([HAVE_EXOTIC_H], [1], [Define to 1 if you have Exotic.])
    EINA_CONFIGURE_HAVE_EXOTIC="#define EINA_HAVE_EXOTIC"
    requirements_pc_eina="exotic ${requirements_pc_eina}"
   ],
   [enable_exotic="no"])

AM_CONDITIONAL([EINA_HAVE_EXOTIC], [test "x${enable_exotic}" = "xyes"])
AC_SUBST([EINA_CONFIGURE_HAVE_EXOTIC])

if ! test "x${requirements_pc_eina}" = "x" ; then
   PKG_CHECK_MODULES([EINA], [${requirements_pc_eina}])
fi

## Options

# Valgrind

AC_ARG_ENABLE([valgrind],
   [AC_HELP_STRING([--enable-valgrind], [improve valgrind support by hinting it of our memory usages, having it to report proper mempool leaks.])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_valgrind="yes"
    else
       want_valgrind="no"
    fi
   ],
   [want_valgrind="auto"])

AC_MSG_CHECKING([whether to enable build with valgrind])
AC_MSG_RESULT([${want_valgrind}])

if test "x${want_valgrind}" = "xyes" || test "x${want_valgrind}" = "xauto"; then
   PKG_CHECK_MODULES([VALGRIND], [valgrind >= 2.4.0],
      [
       have_valgrind="yes"
       requirements_pc_eina="valgrind ${requirements_pc_eina}"
      ],
      [
       have_valgrind="no"
       AC_DEFINE([NVALGRIND], [1], [Valgrind support disabled])
       if test "x${want_valgrind}" = "xyes"; then
          AC_MSG_ERROR([Valgrind >= 2.4.0 is required])
       fi
      ])
else
    AC_DEFINE([NVALGRIND], [1], [Valgrind support disabled])
fi

## Modules

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
AC_MSG_RESULT([${enable_ememoa}])

if test "x${enable_ememoa}" = "xyes" ; then
   PKG_CHECK_MODULES([EMEMOA],
      [ememoa >= 0.0.26 ],
      [enable_ememoa="yes"],
      [enable_ememoa="no"])
fi

## Examples

PKG_CHECK_MODULES([ECORE_EVAS],
   [ecore-evas ecore evas],
   [build_tiler_example="yes"],
   [build_tiler_example="no"])

AM_CONDITIONAL([BUILD_TILER_EXAMPLE], [test "x${build_tiler_example}" = "xyes"])

### Checks for header files
AC_HEADER_ASSERT
AC_HEADER_TIME
AC_HEADER_DIRENT

AC_CHECK_HEADERS([unistd.h libgen.h inttypes.h stdint.h sys/types.h siginfo.h strings.h execinfo.h mcheck.h])

# sys/mman.h could be provided by evil/escape/exotic so we need to set CFLAGS accordingly
CFLAGS_save="${CFLAGS}"
CFLAGS="${CFLAGS} ${EINA_CFLAGS}"
AC_CHECK_HEADERS([sys/mman.h])
CFLAGS="${CFLAGS_save}"

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

# wchar_t

AC_CHECK_SIZEOF([wchar_t])
EINA_SIZEOF_WCHAR_T=$ac_cv_sizeof_wchar_t
AC_SUBST([EINA_SIZEOF_WCHAR_T])
AC_CHECK_TYPES([siginfo_t], [], [],
   [[
#include <signal.h>
#if HAVE_SIGINFO_H
# include <siginfo.h>
#endif
   ]])

# struct dirent

AC_CHECK_TYPES([struct dirent], [have_dirent="yes"], [have_dirent="no"],
   [[
#include <dirent.h>
   ]])

if test "x${have_dirent}" = "xyes" ; then
    EINA_CONFIGURE_HAVE_DIRENT_H="#define EINA_HAVE_DIRENT_H"
    AC_DEFINE([HAVE_DIRENT_H], [1], [Define to 1 if you have a valid <dirent.h> header file.])
fi
AC_SUBST([EINA_CONFIGURE_HAVE_DIRENT_H])

### Checks for structures

### Checks for compiler characteristics
AC_C_BIGENDIAN
AC_C_INLINE
AC_PROG_CC_STDC
EFL_ATTRIBUTE_UNUSED

m4_ifdef([v_mic],
   [
    EFL_COMPILER_FLAG([-Wall])
    EFL_COMPILER_FLAG([-W])
   ])

EFL_COMPILER_FLAG([-Wshadow])

EFL_CHECK_PATH_MAX

### Checks for linker characteristics
lt_enable_auto_import=""
case "${host_os}" in
   mingw*)
      lt_enable_auto_import="-Wl,--enable-auto-import"
   ;;
esac
AC_SUBST([lt_enable_auto_import])

EFL_LINKER_FLAG([-fno-strict-aliasing])

### Checks for library functions
AC_FUNC_ALLOCA

AC_CHECK_FUNCS([strlcpy openat fstatat fpathconf execvp backtrace backtrace_symbols malloc_usable_size mtrace])

EFL_CHECK_FUNCS([eina], [dirfd dlopen dladdr fnmatch iconv shm_open setxattr])

want_debug_malloc="no"
AC_ARG_ENABLE([debug-malloc],
   [AC_HELP_STRING([--enable-debug-malloc], [enable debugging of malloc usage overhead in our allocator @<:@default=enabled@:>@])],
   [
     if test "x${enableval}" = "xyes" ; then
     	want_debug_malloc="yes"
     else
	want_debug_malloc="no"
     fi
   ], [want_debug_malloc="no"])

if test "x${ac_cv_func_malloc_usable_size}" = "xyes" && test "x${want_debug_malloc}" = "xyes"; then
   AC_DEFINE([EINA_DEBUG_MALLOC], [1], [Turn on debugging overhead in mempool])
fi

AC_ARG_ENABLE([log],
   [AC_HELP_STRING([--disable-log], [disable Eina_Log infrastructure completly @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_log="yes"
    else
       want_log="no"
    fi
   ],
   [want_log="yes"])

enable_log="no"
if test "x${efl_func_fnmatch}" = "xyes" && test "x${want_log}" = "xyes" ; then
   enable_log="yes"
fi

AC_MSG_CHECKING([wether to build Eina_Log infrastructure])
AC_MSG_RESULT([${enable_log}])

if test "x${enable_log}" = "xyes"; then
    EINA_CONFIGURE_ENABLE_LOG="#define EINA_ENABLE_LOG"
    AC_DEFINE([HAVE_LOG], [1], [Define to 1 if we log support is on])
fi
AC_SUBST([EINA_CONFIGURE_ENABLE_LOG])


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

EFL_CHECK_TESTS([eina], [enable_tests="yes"], [enable_tests="no"])

EFL_CHECK_BENCHMARK([enable_benchmark="yes"], [enable_benchmark="no"])
EINA_BENCH_MODULE([glib], [${enable_benchmark}], [glib-2.0], [enable_benchmark_glib="yes"], [enable_benchmark_glib="no"])
if test -n "$CXX" && test "x$enable_benchmark" = "xyes" ; then
   AC_DEFINE_UNQUOTED([CITYHASH_BENCH], [1], [enable bench tests for google hash method])
fi
AM_CONDITIONAL([CITYHASH_BENCH], [test -n "$CXX" && test "x$enable_benchmark" = "xyes"])

AC_SUBST([requirements_pc_eina])
AC_SUBST([requirements_libs_eina])

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
echo "  Log support..........: ${enable_log}"
echo "  Thread Support.......: ${efl_have_threads}"
if test "${efl_have_threads}" = "POSIX" ; then
echo "    spinlock...........: ${efl_have_posix_threads_spinlock}"
echo "    debug usage........: ${efl_have_debug_threads}"
echo "    on/off support.....: ${efl_have_on_off_threads}"
fi
echo "  Amalgamation.........: ${do_amalgamation}"
echo "  Iconv support........: ${efl_func_iconv}"
echo "  File dirfd...........: ${efl_func_dirfd}"
echo "  File xattr...........: ${efl_func_setxattr}"
echo "  shm_open.............: ${efl_func_shm_open}"
echo
echo "  Documentation........: ${build_doc}"
echo "  Tests................: ${enable_tests} (Coverage: ${efl_enable_coverage})"
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

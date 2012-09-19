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
      MODULE_ARCH="$host_os-$host_cpu"
      MODULE_EXT=".dll"
   ;;
   mingw*)
      have_win32="yes"
      have_windows="yes"
      want_eeze="no"
      MODULE_ARCH="$host_os-$host_cpu-v_maj.v_min.v_mic"
      MODULE_EXT=".dll"
   ;;
   *)
      MODULE_ARCH="$host_os-$host_cpu-v_maj.v_min.v_mic"
      MODULE_EXT=".so"
   ;;
esac

AC_DEFINE_UNQUOTED([MODULE_ARCH], ["${MODULE_ARCH}"], ["Module architecture"])
AC_DEFINE_UNQUOTED([SHARED_LIB_SUFFIX], ["${MODULE_EXT}"], [Suffix for shared objects])

AM_CONDITIONAL([HAVE_WINCE], [test "x${have_wince}" = "xyes"])
AM_CONDITIONAL([HAVE_WIN32], [test "x${have_win32}" = "xyes"])
AM_CONDITIONAL([HAVE_WINDOWS], [test "x${have_windows}" = "xyes"])


#### Additional options to configure

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
if test "x${PKG_CONFIG}" = "x" ; then
   AC_MSG_ERROR([pkg-config tool not found. Install it or set PKG_CONFIG environment variable to that path tool. Exiting...])
fi

# Check whether pkg-config supports Requires.private
if ${PKG_CONFIG} --atleast-pkgconfig-version 0.22 ; then
   pkgconfig_requires_private="Requires.private"
else
   pkgconfig_requires_private="Requires"
fi
AC_SUBST([pkgconfig_requires_private])

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])


#### Checks for libraries


#### Checks for header files

EFL_CHECK_PATH_MAX


#### Checks for types


#### Checks for structures


#### Checks for compiler characteristics

AC_C_BIGENDIAN
AC_C_INLINE
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

AC_FUNC_ALLOCA


######################  EFL  ######################

#### Evil

if test "x${have_windows}" = "xyes" ; then

AC_MSG_NOTICE([Evil checks])

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
    EVIL_LIBS="-luuid -lole32 -lws2_32 -lsecur32"
    EVIL_DLFCN_LIBS="-lpsapi"
    ;;
esac
requirements_libs_evil="${EVIL_LIBS}"

AC_SUBST([EVIL_LIBS])
AC_SUBST([EVIL_DLFCN_LIBS])

AC_DEFINE([HAVE_EVIL], [1], [Set to 1 if Evil package is installed])

### Checks for header files

AC_CHECK_HEADERS([errno.h])

### Checks for types

### Checks for structures

### Checks for compiler characteristics

EVIL_CPPFLAGS="-DEFL_EVIL_BUILD"
EVIL_DLFCN_CPPFLAGS="-DEFL_EVIL_DLFCN_BUILD -DPSAPI_VERSION=1"
EVIL_CFLAGS="-Wall -Wextra -Wshadow -Wdeclaration-after-statement -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls -mms-bitfields"
EVIL_CXXFLAGS=""

if test "x${have_win32}" = "xyes" ; then
   EVIL_CXXFLAGS="-fno-rtti -fno-exceptions"
   EVIL_CPPFLAGS="${EVIL_CPPFLAGS} -DSECURITY_WIN32"
fi

AC_SUBST([EVIL_CPPFLAGS])
AC_SUBST([EVIL_DLFCN_CPPFLAGS])
AC_SUBST([EVIL_CFLAGS])
AC_SUBST([EVIL_CXXFLAGS])

### Checks for linker characteristics

### Checks for library functions

fi

#### End of Evil


#### Eina

AC_MSG_NOTICE([Eina checks])

### Default values

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
AC_SUBST([EINA_CONFIGURE_MAGIC_DEBUG])

# Safety checks (avoid crashes on wrong api usage)
AC_ARG_ENABLE([safety-checks],
   [AC_HELP_STRING([--disable-safety-checks], [disable safety checks for NULL pointers and like. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_safety_checks="yes"
    else
       have_safety_checks="no"
    fi
   ],
   [have_safety_checks="yes"])

AC_MSG_CHECKING([whether to do safety checking on api parameters])
AC_MSG_RESULT([${have_safety_checks}])

if test "x${have_safety_checks}" = "xyes" ; then
   AC_DEFINE([EINA_SAFETY_CHECKS], [1], [disable safety checks for NULL pointers and like.])
   EINA_CONFIGURE_SAFETY_CHECKS="#define EINA_SAFETY_CHECKS"
fi
AC_SUBST([EINA_CONFIGURE_SAFETY_CHECKS])

AM_CONDITIONAL([SAFETY_CHECKS], [test "x$have_safety_checks" = "xyes"])

# Miximum log level
with_max_log_level="<unset>"
AC_ARG_WITH([internal-maximum-log-level],
   [AC_HELP_STRING([--with-internal-maximum-log-level=NUMBER],
                   [limit eina internal log level to the given number, any call to EINA_LOG() with values greater than this will be compiled out, ignoring runtime settings, but saving function calls.])],
   [
    if test "x${withval}" != "xno" ; then
       if echo "${withval}" | grep -E '^[[0-9]]+$' >/dev/null 2>/dev/null; then
          AC_MSG_NOTICE([ignoring any EINA_LOG() with level greater than ${withval}])
          AC_DEFINE_UNQUOTED([EINA_LOG_LEVEL_MAXIMUM], [${withval}], [if set, logging is limited to this amount.])
          with_max_log_level="${withval}"
       else
          AC_MSG_ERROR([--with-internal-maximum-log-level takes a decimal number, got "${withval}" instead.])
       fi
    fi
    ],
    [:])

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
   [have_default_mempool="no"])

AC_MSG_CHECKING([whether to use default mempool allocator])
AC_MSG_RESULT([${have_default_mempool}])

if test "x${have_default_mempool}" = "xyes" ; then
   EINA_CONFIGURE_DEFAULT_MEMPOOL="#define EINA_DEFAULT_MEMPOOL"
fi
AC_SUBST([EINA_CONFIGURE_DEFAULT_MEMPOOL])

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
   AC_DEFINE([EINA_STRINGSHARE_USAGE], [1], [Report Eina stringshare usage pattern])
fi

# Check if we want to benchmark on real data
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

AM_CONDITIONAL([EINA_ENABLE_BENCHMARK_E17], [test "x${enable_benchmark_e17}" = "xyes"])

### Checks for programs

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

# Exotic library for compilation on Coyote

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

AC_ARG_ENABLE([debug-malloc],
   [AC_HELP_STRING([--enable-debug-malloc], [enable debugging of malloc usage overhead in our allocator @<:@default=enabled@:>@])],
   [
     if test "x${enableval}" = "xyes" ; then
     	want_debug_malloc="yes"
     else
	want_debug_malloc="no"
     fi
   ],
   [want_debug_malloc="no"])

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

## Tests

EFL_CHECK_TESTS(EINA)

## Benchmarks

PKG_CHECK_MODULES([GLIB],
   [glib-2.0],
   [have_glib="yes"],
   [have_glib="no"])

if test "x${have_glib}" = "xyes" ; then
   GLIB_CFLAGS="${GLIB_CFLAGS} -DEINA_BENCH_HAVE_GLIB"
fi

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

m4_ifdef([v_ver],
   [
    EFL_CHECK_COMPILER_FLAGS([EINA], [-Wall -Wextra])
   ])

EFL_CHECK_COMPILER_FLAGS([EINA], [-Wshadow])

### Checks for linker characteristics

EFL_CHECK_LINKER_FLAGS([EINA], [-fno-strict-aliasing])

### Checks for library functions

AC_CHECK_FUNCS([strlcpy openat fstatat fpathconf execvp backtrace backtrace_symbols malloc_usable_size mtrace])

EFL_CHECK_FUNCS([EINA], [dirfd dlopen dladdr fnmatch iconv shm_open setxattr])

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


#### End of Eina


#### Eet

AC_MSG_NOTICE([Eet checks])

### Default values

### Additional options to configure

EFL_ENABLE_BIN([eet])

# Old eet file format support

AC_ARG_ENABLE(old-eet-file-format,
   [AC_HELP_STRING(
      [--disable-old-eet-file-format],
      [disable old eet file format support @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       old_eet_file_format="yes"
    else
       old_eet_file_format="no"
    fi
   ],
   [old_eet_file_format="yes"])

AC_MSG_CHECKING([whether to support old eet file format])
AC_MSG_RESULT([${old_eet_file_format}])

if test "x${old_eet_file_format}" = "xyes" ; then
   AC_DEFINE(EET_OLD_EET_FILE_FORMAT, 1, [support old eet file format])
else
   AC_DEFINE(EET_OLD_EET_FILE_FORMAT, 0, [support old eet file format])
fi

# Openssl support

AC_ARG_ENABLE([openssl],
   [AC_HELP_STRING([--disable-openssl], [disable openssl eet support @<:@default=auto@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_openssl="yes"
    else
       want_openssl="no"
    fi
   ],
   [want_openssl="auto"])

AC_MSG_CHECKING([whether to use OpenSSL])
AC_MSG_RESULT([${want_openssl}])

# GnuTLS support

AC_ARG_ENABLE([gnutls],
   [AC_HELP_STRING([--disable-gnutls], [disable gnutls eet support @<:@default=auto@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_gnutls="yes"
    else
       want_gnutls="no"
    fi
   ],
   [want_gnutls="auto"])

AC_MSG_CHECKING([whether to use Gnutls])
AC_MSG_RESULT([${want_gnutls}])

# Cryptography support

AC_ARG_ENABLE([cipher],
   [AC_HELP_STRING([--disable-cipher], [disable cipher support for eet API @<:@default=yes@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_cipher="yes"
    else
       want_cipher="no"
    fi
   ],
   [want_cipher="yes"])

AC_MSG_CHECKING([whether to use cipher])
AC_MSG_RESULT([${want_cipher}])

AC_ARG_ENABLE([signature],
   [AC_HELP_STRING([--disable-signature], [disable signature file support for eet @<:@default=yes@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_signature="yes"
    else
       want_signature="no"
    fi
   ],
   [want_signature="yes"])

AC_MSG_CHECKING([whether to use signature])
AC_MSG_RESULT([${want_signature}])

### Checks for programs

### Checks for libraries

## Compatibility layers

# Evil library for compilation on Windows

EFL_EET_BUILD=""
case "$host_os" in
   mingw*)
      PKG_CHECK_EXISTS([evil >= 1.1.0])
      AC_DEFINE([HAVE_EVIL], [1], [Set to 1 if Evil package is installed])
      requirements_pc_eet="evil ${requirements_pc_eet}"
      EFL_EET_BUILD="-DEFL_EET_BUILD"
   ;;
esac
AC_SUBST(EFL_EET_BUILD)

# Exotic library for compilation on Coyote

PKG_CHECK_EXISTS([exotic],
   [
    enable_exotic="yes"
    AC_DEFINE([HAVE_EXOTIC_H], [1], [Define to 1 if you have Exotic.])
    requirements_pc_eet="exotic ${requirements_pc_eet}"
   ],
   [enable_exotic="no"])

## Secure layer

# Gnutls library

have_gnutls="no"
if test "x${want_gnutls}" = "xyes" || test "x${want_gnutls}" = "xauto" ; then
   PKG_CHECK_MODULES([GNUTLS], [gnutls >= 1.7.6],
      [
       have_gnutls="yes"
       want_openssl="no"
       AC_DEFINE([HAVE_GNUTLS], [1], [Have Gnutls support])
       requirements_pc_eet="gnutls >= 1.7.6 ${requirements_pc_eet}"
      ],
      [have_gnutls="no"])
fi

# libgcrypt

if test "x${have_gnutls}" = "xyes" ; then
   AC_PATH_GENERIC([libgcrypt], [], [have_gnutls="yes"], [have_gnutls="no"])
   if test "x${have_gnutls}" = "xyes" ; then
      requirements_libs_eet="${LIBGCRYPT_LIBS} ${requirements_libs_eet}"
   fi
fi

# Specific GNUTLS improvement

AC_ARG_ENABLE(new-gnutls-api,
   [AC_HELP_STRING(
      [--disable-new-gnutls-api],
      [enable use of gnutls_x509_crt_verify_hash. @<:@default=yes@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       new_gnutls_api="yes"
    else
       new_gnutls_api="no"
    fi
   ],
   [new_gnutls_api="yes"])

AC_MSG_CHECKING([whether to use gnutls_x509_crt_verify_hash])
AC_MSG_RESULT([${new_gnutls_api}])

if test "x${have_gnutls}" = "xyes" && test "x${new_gnutls_api}" = "xyes" ; then
   CFLAGS_save="${CFLAGS}"
   LIBS_save="${LIBS}"
   CFLAGS="${GNUTLS_CFLAGS}"
   LIBS="${GNUTLS_LIBS}"
   AC_CHECK_LIB([gnutls], [gnutls_x509_crt_verify_hash],
      [
       AC_DEFINE([EET_USE_NEW_GNUTLS_API], [1], [use gnutls_x509_crt_verify_hash])
       new_gnutls_api="yes"
      ],
      [new_gnutls_api="no"])
   CFLAGS="${CFLAGS_save}"
   LIBS="${LIBS_save}"
fi

use_gnutls_privkey_sign_data="no"
if test "x${have_gnutls}" = "xyes" ; then
   CFLAGS_save="${CFLAGS}"
   LIBS_save="${LIBS}"
   CFLAGS="${GNUTLS_CFLAGS}"
   LIBS="${GNUTLS_LIBS}"
   AC_CHECK_LIB([gnutls], [gnutls_privkey_sign_data],
      [
       AC_DEFINE([EET_USE_NEW_PRIVKEY_SIGN_DATA], [1], [use gnutls_privkey_sign_data])
       use_gnutls_privkey_sign_data="yes"
      ],
      [use_gnutls_privkey_sign_data="no"])
   CFLAGS="${CFLAGS_save}"
   LIBS="${LIBS_save}"
fi

AC_MSG_CHECKING([whether to use gnutls_privkey_sign_data])
AC_MSG_RESULT([${use_gnutls_privkey_sign_data}])

use_gnutls_pubkey_verify_hash="no"
if test "x${have_gnutls}" = "xyes" ; then
   CFLAGS_save="${CFLAGS}"
   LIBS_save="${LIBS}"
   CFLAGS="${GNUTLS_CFLAGS}"
   LIBS="${GNUTLS_LIBS}"
   AC_CHECK_LIB([gnutls], [gnutls_pubkey_verify_hash],
      [
       AC_DEFINE([EET_USE_NEW_PUBKEY_VERIFY_HASH], [1], [use gnutls_pubkey_verify_hash])
       use_gnutls_pubkey_verify_hash="yes"
      ],
      [use_gnutls_pubkey_verify_hash="no"])
   CFLAGS="${CFLAGS_save}"
   LIBS="${LIBS_save}"
fi

AC_MSG_CHECKING([whether to use gnutls_pubkey_verify_hash])
AC_MSG_RESULT([${use_gnutls_pubkey_verify_hash}])

# Openssl library

have_openssl="no"
if test "x${want_openssl}" = "xyes" || test "x${want_openssl}" = "xauto" ; then
   PKG_CHECK_EXISTS([openssl],
      [
       have_openssl="yes"
       AC_DEFINE([HAVE_OPENSSL], [1], [Have Openssl support])
       requirements_pc_eet="openssl ${requirements_pc_eet}"
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

# Cryptography support

have_cipher="no"
if test "x${have_gnutls}" = "xyes" && test "x${want_cipher}" = "xyes" ; then
   have_cipher="yes"
   AC_DEFINE([HAVE_CIPHER], [1], [Have cipher support built in eet])
elif test "x${have_openssl}" = "xyes" && test "x${want_cipher}" = "xyes" ; then
   have_cipher="yes"
   AC_DEFINE([HAVE_CIPHER], [1], [Have cipher support built in eet])
fi

AC_MSG_CHECKING([whether to activate cipher support in eet])
AC_MSG_RESULT([${have_cipher}])

have_signature="no"
if test "x${have_gnutls}" = "xyes" && test "x${want_signature}" = "xyes" ; then
   have_signature="yes"
   AC_DEFINE([HAVE_SIGNATURE], [1], [Have signature support for eet file])
elif test "x${have_openssl}" = "xyes" && test "x${want_signature}" = "xyes" ; then
   have_signature="yes"
   AC_DEFINE([HAVE_SIGNATURE], [1], [Have signature support for eet file])
fi

AC_MSG_CHECKING([whether to activate signature support in eet])
AC_MSG_RESULT([${have_signature}])

# libjpeg and zlib

EFL_CHECK_LIBS([eet], [libjpeg zlib])

# Eina library

requirements_pc_eet="eina >= 1.2.0 ${requirements_pc_eet}"
PKG_CHECK_MODULES([EET], [${requirements_pc_eet}])

case "$host_os" in
   mingw32ce*)
      requirements_libs_eet="${requirements_libs_eet} -lws2"
      ;;
   mingw*)
      requirements_libs_eet="${requirements_libs_eet} -lws2_32"
      ;;
esac

EET_LIBS="${EET_LIBS} ${requirements_libs_eet}"

### Checks for header files

AC_CHECK_HEADERS(netinet/in.h unistd.h)

### Checks for types

### Checks for structures

### Checks for compiler characteristics

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

### Checks for linker characteristics

case "${host_os}" in
   openbsd*)
   ;;
   *)
      EFL_CHECK_LINKER_FLAGS([EET], [-Wl,--as-needed])
   ;;
esac

### Checks for library functions

#### End of Eet


#### Evas

#AC_MSG_NOTICE([Evas checks])

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

#### End of Evas


AC_CONFIG_FILES([
Makefile
doc/Makefile
doc/Doxyfile
src/Makefile
src/benchmarks/Makefile
src/benchmarks/eina/Makefile
src/bin/Makefile
src/bin/evil/Makefile
src/bin/eet/Makefile
src/examples/Makefile
src/examples/eina/Makefile
src/examples/eet/Makefile
src/include/Makefile
src/include/eina/Makefile
src/include/eina/eina_config.h
src/lib/Makefile
src/lib/evil/Makefile
src/lib/eina/Makefile
src/lib/eet/Makefile
src/modules/Makefile
src/modules/eina/Makefile
src/modules/eina/mp/Makefile
src/modules/eina/mp/chained_pool/Makefile
src/modules/eina/mp/ememoa_fixed/Makefile
src/modules/eina/mp/ememoa_unknown/Makefile
src/modules/eina/mp/pass_through/Makefile
src/modules/eina/mp/fixed_bitmap/Makefile
src/modules/eina/mp/buddy/Makefile
src/modules/eina/mp/one_big/Makefile
src/scripts/Makefile
src/scripts/eina/Makefile
src/tests/Makefile
src/tests/eina/Makefile
src/tests/eet/Makefile
eina.spec
eet.spec
evil.pc
eina.pc
eet.pc
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
echo "Eina"
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
echo "  Iconv support........: ${efl_func_iconv}"
echo "  File dirfd...........: ${efl_func_dirfd}"
echo "  File xattr...........: ${efl_func_setxattr}"
echo "  shm_open.............: ${efl_func_shm_open}"
echo
echo "  Tests................: make check (Coverage: ${_efl_enable_coverage})"
echo "  Examples.............: make examples"
echo "    Tiler Example......: ${build_tiler_example}"
echo "    installation.......: make install-examples"
echo "  Benchmark............: make benchmark"
if test "x${enable_benchmark}" = "xyes" ; then
echo "    Glib...............: ${enable_benchmark_glib}"
echo "    E17 real data......: ${enable_benchmark_e17}"
fi
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
echo "Eet"
echo
echo "  Secure layer.........: ${secure_layer}"
if test "x${have_gnutls}" = "xyes" || test "x${have_openssl}" = "xyes" ; then
   echo "    Cipher support.....: ${have_cipher}"
   echo "    Signature..........: ${have_signature}"
fi
echo
echo "  Old eet file format..: ${old_eet_file_format}"
echo
echo "  Tests................: make check (Coverage: ${_efl_enable_coverage})"
echo "  Examples.............: make examples"
echo "    installation.......: make install-examples"
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


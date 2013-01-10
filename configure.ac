m4_define([v_maj], [1])
m4_define([v_min], [7])
m4_define([v_mic], [99])

m4_define([v_rev], m4_esyscmd([(svnversion "${SVN_REPO_PATH:-.}" | grep -v '\(export\|Unversioned directory\)' || echo 0) | awk -F : '{printf("%s\n", $1);}' | tr -d ' :MSP\n']))
m4_if(v_rev, [0], [m4_define([v_rev], m4_esyscmd([git log 2> /dev/null | (grep -m1 git-svn-id || echo 0) | sed -e 's/.*@\([0-9]*\).*/\1/' | tr -d '\n']))])

#### FIXME: i am sure that we can do some m4 to automagically do the stuff below for release and snapshots

##--   When released, remove the dnl on the below line
dnl m4_undefine([v_rev])

m4_define([v_rel], [])
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

AC_USE_SYSTEM_EXTENSIONS

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

m4_ifdef([v_rev], , [m4_define([v_rev], [0])])
AC_DEFINE_UNQUOTED([VMAJ], [v_maj], [Major version])
AC_DEFINE_UNQUOTED([VMIN], [v_min], [Minor version])
AC_DEFINE_UNQUOTED([VMIC], [v_mic], [Micro version])
AC_DEFINE_UNQUOTED([VREV], [v_rev], [Revison])
VMAJ=v_maj
VMIN=v_min
AC_SUBST([VMAJ])
AC_SUBST([VMIN])


#### Additional options to configure

AC_ARG_WITH([profile],
   [AC_HELP_STRING([--with-profile=PROFILE],
                   [use the predefined build profile, one of: dev, debug and release.
                    @<:@default=dev@:>@])],
   [build_profile=${withval}],
   [build_profile=dev])

case "${build_profile}" in
   dev|debug|release)
     ;;
   *)
     AC_MSG_ERROR([Unknown build profile --with-profile=${build_profile}])
     ;;
esac

prefer_assert="no"
case "${build_profile}" in
   dev|debug)
     prefer_assert="yes"
     ;;
esac

# TODO: add some build "profile" (server, full, etc...)

AC_ARG_WITH([crypto],
   [AC_HELP_STRING([--with-crypto=CRYPTO],
                   [use the predefined build crypto, one of:
                    openssl, gnutls or none.
                    @<:@default=openssl@:>@])],
   [build_crypto=${withval}],
   [build_crypto=openssl])

case "${build_crypto}" in
   openssl|gnutls|none)
     ;;
   *)
     AC_MSG_ERROR([Unknown build crypto option: --with-crypto=${build_crypto}])
     ;;
esac

AC_ARG_WITH([tests],
   [AC_HELP_STRING([--with-tests=none|regular|coverage],
                   [choose testing method: regular, coverage or none.
                    @<:@default=none@:>@])],
   [build_tests=${withval}],
   [build_tests=auto])

want_coverage="no"
want_tests="no"
case "${build_tests}" in
   auto)
     if test "${build_profile}" = "dev"; then
        want_tests="yes"
     fi
     ;;
   regular)
     want_tests="yes"
     ;;
   coverage)
     want_tests="yes"
     want_coverage="yes"
     ;;
   none)
     ;;
   *)
     AC_MSG_ERROR([Unknown build tests option: --with-tests=${build_tests}])
     ;;
esac

dbusservicedir="${datadir}/dbus-1/services"
AC_ARG_WITH([dbus-services],
   [AC_HELP_STRING([--with-dbus-services=DBUS_SERVICES],
      [specify a directory to store dbus service files.])],
   [dbusservicedir=$withval])
AC_SUBST(dbusservicedir)


#### Default values

platform_pc=""
platform_pc_evil=""
platform_pc_escape=""
platform_pc_exotic=""

platform_libs=""
platform_libs_evil=""
platform_libs_escape=""
platform_libs_exotic=""

platform_cflags=""
platform_cflags_evil=""
platform_cflags_escape=""
platform_cflags_exotic=""

requirements_pc_efl=""
requirements_pc_deps_efl=""
requirements_libs_efl=""
requirements_cflags_efl=""

requirements_pc_crypto=""
requirements_pc_deps_crypto=""
requirements_libs_crypto=""
requirements_cflags_crypto=""

AC_CANONICAL_HOST

# TODO: move me to m4 file that setups module/so related variables
case "$host_os" in
   cegcc*)
      AC_MSG_ERROR([ceGCC compiler is not supported anymore. Exiting...])
   ;;
   mingw32ce*)
      MODULE_ARCH="$host_os-$host_cpu"
      MODULE_EXT=".dll"
   ;;
   mingw*)
      have_win32="yes"
      have_windows="yes"
      MODULE_ARCH="$host_os-$host_cpu-v_maj.v_min.v_mic"
      MODULE_EXT=".dll"
   ;;
   cygwin*)
      MODULE_ARCH="$host_os-$host_cpu-v_maj.v_min.v_mic"
      MODULE_EXT=".dll"
   ;;
   *)
      MODULE_ARCH="$host_os-$host_cpu-v_maj.v_min.v_mic"
      MODULE_EXT=".so"
   ;;
esac

have_wince="no"
have_win32="no"
have_windows="no"
have_darwin="no"
have_linux="no"
have_ps3="no"
case "$host_os" in
   mingw32ce*)
      have_wince="yes"
      have_windows="yes"
   ;;
   mingw*|cygwin*)
      # TODO: check cygwin* here
      have_win32="yes"
      have_windows="yes"
   ;;
   darwin*)
      have_darwin="yes"
   ;;
   linux*)
      have_linux="yes"
   ;;
esac

case "$host_vendor" in
   ps3*)
      have_ps3="yes"
   ;;
esac

AC_SUBST([MODULE_ARCH])
AC_DEFINE_UNQUOTED([MODULE_ARCH], ["${MODULE_ARCH}"], ["Module architecture"])
AC_DEFINE_UNQUOTED([SHARED_LIB_SUFFIX], ["${MODULE_EXT}"], [Suffix for shared objects])

# TODO: move me to m4 file that setups the windows related variables
AM_CONDITIONAL([HAVE_WINCE], [test "x${have_wince}" = "xyes"])
AM_CONDITIONAL([HAVE_WIN32], [test "x${have_win32}" = "xyes"])
AM_CONDITIONAL([HAVE_WINDOWS], [test "x${have_windows}" = "xyes"])
AM_CONDITIONAL([HAVE_PS3], [test "x${have_ps3}" = "xyes"])


#### Checks for programs

### libtool

if test "x${have_windows}" = "xyes" ; then
   lt_cv_deplibs_check_method='pass_all'
fi
LT_INIT([win32-dll disable-static pic-only])
EFL_LTLIBRARY_FLAGS="-no-undefined -version-info lt_cur:lt_rev:lt_age v_rel"
AC_SUBST(EFL_LTLIBRARY_FLAGS)

EFL_LTMODULE_FLAGS="-no-undefined -avoid-version"
AC_SUBST([EFL_LTMODULE_FLAGS])

### gettext

AM_GNU_GETTEXT_VERSION([0.17])

m4_ifdef([AM_GNU_GETTEXT], [
AM_GNU_GETTEXT([external])
po_makefile_in=po/Makefile.in
have_po="yes"
],
[
have_po="no"
])
AC_SUBST([LTLIBINTL])
LOCALE_DIR="${localedir}"
AC_SUBST(LOCALE_DIR)

if test "x${POSUB}" = "x" ; then
   have_po="no"
fi

AM_CONDITIONAL([HAVE_PO], [test "x${have_po}" = "xyes"])

### compilers

# rw_PROG_OBJC_WORKS
# Check whether the Objective C compiler works.
AC_DEFUN([rw_PROG_OBJC_WORKS],
[m4_ifdef([AC_PROG_OBJC], [AC_REQUIRE([AC_PROG_OBJC])dnl
AC_CACHE_CHECK([whether the Objective C compiler works],
               [rw_cv_prog_objc_works],
               [AC_LANG_PUSH([Objective C])
                AC_LINK_IFELSE([AC_LANG_PROGRAM([], [])],
                               [rw_cv_prog_objc_works=yes],
                               [rw_cv_prog_objc_works=no])
                AC_LANG_POP([Objective C])])], [rw_cv_prog_objc_works=no])
])

m4_ifdef([AC_PROG_OBJC],
   [
    AC_PROG_OBJC
    _AM_DEPENDENCIES(OBJC)
   ],
   [
    dnl#AC_CHECK_TOOL([OBJC], [gcc])
    dnl#AC_SUBST([OBJC])
    dnl#AC_SUBST([OBJCFLAGS])
   ])

rw_PROG_OBJC_WORKS

AM_PROG_AS
AC_PROG_CXX
AC_LANG(C)
AC_PROG_CC_C99
AM_PROG_CC_C_O

if test "x${ac_cv_prog_cc_c99}" = "xno" ; then
   AC_MSG_ERROR([ecore requires a c99-capable compiler])
fi

have_gnu_objc=${ac_cv_objc_compiler_gnu}

#echo $ac_cv_prog_CXX
#if test "x${ac_cv_prog_CXX}" = "x" ; then
#   AC_MSG_ERROR([C++ compiler needed. Exiting...])
#fi

AC_SYS_LARGEFILE

# pkg-config

PKG_PROG_PKG_CONFIG
if test "x${PKG_CONFIG}" = "x" ; then
   AC_MSG_ERROR([pkg-config tool not found. Install it or set PKG_CONFIG environment variable to that path tool. Exiting...])
fi

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])

# lcov

if test "${want_coverage}" = "yes" ; then
   AC_CHECK_PROG([have_lcov], [lcov], [yes], [no])
   if test "x${have_lcov}" = "xyes" ; then
      EFL_COV_CFLAGS="${EFL_COV_CFLAGS} -fprofile-arcs -ftest-coverage"
      EFL_COV_LIBS="${EFL_COV_LIBS} -lgcov"
      if test "x${prefer_assert}" = "xno"; then
         EFL_COV_CFLAGS="${EFL_COV_CFLAGS} -DNDEBUG"
      else
         EFL_COV_CFLAGS="${EFL_COV_CFLAGS} -g -O0 -DDEBUG"
      fi
   else
      AC_MSG_ERROR([lcov is not found])
   fi
fi

AM_CONDITIONAL([EFL_ENABLE_COVERAGE], [test "${want_coverage}" = "yes"])

AC_SUBST([EFL_COV_CFLAGS])
AC_SUBST([EFL_COV_LIBS])


#### Checks for libraries

# check unit testing library

if test "${want_tests}" = "yes"; then
   PKG_CHECK_MODULES([CHECK], [check >= 0.9.5])
fi
AM_CONDITIONAL([EFL_ENABLE_TESTS], [test "${want_tests}" = "yes"])


# check for crypto/tls library to use
case "$build_crypto" in
   gnutls)
      EFL_DEPEND_PKG([crypto], [GNUTLS], [gnutls >= 2.11])

      # libgcrypt
      AC_PATH_GENERIC([libgcrypt], [], [:],
         [AC_MSG_ERROR([libgcrypt required but not found])])
      requirements_libs_crypto="${LIBGCRYPT_LIBS} ${requirements_libs_crypto}"
      requirements_cflags_crypto="${LIBGCRYPT_CFLAGS} ${requirements_cflags_crypto}"
      ;;

   openssl)
      EFL_DEPEND_PKG([crypto], [OPENSSL], [openssl])
      ;;
esac
AM_CONDITIONAL([HAVE_CRYPTO_GNUTLS], [test "${build_crypto}" = "gnutls"])
AM_CONDITIONAL([HAVE_CRYPTO_OPENSSL], [test "${build_crypto}" = "openssl"])

#### Checks for header files

# Common Checks (keep names sorted for ease of use):
AC_HEADER_DIRENT
AC_HEADER_TIME

AC_CHECK_HEADERS([ \
execinfo.h \
errno.h \
libgen.h \
mcheck.h \
netinet/in.h \
sys/epoll.h \
sys/inotify.h \
sys/signalfd.h \
sys/types.h \
sys/param.h \
sys/mman.h \
])

have_inotify="${ac_cv_header_sys_inotify_h}"
AM_CONDITIONAL([HAVE_INOTIFY], [test "x${have_inotify}" = "xyes"])

have_notify_win32="${have_win32}"
AC_DEFINE_IF([HAVE_NOTIFY_WIN32],
   [test "x${have_notify_win32}" = "xyes"], [1],
   [File monitoring with Windows notification])
AM_CONDITIONAL([HAVE_NOTIFY_WIN32], [test "x${have_notify_win32}" = "xyes"])


EFL_CHECK_PATH_MAX


#### Checks for types

# wchar_t
AC_CHECK_SIZEOF([wchar_t])
EINA_SIZEOF_WCHAR_T=$ac_cv_sizeof_wchar_t
AC_SUBST([EINA_SIZEOF_WCHAR_T])

AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)

#### Checks for structures


#### Checks for compiler characteristics

AC_C_BIGENDIAN
AC_C_INLINE
EFL_CHECK_COMPILER_FLAGS([EFL], [-Wall -Wextra -Wpointer-arith -Wno-missing-field-initializers -fvisibility=hidden -fdata-sections -ffunction-sections])
EFL_CHECK_LINKER_FLAGS([EFL], [-fvisibility=hidden -fdata-sections -ffunction-sections -Wl,--gc-sections -fno-strict-aliasing -Wl,--as-needed])
case "${build_profile}" in
   dev)
     EFL_CHECK_COMPILER_FLAGS([EFL], [-Wshadow])
     ;;

   debug)
     ;;

   release)
     ;;
esac

EFL_ATTRIBUTE_VECTOR
EFL_ATTRIBUTE_ALWAYS_INLINE

EFL_LIBS="${EFL_LIBS}"
EFL_CFLAGS="${EFL_CFLAGS}"

## CPU architecture specific assembly

build_cpu_mmx="no"
build_cpu_sse3="no"
build_cpu_altivec="no"
build_cpu_neon="no"

SSE3_CFLAGS=""
ALTIVEC_CFLAGS=""

case $host_cpu in
  i*86|x86_64|amd64)
    AC_DEFINE([BUILD_MMX], [1], [Build MMX Code])
    build_cpu_mmx="yes"
    AC_CHECK_HEADER([immintrin.h],
       [
        AC_DEFINE(BUILD_SSE3, 1, [Build SSE3 Code])
        build_cpu_sse3="yes"
       ],
       [build_cpu_sse3="no"])
    AC_MSG_CHECKING([whether to build SSE3 code])
    AC_MSG_RESULT([${build_cpu_sse3}])

    if test "x$build_cpu_sse3" = "xyes" ; then
       SSE3_CFLAGS="-msse3"
    fi
    ;;
  *power* | *ppc*)
    build_cpu_altivec="yes"
    AC_CHECK_HEADER([altivec.h],
       [
        AC_DEFINE([BUILD_ALTIVEC], [1], [Build Altivec Code])
        AC_DEFINE([HAVE_ALTIVEC_H], [1], [Have altivec.h header file])
        build_cpu_altivec="yes"
       ],
       [
        save_CFLAGS=$CFLAGS
        save_CPPFLAGS=$CPPFLAGS
        CFLAGS=$CFLAGS" -maltivec"
        CPPFLAGS=$CPPFLAGS" -maltivec"
        unset ac_cv_header_altivec_h
        AC_CHECK_HEADER([altivec.h],
          [
            AC_DEFINE([BUILD_ALTIVEC], [1], [Build Altivec Code])
            AC_DEFINE([HAVE_ALTIVEC_H], [1], [Have altivec.h header file])
            build_cpu_altivec="yes"
          ],
          [build_cpu_altivec="no"]
        )
        CFLAGS=$save_CFLAGS
        CPPFLAGS=$save_CPPFLAGS
       ]
    )
    if test "x${build_cpu_altivec}" = "xyes"; then
       AC_MSG_CHECKING([whether to use altivec compiler flag])
       if test "x$GCC" = "xyes"; then
          if echo "int main(){return 0;}" | ${CPP} -faltivec - > /dev/null 2>&1; then
             altivec_cflags="-faltivec"
             AC_DEFINE([BUILD_ALTIVEC], [1], [Build Altivec Code])
          elif echo "int main(){return 0;}" | ${CPP} -maltivec - > /dev/null 2>&1; then
             altivec_cflags="-maltivec"
             AC_DEFINE([BUILD_ALTIVEC], [1], [Build Altivec Code])
          fi
       fi
       AC_MSG_RESULT([${altivec_cflags}])
       CFLAGS="$CFLAGS ${altivec_cflags}"
       ALTIVEC_CFLAGS="-maltivec"
    fi
    ;;
  arm*)
    build_cpu_neon="yes"
    AC_MSG_CHECKING([whether to use NEON instructions])
    AC_TRY_COMPILE([#include <arm_neon.h>],
       [asm volatile ("vqadd.u8 d0, d1, d0\n")],
       [
        AC_MSG_RESULT([yes])
        AC_DEFINE([BUILD_NEON], [1], [Build NEON Code])
        build_cpu_neon="yes"
       ],
       [
        AC_MSG_RESULT([no])
        build_cpu_neon="no"
       ])
    ;;
esac

AC_SUBST([ALTIVEC_CFLAGS])
AC_SUBST([SSE3_CFLAGS])

#### Checks for linker characteristics


#### Checks for library functions

AC_CHECK_FUNCS([\
backtrace \
backtrace_symbols \
clearenv \
dirfd \
execvp \
fpathconf \
fstatat \
malloc_usable_size \
mkdirat \
mtrace \
openat \
realpath \
strlcpy
])

AC_FUNC_ALLOCA

EFL_CHECK_FUNCS([EFL], [fnmatch gettimeofday])

have_atfile_source="${ac_cv_func_fstatat}"
AC_DEFINE_IF([HAVE_ATFILE_SOURCE],
   [test "x${have_atfile_source}" = "xyes"],
   [1], [Use fstatat and other -at file functions])



######################  EFL  ######################

#### Platform-dependent
DL_LIBS=""

#### Evil

USE_EVIL_CFLAGS=""
USE_EVIL_LIBS=""

EFL_LIB_START_OPTIONAL([Evil], [test "${have_windows}" = "yes"])

### Default values

### Additional options to configure
EFL_SELECT_WINDOWS_VERSION

### Checks for programs

### Checks for libraries

EVIL_LIBS=""
EVIL_DLFCN_LIBS=""
if test "${have_wince}" = "xyes"; then
    EVIL_LIBS="-lws2"
    EVIL_DLFCN_LIBS="-ltoolhelp"
else
    EVIL_LIBS="-lole32 -lws2_32 -lsecur32 -luuid"
    EVIL_DLFCN_LIBS="-lpsapi"
fi
EFL_ADD_LIBS([EVIL], [${EVIL_LIBS}])

AC_SUBST([EVIL_DLFCN_LIBS])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

EVIL_CPPFLAGS="-DEFL_EVIL_BUILD"
EVIL_DLFCN_CPPFLAGS="-DEFL_EVIL_DLFCN_BUILD -DPSAPI_VERSION=1"
EVIL_CFLAGS="-Wall -Wextra -Wshadow -Wdeclaration-after-statement -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls -Wpointer-arith"
EVIL_CXXFLAGS=""

if test "x${have_win32}" = "xyes" ; then
   EVIL_CXXFLAGS="-fno-rtti -fno-exceptions"
   EVIL_CPPFLAGS="${EVIL_CPPFLAGS} -DSECURITY_WIN32"
fi

AC_SUBST([EVIL_CPPFLAGS])
AC_SUBST([EVIL_DLFCN_CPPFLAGS])
AC_SUBST([EVIL_CXXFLAGS])

### Checks for linker characteristics

### Checks for library functions

platform_pc_evil="evil >= ${PACKAGE_VERSION}"
platform_pc="${platform_pc} ${platform_pc_evil}"

if test "x${have_wince}" = "xyes"; then
   platform_libs_evil="-lws2"
else
   platform_libs_evil="-lws2_32"
fi
platform_libs="${platform_libs} ${platform_libs_evil}"

USE_EVIL_CFLAGS="-I\$(top_srcdir)/src/lib/evil"
USE_EVIL_LIBS="lib/evil/libevil.la"
DL_LIBS="lib/evil/libdl.la"

EFL_LIB_END_OPTIONAL([Evil])

AC_SUBST([USE_EVIL_CFLAGS])
AC_SUBST([USE_EVIL_LIBS])
#### End of Evil


#### Escape

USE_ESCAPE_CFLAGS=""
USE_ESCAPE_LIBS=""

EFL_LIB_START_OPTIONAL([Escape], [test "${have_ps3}" = "yes"])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_ADD_LIBS([ESCAPE], [-llv2 -lm -lnet -lsysmodule -liberty])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

platform_pc_escape="escape >= ${PACKAGE_VERSION}"
platform_pc="${platform_pc} ${platform_pc_escape}"

USE_ESCAPE_CFLAGS="-I\$(top_srcdir)/src/lib/escape"
USE_ESCAPE_LIBS="lib/escape/libescape.la"

EFL_LIB_END_OPTIONAL([Escape])

AC_SUBST([USE_ESCAPE_CFLAGS])
AC_SUBST([USE_ESCAPE_LIBS])
#### End of Escape

AC_SUBST([DL_LIBS])
#### End of Platform-dependent


#### Eina

EFL_LIB_START([Eina])

### Default values

have_magic_debug="yes"
have_safety_checks="yes"
want_log="yes"
case "${build_profile}" in
   dev)
     with_max_log_level=""
     have_stringshare_usage="no"
     want_valgrind="auto"
     want_debug_malloc="no"
     want_debug_threads="no"
     want_default_mempool="no"
     ;;

   debug)
     with_max_log_level=""
     have_stringshare_usage="yes"
     want_valgrind="auto"
     want_debug_malloc="yes"
     want_debug_threads="yes"
     want_default_mempool="yes"
     ;;

   release)
     with_max_log_level="3"
     have_stringshare_usage="no"
     want_valgrind="no"
     want_debug_malloc="no"
     want_debug_threads="no"
     want_default_mempool="no"
     ;;
esac

EINA_CONFIG([HAVE_ALLOCA_H], [test "x${ac_cv_working_alloca_h}" = "xyes"])
EINA_CONFIG([MAGIC_DEBUG], [test "x${have_magic_debug}" = "xyes"])
EINA_CONFIG([SAFETY_CHECKS], [test "x${have_safety_checks}" = "xyes"])
EINA_CONFIG([DEFAULT_MEMPOOL], [test "x${want_default_mempool}" = "xyes"])

if test -n "${with_max_log_level}"; then
   AC_MSG_NOTICE([ignoring any EINA_LOG() with level greater than ${with_max_log_level}])
   AC_DEFINE_UNQUOTED([EINA_LOG_LEVEL_MAXIMUM], [${with_max_log_level}], [if set, logging is limited to this amount.])
fi

AC_DEFINE_IF([EINA_STRINGSHARE_USAGE],
   [test "x${have_stringshare_usage}" = "xyes"],
   [1], [Report Eina stringshare usage pattern])

### Additional options to configure

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([EINA], [all])

EFL_ADD_LIBS([EINA], [-lm])

## Options

# Valgrind
AC_ARG_ENABLE([valgrind],
   [AC_HELP_STRING([--disable-valgrind],
       [enable valgrind mempool declaration. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_valgrind="yes"
    else
       want_valgrind="no"
    fi
   ])

if test "${want_valgrind}" = "auto"; then
   PKG_CHECK_EXISTS([valgrind >= 2.4.0], [want_valgrind="yes"],
      [want_valgrind="no"
       AC_MSG_WARN([valgrind support desired by --with-profile=${build_profile} but not found. If your platform supports it, install valgrind.])])
fi

if test "${want_valgrind}" = "no"; then
    AC_DEFINE([NVALGRIND], [1], [Valgrind support disabled])
else
    PKG_CHECK_MODULES([VALGRIND], [valgrind >= 2.4.0])
    AC_DEFINE([HAVE_VALGRIND], [1], [Valgrind support enabled])
fi

AC_DEFINE_IF([EINA_DEBUG_MALLOC],
   [test "x${ac_cv_func_malloc_usable_size}" = "xyes" && test "x${want_debug_malloc}" = "xyes"],
   [1], [Turn on debugging overhead in mempool])

EFL_EVAL_PKGS([EINA])

## Examples

## Benchmarks

PKG_CHECK_MODULES([GLIB],
   [glib-2.0],
   [have_glib="yes"],
   [have_glib="no"])

if test "x${have_glib}" = "xyes" ; then
   GLIB_CFLAGS="${GLIB_CFLAGS} -DEINA_BENCH_HAVE_GLIB"
fi

### Checks for header files

# sys/mman.h could be provided by evil/escape/exotic so we need to set CFLAGS accordingly
CFLAGS_save="${CFLAGS}"
CFLAGS="${CFLAGS} ${EINA_CFLAGS}"
AC_CHECK_HEADERS([sys/mman.h])
CFLAGS="${CFLAGS_save}"

### Checks for types

EINA_CONFIG([HAVE_DIRENT_H], [test "x${have_dirent}" = "xyes"])
AC_DEFINE_IF([HAVE_DIRENT_H], [test "x${have_dirent}" = "xyes"],
   [1], [Define to 1 if you have a valid <dirent.h> header file.])

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_CHECK_FUNCS([EINA], [dlopen dladdr iconv shm_open splice setxattr])

enable_log="no"
if test "x${efl_func_fnmatch}" = "xyes" && test "x${want_log}" = "xyes" ; then
   enable_log="yes"
fi

AC_MSG_CHECKING([wether to build Eina_Log infrastructure])
AC_MSG_RESULT([${enable_log}])

EINA_CONFIG([ENABLE_LOG], [test "x${enable_log}" = "xyes"])

EFL_CHECK_THREADS

EFL_ADD_LIBS([EINA], [${EFL_PTHREAD_LIBS}])
EFL_ADD_CFLAGS([EINA], [${EFL_PTHREAD_CFLAGS}])

EINA_CONFIG([HAVE_PTHREAD_BARRIER], [test "x${efl_have_pthread_barrier}" = "xyes"])
EINA_CONFIG([HAVE_PTHREAD_AFFINITY], [test "x${efl_have_setaffinity}" = "xyes"])
EINA_CONFIG([HAVE_DEBUG_THREADS], [test "$want_debug_threads" = "yes"])

### Modules

EINA_CHECK_MODULE([chained-pool], [static], [chained pool])
EINA_CHECK_MODULE([pass-through], [static], [pass through])
EINA_CHECK_MODULE([one-big],      [static], [one big])

EFL_LIB_END([Eina])
#### End of Eina


#### Eet

EFL_LIB_START([Eet])

### Default values

### Additional options to configure

### Checks for programs

### Checks for libraries

## Compatibility layers
EFL_PLATFORM_DEPEND([EET], [evil])
EFL_CRYPTO_DEPEND([EET])

EFL_ADD_LIBS([EET], [-lm])

# Cryptography support
if test "$build_crypto" != "none" ; then
   AC_DEFINE([HAVE_CIPHER], [1], [Have cipher support built in eet])
   AC_DEFINE([HAVE_SIGNATURE], [1], [Have signature support for eet file])
fi

EFL_CHECK_LIBS([EET], [libjpeg zlib])

EFL_INTERNAL_DEPEND_PKG([EET], [eina])

EFL_EVAL_PKGS([EET])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Check availability

EFL_LIB_END([Eet])
#### End of Eet


#### Eo

EFL_LIB_START([Eo])

### Default values

### Additional options to configure

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([EO], [eina])

# Example (evas one)

# TODO: add once elementary is merged
#PKG_CHECK_MODULES([ELM], [elementary >= 1.7.0], [have_elm="yes"], [have_elm="no"])
AM_CONDITIONAL([EO_BUILD_EXAMPLE_EVAS], [test "x${have_elm}" = "xyes"])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Check availability

EFL_LIB_END([Eo])
#### End of Eo


#### Evas

EFL_LIB_START([Evas])

### Additional options to configure

# X11
AC_ARG_WITH([x11],
   [AC_HELP_STRING([--with-x11=xlib|xcb|none],
                   [X11 method to use: xlib, xcb or none])])

if test "x${have_windows}" = "xyes" || test "x${have_ps3}" = "xyes"; then
   with_x11="none"
elif test "x${with_x11}" = "x"; then
   with_x11="xlib"
fi

want_x11_xlib="no"
want_x11_xcb="no"
want_x11_none="no"
want_x11_any="no"
case "${with_x11}" in
   xlib)
     want_x11_xlib="yes"
     want_x11_any="yes"
     ;;
   xcb)
     want_x11_xcb="yes"
     want_x11_any="yes"
     ;;
   none)
     want_x11_none="yes"
     ;;
   *)
     AC_MSG_ERROR([Unknown build x11 --with-x11=${with_x11}])
     ;;
esac

# OpenGL
AC_ARG_WITH([opengl],
   [AC_HELP_STRING([--with-opengl=full|es|none],
                   [OpenGL method to use: full, es or none])])

if test "x${have_windows}" = "xyes" || test "x${have_ps3}" = "xyes"; then
   with_opengl="none"
elif test "x${with_opengl}" = "x"; then
   with_opengl="full"
fi

case "${with_opengl}" in
   full|es|none)
     ;;
   *)
     AC_MSG_ERROR([Unknown build opengl --with-opengl=${with_opengl}])
     ;;
esac

want_x11_xlib_opengl="no"
want_x11_xcb_opengl="no"
if test "${with_opengl}" != "none"; then
   want_x11_xlib_opengl="${want_x11_xlib}"
   want_x11_xcb_opengl="${want_x11_xcb}"
   want_x11_any_opengl="${want_x11_any}"
fi

# Wayland
AC_ARG_ENABLE([wayland],
   [AC_HELP_STRING([--enable-wayland],
       [enable wayland display server. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_wayland="yes"
    else
       want_wayland="no"
    fi
   ],
   [want_wayland="no"])

if test "${want_wayland}" = "yes"; then
   EFL_PKG_CHECK_STRICT([wayland-client])
fi

# Fb
AC_ARG_ENABLE([fb],
   [AC_HELP_STRING([--enable-fb],
       [enable raw Framebuffer access. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_fb="yes"
    else
       want_fb="no"
    fi
   ],
   [want_fb="no"])

# SDL
AC_ARG_ENABLE([sdl],
   [AC_HELP_STRING([--enable-sdl],
       [enable SDL support. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_sdl="yes"
    else
       want_sdl="no"
    fi
   ],
   [want_sdl="no"])

if test "${want_sdl}" = "yes"; then
   EFL_PKG_CHECK_STRICT([sdl >= 1.2.0])
fi

want_gl_sdl="no"
if test "${want_sdl}" = "yes" && test "${with_opengl}" != "none"; then
   want_gl_sdl="yes"
fi

# Fontconfig
AC_ARG_ENABLE([fontconfig],
   [AC_HELP_STRING([--disable-fontconfig],
       [disable fontconfig for finding fonts. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_fontconfig="yes"
    else
       want_fontconfig="no"
    fi
   ],
   [want_fontconfig="yes"])

# Fribidi
AC_ARG_ENABLE([fribidi],
   [AC_HELP_STRING([--disable-fribidi],
       [disable bidirectional text support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_fribidi="yes"
    else
       want_fribidi="no"
    fi
   ],
   [want_fribidi="yes"])

# Harfbuzz
AC_ARG_ENABLE([harfbuzz],
   [AC_HELP_STRING([--enable-harfbuzz],
       [enable complex text shaping and layouting support. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_harfbuzz="yes"
    else
       want_harfbuzz="no"
    fi
   ],
   [want_harfbuzz="no"])

# Egl
AC_ARG_ENABLE([egl],
   [AC_HELP_STRING([--enable-egl],
       [enable EGL rendering. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_egl="yes"
    else
       want_egl="no"
    fi
   ],
   [want_egl="no"])

# Pixman
AC_ARG_ENABLE([pixman],
   [AC_HELP_STRING([--enable-pixman],
       [enable pixman for software rendering. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_pixman="yes"
    else
       want_pixman="no"
    fi
   ],
   [want_pixman="no"])

AC_ARG_ENABLE([pixman-font],
   [AC_HELP_STRING([--enable-pixman-font],
       [Allow pixman to render fonts. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_pixman_font="yes"
    else
       have_pixman_font="no"
    fi
   ],
   [have_pixman_font="no"])

AC_ARG_ENABLE([pixman-rect],
   [AC_HELP_STRING([--enable-pixman-rect],
       [Allow pixman to render rects. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_pixman_rect="yes"
    else
       have_pixman_rect="no"
    fi
   ],
   [have_pixman_rect="no"])

AC_ARG_ENABLE([pixman-line],
   [AC_HELP_STRING([--enable-pixman-line],
       [Allow pixman to render lines. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_pixman_line="yes"
    else
       have_pixman_line="no"
    fi
   ],
   [have_pixman_line="no"])

AC_ARG_ENABLE(pixman-poly,
   AC_HELP_STRING([--enable-pixman-poly],
      [Allow pixman to render polys. @<:@default=disabled@:>@]),
   [
    if test "x${enableval}" = "xyes" ; then
       have_pixman_poly="yes"
    else
       have_pixman_poly="no"
    fi
   ],
   [have_pixman_poly="no"])

AC_ARG_ENABLE([pixman-image],
   [AC_HELP_STRING([--enable-pixman-image],
       [Allow pixman to render images. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_pixman_image="yes"
    else
       have_pixman_image="no"
    fi
   ],
   [have_pixman_image="no"])

AC_ARG_ENABLE([pixman-image-scale-sample],
   [AC_HELP_STRING([--enable-pixman-image-scale-sample],
       [Allow pixman to render sampled scaled images. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_pixman_image_scale_sample="yes"
    else
       have_pixman_image_scale_sample="no"
    fi
   ],
   [have_pixman_image_scale_sample="no"])

# Tile rotate
AC_ARG_ENABLE([tile-rotate],
   [AC_HELP_STRING([--enable-tile-rotate],
       [Enable tiled rotate algorithm. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_tile_rotate="yes"
    else
       have_tile_rotate="no"
    fi
  ],
  [have_tile_rotate="no"])


# Loaders

AC_ARG_ENABLE([image-loader-gif],
   [AC_HELP_STRING([--enable-image-loader-gif], [enable Gif image loader. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_evas_image_loader_gif="yes"
    else
       if test "x${enableval}" = "xstatic" ; then
          want_evas_image_loader_gif="static"
       else
          want_evas_image_loader_gif="no"
       fi
    fi
   ],
   [want_evas_image_loader_gif="yes"])

AC_ARG_ENABLE([image-loader-svg],
   [AC_HELP_STRING([--enable-image-loader-svg], [enable SVG image loader. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_evas_image_loader_svg="yes"
    else
       if test "x${enableval}" = "xstatic" ; then
          want_evas_image_loader_svg="static"
       else
          want_evas_image_loader_svg="no"
       fi
    fi
   ],
   [want_evas_image_loader_svg="no"])

AC_ARG_ENABLE([image-loader-tiff],
   [AC_HELP_STRING([--enable-image-loader-tiff], [enable Tiff image loader. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_evas_image_loader_tiff="yes"
    else
       if test "x${enableval}" = "xstatic" ; then
          want_evas_image_loader_tiff="static"
       else
          want_evas_image_loader_tiff="no"
       fi
    fi
   ],
   [want_evas_image_loader_tiff="yes"])

AC_ARG_ENABLE([image-loader-webp],
   [AC_HELP_STRING([--enable-image-loader-webp], [enable WEBP image loader. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_evas_image_loader_webp="yes"
    else
       if test "x${enableval}" = "xstatic" ; then
          want_evas_image_loader_webp="static"
       else
          want_evas_image_loader_webp="no"
       fi
    fi
   ],
   [want_evas_image_loader_webp="no"])

### Default values

want_evas_cserve2="yes"

want_evas_engine_software_gdi="${have_win32}"
want_evas_engine_software_ddraw="${have_win32}"
want_evas_engine_gl_cocoa="no" # TODO: ${have_darwin}
want_evas_engine_wayland_egl="no"
if test "${want_wayland}" = "yes" && test "${want_egl}" = "yes"; then
   want_evas_engine_wayland_egl="yes"
fi

want_evas_image_loader_bmp="static"
want_evas_image_loader_eet="static"
want_evas_image_loader_generic="static"
#want_evas_image_loader_gif="yes"
want_evas_image_loader_ico="static"
want_evas_image_loader_jpeg="static"
want_evas_image_loader_pmaps="static"
want_evas_image_loader_png="static"
want_evas_image_loader_psd="static"
#want_evas_image_loader_svg="yes"
want_evas_image_loader_tga="static"
#want_evas_image_loader_tiff="yes"
want_evas_image_loader_wbmp="static"
#want_evas_image_loader_webp="yes"
want_evas_image_loader_xpm="static"

if test "${have_wince}" = "yes"; then
   want_fontconfig="no"
   want_evas_image_loader_gif="no"
   want_evas_image_loader_svg="no"
   want_evas_image_loader_tiff="no"
   want_evas_image_loader_webp="no"
fi

EFL_CHECK_FUNC([EVAS], [dlsym])

if test "x${efl_func_dlsym}" = "xno" && test "${with_opengl}" != "none"; then
   AC_MSG_ERROR([OpenGL cannot work without dlsym()])
fi

### Checks for programs

### Checks for libraries

## Compatibility layers
EFL_PLATFORM_DEPEND([EVAS], [all])
EFL_INTERNAL_DEPEND_PKG([EVAS], [eo])
EFL_INTERNAL_DEPEND_PKG([EVAS], [eet])
EFL_INTERNAL_DEPEND_PKG([EVAS], [eina])

EFL_ADD_LIBS([EVAS], [-lm])

# Freetype
EFL_DEPEND_PKG([EVAS], [FREETYPE], [freetype2 >= 9.3.0])

## optional dependencies

# FontConfig
EFL_OPTIONAL_DEPEND_PKG([EVAS], [${want_fontconfig}],
   [FONTCONFIG], [fontconfig >= 2.5.0])

# fribidi support
EFL_OPTIONAL_DEPEND_PKG([EVAS], [${want_fribidi}],
   [FRIBIDI], [fribidi >= 0.19.2])

# harfbuzz support
have_harfbuzz_ft="no"
EFL_OPTIONAL_DEPEND_PKG([EVAS], [${want_harfbuzz}],
   [HARFBUZZ], [harfbuzz >= 0.9.0])

# Pixman
EFL_OPTIONAL_DEPEND_PKG([EVAS], [${want_pixman}], [PIXMAN], [pixman-1])

if test "x${have_pixman}" = "xyes" ; then
   if test "x${have_pixman_font}" = "xyes" ; then
      AC_DEFINE(PIXMAN_FONT, 1, [Allow pixman to render fonts])
   fi
   if test "x${have_pixman_rect}" = "xyes" ; then
      AC_DEFINE(PIXMAN_RECT, 1, [Allow pixman to render rects])
   fi
   if test "x${have_pixman_line}" = "xyes" ; then
      AC_DEFINE(PIXMAN_LINE, 1, [Allow pixman to render lines])
   fi
   if test "x${have_pixman_poly}" = "xyes" ; then
      AC_DEFINE(PIXMAN_POLY, 1, [Allow pixman to render polys])
   fi
   if test "x${have_pixman_image}" = "xyes" ; then
      AC_DEFINE(PIXMAN_IMAGE, 1, [Allow pixman to render images])
   fi
   if test "x${have_pixman_image_scale_sample}" = "xyes" ; then
      AC_DEFINE(PIXMAN_IMAGE_SCALE_SAMPLE, 1, [Allow pixman to render image sampled scaling])
   fi
fi

## Engines

define([EVAS_ENGINE_DEP_CHECK_FB], [
AC_CHECK_HEADER([linux/fb.h], [:], [AC_MSG_ERROR([Missing linux/fb.h])])
])

define([EVAS_ENGINE_DEP_CHECK_PSL1GHT], [
AC_CHECK_HEADER([rsx/rsx.h], [:], [AC_MSG_ERROR([Missing rsx/rsx.h])])
])

EVAS_ENGINE([buffer], [static])
EVAS_ENGINE([fb], [${want_fb}], [EVAS_ENGINE_DEP_CHECK_FB])
EVAS_ENGINE([psl1ght], [${have_ps3}], [EVAS_ENGINE_DEP_CHECK_PSL1GHT])
EVAS_ENGINE([wayland-shm], [${want_wayland}])

# XXX TODO:
EVAS_CHECK_ENGINE([gl-cocoa], [${want_evas_engine_gl_cocoa}], [no], [OpenGL Cocoa])
EVAS_CHECK_ENGINE([gl-sdl], [${want_gl_sdl}], [no], [OpenGL SDL])
EVAS_CHECK_ENGINE([software-gdi], [${want_evas_engine_software_gdi}], [no], [Software GDI])
EVAS_CHECK_ENGINE([software-ddraw], [${want_evas_engine_software_ddraw}], [no], [Software DirectDraw])
EVAS_CHECK_ENGINE([wayland-egl], [${want_evas_engine_wayland_egl}], [no], [Wayland Egl])

# Software XCB

have_evas_engine_software_xcb="no"
if test "${with_x11}" = "xcb" ; then
   EVAS_CHECK_ENGINE([software-xcb], [${want_x11_xcb}], [no], [Software XCB])
else
   AM_CONDITIONAL([BUILD_ENGINE_SOFTWARE_XCB], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_XCB], [false])
fi

AC_MSG_CHECKING([whether to build Software XCB Engine])
AC_MSG_RESULT([${have_evas_engine_software_xcb}])

# No Software XCB ? Then try Software Xlib

have_evas_engine_software_xlib="no"
if test "x${want_x11_xcb}" = "xno" ; then
   EVAS_CHECK_ENGINE([software-xlib], [${want_x11_xlib}], [no], [Software Xlib])
else
   AM_CONDITIONAL([BUILD_ENGINE_SOFTWARE_XLIB], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_XLIB], [false])
fi

AC_MSG_CHECKING([whether to build Software Xlib Engine])
AC_MSG_RESULT([${have_evas_engine_software_xlib}])

# If software_x11 is available, define everything needed for X11

have_evas_engine_software_x11="no"
if test "x${have_evas_engine_software_xlib}" = "xyes" || test "x${have_evas_engine_software_xlib}" = "xstatic" || test "x${have_evas_engine_software_xcb}" = "xyes" || test "x${have_evas_engine_software_xcb}" = "xstatic" ; then
   have_evas_engine_software_x11="yes"
   AC_DEFINE_UNQUOTED([BUILD_ENGINE_SOFTWARE_X11], [1], [Build software X11 engine])
fi
AM_CONDITIONAL([BUILD_ENGINE_SOFTWARE_X11], [test "x${have_evas_engine_software_x11}" = "xyes"])

if test "x${have_evas_engine_software_xlib}" = "xstatic" || test "x${have_evas_engine_software_xcb}" = "xstatic"; then
   AC_DEFINE_UNQUOTED([EVAS_STATIC_BUILD_SOFTWARE_X11], [1], [Build software X11 engine as part of libevas])
fi
AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_X11], [test "x${have_evas_engine_software_xcb}" = "xstatic" || test "x${have_evas_engine_software_xlib}" = "xstatic"])

# Needed for evas-software-x11.pc

AC_SUBST([have_evas_engine_software_xlib])
AC_SUBST([have_evas_engine_software_xcb])

# Software generic
AC_DEFINE([EVAS_STATIC_BUILD_SOFTWARE_GENERIC], [1], [Build software generic engine as part of libevas])
AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_GENERIC], [true])

# OpenGL

if test "${with_opengl}" = "es" ; then
   AC_DEFINE(GL_GLES, 1, [GLSL runtime shader GLES2 support])
fi

# OpenGL XCB

have_evas_engine_gl_xcb="no"
if test "${want_x11_xcb_opengl}" = "yes"; then
   EVAS_CHECK_ENGINE([gl-xcb], [${want_x11_xcb_opengl}], [no], [OpenGL XCB])
else
   AM_CONDITIONAL([BUILD_ENGINE_GL_XCB], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_XCB], [false])
fi

AC_MSG_CHECKING([whether to build OpenGL XCB Engine])
AC_MSG_RESULT([${have_evas_engine_gl_xcb}])

# No openGL XCB ? Then try OpenGL Xlib

have_evas_engine_gl_xlib="no"
if test "x${have_evas_engine_gl_xcb}" = "xno" ; then
   EVAS_CHECK_ENGINE([gl-xlib], [${want_x11_xlib_opengl}], [no], [OpenGL XLib])
else
   AM_CONDITIONAL([BUILD_ENGINE_GL_XLIB], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_XLIB], [false])
fi

AC_MSG_CHECKING([whether to build OpenGL Xlib Engine])
AC_MSG_RESULT([${have_evas_engine_gl_xlib}])

# If opengl_x11 is available, define everything needed for X11

have_evas_engine_gl_x11="no"
if test "x${have_evas_engine_gl_xlib}" = "xyes" || test "x${have_evas_engine_gl_xlib}" = "xstatic" || test "x${have_evas_engine_gl_xcb}" = "xyes" || test "x${have_evas_engine_gl_xcb}" = "xstatic" ; then
   have_evas_engine_gl_x11="yes"
   AC_DEFINE_UNQUOTED([BUILD_ENGINE_GL_X11], [1], [Build OpenGL X11 engine])
fi
AM_CONDITIONAL([BUILD_ENGINE_GL_X11], [test "x${have_evas_engine_gl_x11}" = "xyes"])

if test "x${have_evas_engine_gl_xlib}" = "xstatic" || test "x${have_evas_engine_gl_xcb}" = "xstatic"; then
   AC_DEFINE_UNQUOTED([EVAS_STATIC_BUILD_GL_X11], [1], [Build OpenGL X11 engine as part of libevas])
fi
AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_X11], [test "x${have_evas_engine_gl_xcb}" = "xstatic" || test "x${have_evas_engine_gl_xlib}" = "xstatic"])

# Needed for evas-opengl-x11.pc

AC_SUBST([have_evas_engine_gl_xlib])
AC_SUBST([have_evas_engine_gl_xcb])

# OpenGL SDL

if test "x$have_evas_engine_gl_sdl" = "xyes" || test "x$have_evas_engine_gl_sdl" = "xstatic" ; then
   AC_CHECK_DECL([SDL_GL_CONTEXT_MAJOR_VERSION],
      [AC_DEFINE([HAVE_SDL_GL_CONTEXT_VERSION], [1], [SDL_GL version attributes present])],,
      [#include <SDL/SDL_video.h>])
fi

if test "${with_opengl}" = "es"; then
   AC_CHECK_DECL([SDL_OPENGLES],
      [AC_DEFINE([HAVE_SDL_FLAG_OPENGLES], [1], [SDL_OPENGLES flag is present])],,
      [#include <SDL/SDL_video.h>])
fi

# OpenGL common
evas_engine_gl_common_cflags=""
evas_engine_gl_common_libs=""
AC_SUBST([evas_engine_gl_common_cflags])
AC_SUBST([evas_engine_gl_common_libs])

have_evas_engine_gl_common="no"
have_static_evas_engine_gl_common="no"
if test "x$have_evas_engine_gl_xlib" = "xyes" || \
   test "x$have_evas_engine_gl_xcb" = "xyes" || \
   test "x$have_evas_engine_gl_sdl" = "xyes" || \
   test "x$have_evas_engine_gl_cocoa" = "xyes" || \
   test "x$have_evas_engine_wayland_egl" = "xyes"; then
   have_evas_engine_gl_common="yes"
fi
if test "x$have_evas_engine_gl_xlib" = "xstatic" || \
   test "x$have_evas_engine_gl_xcb" = "xstatic" || \
   test "x$have_evas_engine_gl_sdl" = "xstatic" || \
   test "x$have_evas_engine_gl_cocoa" = "xstatic" || \
   test "x$have_evas_engine_wayland_egl" = "xstatic"; then
   have_evas_engine_gl_common="yes"
   have_static_evas_engine_gl_common="yes"
fi

if test "x${have_evas_engine_gl_common}" = "xyes"; then
   AC_DEFINE([BUILD_ENGINE_GL_COMMON], [1], [Generic OpenGL Rendering Support])
fi

AM_CONDITIONAL([BUILD_ENGINE_GL_COMMON], [test "x$have_evas_engine_gl_common" = "xyes"])

if test "x${have_static_evas_engine_gl_common}" = "xyes"; then
   AC_DEFINE([EVAS_STATIC_BUILD_GL_COMMON], [1], [Build GL generic engine as part of libevas])
fi

AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_COMMON], [test "x${have_static_evas_engine_gl_common}" = "xyes"])

## Image Loaders

EVAS_CHECK_IMAGE_LOADER([BMP],     [${want_evas_image_loader_bmp}])
EVAS_CHECK_IMAGE_LOADER([Eet],     [${want_evas_image_loader_eet}])
EVAS_CHECK_IMAGE_LOADER([Generic], [${want_evas_image_loader_generic}])
EVAS_CHECK_IMAGE_LOADER([Gif],     [${want_evas_image_loader_gif}])
EVAS_CHECK_IMAGE_LOADER([ICO],     [${want_evas_image_loader_ico}])
EVAS_CHECK_IMAGE_LOADER([JPEG],    [${want_evas_image_loader_jpeg}])
EVAS_CHECK_IMAGE_LOADER([PMAPS],   [${want_evas_image_loader_pmaps}])
EVAS_CHECK_IMAGE_LOADER([PNG],     [${want_evas_image_loader_png}])
EVAS_CHECK_IMAGE_LOADER([PSD],     [${want_evas_image_loader_psd}])
EVAS_CHECK_IMAGE_LOADER([SVG],     [${want_evas_image_loader_svg}])
EVAS_CHECK_IMAGE_LOADER([Tga],     [${want_evas_image_loader_tga}])
EVAS_CHECK_IMAGE_LOADER([Tiff],    [${want_evas_image_loader_tiff}])
EVAS_CHECK_IMAGE_LOADER([WBMP],    [${want_evas_image_loader_wbmp}])
EVAS_CHECK_IMAGE_LOADER([WEBP],    [${want_evas_image_loader_webp}])
EVAS_CHECK_IMAGE_LOADER([XPM],     [${want_evas_image_loader_xpm}])

dnl Windows has no sigsetjmp function, nor equivalent.
dnl So we disable the jpeg saver.
dnl TODO: must find a workaround
if test "${have_windows}" = "yes"; then
   have_evas_image_saver_jpeg="no"
elif test "x${have_evas_image_loader_jpeg}" = "xyes" || \
     test "x${have_evas_image_loader_jpeg}" = "xstatic"; then
   AC_DEFINE([EVAS_BUILD_SAVER_JPEG], [1], [Build JPEG saver])
   have_evas_image_saver_jpeg="yes"
fi

AM_CONDITIONAL([BUILD_SAVER_JPEG], [test "x${have_evas_image_saver_jpeg}" = "xyes"])

EFL_EVAL_PKGS([EVAS])

### Checks for header files

if test "x$have_harfbuzz" = "xyes" ; then

   CPPFLAGS_SAVE="$CPPFLAGS"
   CPPFLAGS="$CPPFLAGS $EVAS_CFLAGS"
# must have for usage with harfbuzz although harfbuzz may not have it.

   AC_CHECK_HEADER([hb-ft.h],
      [
       have_harfbuzz_ft="yes"
       #Depend on harfbuzz ft for harfbuzz support
       AC_DEFINE([HAVE_HARFBUZZ], [1], [have harfbuzz support])
      ],
      [AC_MSG_ERROR([Harfbuzz-ft (hb-ft.h) not found])])

   CPPFLAGS="$CPPFLAGS_SAVE"
fi

### Checks for types

CPPFLAGS_SAVE="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $EVAS_CFLAGS -Isrc/lib/eina"
AC_CHECK_SIZEOF([Eina_Unicode], [], [#include <Eina.h>])
CPPFLAGS="$CPPFLAGS_SAVE"


if test "x$have_fribidi" = "xyes" ; then
   CPPFLAGS_SAVE="$CPPFLAGS"
   CPPFLAGS="$CPPFLAGS $EVAS_CFLAGS"
   AC_CHECK_SIZEOF([FriBidiChar], [], [#include <fribidi.h>])
   CPPFLAGS="$CPPFLAGS_SAVE"
fi

### Checks for structures

AC_CHECK_TYPES([struct sigaction], [], [],
               [[#include <signal.h>]])

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

AC_CHECK_FUNCS([siglongjmp])

AC_CHECK_LIB([m], [lround], [AC_DEFINE([HAVE_LROUND], [1], [C99 lround function exists])])

if test "x${want_evas_image_loader_generic}" = "xyes" || test "x${want_evas_cserve2}" = "xyes" ; then
   EFL_CHECK_FUNC([EVAS], [shm_open])
fi

# cserve2 only works on Linux so far.

if test "x${efl_func_shm_open}" = "xno" ; then
   want_evas_cserve2="no"
fi

if test "x${want_evas_cserve2}" = "xyes"; then
   if test "x${ac_cv_header_sys_epoll_h}" = "xno" || test "x${ac_cv_header_sys_inotify_h}" = "xno" || test "x${ac_cv_header_sys_signalfd_h}" = "xno"; then
      want_evas_cserve2="no"
   fi
fi

AC_DEFINE_IF([EVAS_CSERVE2],
   [test "x${want_evas_cserve2}" = "xyes"],
   [1], [Shared cache server.])
AM_CONDITIONAL([EVAS_CSERVE2], [test "x${want_evas_cserve2}" = "xyes"])

### Configuration

## Tile rotation

if test "x${have_tile_rotate}" = "xyes" ; then
   AC_DEFINE(TILE_ROTATE, 1, [Enable tiled rotate algorithm])
fi


## dither options

AC_ARG_WITH([evas-dither-mask],
   [AC_HELP_STRING([--with-evas-dither-mask=TYPE],
                   [use the specified dither mask to convert bitdepths in Evas, one of: big, small, line or none.
                    @<:@default=big@:>@])],
   [build_evas_dither_mask=${withval}],
   [build_evas_dither_mask=big])

case "${build_evas_dither_mask}" in
   big)
     AC_DEFINE([BUILD_BIG_DITHER_MASK], [1], [Use biggest dither mask while converting in Evas])
     ;;
   small)
     AC_DEFINE([BUILD_SMALL_DITHER_MASK], [1], [Use smaller dither mask while converting in Evas])
     ;;
   line)
     AC_DEFINE([BUILD_LINE_DITHER_MASK], [1], [Use simpler line-only dither mask while converting in Evas])
     ;;
   none)
     AC_DEFINE([BUILD_NO_DITHER_MASK], [1], [Do not use dither masks while converting in Evas])
     ;;
   *)
     AC_MSG_ERROR([Unknown Evas dither mask --with-evas-dither-mask=${build_evas_dither_mask}])
     ;;
esac

EFL_LIB_END([Evas])
#### End of Evas


#### Embryo

EFL_LIB_START([Embryo])

### Default values

### Additional options to configure

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([EMBRYO], [all])
EFL_INTERNAL_DEPEND_PKG([EMBRYO], [eina])

EFL_ADD_LIBS([EMBRYO], [-lm])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

if ! test "x${efl_func_fnmatch}" = "xyes" ; then
   AC_MSG_ERROR([Cannot find fnmatch()])
fi

if ! test "x${efl_func_gettimeofday}" = "xyes" ; then
   AC_MSG_ERROR([Cannot find gettimeofday()])
fi

### Check availability

EFL_LIB_END([Embryo])
#### End of Embryo


#### Ecore

EFL_LIB_START([Ecore])

### Additional options to configure

# glib

AC_ARG_WITH([glib],
   [AC_HELP_STRING([--with-glib=yes|no|always],
                   [add glib support. @<:@default=enabled@:>@])],
   [
    if test "x${withval}" = "xyes" ; then
       with_glib="yes"
    else
       if test "x${withval}" = "xalways" ; then
          with_glib="always"
       else
          with_glib="no"
       fi
    fi
   ],
   [with_glib="yes"])

AC_ARG_ENABLE([g-main-loop],
   [AC_HELP_STRING([--enable-g-main-loop],
       [enable ecore_main_loop based on g_main_loop. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_g_main_loop="yes"
    else
       want_g_main_loop="no"
    fi
   ],
   [want_g_main_loop="no"])

AC_ARG_ENABLE([gstreamer],
   [AC_HELP_STRING([--disable-gstreamer],
                   [disable gstreamer support. @<:@default=enabled@:>@])],
   [
    if test "x${withval}" = "xyes" ; then
       want_gstreamer="yes"
    else
       want_gstreamer="no"
    fi
   ],
   [want_gstreamer="yes"])

### Default values

if test "${have_windows}" = "yes"; then
   with_glib="no"
fi

want_glib="no"
if test "x${with_glib}" = "xyes" || test "x${with_glib}" = "xalways" ; then
   want_glib="yes"
fi

want_ecore_timer_dump="no"
if test "x${build_profile}" = "xdebug" && test "x${ac_cv_func_backtrace}" = "xyes"; then
   want_ecore_timer_dump="yes"
   AC_DEFINE([WANT_ECORE_TIMER_DUMP], [1], [Want Ecore_Timer dump infrastructure])
fi

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([ECORE], [all])
EFL_INTERNAL_DEPEND_PKG([ECORE], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE], [eina])

EFL_ADD_LIBS([ECORE], [-lm])

# glib

EFL_OPTIONAL_DEPEND_PKG([ECORE], [${want_glib}], [GLIB], [glib-2.0])
if test "x${have_glib}" = "xno"; then
   want_g_main_loop="no"
fi

want_glib_integration_always=no
if test "x${with_glib}" = "xalways" ; then
  want_glib_integration_always="yes"
  AC_DEFINE([GLIB_INTEGRATION_ALWAYS], [1], [Always integrate glib if support compiled])
fi

if test "x${want_g_main_loop}" = "xyes" ; then
  AC_DEFINE([USE_G_MAIN_LOOP], [1], [Use g_main_loop in ecore])
fi

# not EFL_OPTIONAL_DEPEND_PKG() because it's only used for ecore examples
if test "${want_gstreamer}" = "yes"; then
  PKG_CHECK_MODULES([GSTREAMER], [gstreamer-0.10])
fi
AM_CONDITIONAL([HAVE_GSTREAMER], [test "${want_gstreamer}" = "yes"])

EFL_EVAL_PKGS([ECORE])

EFL_ADD_LIBS([ECORE], [${platform_libs}])

### Checks for header files

AC_HEADER_SYS_WAIT

AC_CHECK_HEADERS([ \
features.h \
langinfo.h \
locale.h \
sys/time.h \
sys/prctl.h \
sys/resource.h \
sys/select.h \
sys/timerfd.h \
])

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

## ecore

# isfinite

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
       [[
#include <math.h>
       ]],
       [[
int i = isfinite(0);
       ]])],
   [
    AC_DEFINE(HAVE_ISFINITE, 1, [Define to 1 if you have `isfinite', as a function or macro.])
    have_isfinite="yes"
   ],
   [have_isfinite="no"])

AC_MSG_CHECKING([for isfinite])
AC_MSG_RESULT([${have_isfinite}])

# mallinfo, timerfd_create, clock_gettime

AC_CHECK_FUNCS_ONCE([mallinfo timerfd_create clock_gettime])

if ! test "x${ac_cv_func_clock_gettime}" = "xyes" ; then
   AC_CHECK_LIB([rt], [clock_gettime],
      [
        EFL_ADD_LIBS([ECORE], [-lrt])
        AC_DEFINE([HAVE_CLOCK_GETTIME], [1], [Have clock_gettime()])
       ])
fi

EFL_ADD_LIBS([ECORE], [${LTLIBINTL}])

### Check availability

EFL_LIB_END([Ecore])
#### End of Ecore


#### Ecore_Con

EFL_LIB_START([Ecore_Con])

### Additional options to configure

AC_ARG_ENABLE([curl],
   [AC_HELP_STRING([--disable-curl], [disable curl support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_curl="yes"
    else
       want_curl="no"
    fi
   ],
   [want_curl="yes"])

### Default values

want_ecore_con_local_sockets="yes"
want_ecore_con_abstract_sockets="yes"

if test "${have_wince}" = "yes"; then
   want_curl="no"
   want_cares="no"
   want_ecore_con_local_sockets="no"
   want_ecore_con_abstract_sockets="no"
elif test "${have_win32}" = "yes"; then
   want_cares="yes"
   want_ecore_con_abstract_sockets="no"
elif test "${have_darwin}" = "yes"; then
   want_cares="no"
   want_ecore_con_abstract_sockets="no"
elif test "${have_ps3}" = "yes"; then
   want_cares="no"
   want_ecore_con_local_sockets="no"
   want_ecore_con_abstract_sockets="no"
else
   want_cares="no"
fi

AC_DEFINE_IF([HAVE_LOCAL_SOCKETS],
   [test "x${want_ecore_con_local_sockets}" = "xyes"],
   [1], [Have local sockets support])
AC_DEFINE_IF([HAVE_ABSTRACT_SOCKETS],
   [test "x${want_ecore_con_abstract_sockets}" = "xyes"],
   [1], [Have abstract sockets namespace])

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([ECORE_CON], [all])
EFL_CRYPTO_DEPEND([ECORE_CON])
EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [eet])
EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [eina])
EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [ecore])

EFL_ADD_LIBS([ECORE_CON], [-lm])

EFL_OPTIONAL_DEPEND_PKG([ECORE_CON], [${want_curl}], [CURL], [libcurl])

EFL_OPTIONAL_DEPEND_PKG([ECORE_CON], [${want_cares}],
   [CARES], [libcares >= 1.6.1])
AM_CONDITIONAL([HAVE_CARES], [test "x${have_cares}" = "xyes"])

EFL_EVAL_PKGS([ECORE_CON])

### Checks for header files

AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([net/if.h], [], [],
[
#include <stdio.h>
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
])
AC_CHECK_HEADERS([sys/un.h], [], [],
[
#include <stdio.h>
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
])
AC_CHECK_HEADERS([arpa/inet.h arpa/nameser.h netinet/tcp.h netinet/in.h ws2tcpip.h netdb.h])

if test "x${ac_cv_header_netdb_h}" = "xno" && test "x${have_windows}" = "xno"; then
   AC_MSG_ERROR([netdb.h is requested to have Ecore_Con. Exiting...])
fi

### Checks for types

have_ipv6="no"
AC_CHECK_TYPES([struct ipv6_mreq],
   [have_ipv6="yes"],
   [have_ipv6="no"],
   [[
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif
]])

AC_DEFINE_IF([HAVE_IPV6],
   [test "x${have_ipv6}" = "xyes"],
   [1], [Define if IPV6 is supported])
AM_CONDITIONAL([HAVE_IPV6], [test "x${have_ipv6}" = "xyes"])

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Ecore_Con])
#### End of Ecore_Con


#### Ecore_Ipc

EFL_LIB_START([Ecore_Ipc])

### Default values

### Additional options to configure

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_IPC], [ecore-con])
EFL_INTERNAL_DEPEND_PKG([ECORE_IPC], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_IPC], [eina])

EFL_ADD_LIBS([ECORE_IPC], [-lm])

### Checks for header files

AC_CHECK_HEADERS([winsock2.h])

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Ecore_Ipc])
#### End of Ecore_Ipc


#### Ecore_File

EFL_LIB_START([Ecore_File])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([ECORE_FILE], [escape])
EFL_INTERNAL_DEPEND_PKG([ECORE_FILE], [ecore-con])
EFL_INTERNAL_DEPEND_PKG([ECORE_FILE], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_FILE], [eina])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Ecore_File])
#### End of Ecore_File


#### Ecore_Input
EFL_LIB_START([Ecore_Input])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT], [eina])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Ecore_Input])
#### End of Ecore_Input


#### Ecore_Input_Evas
EFL_LIB_START([Ecore_Input_Evas])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [evas])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [eina])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Ecore_Input_Evas])
#### End of Ecore_Input_Evas


#### Ecore_Cocoa

EFL_LIB_START([Ecore_Cocoa])

### Additional options to configure

### Default values

have_ecore_cocoa="no"
# TODO: fix cocoa and: have_ecore_cocoa="${have_darwin}"

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_COCOA], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_COCOA], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_COCOA], [eina])

### Checks for header files

ECORE_COCOA_LIBS=""
if test "x${have_ecore_cocoa}" = "xyes" ; then
   m4_ifdef([AC_PROG_OBJC],
      [
       if test "x${have_gnu_objc}" = "xyes" ; then
          AC_LANG_PUSH([Objective C])
          LIBS_save="$LIBS"
          LIBS="$LIBS -framework Cocoa"
          AC_LINK_IFELSE(
             [AC_LANG_PROGRAM(
                 [[
#include <Cocoa/Cocoa.h>
                 ]],
                 [[
NSWindow *window;
window = [[NSWindow alloc]
           initWithContentRect:NSMakeRect(0, 0, 1, 1)
           styleMask:(NSTitledWindowMask)
           backing:NSBackingStoreBuffered
           defer:NO
           screen:nil
         ];
                 ]])],
             [
              have_ecore_cocoa="yes"
              EFL_ADD_LIBS([ECORE_COCOA], [-framework Cocoa])
             ],
             [have_ecore_cocoa="no"])
          LIBS="$LIBS_save"
          AC_MSG_CHECKING([whether Cocoa framework is supported])
          AC_MSG_RESULT([${have_ecore_cocoa}])
          AC_LANG_POP([Objective C])
       fi
   ])
fi
AC_SUBST([ECORE_COCOA_LIBS])

AM_CONDITIONAL([HAVE_ECORE_COCOA], [test "x${have_ecore_cocoa}" = "xyes"])

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Ecore_Cocoa])
#### End of Ecore_Cocoa




#### Ecore_FB
EFL_LIB_START_OPTIONAL([Ecore_FB], [test "${want_fb}" = "yes"])

### Additional options to configure
AC_ARG_ENABLE([tslib],
   [AC_HELP_STRING([--disable-tslib],
       [disable tslib for touchscreen events.])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_tslib="yes"
    else
       want_tslib="no"
    fi
   ], [want_tslib="yes"])

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_FB], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_FB], [eina])

EFL_OPTIONAL_DEPEND_PKG([ECORE_FB], [${want_tslib}], [TSLIB], [tslib])

EFL_EVAL_PKGS([ECORE_FB])

### Checks for header files

have_ecore_fb="no"
AC_CHECK_HEADER([linux/fb.h],
   [AC_CHECK_HEADER([linux/input.h], [have_ecore_fb="yes"])])
if test "${have_ecore_fb}" = "no"; then
   AC_MSG_ERROR([Missing linux/input.h or linux/fb.h])
fi

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_FB])
#### End of Ecore_FB


#### Ecore_Psl1ght
EFL_LIB_START_OPTIONAL([Ecore_Psl1ght], [test "${have_ps3}" = "yes"])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_PSL1GHT], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_PSL1GHT], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_PSL1GHT], [eina])
EFL_INTERNAL_DEPEND_PKG([ECORE_PSL1GHT], [escape])

EFL_ADD_LIBS([ECORE_PSL1GHT], [-lio -lsysutil -lgem -lcamera -lspurs])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Psl1ght])
#### End of Ecore_Psl1ght


#### Ecore_SDL
EFL_LIB_START_OPTIONAL([Ecore_SDL], [test "${want_sdl}" = "yes"])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([ECORE_SDL], [all])
EFL_INTERNAL_DEPEND_PKG([ECORE_SDL], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_SDL], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_SDL], [eina])

EFL_DEPEND_PKG([ECORE_SDL], [SDL], [sdl >= 1.2.0])

PKG_CHECK_EXISTS([sdl >= 1.3.0],
   [AC_DEFINE(BUILD_ECORE_EVAS_SDL_130, 1, [Support for SVN SDL])])

EFL_EVAL_PKGS([ECORE_SDL])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_SDL])
#### End of Ecore_SDL


#### Ecore_Wayland
EFL_LIB_START_OPTIONAL([Ecore_Wayland], [test "${want_wayland}" = "yes"])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_WAYLAND], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_WAYLAND], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_WAYLAND], [eina])

EFL_DEPEND_PKG([ECORE_WAYLAND], [WAYLAND],
   [wayland-client wayland-cursor xkbcommon])

EFL_EVAL_PKGS([ECORE_WAYLAND])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions
EFL_LIB_END_OPTIONAL([Ecore_Wayland])
#### End of Ecore_Wayland


#### Ecore_Audio

AC_ARG_ENABLE([audio],
   [AC_HELP_STRING([--disable-audio],
       [disable audio support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_audio="yes"
    else
       want_audio="no"
    fi
   ],
   [want_audio="yes"])

EFL_LIB_START_OPTIONAL([Ecore_Audio], [test "${want_audio}" = "yes"])

### Additional options to configure

# ALSA support is still not there, thus no option for it yet.
want_alsa="no"

# sndfile is mandatory otherwise it won't read from/write to files.
# TODO: if confirmed sndfile is mandatory, remove this variable
# TODO: and the EFL_OPTIONAL_DEPEND_PKG(), use EFL_DEPEND_PKG()
want_sndfile="yes"

AC_ARG_ENABLE([pulseaudio],
   [AC_HELP_STRING([--disable-pulseaudio],
       [disable pulseaudio sound support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_pulseaudio="yes"
    else
       want_pulseaudio="no"
    fi
   ],
   [want_pulseaudio="yes"])


### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_AUDIO], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_AUDIO], [eet])
EFL_INTERNAL_DEPEND_PKG([ECORE_AUDIO], [eina])

EFL_ADD_LIBS([ECORE_AUDIO], [-lm])

EFL_OPTIONAL_DEPEND_PKG([ECORE_AUDIO], [${want_alsa}], [ALSA], [alsa])
EFL_OPTIONAL_DEPEND_PKG([ECORE_AUDIO], [${want_pulseaudio}], [PULSE], [libpulse])
EFL_OPTIONAL_DEPEND_PKG([ECORE_AUDIO], [${want_sndfile}], [SNDFILE], [sndfile])

EFL_EVAL_PKGS([ECORE_AUDIO])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Audio])
#### End of Ecore_Audio

#### Ecore_Win32
EFL_LIB_START_OPTIONAL([Ecore_Win32], [test "${have_win32}" = "yes"])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([ECORE_WIN32], [evil])
EFL_INTERNAL_DEPEND_PKG([ECORE_WIN32], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_WIN32], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_WIN32], [eina])

EFL_ADD_LIBS([ECORE_WIN32], [-lole32 -lgdi32])
AC_SUBST([ECORE_WIN32_LIBS])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Win32])
#### End of Ecore_Win32


#### Ecore_WinCE
EFL_LIB_START_OPTIONAL([Ecore_WinCE], [test "${have_wince}" = "yes"])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([ECORE_WINCE], [evil])
EFL_INTERNAL_DEPEND_PKG([ECORE_WINCE], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_WINCE], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_WINCE], [eina])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_WinCE])
#### End of Ecore_WinCE


#### Ecore_X
EFL_LIB_START_OPTIONAL([Ecore_X], [test "${want_x11_any}" = "yes"])

### Additional options to configure

AC_ARG_ENABLE([gesture],
   [AC_HELP_STRING([--enable-gesture], [enable X11 Gesture extension support])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_gesture="yes"
    else
       want_gesture="no"
    fi
   ],
   [want_gesture="no"])

AC_ARG_ENABLE([xinput2],
   [AC_HELP_STRING([--disable-xinput2], [disable X11 XInput v2.x support])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_xinput2="yes"
    else
       want_xinput2="no"
    fi
   ],
   [want_xinput2="yes"])

AC_ARG_ENABLE([xinput22],
   [AC_HELP_STRING([--enable-xinput22], [enable X11 XInput v2.2+ support])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_xinput22="yes"
    else
       want_xinput22="no"
    fi
   ],
   [want_xinput22="no"])

AC_ARG_ENABLE([xim],
   [AC_HELP_STRING([--disable-xim], [disable X Input Method.])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_xim="yes"
    else
       want_xim="no"
    fi
   ],
   [want_xim="yes"])

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_X], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_X], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_X], [eina])

## Xlib

## XCB

### Checks for header files

AC_CHECK_DECL([MAXHOSTNAMELEN], [FOUND_MAXHOSTNAMELEN=yes])

if test "x${FOUND_MAXHOSTNAMELEN}" != "xyes" ; then
   FOUND_MAXHOSTNAMELEN="not found"

   AC_COMPILE_IFELSE(
      [
       AC_LANG_PROGRAM(
          [[
#include <sys/param.h>
          ]],
          [[
int h = MAXHOSTNAMELEN;
          ]])
      ],
      [
       FOUND_MAXHOSTNAMELEN="sys/param.h"
       AC_DEFINE([NEED_SYS_PARAM_H], [1], [Define to 1 if you need <sys/param.h> to define MAXHOSTNAMELEN])
      ])

   AC_COMPILE_IFELSE(
      [
       AC_LANG_PROGRAM(
          [[
#include <netdb.h>
          ]],
          [[
int h = MAXHOSTNAMELEN;
          ]])
      ],
      [
       FOUND_MAXHOSTNAMELEN="netdb.h"
       AC_DEFINE([NEED_NETDB_H], [1], [Define to 1 if you need <netdb.h> to define MAXHOSTNAMELEN])
      ])

   AC_MSG_CHECKING([for header that defines MAXHOSTNAMELEN])
   AC_MSG_RESULT([$FOUND_MAXHOSTNAMELEN])
fi

## Xlib
if test "x${want_x11_xcb}" = "xyes" ; then
   KEYSYMDEFDIR=`$PKG_CONFIG --variable=includedir xproto`/X11
   FILES="keysymdef.h XF86keysym.h Sunkeysym.h DECkeysym.h HPkeysym.h"
   for i in $FILES; do
       if test -f "$KEYSYMDEFDIR/$i"; then
         KEYSYMDEFS="$KEYSYMDEFS $KEYSYMDEFDIR/$i"
       elif test "x$i" = "xkeysymdef.h"; then
         AC_MSG_ERROR([Cannot find keysymdef.h])
       fi
   done
   AC_MSG_CHECKING([keysym definitions])
   AC_MSG_RESULT([$KEYSYMDEFS])
   AC_SUBST([KEYSYMDEFS])
fi

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

## Xlib
if test "x${want_x11_xlib}" = "xyes"; then
   EFL_FIND_X(ECORE_X_XLIB,
     [X11/Xlib.h X11/Xcursor/Xcursor.h],
     [X11 XOpenDisplay Xcursor XcursorImageLoadCursor],
     [
       use_Xcursor="yes"
       AC_DEFINE([ECORE_XCURSOR], 1, [Build support for Xcursor])
       EFL_ADD_LIBS([ECORE_X], [$ECORE_X_XLIB_libs])
       EFL_ADD_CFLAGS([ECORE_X], [$ECORE_X_XLIB_cflags])
     ],[
       AC_MSG_ERROR([Xcursor is missing])
     ])
fi

if test "x${want_x11_xlib}" = "xyes" ; then
   ECORE_CHECK_X_EXTENSION([Xkb], [XKB.h], [X11], [XkbSetDetectableAutoRepeat])
   ECORE_CHECK_X_EXTENSION([Xcomposite], [Xcomposite.h], [Xcomposite], [XCompositeQueryExtension])
   ECORE_CHECK_X_EXTENSION([Xdamage], [Xdamage.h], [Xdamage], [XDamageSubtract])
   ECORE_CHECK_X_EXTENSION([Xdpms], [dpms.h], [Xext], [DPMSQueryExtension])
   ECORE_CHECK_X_EXTENSION([Xfixes], [Xfixes.h], [Xfixes], [XFixesExpandRegion])
   ECORE_CHECK_X_EXTENSION([Xinerama], [Xinerama.h], [Xinerama], [XineramaQueryScreens])
   ECORE_CHECK_X_EXTENSION([Xprint], [Print.h], [Xp], [XpQueryScreens])
   ECORE_CHECK_X_EXTENSION([Xrandr], [Xrandr.h], [Xrandr], [XRRGetScreenResourcesCurrent])
   ECORE_CHECK_X_EXTENSION([Xrender], [Xrender.h], [Xrender], [XRenderFindVisualFormat])
   ECORE_CHECK_X_EXTENSION([Xtest], [XTest.h], [Xtst], [XTestFakeKeyEvent])
   ECORE_CHECK_X_EXTENSION([Xss], [scrnsaver.h], [Xss], [XScreenSaverSelectInput])

   if test "${want_gesture}" = "yes"; then
      ECORE_CHECK_X_EXTENSION([Xgesture], [gesture.h], [Xgesture], [XGestureQueryExtension])
   fi
   if test "${want_xinput2}" = "yes"; then
      ECORE_CHECK_X_EXTENSION([Xi2], [XInput2.h], [Xi], [XIQueryDevice])
   fi
   if test "${want_xinput22}" = "yes"; then
      ECORE_CHECK_X_EXTENSION([Xi2_2], [XInput2.h], [Xi],[XIGrabTouchBegin])
   fi

   AC_DEFINE([HAVE_ECORE_X_XLIB], [1], [Defined to 1 if Xlib is enabled.])
   HAVE_ECORE_X_BACKEND="HAVE_ECORE_X_XLIB"
fi

## XCB

if test "${want_x11_xcb}" = "yes"; then
dnl note: added pixman-1 as ecore_xcb_region uses that
   EFL_DEPEND_PKG([ECORE_X], [ECORE_X_XCB],
      [x11-xcb xcb xcb-shm xcb-event xcb-icccm >= 0.3.8 xcb-util >= 0.3.8 xcb-image xcb-keysyms >= 0.3.8 xcb-composite xcb-damage xcb-dpms xcb-randr xcb-render xcb-screensaver xcb-shape xcb-sync xcb-xfixes xcb-xinerama xcb-xprint xcb-xtest xcb-renderutil pixman-1])

dnl TODO: remove these ifdefs from code!
   AC_DEFINE([ECORE_XCB_COMPOSITE], [1], [Build support for XCB composite])
   AC_DEFINE([ECORE_XCB_DAMAGE], [1], [Build support for XCB damage])
   AC_DEFINE([ECORE_XCB_DPMS], [1], [Build support for XCB dpms])
   AC_DEFINE([ECORE_XCB_RANDR], [1], [Build support for XCB randr])
   AC_DEFINE([ECORE_XCB_RENDER], [1], [Build support for XCB render])
   AC_DEFINE([ECORE_XCB_SCREENSAVER], [1], [Build support for XCB screensaver])
   AC_DEFINE([ECORE_XCB_SHAPE], [1], [Build support for XCB shape])
   AC_DEFINE([ECORE_XCB_SYNC], [1], [Build support for XCB sync])
   AC_DEFINE([ECORE_XCB_XFIXES], [1], [Build support for XCB xfixes])
   AC_DEFINE([ECORE_XCB_XINERAMA], [1], [Build support for XCB xinerama])
   AC_DEFINE([ECORE_XCB_XPRINT], [1], [Build support for XCB xprint])
   AC_DEFINE([ECORE_XCB_XTEST], [1], [Build support for XCB xtest])
   AC_DEFINE([ECORE_XCB_CURSOR], [1], [Build support for XCB cursor])

   EFL_OPTIONAL_DEPEND_PKG([ECORE_X], [${want_gesture}], [ECORE_XCB_GESTURE],
      [xcb-gesture])
   AC_DEFINE_IF([ECORE_XCB_XGESTURE], [test "${want_gesture}" = "yes"],
      [1], [Build support for XCB xgesture])

dnl input extension disabled currently in xcb as xcb-input has some issues
dnl remember to add xcb-xinput to EFL_DEPEND_PKG()
dnl AC_DEFINE([ECORE_XCB_XINPUT], [1], [Build support for XCB input])

dnl dri extension disabled currently in xcb
dnl remember to add xcb-dri2 to EFL_DEPEND_PKG()
dnl AC_DEFINE([ECORE_XCB_DRI], [1], [Build support for XCB dri])

   EFL_EVAL_PKGS([ECORE_X])
   EFL_CHECK_FUNCS([ECORE_X], [dlopen iconv])

   HAVE_ECORE_X_BACKEND="HAVE_ECORE_X_XCB"
fi

EFL_ADD_LIBS([ECORE_X], [${ECORE_X_LIBS}])

AC_SUBST([HAVE_ECORE_X_BACKEND])

EFL_LIB_END_OPTIONAL([Ecore_X])

AM_CONDITIONAL([HAVE_ECORE_X_XLIB], [test "${want_x11_xlib}" = "yes"])
AM_CONDITIONAL([HAVE_ECORE_X_XCB], [test "${want_x11_xcb}" = "yes"])
#### End of Ecore_X


#### Ecore_Imf
EFL_LIB_START([Ecore_Imf])

### Additional options to configure

### Default values

want_ecore_imf="yes"
want_ecore_imf_xim="no"
want_ecore_imf_scim="no"
want_ecore_imf_ibus="no"

if test "${have_wince}" = "yes"; then
   want_ecore_imf="no"
else
   want_ecore_imf="yes"

   if test "${have_windows}" = "no" && test "${have_darwin}" = "no"; then
      want_ecore_imf="yes"
      want_ecore_imf_xim="yes"
      want_ecore_imf_scim="yes"
      want_ecore_imf_ibus="yes"
   fi
fi

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([ECORE_IMF], [escape])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF], [eina])

if test "x${want_ecore_imf}" = "xyes" ; then
   AC_DEFINE([HAVE_ECORE_IMF], [1], [Ecore IMF Support])
fi

## modules

# ibus

have_ecore_imf_ibus="no"
if test "x${want_ecore_imf_ibus}" = "xyes" && test "x${have_glib}" = "xyes" ; then
   PKG_CHECK_MODULES([IBUS],
      [ibus-1.0 >= 1.4 glib-2.0],
      [
       have_ecore_imf_ibus="yes"
       AC_DEFINE([BUILD_ECORE_IMF_IBUS], [1], [Ecore Imf IBUS Support])
      ],
      [have_ecore_imf_ibus="no"])
fi

AM_CONDITIONAL([BUILD_ECORE_IMF_IBUS], [test "x${have_ecore_imf_ibus}" = "xyes"])

# scim

if test "x${want_ecore_imf_scim}" = "xyes" ; then
   PKG_CHECK_MODULES([SCIM],
      [scim],
      [
       have_ecore_imf_scim="yes"
       AC_DEFINE([BUILD_ECORE_IMF_SCIM], [1], [Ecore Imf SCIM Support])
      ],
      [have_ecore_imf_scim="no"])
fi

AM_CONDITIONAL([BUILD_ECORE_IMF_SCIM], [test "x${have_ecore_imf_scim}" = "xyes"])

# xim

have_ecore_imf_xim="no"
if test "x${want_xim}" = "xyes" && test "x${want_ecore_imf_xim}" = "xyes" ; then

 EFL_FIND_X(ecore_imf_xim,
   [X11/Xlib.h],
   [X11 XOpenIM],
   [
     have_ecore_imf_xim=yes
     AC_DEFINE([ENABLE_XIM], [1], [Enable X Input Method])
   ])
fi

AM_CONDITIONAL([BUILD_ECORE_IMF_XIM], [test "x${have_ecore_imf_xim}" = "xyes"])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Ecore_Imf])
#### End of Ecore_Imf


#### Ecore_Imf_Evas
EFL_LIB_START([Ecore_Imf_Evas])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([ECORE_IMF_EVAS], [escape])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF_EVAS], [ecore-imf])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF_EVAS], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF_EVAS], [evas])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF_EVAS], [eina])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Ecore_Imf_Evas])
#### End of Ecore_Imf_Evas


#### Ecore_Evas
EFL_LIB_START([Ecore_Evas])

### Additional options to configure

### Default values

have_ecore_evas="yes"
have_extn="yes"

want_ecore_evas_software_gdi="${have_evas_engine_software_gdi}"
want_ecore_evas_software_ddraw="${have_evas_engine_software_ddraw}"
want_ecore_evas_gl_cocoa="${have_evas_engine_gl_cocoa}"
want_ecore_evas_wayland_egl="${have_evas_engine_wayland_egl}"
want_ecore_evas_extn="yes"

if test "${have_windows}" = "yes"; then
   want_ecore_evas_extn="no"
fi

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [ecore-input-evas])
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [evas])
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [eina])

## modules

ECORE_EVAS_MODULE([buffer], [static])
ECORE_EVAS_MODULE([ews], [yes])
ECORE_EVAS_MODULE([fb], [${want_fb}])
ECORE_EVAS_MODULE([psl1ght], [${have_ps3}])

ECORE_EVAS_MODULE([opengl-cocoa], [${want_ecore_evas_gl_cocoa}])

ECORE_EVAS_MODULE([software-sdl], [${want_sdl}])
ECORE_EVAS_MODULE([opengl-sdl], [${want_gl_sdl}])

build_ecore_evas_sdl="no"
if test "x${have_ecore_evas_software_sdl}" = "xyes" || \
   test "x${have_ecore_evas_opengl_sdl}" = "xyes" ; then
   build_ecore_evas_sdl="yes"
   AC_DEFINE(BUILD_ECORE_EVAS_SDL, 1, [Support for SDL Engine in Ecore_Evas])
fi
AM_CONDITIONAL([BUILD_ECORE_EVAS_SDL],
   [test "${build_ecore_evas_sdl}" = "yes"])

ECORE_EVAS_MODULE([wayland-shm], [${want_wayland}])
ECORE_EVAS_MODULE([wayland-egl], [${want_ecore_evas_wayland_egl}])

build_ecore_evas_wayland="no"
if test "x${have_ecore_evas_wayland_shm}" = "xyes" || \
   test "x${have_ecore_evas_wayland_egl}" = "xyes" ; then
   build_ecore_evas_wayland="yes"
   AC_DEFINE(BUILD_ECORE_EVAS_WAYLAND, 1, [Support for Wayland Engine in Ecore_Evas])
fi
AM_CONDITIONAL([BUILD_ECORE_EVAS_WAYLAND],
   [test "${build_ecore_evas_wayland}" = "yes"])

ECORE_EVAS_MODULE([software-gdi], [${want_ecore_evas_software_gdi}])
ECORE_EVAS_MODULE([software-ddraw], [${want_ecore_evas_software_ddraw}])

build_ecore_evas_win32="no"
if test "x${have_ecore_evas_software_gdi}" = "xyes" || \
   test "x${have_ecore_evas_software_ddraw}" = "xyes" ; then
   build_ecore_evas_win32="yes"
   AC_DEFINE(BUILD_ECORE_EVAS_WIN32, 1, [Support for Win32 Engine in Ecore_Evas])
fi
AM_CONDITIONAL([BUILD_ECORE_EVAS_WIN32],
   [test "${build_ecore_evas_win32}" = "yes"])

# XXX TODO: ecore_evas_extn

if test "x${have_ecore_ipc}" = "xno" || \
   test "x${have_ecore_evas_software_buffer}" = "xno" || \
   test "x${have_shm_open}" = "xno" || \
   test "x${have_windows}" = "xyes" ; then
   have_extn="no"
fi

dnl THIS IS SPECIAL - dont use normal ECORE_EVAS_CHECK_MODULE
have_ecore_evas_extn="no"
if test "x${want_ecore_evas_extn}" = "xyes" && \
   test "x${have_extn}" = "xyes" && \
   test "x${have_ecore_evas}" = "xyes"; then
   have_ecore_evas_extn="yes"
   AC_DEFINE([BUILD_ECORE_EVAS_EXTN], [1], [Support for Extn Engine in Ecore_Evas])
fi


# XXX TODO: ecore_evas_x11

ECORE_EVAS_MODULE([software-x11], [${want_x11_any}])

have_ecore_evas_software_xlib="no"
have_ecore_evas_software_xcb="no"
if test "x$have_ecore_evas_software_x11" = "xyes" ; then
   have_ecore_evas_software_xlib=${have_evas_engine_software_xlib}
   if test "x${have_ecore_evas_software_xlib}" = "xstatic"; then
      have_ecore_evas_software_xlib="yes"
   fi
   if test "x${have_ecore_evas_software_xlib}" = "xyes"; then
      AC_DEFINE([BUILD_ECORE_EVAS_SOFTWARE_XLIB], [1], [Evas Software Xlib Engine Support])
   fi
   have_ecore_evas_software_xcb=${have_evas_engine_software_xcb}
   if test "x$have_ecore_evas_software_xcb" = "xstatic"; then
      have_ecore_evas_software_xcb="yes"
   fi
   if test "x$have_ecore_evas_software_xcb" = "xyes"; then
      AC_DEFINE([BUILD_ECORE_EVAS_SOFTWARE_XCB], [1], [Evas Software XCB Engine Support])
   fi
fi

# XXX TODO: ecore_evas_opengl_x11

ECORE_EVAS_MODULE([opengl-x11], [${want_x11_any_opengl}])

have_ecore_evas_opengl_xlib="no"
have_ecore_evas_opengl_xcb="no"
if test "x${have_ecore_evas_opengl_x11}" = "xyes" || test "x${have_ecore_evas_opengl_x11}" = "xstatic" ; then
   have_ecore_evas_opengl_xlib=${have_evas_engine_gl_xlib}
   if test "x${have_ecore_evas_opengl_xlib}" = "xyes" ; then
      AC_DEFINE([BUILD_ECORE_EVAS_OPENGL_XLIB], [1], [OpenGL Xlib rendering backend])
   fi

# opengl does not work with xcb (yet)
   have_ecore_evas_opengl_xcb=${have_evas_engine_gl_xcb}
   if test "x${have_ecore_evas_opengl_xcb}" = "xstatic"; then
      have_ecore_evas_opengl_xcb="yes"
   fi
   if test "x${have_ecore_evas_opengl_xcb}" = "xyes"; then
      PKG_CHECK_MODULES([XCB_X11],
         [x11-xcb],
         [
          have_ecore_x_opengl_xcb="yes"
          requirements_ecore_x="x11-xcb ${requirements_ecore_x}"
          AC_DEFINE([BUILD_ECORE_X_OPENGL_XCB], [1], [Build support for XCB-based OpenGL])
          AC_DEFINE([BUILD_ECORE_EVAS_OPENGL_XCB], [1], [OpenGL XCB rendering backend])
         ],
         [have_ecore_x_opengl_xcb="no"])
    else
      have_ecore_x_opengl_xcb="no"
      AC_MSG_NOTICE(["XCB-based OpenGL explicitly disabled"])
    fi
fi

build_ecore_evas_x11="no"
if test "x$have_ecore_evas_software_x11" = "xyes" || \
   test "x$have_ecore_evas_opengl_x11" = "xyes" || \
   test "x$have_ecore_evas_software_xcb" = "xyes"; then
   AC_DEFINE([BUILD_ECORE_EVAS_X11], [1], [Support for X Window Engines in Ecore_Evas])
   build_ecore_evas_x11="yes"
fi
AM_CONDITIONAL([BUILD_ECORE_EVAS_X11], [test "${build_ecore_evas_x11}" = "yes"])

EFL_EVAL_PKGS([ECORE_EVAS])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Ecore_Evas])
#### End of Ecore_Evas

#### Eio
EFL_LIB_START([Eio])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([EIO], [ecore])
EFL_INTERNAL_DEPEND_PKG([EIO], [eet])
EFL_INTERNAL_DEPEND_PKG([EIO], [eina])

EFL_ADD_LIBS([EIO], [-lm])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Eio])
dnl TODO: remove these ifdefs from code!
AC_DEFINE([HAVE_EIO], [1], [Have eio library])
#### End of Eio


#### EDBus
EFL_LIB_START([EDBus])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([EDBUS], [ecore])
EFL_INTERNAL_DEPEND_PKG([EDBUS], [eina])

EFL_DEPEND_PKG([EDBUS], [DBUS], [dbus-1])

EFL_EVAL_PKGS([EDBUS])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([EDBus])
#### End of EDBus


#### Efreet
EFL_LIB_START([Efreet])

### Additional options to configure

### Default values

AC_DEFINE([SLOPPY_SPEC], [1], [Sloppy Spec Compliance])

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([EFREET], [eet])
EFL_INTERNAL_DEPEND_PKG([EFREET], [ecore])
EFL_INTERNAL_DEPEND_PKG([EFREET], [ecore-file])
EFL_INTERNAL_DEPEND_PKG([EFREET], [edbus2])
EFL_INTERNAL_DEPEND_PKG([EFREET], [eina])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Efreet])
#### End of Efreet

#### Eeze
EFL_LIB_START_OPTIONAL([Eeze], [test "${have_linux}" = "yes"])

### Additional options to configure
AC_ARG_ENABLE([tizen],
   [AC_HELP_STRING([--enable-tizen],
       [enable tizen support. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_tizen="yes"
    else
       want_tizen="no"
    fi
   ],
   [want_tizen="no"])

AC_ARG_ENABLE([mount],
   [AC_HELP_STRING([--disable-mount],
       [disable mount support (libmount). @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_mount="yes"
    else
       want_mount="no"
    fi
   ],
   [want_mount="yes"])

AC_ARG_WITH([mount],
  [AS_HELP_STRING([--with-mount], [specify mount bin @<:@default=detect@:>@])],
  [with_eeze_mount=$withval], [with_eeze_mount="detect"])
AC_ARG_WITH([umount],
  [AS_HELP_STRING([--with-umount], [specify umount bin @<:@default=detect@:>@])],
  [with_eeze_umount=$withval], [with_eeze_umount="detect"])
AC_ARG_WITH([eject],
  [AS_HELP_STRING([--with-eject], [specify eject bin @<:@default=detect@:>@])],
  [with_eeze_eject=$withval], [with_eeze_eject="detect"])

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([EEZE], [eina])
EFL_INTERNAL_DEPEND_PKG([EEZE], [ecore])
EFL_OPTIONAL_INTERNAL_DEPEND_PKG([EEZE], [${want_mount}], [ecore-file])
EFL_OPTIONAL_INTERNAL_DEPEND_PKG([EEZE], [${want_mount}], [ecore-con])
EFL_OPTIONAL_INTERNAL_DEPEND_PKG([EEZE], [${want_mount}], [eet])

EFL_DEPEND_PKG([EEZE], [UDEV], [libudev >= 148])
EFL_OPTIONAL_DEPEND_PKG([EEZE], [${want_mount}], [EEZE_MOUNT], [mount >= 2.20.0])

## modules
if test "${want_tizen}"  = "yes"; then
  PKG_CHECK_MODULES([TIZEN_SENSOR], [capi-system-sensor])
fi

EFL_EVAL_PKGS([EEZE])

AM_CONDITIONAL([HAVE_EEZE_MOUNT], [test "${want_mount}" = "yes"])
AM_CONDITIONAL([HAVE_EEZE_TIZEN], [test "${want_tizen}" = "yes"])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Checks for binaries
if test "x$with_eeze_mount" = "xdetect"; then
  AC_PATH_PROG([with_eeze_mount], [mount], [])
fi
AC_DEFINE_UNQUOTED([EEZE_MOUNT_BIN], ["$with_eeze_mount"], [mount bin to use])

if test "x$with_eeze_umount" = "xdetect";then
  AC_PATH_PROG([with_eeze_umount], [umount], [])
fi
AC_DEFINE_UNQUOTED([EEZE_UNMOUNT_BIN], ["$with_eeze_umount"], [umount bin to use])

if test "x$with_eeze_eject" = "xdetect";then
  AC_PATH_PROG([with_eeze_eject], [eject], [])
fi
AC_DEFINE_UNQUOTED([EEZE_EJECT_BIN], ["$with_eeze_eject"], [eject bin to use])

EFL_LIB_END_OPTIONAL([Eeze])
#### End of Eeze


#### EPhysics
AC_ARG_ENABLE([physics],
   [AC_HELP_STRING([--disable-physics],
       [disable physics effects and support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_physics="yes"
    else
       want_physics="no"
    fi
   ],
   [want_physics="yes"])

EFL_LIB_START_OPTIONAL([EPhysics], [test "${want_physics}" = "yes"])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([EPHYSICS], [evil])

EFL_INTERNAL_DEPEND_PKG([EPHYSICS], [eina])
EFL_INTERNAL_DEPEND_PKG([EPHYSICS], [evas])
EFL_INTERNAL_DEPEND_PKG([EPHYSICS], [ecore])

EFL_DEPEND_PKG([EPHYSICS], [BULLET], [bullet >= 2.80])

EFL_EVAL_PKGS([EPHYSICS])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([EPhysics])
#### End of EPhysics


#### Edje
EFL_LIB_START([Edje])

### Additional options to configure

### Default values
want_multisense="no" # TODO: move to ecore_audio and enable

# TODO: should we keep or remove these?
want_edje_program_cache="no"
want_edje_calc_cache="yes"
want_fixed_point="no"

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([EDJE], [evil])

EFL_INTERNAL_DEPEND_PKG([EDJE], [eina])
EFL_INTERNAL_DEPEND_PKG([EDJE], [eo])
EFL_INTERNAL_DEPEND_PKG([EDJE], [eet])
EFL_INTERNAL_DEPEND_PKG([EDJE], [evas])
EFL_INTERNAL_DEPEND_PKG([EDJE], [ecore])
EFL_INTERNAL_DEPEND_PKG([EDJE], [ecore-evas])
EFL_INTERNAL_DEPEND_PKG([EDJE], [ecore-file])
EFL_INTERNAL_DEPEND_PKG([EDJE], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([EDJE], [ecore-imf])
EFL_INTERNAL_DEPEND_PKG([EDJE], [ecore-imf-evas])
EFL_INTERNAL_DEPEND_PKG([EDJE], [embryo])
EFL_INTERNAL_DEPEND_PKG([EDJE], [eio])

EFL_OPTIONAL_INTERNAL_DEPEND_PKG([EDJE], [${want_physics}], [ephysics])
#EFL_OPTIONAL_INTERNAL_DEPEND_PKG([EDJE], [${want_multisense}], [ecore-audio])

EFL_CHECK_LUA_5_1([EDJE])
EFL_ADD_LIBS([EDJE], [-lm])

EFL_EVAL_PKGS([EDJE])

AC_DEFINE_IF([EDJE_PROGRAM_CACHE], [test "${want_edje_program_cache}" = "yes"],
   [1], [Cache result of program glob matches])
AC_DEFINE_IF([EDJE_CALC_CACHE], [test "${want_edje_calc_cache}" = "yes"],
   [1], [Cache result of calc glob matches])
AC_DEFINE_IF([BUILD_EDJE_FP], [test "${want_fixed_point}" = "yes"],
   [1], [Use Fixed Point instead of FPU])

AM_CONDITIONAL([ENABLE_MULTISENSE], [test "${want_multisense}" = "yes"])
AC_DEFINE_IF([ENABLE_MULTISENSE], [test "${want_multisense}" = "yes"],
  [1], [Use Multisense])
AC_SUBST([want_multisense])
AC_SUBST([want_physics])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Edje])
#### End of Edje

#### Emotion
EFL_LIB_START([Emotion])

### Default values
if test "${efl_func_shm_open}" = "yes"; then
   want_emotion_generic="static"
else
   want_emotion_generic="no"
fi

### Additional options to configure
AC_ARG_ENABLE([xine],
   [AC_HELP_STRING([--enable-xine],
                   [enable xine support. @<:@default=disabled@:>@])],
   [
    if test "x${withval}" = "xyes" ; then
       want_xine="yes"
    else
       want_xine="no"
    fi
   ],
   [want_xine="no"])

AC_ARG_ENABLE([v4l2],
   [AC_HELP_STRING([--enable-v4l2], [enable v4l2 support.])],
   [
    if test "x${withval}" = "xyes" ; then
       want_v4l2="yes"
    else
       want_v4l2="no"
    fi
   ],
   [want_v4l2="${efl_lib_optional_eeze}"])

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([EMOTION], [eina])
EFL_INTERNAL_DEPEND_PKG([EMOTION], [eo])
EFL_INTERNAL_DEPEND_PKG([EMOTION], [ecore])
EFL_INTERNAL_DEPEND_PKG([EMOTION], [eet])
EFL_INTERNAL_DEPEND_PKG([EMOTION], [evas])
EFL_INTERNAL_DEPEND_PKG([EMOTION], [eio])

EFL_OPTIONAL_INTERNAL_DEPEND_PKG([EMOTION], [${want_v4l2}], [eeze])
AM_CONDITIONAL([EMOTION_HAVE_V4L2], [test "${want_v4l2}" = "yes"])

## modules

have_gst_xoverlay="no"

EMOTION_MODULE([xine], [${want_xine}])
EMOTION_MODULE([gstreamer], [${want_gstreamer}])
EMOTION_MODULE([generic], [${want_emotion_generic}])

EFL_EVAL_PKGS([EMOTION])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

if test "${want_v4l2}" = "yes"; then
   AC_CHECK_DECL([V4L2_CAP_VIDEO_CAPTURE],
      [AC_DEFINE([HAVE_V4L2], [1], [Define to 1 if you have Video4Linux 2 available])],
      [AC_MSG_ERROR([Video4Linux 2 desired but not found. See --disable-v4l2.])],
      [#include <linux/videodev2.h>])
fi

### Check availability

EFL_LIB_END([Emotion])
#### End of Emotion


AC_ARG_ENABLE([always-build-examples],
   [AC_HELP_STRING([--enable-always-build-examples],
       [always build examples. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_always_build_examples="yes"
    else
       want_always_build_examples="no"
    fi
   ],
   [want_always_build_examples="no"])
AM_CONDITIONAL([ALWAYS_BUILD_EXAMPLES], [test "${want_always_build_examples}" = "yes"])


AC_CONFIG_FILES([
Makefile
data/Makefile
doc/Makefile
doc/Doxyfile
src/Makefile
src/benchmarks/Makefile
src/benchmarks/eina/Makefile
src/benchmarks/eo/Makefile
src/examples/Makefile
src/examples/eina/Makefile
src/examples/eet/Makefile
src/examples/eo/Makefile
src/examples/evas/Makefile
src/examples/ecore/Makefile
src/examples/eio/Makefile
src/examples/edbus/Makefile
src/examples/ephysics/Makefile
src/examples/edje/Makefile
src/examples/emotion/Makefile
src/lib/eina/eina_config.h
src/lib/ecore_x/ecore_x_version.h
spec/efl.spec
pc/evil.pc
pc/escape.pc
pc/eina.pc
pc/eet.pc
pc/eo.pc
pc/evas-fb.pc
pc/evas-opengl-x11.pc
pc/evas-opengl-sdl.pc
pc/evas-opengl-cocoa.pc
pc/evas-psl1ght.pc
pc/evas-software-buffer.pc
pc/evas-software-x11.pc
pc/evas-software-gdi.pc
pc/evas-software-ddraw.pc
pc/evas-software-sdl.pc
pc/evas-wayland-shm.pc
pc/evas-wayland-egl.pc
pc/evas.pc
pc/ecore.pc
pc/ecore-con.pc
pc/ecore-ipc.pc
pc/ecore-file.pc
pc/ecore-input.pc
pc/ecore-input-evas.pc
pc/ecore-cocoa.pc
pc/ecore-fb.pc
pc/ecore-psl1ght.pc
pc/ecore-sdl.pc
pc/ecore-wayland.pc
pc/ecore-win32.pc
pc/ecore-wince.pc
pc/ecore-x.pc
pc/ecore-evas.pc
pc/ecore-imf.pc
pc/ecore-imf-evas.pc
pc/ecore-audio.pc
pc/embryo.pc
pc/eio.pc
pc/edbus2.pc
pc/efreet.pc
pc/efreet-mime.pc
pc/efreet-trash.pc
pc/eeze.pc
pc/ephysics.pc
pc/edje.pc
pc/emotion.pc
dbus-services/org.enlightenment.Efreet.service
$po_makefile_in
])

AC_OUTPUT


#### Info

cpu_summary="$host_cpu ("
case $host_cpu in
  i*86|x86_64|amd64)
    if test "x${build_cpu_mmx}" = "xyes"; then
       cpu_summary="${cpu_summary}+mmx"
    else
       cpu_summary="${cpu_summary}-mmx"
    fi

    cpu_summary="${cpu_summary} "

    if test "x${build_cpu_sse3}" = "xyes"; then
       cpu_summary="${cpu_summary}+sse3"
    else
       cpu_summary="${cpu_summary}-sse3"
    fi
    ;;
  *power* | *ppc*)
    if test "x${build_cpu_altivec}" = "xyes"; then
       cpu_summary="${cpu_summary}+altivec"
    else
       cpu_summary="${cpu_summary}-altivec"
    fi
    ;;
  arm*)
    if test "x${build_cpu_neon}" = "xyes"; then
       cpu_summary="${cpu_summary}+neon"
    else
       cpu_summary="${cpu_summary}-neon"
    fi
    ;;
esac
cpu_summary="${cpu_summary})"

sys_features=""
if test "x${have_inotify}" = "xyes"; then
   sys_features="${sys_features} +inotify"
else
   sys_features="${sys_features} -inotify"
fi
if test "x${have_notify_win32}" = "xyes"; then
   sys_features="${sys_features} +notify_win32"
else
   sys_features="${sys_features} -notify_win32"
fi
if test "x${have_atfile_source}" = "xyes"; then
   sys_features="${sys_features} +atfile"
else
   sys_features="${sys_features} -atfile"
fi
if test "x${have_ipv6}" = "xyes"; then
   sys_features="${sys_features} +ipv6"
else
   sys_features="${sys_features} -ipv6"
fi

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
echo "  Build Profile........: ${build_profile}"
echo "  CPU Extensions.......: ${cpu_summary}"
echo "  System Features......:${sys_features}"
echo "  Threads Type.........: ${efl_have_threads}"
echo "    spinlocks..........: ${efl_have_posix_threads_spinlock}"
echo "    barrier............: ${efl_have_pthread_barrier}"
echo "    affinity...........: ${efl_have_setaffinity}"
echo "  Cryptographic System.: ${build_crypto}"
echo

echo "Evas:"
echo
echo "  Engines:"
echo "    Software X11...............: ${with_x11}"
echo "    OpenGL X11.................: ${with_x11} (opengl=${with_opengl})"
echo "    Software GDI...............: $have_evas_engine_software_gdi"
echo "    Software DirectDraw........: $have_evas_engine_software_ddraw"
echo "    OpenGL SDL.................: $have_evas_engine_gl_sdl (opengl=${with_opengl})"
echo "    OpenGL Cocoa...............: $have_evas_engine_gl_cocoa"
echo "    Software Framebuffer.......: $have_evas_engine_fb"
echo "    PSL1GHT....................: $have_ps3"
echo "    Wayland Shm................: $have_evas_engine_wayland_shm"
echo "    Wayland Egl................: $have_evas_engine_wayland_egl"
echo
echo "  Image Loaders:"
echo "    JPEG region decode......: $have_jpeg_region"
echo "    WEBP....................: $have_evas_image_loader_webp"
echo "    GIF.....................: $have_evas_image_loader_gif"
echo "    TIFF....................: $have_evas_image_loader_tiff"
echo "    SVG.....................: $have_evas_image_loader_svg"
echo
echo "  Font Searching Systems:"
echo "    Fontconfig..............: $have_fontconfig"
echo
echo "  Font Rendering Helpers:"
echo "    Fribidi.................: $have_fribidi"
echo "    Harfbuzz................: $have_harfbuzz"
# FIXME: add non freetype2 font engine support
# FIXME: make freetype2 optional
echo
echo
echo "  Features:"
echo "    Cache Server 2..........: $want_evas_cserve2"
echo
echo "  Optional pixman rendering path:"
echo "    Pixman..................: $have_pixman"
echo "    Pixman Fonts............: $have_pixman_font"
echo "    Pixman Rects............: $have_pixman_rect"
echo "    Pixman Lines............: $have_pixman_line"
echo "    Pixman Polygons.........: $have_pixman_poly"
echo "    Pixman Images...........: $have_pixman_image"
echo "    Pixman Image ScaleSample: $have_pixman_image_scale_sample"
echo
echo "  Conversion Options:"
echo "    Dither Mask.............: $build_evas_dither_mask"
echo "    Tiled 32BPP rotate......: $have_tile_rotate"
echo

echo "Ecore:"
echo "  GLib support...............: $with_glib"
echo "  Use g_main_loop............: $want_g_main_loop"
echo "  Gathering memory statistic.: $ac_cv_func_mallinfo"
echo "  Gathering timer allocation.: $want_ecore_timer_dump"
echo

echo "Ecore_Con:"
echo "  CURL.......................: $have_curl"
echo "  Local Sockets..............: $want_ecore_con_local_sockets"
if test "x$want_ecore_con_local_sockets" = "xyes" ; then
echo "  Abstract Sockets...........: $want_ecore_con_abstract_sockets"
fi
if test "x$have_cares" = "xyes" ; then
echo "  Resolver...................: c-ares"
elif test "x$have_ipv6" = "xyes" ; then
echo "  Resolver...................: dns.c"
else
echo "  Resolver...................: fork"
fi
echo

echo "Ecore_File:"
echo "  CURL.......................: $have_curl"
echo

echo "Ecore_IMF....................: $want_ecore_imf"
echo "  XIM........................: $have_ecore_imf_xim"
echo "  SCIM.......................: $have_ecore_imf_scim"
echo "  IBUS.......................: $have_ecore_imf_ibus"
echo

echo "Ecore Graphic systems:"
echo

echo "  Ecore_X....................: ${with_x11} (xgesture=${want_gesture}) (xi2=${want_xinput2}) (xi2.2=${want_xinput22})"
echo "  Ecore_Win32................: $have_win32"
echo "  Ecore_Cocoa................: $have_ecore_cocoa"
echo "  Ecore_SDL..................: $want_sdl"
echo "  Ecore_FB...................: $want_fb (tslib=${have_tslib})"
echo "  Ecore_WinCE................: $have_wince"
echo "  Ecore_PSL1GHT..............: $have_ps3"
echo "  Ecore_Wayland..............: $want_wayland"
if test "${want_audio}" = "yes"; then
  echo "  Ecore_Audio................: yes (alsa=${have_alsa}, pulseaudio=${have_pulse}, sndfile=${have_sndfile})"
else
  echo "  Ecore_Audio................: no"
fi

echo "  Ecore_Evas...................: $have_ecore_evas"
#if test "x${have_ecore_evas}" = "xyes" ; then
  echo "    Software X11...............: ${with_x11}"
  echo "    OpenGL X11.................: ${with_x11} (opengl=${with_opengl})"
  echo "    Software GDI...............: $have_ecore_evas_software_gdi"
  echo "    Software DirectDraw........: $have_ecore_evas_software_ddraw"
  echo "    Software SDL...............: $have_ecore_evas_software_sdl"
  echo "    OpenGL SDL.................: $have_ecore_evas_opengl_sdl"
  echo "    OpenGL Cocoa...............: $have_ecore_evas_opengl_cocoa"
  echo "    PSL1GHT....................: $have_ps3"
  echo "    Software Framebuffer.......: $have_ecore_evas_fb"
  echo "    Sing.Proc. Windowing System: $have_ecore_evas_ews"
  echo "    Wayland Shm................: $have_ecore_evas_wayland_shm"
  echo "    Wayland Egl................: $have_ecore_evas_wayland_egl"
  echo "    Extn (Plug/socket Extn)....: $have_ecore_evas_extn"
#fi
echo

echo "Eeze...................: ${efl_lib_optional_eeze} (tizen: ${want_tizen})"
echo "EPhysics...............: ${efl_lib_optional_ephysics}"
echo "Edje...................: yes (physics=${efl_lib_optional_ephysics}, multisense=${want_multisense})"
echo "Emotion................: yes (gstreamer=${have_emotion_module_gstreamer} (xoverlay=${have_gst_xoverlay}), xine=${have_emotion_module_xine}, generic=${have_emotion_module_generic}, v4l2=${want_v4l2})"
echo "Tests..................: ${build_tests}"

echo "Examples...............: make examples"
echo "    installation.......: make install-examples"
if test "x${build_doc}" = "xyes"; then
echo "Documentation..........: make doc"
else
echo "Documentation..........: no"
fi
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

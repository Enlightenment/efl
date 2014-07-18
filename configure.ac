EFL_VERSION([1], [10], [99], [dev])
AC_INIT([efl], [efl_version], [enlightenment-devel@lists.sourceforge.net])

AC_PREREQ([2.60])
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

AM_INIT_AUTOMAKE([1.6 dist-bzip2 dist-xz -Wall -Wno-portability color-tests])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

CFOPT_WARNING=""

#### Apply configuring with legacy api's only, eo api's or both.

EFL_API_LEGACY_DEF="#define EFL_NOLEGACY_API_SUPPORT"
EFL_API_EO_DEF="#define EFL_EO_API_SUPPORT"
efl_api="both"

#### For the moment the Eo EFL API is not ready you need to explicitely optin.
EFL_ENABLE_EO_API_SUPPORT
EFL_ENABLE_BETA_API_SUPPORT

AC_ARG_WITH([api],
   [AS_HELP_STRING([--with-api=@<:@eo/legacy/both@:>@],[Select the EFL API Model @<:@default=both@:>@])],
   [efl_api=${withval}],
   [efl_api="both"])

case "$efl_api" in
    eo)
        EFL_API_LEGACY_DEF="#define EFL_NOLEGACY_API_SUPPORT"
        EFL_API_EO_DEF="#define EFL_EO_API_SUPPORT"
        ;;
    legacy)
        EFL_API_LEGACY_DEF=""
        EFL_API_EO_DEF=""
        ;;
    both)
        EFL_API_LEGACY_DEF=""
        EFL_API_EO_DEF="#define EFL_EO_API_SUPPORT"
        ;;
    *) AC_MSG_ERROR([Invalid api (${efl_api}): must be eo, legacy or both]) ;;
esac

AC_SUBST(EFL_API_LEGACY_DEF)
AC_SUBST(EFL_API_EO_DEF)

#### Additional options to configure

# string to identify the build
AC_ARG_WITH([id],
   [AS_HELP_STRING([--with-id=BUILD_ID],[Specify a string to identify the build (vendor, maintainer, etc).
                     @<:@default=none@:>@])],
   [EFL_BUILD_ID="${withval}"],
   [EFL_BUILD_ID="none"])
AC_SUBST(EFL_BUILD_ID)

# profile
AC_ARG_WITH([profile],
   [AS_HELP_STRING([--with-profile=PROFILE],[use the predefined build profile, one of: dev, debug and release.
                    @<:@default=dev@:>@])],
   [build_profile=${withval}],
   [build_profile=def_build_profile])

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
   [AS_HELP_STRING([--with-crypto=CRYPTO],[use the predefined build crypto, one of:
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
   [AS_HELP_STRING([--with-tests=none|regular|coverage],[choose testing method: regular, coverage or none.
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
   no*)
     ;;
   *)
     AC_MSG_ERROR([Unknown build tests option: --with-tests=${build_tests}])
     ;;
esac

dbusservicedir="${datadir}/dbus-1/services"
AC_ARG_WITH([dbus-services],
   [AS_HELP_STRING([--with-dbus-services=DBUS_SERVICES],[specify a directory to store dbus service files.])],
   [dbusservicedir=$withval])
AC_SUBST(dbusservicedir)

efl_deprecated_option="no"
EFL_WITH_BIN([edje], [edje-cc])
EFL_WITH_BIN([eolian], [eolian-gen])
EFL_WITH_BIN([eolian_cxx], [eolian-cxx])
EFL_WITH_BIN_SUFFIX([elua], [elua], [_bin])

#### Default values

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
   cegcc*|mingw32ce*)
      AC_MSG_ERROR([ceGCC compiler is not supported anymore. Exiting...])
   ;;
   mingw*)
      have_win32="yes"
      have_windows="yes"
      MODULE_ARCH="v-v_maj.v_min"
      MODULE_EXT=".dll"
   ;;
   cygwin*)
      MODULE_ARCH="v-v_maj.v_min"
      MODULE_EXT=".dll"
   ;;
   *)
      MODULE_ARCH="v-v_maj.v_min"
      MODULE_EXT=".so"
   ;;
esac

EFL_VERSION_MAJOR="v_maj"
EFL_VERSION_MINOR="v_min"
AC_SUBST(EFL_VERSION_MAJOR)
AC_SUBST(EFL_VERSION_MINOR)

have_systemd_pkg="no"
have_win32="no"
have_windows="no"
have_darwin="no"
have_linux="no"
have_ps3="no"
case "$host_os" in
   mingw*|cygwin*)
      # TODO: check cygwin* here
      have_win32="yes"
      have_windows="yes"
      EFL_CFLAGS="${EFL_CFLAGS} -D__USE_MINGW_ANSI_STDIO"
   ;;
   darwin*)
      have_darwin="yes"
   ;;
   linux*)
      have_linux="yes"
      have_systemd_pkg="auto"
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
AM_CONDITIONAL([HAVE_WIN32], [test "x${have_win32}" = "xyes"])
AM_CONDITIONAL([HAVE_WINDOWS], [test "x${have_windows}" = "xyes"])
AM_CONDITIONAL([HAVE_PS3], [test "x${have_ps3}" = "xyes"])


#### Checks for programs

### libtool

if test "x${have_windows}" = "xyes" ; then
   lt_cv_deplibs_check_method='pass_all'
fi
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT([win32-dll disable-static pic-only])
EFL_INIT

### gettext

AM_GNU_GETTEXT_VERSION([0.17])

m4_ifdef([AC_GNU_GETTEXT], [
AC_GNU_GETTEXT([external])
po_makefile_in=po/Makefile.in
po_makevars="${srcdir}/po/Makevars:po/Makevars.in"
have_po="yes"
],
[
m4_ifdef([AM_GNU_GETTEXT], [
AM_GNU_GETTEXT([external])
po_makefile_in=po/Makefile.in
po_makevars="${srcdir}/po/Makevars:po/Makevars.in"
have_po="yes"
],
[
have_po="no"
])
])
AC_SUBST([LTLIBINTL])
LOCALE_DIR="${localedir}"
AC_SUBST(LOCALE_DIR)

if test "x${POSUB}" = "x" ; then
   have_po="no"
fi

AM_CONDITIONAL([HAVE_PO], [test "x${have_po}" = "xyes"])

### compilers

AM_PROG_AS
AC_PROG_CXX
AC_PROG_OBJC
AC_LANG(C)
AC_PROG_CC_C99
AM_PROG_CC_C_O
AC_PROG_SED

AM_CONDITIONAL([BUILD_EFL_NATIVE], [test "x${cross_compiling}" = "xno"])

if test "x${ac_cv_prog_cc_c99}" = "xno" ; then
   AC_MSG_ERROR([efl requires a c99-capable compiler])
fi
# We should be using ${CXX} here, but there is a bug in
# autotools macro and CXX is always set to g++ even if
# it's not found. So we are using an internal variable
# that does the work for now, may get broken in the future.
if test "x${ac_ct_CXX}" = "x" -a "x${CXX}" = "xg++"; then
   AC_MSG_ERROR([efl requires a C++ compiler got ${ac_ct_CXX} and ${CXX}.])
fi

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

#### Checks for libraries

# check unit testing library

if test "${want_tests}" = "yes"; then
   PKG_CHECK_MODULES([CHECK], [check >= 0.9.5])
fi
AM_CONDITIONAL([EFL_ENABLE_TESTS], [test "${want_tests}" = "yes"])


# check for crypto/tls library to use
case "$build_crypto" in
   gnutls)
      CFOPT_WARNING="xyes"
      EFL_DEPEND_PKG([crypto], [GNUTLS], [gnutls >= 2.12.16])

      AM_PATH_LIBGCRYPT([], [:],
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

# check for lua old
want_lua_old="no"
AC_ARG_ENABLE([lua-old],
   [AS_HELP_STRING([--enable-lua-old],[Enable interpreted Lua support (5.1 or 5.2). @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_lua_old="yes"
    else
       want_lua_old="no"
    fi
   ],
   [want_lua_old="no"])

AM_CONDITIONAL([ENABLE_LUA_OLD], [test "${want_lua_old}" = "yes"])
AC_DEFINE_IF([ENABLE_LUA_OLD], [test "${want_lua_old}" = "yes"],
  [1], [Use interpreted Lua (5.1 or 5.2)])
AC_SUBST([want_lua_old])

#### Checks for header files

# Common Checks (keep names sorted for ease of use):
AC_HEADER_DIRENT
AC_HEADER_TIME

AC_CHECK_HEADERS([ \
execinfo.h \
mcheck.h \
sys/epoll.h \
sys/inotify.h \
sys/signalfd.h \
sys/types.h \
sys/param.h \
sys/mman.h \
netinet/in.h \
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

AC_CHECK_SIZEOF([uintptr_t])

AC_CHECK_TYPES([siginfo_t], [], [],
   [[
#include <signal.h>
#if HAVE_SIGINFO_H
# include <siginfo.h>
#endif
   ]])

#### Checks for structures


#### Checks for compiler characteristics

AC_C_BIGENDIAN
AC_C_INLINE
EFL_CHECK_COMPILER_FLAGS([EFL], [-Wall -Wextra -Wpointer-arith -Wno-missing-field-initializers -fvisibility=hidden -fdata-sections -ffunction-sections])
EFL_CHECK_LINKER_FLAGS([EFL], [-fvisibility=hidden -fdata-sections -ffunction-sections -Wl,--gc-sections -fno-strict-aliasing -Wl,--as-needed -Wl,--no-copy-dt-needed-entries])
case "${build_profile}" in
   dev)
     dnl Check if compiler has a dodgy -Wshadow that emits errors when shadowing a global
     AC_MSG_CHECKING([whether -Wshadow generates spurious warnings])
     CFLAGS_save="${CFLAGS}"
     CFLAGS="${CFLAGS} -Werror -Wshadow"
     AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[int x;]], [[int x = 0; (void)x;]])],[AC_MSG_RESULT([no])
        EFL_CHECK_COMPILER_FLAGS([EFL], [-Wshadow])],[AC_MSG_RESULT([yes])])
     CFLAGS="${CFLAGS_save}"
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

want_neon="yes"
AC_ARG_ENABLE([neon],
   [AS_HELP_STRING([--disable-neon],[disable neon support @<:@default=enable@:>@])],
   [
    if test "x${enableval}" = "xyes"; then
       want_neon="yes"
    else
       want_neon="no"
    fi
   ])

SSE3_CFLAGS=""
ALTIVEC_CFLAGS=""
NEON_CFLAGS=""

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
    if test "x${want_neon}" = "xyes"; then
       build_cpu_neon="yes"
       AC_MSG_CHECKING([whether to use NEON instructions])
       CFLAGS_save="${CFLAGS}"
       CFLAGS="${CFLAGS} -mfpu=neon -ftree-vectorize"
       AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <arm_neon.h>]], [[asm volatile ("vqadd.u8 d0, d1, d0\n")]])],[
           AC_MSG_RESULT([yes])
           AC_DEFINE([BUILD_NEON], [1], [Build NEON Code])
           build_cpu_neon="yes"
           NEON_CFLAGS="-mfpu=neon"
       	 ],[
	   AC_MSG_RESULT([no])
           build_cpu_neon="no"
       	 ])
       CFLAGS="${CFLAGS_save}"
    fi
    ;;
esac

AC_SUBST([ALTIVEC_CFLAGS])
AC_SUBST([SSE3_CFLAGS])
AC_SUBST([NEON_CFLAGS])

#### Checks for linker characteristics


#### Checks for library functions

AC_CHECK_FUNCS([\
backtrace \
backtrace_symbols \
execvp \
fpathconf \
fstatat \
malloc_usable_size \
mkdirat \
mtrace \
realpath \
strlcpy \
geteuid \
getuid \
pause \
])

AC_FUNC_ALLOCA

EFL_CHECK_FUNCS([EFL], [fnmatch gettimeofday dirfd fcntl])

have_atfile_source="${ac_cv_func_fstatat}"
AC_DEFINE_IF([HAVE_ATFILE_SOURCE],
   [test "x${have_atfile_source}" = "xyes"],
   [1], [Use fstatat and other -at file functions])



######################  EFL  ######################
build_gui="yes"
AC_ARG_ENABLE([gui],
   [AS_HELP_STRING([--disable-gui],[disable GUI libraries @<:@default=enable@:>@])],
   [
    if test "x${enableval}" = "xyes"; then
       build_gui="yes"
       CFOPT_WARNING="yes"
    else
       build_gui="no"
    fi
   ])
AM_CONDITIONAL([BUILD_GUI], [test "x${build_gui}" = "xyes"])

AC_ARG_ENABLE([systemd],
   [AS_HELP_STRING([--enable-systemd],[Enable systemd support. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_systemd="yes"
    else
       want_systemd="no"
    fi
   ], [
    want_systemd="no"
   ])

systemd_dbus_prefix="# "
if test "${want_systemd}" = "yes"; then
   systemd_dbus_prefix=""
fi
AC_SUBST(systemd_dbus_prefix)

AC_ARG_WITH([systemdunitdir],
         AS_HELP_STRING([--with-systemdunitdir=DIR],[path to systemd user services directory]),
         [USER_SESSION_DIR=${withval}])
if test "$want_systemd" == "no"; then
  have_systemd_user_session="no"
elif test -n "${USER_SESSION_DIR}"; then
  have_systemd_user_session="yes"
  AC_MSG_NOTICE([Using systemd user services directory as ${USER_SESSION_DIR}])
else
  # Detect systemd user session directory properly
  EFL_PKG_CHECK_VAR([USER_SESSION_DIR], [systemd >= 192], [systemduserunitdir],
	[have_systemd_user_session="yes"], [have_systemd_user_session="no"])

  if test "$want_systemd" = "yes" -a "$have_systemd_user_session" = "no"; then
     AC_MSG_ERROR([systemd support wanted, but systemd was not found.])
  fi
fi

AM_CONDITIONAL([HAVE_SYSTEMD_USER_SESSION], [test "x${have_systemd_user_session}" = "xyes"])
AC_SUBST([USER_SESSION_DIR])


if test "x${have_systemd_pkg}" = "xauto" -o "x${have_systemd_pkg}" = "xyes"; then
   PKG_CHECK_MODULES([SYSTEMD], [libsystemd-daemon >= 192 libsystemd-journal >= 192],
   [have_systemd_pkg="yes"],
   [have_systemd_pkg="no"])
fi

# check for systemd library if requested
if test "x${want_systemd}" = "xyes" -a "x${have_systemd_pkg}" = "xno"; then
   AC_MSG_ERROR([Systemd dependencie requested but not found])
fi

AM_CONDITIONAL([WANT_SYSTEMD], [test "${want_systemd}" = "yes"])

#### Platform-dependent
DL_LIBS=""
DL_INTERNAL_LIBS=""

#### Evil
EFL_LIB_START_OPTIONAL([Evil], [test "${have_windows}" = "yes"])

### Default values

### Additional options to configure
EFL_SELECT_WINDOWS_VERSION

### Checks for programs

### Checks for libraries

EFL_ADD_LIBS([EVIL], [-lole32 -lws2_32 -lsecur32 -luuid])
EVIL_DLFCN_LIBS="-lpsapi"

AC_SUBST([EVIL_DLFCN_LIBS])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

EVIL_DLFCN_CPPFLAGS="-DEFL_EVIL_DLFCN_BUILD -DPSAPI_VERSION=1"
# TODO: should we have these at EFL (global?)
# Note: these warnings should not be used with C++ code
EVIL_CFLAGS_WRN="-Wdeclaration-after-statement -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls"
EVIL_CXXFLAGS="${EVIL_CXXFLAGS}"

if test "x${have_win32}" = "xyes" ; then
   EVIL_CFLAGS="-DSECURITY_WIN32 ${EVIL_CFLAGS}"
   EVIL_CXXFLAGS="-fno-rtti -fno-exceptions -DSECURITY_WIN32 ${EVIL_CXXFLAGS}"
fi

AC_SUBST([EVIL_DLFCN_CPPFLAGS])
AC_SUBST([EVIL_CFLAGS_WRN])
AC_SUBST([EVIL_CXXFLAGS])

### Checks for linker characteristics

### Checks for library functions

DL_LIBS="lib/evil/libdl.la"
DL_INTERNAL_LIBS="lib/evil/libdl.la"

EFL_LIB_END_OPTIONAL([Evil])

AC_SUBST([USE_EVIL_CFLAGS])
AC_SUBST([USE_EVIL_LIBS])
#### End of Evil


#### Escape
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

EFL_LIB_END_OPTIONAL([Escape])
#### End of Escape

EFL_CHECK_FUNC([SHM], [shm_open])
SHM_LIBS="${requirements_libs_shm}"
AC_SUBST([SHM_LIBS])

AC_SUBST([DL_LIBS])
AC_SUBST([DL_INTERNAL_LIBS])
#### End of Platform-dependent


#### Eina

EFL_LIB_START([Eina])

### Default values

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
     want_cow_magic="no"
     ;;

   debug)
     with_max_log_level=""
     have_stringshare_usage="yes"
     want_valgrind="auto"
     want_debug_malloc="yes"
     want_debug_threads="yes"
     want_default_mempool="yes"
     want_cow_magic="yes"
     ;;

   release)
     with_max_log_level="3"
     have_stringshare_usage="no"
     want_valgrind="no"
     want_debug_malloc="no"
     want_debug_threads="no"
     want_default_mempool="no"
     want_cow_magic="no"
     ;;
esac

EINA_CONFIG([HAVE_ALLOCA_H], [test "x${ac_cv_working_alloca_h}" = "xyes"])
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
AC_ARG_ENABLE([magic-debug],
   [AS_HELP_STRING([--disable-magic-debug],[disable magic debug of eina structure @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_magic_debug="yes"
    else
       have_magic_debug="no"
    fi
   ],
   [have_magic_debug="yes"])

EINA_CONFIG([MAGIC_DEBUG], [test "x${have_magic_debug}" = "xyes"])

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([EINA], [all])

EFL_ADD_LIBS([EINA], [-lm])

## Options

# Valgrind
AC_ARG_ENABLE([valgrind],
   [AS_HELP_STRING([--disable-valgrind],[enable valgrind mempool declaration. @<:@default=disabled@:>@])],
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

AC_DEFINE_IF([EINA_COW_MAGIC_ON],
   [test "x${want_cow_magic}" = "xyes" ],
   [1], [Turn on Eina_Magic in Eina_Cow])

EFL_OPTIONAL_DEPEND_PKG([EINA], [${want_systemd}], [SYSTEMD], [libsystemd-journal])

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

AC_CHECK_HEADER([byteswap.h], [have_byteswap="yes"], [have_byteswap="no"])

### Checks for types

EINA_CONFIG([HAVE_DIRENT_H], [test "x${have_dirent}" = "xyes"])
AC_DEFINE_IF([HAVE_DIRENT_H], [test "x${have_dirent}" = "xyes"],
   [1], [Define to 1 if you have a valid <dirent.h> header file.])

### Checks for structures

### Checks for compiler characteristics
EINA_CONFIG([HAVE_BYTESWAP_H], [test "x${have_byteswap}" = "xyes"])

EFL_CHECK_GCC_BUILTIN([bswap16], [HAVE_BSWAP16])
EFL_CHECK_GCC_BUILTIN([bswap32], [HAVE_BSWAP32])
EFL_CHECK_GCC_BUILTIN([bswap64], [HAVE_BSWAP64])

### Checks for linker characteristics

### Checks for library functions

AC_CHECK_FUNCS([fchmod])

EFL_CHECK_FUNCS([EINA], [dlopen dladdr iconv shm_open splice setxattr getpagesize])

enable_log="no"
if test "x${efl_func_fnmatch}" = "xyes" && test "x${want_log}" = "xyes" ; then
   enable_log="yes"
fi

AC_MSG_CHECKING([wether to build Eina_Log infrastructure])
AC_MSG_RESULT([${enable_log}])

EINA_CONFIG([ENABLE_LOG], [test "x${enable_log}" = "xyes"])

EFL_CHECK_THREADS
if test "x${efl_have_threads}" = "xno"; then
   CFOPT_WARNING="xyes"
fi

EFL_ADD_PUBLIC_LIBS([EINA], [${EFL_PTHREAD_LIBS}])
EFL_ADD_CFLAGS([EINA], [${EFL_PTHREAD_CFLAGS}])

EINA_CONFIG([HAVE_PTHREAD_BARRIER], [test "x${efl_have_pthread_barrier}" = "xyes"])
EINA_CONFIG([HAVE_PTHREAD_AFFINITY], [test "x${efl_have_setaffinity}" = "xyes"])
EINA_CONFIG([HAVE_DEBUG_THREADS], [test "x${want_debug_threads}" = "xyes"])
EINA_CONFIG([HAVE_POSIX_SPINLOCK], [test "x${efl_have_posix_threads_spinlock}" = "xyes"])

### Modules

EINA_CHECK_MODULE([chained-pool], [static], [chained pool])
EINA_CHECK_MODULE([pass-through], [static], [pass through])
EINA_CHECK_MODULE([one-big],      [static], [one big])

EFL_ADD_FEATURE([EINA], [systemd-journal], [${want_systemd}])

EFL_LIB_END([Eina])
#### End of Eina


#### Eina CXX
EFL_LIB_START([Eina_Cxx])

EFL_CXX_COMPILE_STDCXX_11([ext])
want_cxx11="yes"
AC_ARG_ENABLE([c++11],
   [AS_HELP_STRING([--disable-c++11],[disable C++11 bindings. @<:@default=enabled@:>@])],
   [want_cxx11="${enableval}"])

if test "x${HAVE_CXX11}" = "x1" -a "x${want_cxx11}" = "xyes"; then
  have_cxx11="yes"
else
  have_cxx11="no"
fi

AM_CONDITIONAL([HAVE_CXX11], [test "x${have_cxx11}" = "xyes"])

EFL_INTERNAL_DEPEND_PKG([EINA_CXX], [Eina])
EFL_ADD_CFLAGS([EINA_CXX], [${EFL_PTHREAD_CFLAGS}])
EFL_EVAL_PKGS([EINA_CXX])

EFL_LIB_END([Eina_Cxx])
#### End of Eina CXX

#### Eet

EFL_LIB_START([Eet])

### Default values

### Additional options to configure

### Checks for programs

### Checks for libraries

## Compatibility layers
EFL_PLATFORM_DEPEND([EET], [evil])

EFL_ADD_LIBS([EET], [-lm])

# Cryptography support
if test "$build_crypto" != "none" ; then
   AC_DEFINE([HAVE_CIPHER], [1], [Have cipher support built in eet])
   AC_DEFINE([HAVE_SIGNATURE], [1], [Have signature support for eet file])
   EFL_CRYPTO_DEPEND([EET])
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

#### Eet CXX
EFL_LIB_START([Eet_Cxx])

EFL_INTERNAL_DEPEND_PKG([EET_CXX], [Eina_Cxx])
EFL_INTERNAL_DEPEND_PKG([EET_CXX], [Eet])

EFL_EVAL_PKGS([EET_CXX])

EFL_LIB_END([Eet_Cxx])
#### End of Eet CXX

#### Eo

EFL_LIB_START([Eo])

### Default values

### Additional options to configure

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([EO], [evil])

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
EFL_CHECK_FUNCS([EO], [dladdr])

### Check availability

EFL_LIB_END([Eo])
#### End of Eo

#### Eo CXX
EFL_LIB_START([Eo_Cxx])

EFL_EVAL_PKGS([EO_CXX])

EFL_LIB_END([Eo_Cxx])
#### End of Eo CXX

#### Eolian

EFL_LIB_START([Eolian])

### Default values

### Additional options to configure
EFL_ADD_FEATURE([EOLIAN], [cxx], [${have_cxx11}])

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([EOLIAN], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([EOLIAN], [eina])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Check availability

EFL_LIB_END([Eolian])
#### End of Eolian

EFL_LIB_START([Eolian_Cxx])
### Default values

### Additional options to configure

### Checks for programs

## Compatibility layers

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([EOLIAN_CXX], [eina])
EFL_INTERNAL_DEPEND_PKG([EOLIAN_CXX], [eo])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Check availability
EFL_LIB_END([Eolian_Cxx])

#### Evas
EFL_LIB_START_OPTIONAL([Evas], [test "x${build_gui}" = "xyes"])

### Additional options to configure

# X11
AC_ARG_WITH([x11],
   [AS_HELP_STRING([--with-x11=xlib|xcb|none],[X11 method to use: xlib, xcb or none])])

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
     CFOPT_WARNING="xyes"
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
   [AS_HELP_STRING([--with-opengl=full|es|none],[OpenGL method to use: full, es or none])])

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
   [AS_HELP_STRING([--enable-wayland],[enable wayland display server. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_wayland="yes"
    else
       want_wayland="no"
    fi
   ],
   [want_wayland="no"])

if test "${want_wayland}" = "yes"; then
   EFL_PKG_CHECK_STRICT([wayland-client >= 1.3.0])
fi

# Fb
AC_ARG_ENABLE([fb],
   [AS_HELP_STRING([--enable-fb],[enable raw Framebuffer access. @<:@default=disabled@:>@])],
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
   [AS_HELP_STRING([--enable-sdl],[enable SDL support. @<:@default=disabled@:>@])],
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

# We only enable SDL with opengl if it is not the full version and not ES
# This can be changed if we ported our SDL code over to SDL 2.0. For older
# versions the SDL_opengles.h has never been released.
want_gl_sdl="no"
if test "${want_sdl}" = "yes" && test "${with_opengl}" = "es"; then
   AC_MSG_ERROR([We currently do not support SDL with OpenGL ES. Please consider full OpenGL if you want to use it with SDL.])
fi
if test "${want_sdl}" = "yes" && test "${with_opengl}" = "full"; then
   want_gl_sdl="yes"
fi

# Cocoa
AC_ARG_ENABLE([cocoa],
    [AS_HELP_STRING([--enable-cocoa],[enable MacOS X Cocoa . @<:@default=disabled@:>@])],
    [
     if test "x${enableval}" = "xyes" ; then
        want_cocoa="yes"
     else
        want_cocoa="no"
     fi
    ],
    [want_cocoa="no"])

 if test "${want_cocoa}" = "yes"; then
    #test cocoa requirements (objc and Cocoa/Cocoa.h)
 	cocoa_ldflags=""
   have_cocoa="no"
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
              have_cocoa="yes"
              cocoa_ldflags="-framework Cocoa"
             ],
             [have_cocoa="no"])
          LIBS="$LIBS_save"
          AC_MSG_CHECKING([whether Cocoa framework is supported])
          AC_MSG_RESULT([${have_cocoa}])
          AC_LANG_POP([Objective C])
       fi
   ])
fi
AC_SUBST(cocoa_ldflags)

# Drm
AC_ARG_ENABLE([drm],
   [AS_HELP_STRING([--enable-drm],[enable drm engine. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_drm="yes"
    else
       want_drm="no"
    fi
   ],
   [want_drm="no"])

want_drm_hw_accel="no"
AC_ARG_ENABLE([drm-hw-accel],
   [AS_HELP_STRING([--enable-drm-hw-accel],[enable drm hardware acceleration. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_drm_hw_accel="yes"
    else
       want_drm_hw_accel="no"
    fi
   ],
   [want_drm_hw_accel="no"])


# Fontconfig
AC_ARG_ENABLE([fontconfig],
   [AS_HELP_STRING([--disable-fontconfig],[disable fontconfig for finding fonts. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_fontconfig="yes"
    else
       want_fontconfig="no"
       CFOPT_WARNING="xyes"
    fi
   ],
   [want_fontconfig="yes"])

# Fribidi
AC_ARG_ENABLE([fribidi],
   [AS_HELP_STRING([--disable-fribidi],[disable bidirectional text support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_fribidi="yes"
    else
       want_fribidi="no"
       CFOPT_WARNING="xyes"
    fi
   ],
   [want_fribidi="yes"])

# Eo Id
AC_ARG_ENABLE([eo_id],
   [AS_HELP_STRING([--disable-eo-id],[disable Eo indirection. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_eo_id="yes"
    else
       want_eo_id="no"
       CFOPT_WARNING="xyes"
    fi
   ],
   [want_eo_id="yes"])

# Harfbuzz
AC_ARG_ENABLE([harfbuzz],
   [AS_HELP_STRING([--enable-harfbuzz],[enable complex text shaping and layouting support. @<:@default=disabled@:>@])],
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
   [AS_HELP_STRING([--enable-egl],[enable EGL rendering. @<:@default=disabled@:>@])],
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
   [AS_HELP_STRING([--enable-pixman],[enable pixman for software rendering. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_pixman="yes"
       CFOPT_WARNING="xyes"
    else
       want_pixman="no"
    fi
   ],
   [want_pixman="no"])

AC_ARG_ENABLE([pixman-font],
   [AS_HELP_STRING([--enable-pixman-font],[Allow pixman to render fonts. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_pixman_font="yes"
    else
       have_pixman_font="no"
    fi
   ],
   [have_pixman_font="no"])

AC_ARG_ENABLE([pixman-rect],
   [AS_HELP_STRING([--enable-pixman-rect],[Allow pixman to render rects. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_pixman_rect="yes"
    else
       have_pixman_rect="no"
    fi
   ],
   [have_pixman_rect="no"])

AC_ARG_ENABLE([pixman-line],
   [AS_HELP_STRING([--enable-pixman-line],[Allow pixman to render lines. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_pixman_line="yes"
    else
       have_pixman_line="no"
    fi
   ],
   [have_pixman_line="no"])

AC_ARG_ENABLE(pixman-poly,
   AS_HELP_STRING([--enable-pixman-poly],[Allow pixman to render polys. @<:@default=disabled@:>@]),
   [
    if test "x${enableval}" = "xyes" ; then
       have_pixman_poly="yes"
    else
       have_pixman_poly="no"
    fi
   ],
   [have_pixman_poly="no"])

AC_ARG_ENABLE([pixman-image],
   [AS_HELP_STRING([--enable-pixman-image],[Allow pixman to render images. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_pixman_image="yes"
    else
       have_pixman_image="no"
    fi
   ],
   [have_pixman_image="no"])

AC_ARG_ENABLE([pixman-image-scale-sample],
   [AS_HELP_STRING([--enable-pixman-image-scale-sample],[Allow pixman to render sampled scaled images. @<:@default=disabled@:>@])],
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
   [AS_HELP_STRING([--enable-tile-rotate],[Enable tiled rotate algorithm. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_tile_rotate="yes"
       CFOPT_WARNING="xyes"
    else
       have_tile_rotate="no"
    fi
  ],
  [have_tile_rotate="no"])


# Image Loaders

ARG_ENABLE_EVAS_IMAGE_LOADER(BMP, static)
ARG_ENABLE_EVAS_IMAGE_LOADER(Eet, static)
ARG_ENABLE_EVAS_IMAGE_LOADER(Generic, static)
ARG_ENABLE_EVAS_IMAGE_LOADER(Gif, yes)
ARG_ENABLE_EVAS_IMAGE_LOADER(ICO, static)
ARG_ENABLE_EVAS_IMAGE_LOADER(JPEG, static)
ARG_ENABLE_EVAS_IMAGE_LOADER(JP2K, auto)
ARG_ENABLE_EVAS_IMAGE_LOADER(PMAPS, static)
ARG_ENABLE_EVAS_IMAGE_LOADER(PNG, static)
ARG_ENABLE_EVAS_IMAGE_LOADER(PSD, static)
ARG_ENABLE_EVAS_IMAGE_LOADER(Tga, static)
ARG_ENABLE_EVAS_IMAGE_LOADER(Tiff, yes)
ARG_ENABLE_EVAS_IMAGE_LOADER(WBMP, static)
ARG_ENABLE_EVAS_IMAGE_LOADER(WEBP, no)
ARG_ENABLE_EVAS_IMAGE_LOADER(XPM, static)
ARG_ENABLE_EVAS_IMAGE_LOADER(TGV, static)
ARG_ENABLE_EVAS_IMAGE_LOADER(DDS, static)

### Default values

want_evas_engine_software_gdi="${have_win32}"
want_evas_engine_software_ddraw="${have_win32}"
want_evas_engine_gl_cocoa="${want_cocoa}"
want_evas_engine_wayland_egl="no"
if test "${want_wayland}" = "yes" && test "${want_egl}" = "yes" && test "${with_opengl}" = "es"; then
   want_evas_engine_wayland_egl="yes"
fi

if test "${want_lua_old}" = "yes"; then
   EFL_CHECK_LUA_OLD([EVAS])
else
   EFL_DEPEND_PKG([EVAS], [LUAJIT], [luajit >= 2.0.0])
fi

EFL_ADD_FEATURE([EVAS], [lua-old])

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
   EFL_ADD_FEATURE([EVAS_PIXMAN], [font], [${have_pixman_font}])

   if test "x${have_pixman_rect}" = "xyes" ; then
      AC_DEFINE(PIXMAN_RECT, 1, [Allow pixman to render rects])
   fi
   EFL_ADD_FEATURE([EVAS_PIXMAN], [rect], [${have_pixman_rect}])

   if test "x${have_pixman_line}" = "xyes" ; then
      AC_DEFINE(PIXMAN_LINE, 1, [Allow pixman to render lines])
   fi
   EFL_ADD_FEATURE([EVAS_PIXMAN], [line], [${have_pixman_line}])

   if test "x${have_pixman_poly}" = "xyes" ; then
      AC_DEFINE(PIXMAN_POLY, 1, [Allow pixman to render polys])
   fi
   EFL_ADD_FEATURE([EVAS_PIXMAN], [poly], [${have_pixman_poly}])

   if test "x${have_pixman_image}" = "xyes" ; then
      AC_DEFINE(PIXMAN_IMAGE, 1, [Allow pixman to render images])
   fi
   EFL_ADD_FEATURE([EVAS_PIXMAN], [image], [${have_pixman_image}])

   if test "x${have_pixman_image_scale_sample}" = "xyes" ; then
      AC_DEFINE(PIXMAN_IMAGE_SCALE_SAMPLE, 1, [Allow pixman to render image sampled scaling])
   fi
   EFL_ADD_FEATURE([EVAS_PIXMAN], [scale_sample], [${have_pixman_scale_sample}])
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

# XXX TODO:
EVAS_CHECK_ENGINE([gl-cocoa], [${want_evas_engine_gl_cocoa}], [no], [OpenGL Cocoa])
EVAS_CHECK_ENGINE([gl-sdl], [${want_gl_sdl}], [no], [OpenGL SDL])
EVAS_CHECK_ENGINE([software-gdi], [${want_evas_engine_software_gdi}], [no], [Software GDI])
EVAS_CHECK_ENGINE([software-ddraw], [${want_evas_engine_software_ddraw}], [no], [Software DirectDraw])
EVAS_CHECK_ENGINE([wayland-egl], [${want_evas_engine_wayland_egl}], [no], [Wayland Egl])
EVAS_CHECK_ENGINE([wayland-shm], [${want_wayland}], [no], [Wayland Shm])
EVAS_CHECK_ENGINE([drm], [${want_drm}], [no], [Drm])


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
if text "x${have_egl}" = "xyes"; then
   evas_engine_gl_common_libs="-lEGL"
else
   evas_engine_gl_common_libs="-lGL"
fi

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
EVAS_CHECK_IMAGE_LOADER([JP2K],    [${want_evas_image_loader_jp2k}])
EVAS_CHECK_IMAGE_LOADER([PMAPS],   [${want_evas_image_loader_pmaps}])
EVAS_CHECK_IMAGE_LOADER([PNG],     [${want_evas_image_loader_png}])
EVAS_CHECK_IMAGE_LOADER([PSD],     [${want_evas_image_loader_psd}])
EVAS_CHECK_IMAGE_LOADER([Tga],     [${want_evas_image_loader_tga}])
EVAS_CHECK_IMAGE_LOADER([Tiff],    [${want_evas_image_loader_tiff}])
EVAS_CHECK_IMAGE_LOADER([WBMP],    [${want_evas_image_loader_wbmp}])
EVAS_CHECK_IMAGE_LOADER([WEBP],    [${want_evas_image_loader_webp}])
EVAS_CHECK_IMAGE_LOADER([XPM],     [${want_evas_image_loader_xpm}])
EVAS_CHECK_IMAGE_LOADER([TGV],     [${want_evas_image_loader_tgv}])
EVAS_CHECK_IMAGE_LOADER([DDS],     [${want_evas_image_loader_dds}])

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

AC_CHECK_LIB([m], [lround],
[AC_DEFINE([HAVE_LROUND], [1], [C99 lround function exists])],
[EFL_CHECK_GCC_BUILTIN([lround], [HAVE_LROUND])]
)


# Evas cserve2

AC_ARG_ENABLE([cserve],
   [AS_HELP_STRING([--enable-cserve],[enable shared cache server (cserve2). @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_evas_cserve2="yes"
    else
       want_evas_cserve2="no"
       CFOPT_WARNING="xyes"
    fi
   ],
   [want_evas_cserve2="yes"])

if test "x${want_evas_image_loader_generic}" = "xyes" || test "x${want_evas_cserve2}" = "xyes" ; then
   EFL_ADD_LIBS([EVAS], [${requirements_libs_shm}])
fi

# cserve2 only works on Linux so far.

if test "x${efl_func_shm_open}" = "xno" ; then
   want_evas_cserve2="no"
fi

if test "x${want_evas_cserve2}" = "xyes"; then
   if test "x${ac_cv_header_sys_epoll_h}" = "xno" || test "x${ac_cv_header_sys_inotify_h}" = "xno" || test "x${ac_cv_header_sys_signalfd_h}" = "xno"; then
      want_evas_cserve2="no"
   else
      EVAS_CSERVE2_SLAVE_LIBS="${EFL_COV_LIBS} ${EFL_LIBS} ${requirements_libs_shm}"
      AC_SUBST([EVAS_CSERVE2_SLAVE_LIBS])
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
   [AS_HELP_STRING([--with-evas-dither-mask=TYPE],[use the specified dither mask to convert bitdepths in Evas, one of: big, small, line or none.
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
EFL_ADD_FEATURE([EVAS], [fontconfig])
EFL_ADD_FEATURE([EVAS], [fribidi])
EFL_ADD_FEATURE([EVAS], [harfbuzz])
EFL_ADD_FEATURE([EVAS], [cserve], [${want_evas_cserve2}])
EFL_ADD_FEATURE([EVAS], [tile-rotate])
EFL_ADD_FEATURE([EVAS], [dither-mask], [${build_evas_dither_mask}])

EFL_LIB_END_OPTIONAL([Evas])

if test "x${build_gui}" = "xno"; then
   AM_CONDITIONAL([BUILD_ENGINE_BUFFER], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_BUFFER], [false])
   AM_CONDITIONAL([BUILD_ENGINE_FB], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_FB], [false])
   AM_CONDITIONAL([BUILD_ENGINE_PSL1GHT], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_PSL1GHT], [false])
   AM_CONDITIONAL([BUILD_ENGINE_GL_COCOA], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_COCOA], [false])
   AM_CONDITIONAL([BUILD_ENGINE_GL_SDL], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_SDL], [false])
   AM_CONDITIONAL([BUILD_ENGINE_SOFTWARE_GDI], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_GDI], [false])
   AM_CONDITIONAL([BUILD_ENGINE_SOFTWARE_DDRAW], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_DDRAW], [false])
   AM_CONDITIONAL([BUILD_ENGINE_WAYLAND_EGL], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_WAYLAND_EGL], [false])
   AM_CONDITIONAL([BUILD_ENGINE_WAYLAND_SHM], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_WAYLAND_SHM], [false])
   AM_CONDITIONAL([BUILD_ENGINE_DRM], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_DRM], [false])
   AM_CONDITIONAL([BUILD_ENGINE_SOFTWARE_XCB], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_XCB], [false])
   AM_CONDITIONAL([BUILD_ENGINE_SOFTWARE_XLIB], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_XLIB], [false])
   AM_CONDITIONAL([BUILD_ENGINE_SOFTWARE_X11], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_X11], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_GENERIC], [false])
   AM_CONDITIONAL([BUILD_ENGINE_GL_XCB], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_XCB], [false])
   AM_CONDITIONAL([BUILD_ENGINE_GL_XLIB], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_XLIB], [false])
   AM_CONDITIONAL([BUILD_ENGINE_GL_X11], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_X11], [false])
   AM_CONDITIONAL([BUILD_ENGINE_GL_COMMON], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_COMMON], [false])
   AM_CONDITIONAL([BUILD_LOADER_BMP], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_BMP], [false])
   AM_CONDITIONAL([BUILD_LOADER_EET], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_EET], [false])
   AM_CONDITIONAL([BUILD_LOADER_GENERIC], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_GENERIC], [false])
   AM_CONDITIONAL([BUILD_LOADER_GIF], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_GIF], [false])
   AM_CONDITIONAL([BUILD_LOADER_ICO], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_ICO], [false])
   AM_CONDITIONAL([BUILD_LOADER_JPEG], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_JPEG], [false])
   AM_CONDITIONAL([BUILD_LOADER_JP2K], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_JP2K], [false])
   AM_CONDITIONAL([BUILD_LOADER_PMAPS], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_PMAPS], [false])
   AM_CONDITIONAL([BUILD_LOADER_PNG], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_PNG], [false])
   AM_CONDITIONAL([BUILD_LOADER_PSD], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_PSD], [false])
   AM_CONDITIONAL([BUILD_LOADER_TGA], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_TGA], [false])
   AM_CONDITIONAL([BUILD_LOADER_TIFF], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_TIFF], [false])
   AM_CONDITIONAL([BUILD_LOADER_WBMP], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_WBMP], [false])
   AM_CONDITIONAL([BUILD_LOADER_WEBP], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_WEBP], [false])
   AM_CONDITIONAL([BUILD_LOADER_XPM], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_XPM], [false])
   AM_CONDITIONAL([BUILD_LOADER_TGV], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_TGV], [false])
   AM_CONDITIONAL([BUILD_LOADER_DDS], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_DDS], [false])
   AM_CONDITIONAL([EVAS_CSERVE2], [false])
fi

#### End of Evas

#### Edje CXX
EFL_LIB_START([Evas_Cxx])

EFL_EVAL_PKGS([EVAS_CXX])

EFL_LIB_END([Evas_Cxx])
#### End of Edje CXX

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
   [AS_HELP_STRING([--with-glib=yes|no|always],[add glib support. @<:@default=enabled@:>@])],
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
   [AS_HELP_STRING([--enable-g-main-loop],[enable ecore_main_loop based on g_main_loop. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_g_main_loop="yes"
       CFOPT_WARNING="xyes"
    else
       want_g_main_loop="no"
    fi
   ],
   [want_g_main_loop="no"])

AC_ARG_ENABLE([gstreamer],
   [AS_HELP_STRING([--enable-gstreamer],[enable gstreamer 0.10 support. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_gstreamer="yes"
       CFOPT_WARNING="xyes"
    else
       want_gstreamer="no"
    fi
   ],
   [want_gstreamer="no"])

AC_ARG_ENABLE([gstreamer1],
   [AS_HELP_STRING([--disable-gstreamer1],[disable gstreamer 1.0 support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_gstreamer1="yes"
    else
       want_gstreamer1="no"
       CFOPT_WARNING="xyes"
    fi
   ],
   [want_gstreamer1="yes"])
if test "x${build_gui}" = "xno"; then
   want_gstreamer1="no"
fi

AC_ARG_ENABLE([tizen],
   [AS_HELP_STRING([--enable-tizen],[enable tizen support. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_tizen="yes"
    else
       want_tizen="no"
    fi
   ],
   [want_tizen="no"])

if test "${want_tizen}" = "yes"; then
     PKG_CHECK_MODULES([TIZEN_CONFIGURATION_MANAGER],
      [vconf],
      [have_tizen_vconf="yes"],
      [have_tizen_vconf="no"])
  PKG_CHECK_MODULES([TIZEN_CONFIGURATION_MANAGER], [vconf])
fi

AM_CONDITIONAL([HAVE_TIZEN_CONFIGURATION_MANAGER], [test "${have_tizen_vconf}" = "yes"])
if test "${have_tizen_vconf}"  = "yes"; then
  AC_DEFINE(HAVE_TIZEN_CONFIGURATION_MANAGER, 1, [Define to 1 if you have Tizen configuration manager(vconf).])
fi

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

EFL_OPTIONAL_DEPEND_PKG([ECORE], [${want_glib}], [GLIB], [glib-2.0 gthread-2.0])
if test "x${have_glib}" = "xno"; then
   want_g_main_loop="no"
fi

EFL_OPTIONAL_DEPEND_PKG([ECORE], [${want_systemd}], [SYSTEMD], [libsystemd-daemon])

EFL_ADD_FEATURE([ECORE], [systemd-daemon], [${want_systemd}])
EFL_ADD_FEATURE([ECORE], [glib])
EFL_ADD_FEATURE([ECORE], [g-main-loop])

want_glib_integration_always=no
if test "x${with_glib}" = "xalways" ; then
  want_glib_integration_always="yes"
  AC_DEFINE([GLIB_INTEGRATION_ALWAYS], [1], [Always integrate glib if support compiled])
fi

if test "x${want_g_main_loop}" = "xyes" ; then
  AC_DEFINE([USE_G_MAIN_LOOP], [1], [Use g_main_loop in ecore])
fi

# not EFL_OPTIONAL_DEPEND_PKG() because it's only used for ecore examples
if test "${want_gstreamer1}" = "yes" -a "${want_gstreamer}" = "yes"; then
   AC_MSG_ERROR([You can only enable either GStreamer 1.0 or GStreamer 0.10 support])
fi

if test "${want_gstreamer1}" = "yes"; then
  PKG_CHECK_MODULES([GSTREAMER], [gstreamer-1.0])
fi
if test "${want_gstreamer}" = "yes"; then
  PKG_CHECK_MODULES([GSTREAMER], [gstreamer-0.10])
fi
AM_CONDITIONAL([HAVE_GSTREAMER], [test "${want_gstreamer}" = "yes" -o "${want_gstreamer1}" = "yes"])

EFL_EVAL_PKGS([ECORE])

### Checks for header files

AC_HEADER_SYS_WAIT

AC_CHECK_HEADERS([sys/socket.h])

AC_CHECK_HEADERS([ \
arpa/inet.h \
langinfo.h \
features.h \
netinet/in.h \
netinet/tcp.h \
sys/prctl.h \
sys/resource.h \
sys/timerfd.h \
sys/un.h \
],[],[],
[
 #ifdef HAVE_SYS_SOCKET_H
 # include <sys/socket.h>
 #endif
])

AC_CHECK_HEADERS([net/if.h], [], [],
[#include <stdio.h>
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
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

AC_CHECK_FUNCS_ONCE([mallinfo timerfd_create clock_gettime malloc_info])

if ! test "x${ac_cv_func_clock_gettime}" = "xyes" ; then
   AC_CHECK_LIB([rt], [clock_gettime],
      [
        EFL_ADD_LIBS([ECORE], [-lrt])
        AC_DEFINE([HAVE_CLOCK_GETTIME], [1], [Have clock_gettime()])
       ])
fi

EFL_ADD_LIBS([ECORE], [${LTLIBINTL}])

# coroutine function specific

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
       [[
#include <ucontext.h>
       ]],
       [[
ucontext_t test;
getcontext(&test);
       ]])],
   [have_ucontext="yes"],
   [have_ucontext="no"])

AC_MSG_CHECKING([for ucontext])
AC_MSG_RESULT([${have_ucontext}])

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
       [[
#include <setjmp.h>
       ]],
       [[
jmp_buf context;
setjmp(&context);
       ]])],
   [have_setjmp="yes"],
   [have_setjmp="no"])

AC_MSG_CHECKING([for setjmp])
AC_MSG_RESULT([${have_setjmp}])

if test "X${have_windows}" = "xyes"; then
   AC_DEFINE(USE_FIBER, 1, [Define to 1 if you have Windows Fiber support.])
   EFL_ADD_FEATURE([system], [coroutine], [fiber])
elif test "x${have_ucontext}" = "xyes"; then
   AC_DEFINE(USE_UCONTEXT, 1, [Define to 1 if you have posix ucontext functions.])
   EFL_ADD_FEATURE([system], [coroutine], [ucontext])
elif test "x${have_setjmp}" = "xyes"; then
   AC_DEFINE(USE_SETJMP, 1, [Define to 1 if you have setjmp/longjmp functions.])
   EFL_ADD_FEATURE([system], [coroutine], [setjmp])
else
   AC_MSG_ERROR([You don't have a working way to implement coroutine. Exiting...])
fi

### Check availability

EFL_LIB_END([Ecore])
#### End of Ecore

#### Ecore CXX
EFL_LIB_START([Ecore_Cxx])

EFL_INTERNAL_DEPEND_PKG([ECORE_CXX], [Eina_Cxx])
EFL_INTERNAL_DEPEND_PKG([ECORE_CXX], [Ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_CXX], [Eina])
EFL_INTERNAL_DEPEND_PKG([ECORE_CXX], [Eo])

EFL_EVAL_PKGS([ECORE_CXX])

EFL_LIB_END([Ecore_Cxx])
#### End of Ecore CXX

#### Ecore_Con

EFL_LIB_START([Ecore_Con])

### Default values

want_ecore_con_local_sockets="yes"
want_ecore_con_abstract_sockets="yes"

if test "${have_win32}" = "yes"; then
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
if test "$build_crypto" != "none" ; then
   EFL_CRYPTO_DEPEND([ECORE_CON])
fi
EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [eet])
EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [eina])
EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [ecore])

EFL_ADD_LIBS([ECORE_CON], [-lm])

EFL_OPTIONAL_DEPEND_PKG([ECORE_CON], [${want_cares}],
   [CARES], [libcares >= 1.6.1])
AM_CONDITIONAL([HAVE_CARES], [test "x${have_cares}" = "xyes"])

if test "x$have_cares" = "xyes" ; then
   ecore_con_resolver="cares"
elif test "x$ac_cv_prog_cc_c99" != "xno" ; then
   ecore_con_resolver="dns.c"
else
   ecore_con_resolver="fork"
fi

EFL_OPTIONAL_DEPEND_PKG([ECORE_CON], [${want_systemd}], [SYSTEMD], [libsystemd-daemon])

EFL_ADD_FEATURE([ECORE_CON], [cares])
EFL_ADD_FEATURE([ECORE_CON], [local-sockets], [${want_ecore_con_local_sockets}])
EFL_ADD_FEATURE([ECORE_CON], [abstract-sockets], [${want_ecore_con_abstract_sockets}])
EFL_ADD_FEATURE([ECORE_CON], [resolver], [${ecore_con_resolver}])
EFL_ADD_FEATURE([ECORE_CON], [systemd-daemon], [${want_systemd}])

EFL_EVAL_PKGS([ECORE_CON])

### Checks for header files

AC_CHECK_HEADERS([ws2tcpip.h netdb.h])

if test "x${ac_cv_header_netdb_h}" = "xno" && test "x${have_windows}" = "xno"; then
   AC_MSG_ERROR([netdb.h is requested to have Ecore_Con. Exiting...])
fi

### Checks for types

have_ipv6="no"
AC_CHECK_TYPES([struct ipv6_mreq],
   [have_ipv6="yes"],
   [have_ipv6="no"],
   [[
#include <netinet/in.h>
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

## Compatibility layers
EFL_PLATFORM_DEPEND([ECORE_IPC], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_IPC], [ecore-con])
EFL_INTERNAL_DEPEND_PKG([ECORE_IPC], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_IPC], [eo])
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

## Compatibility layers
EFL_PLATFORM_DEPEND([ECORE_FILE], [evil])

### Checks for libraries
EFL_PLATFORM_DEPEND([ECORE_FILE], [escape])
EFL_INTERNAL_DEPEND_PKG([ECORE_FILE], [ecore-con])
EFL_INTERNAL_DEPEND_PKG([ECORE_FILE], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_FILE], [eo])
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
EFL_LIB_START_OPTIONAL([Ecore_Input], [test "x${build_gui}" = "xyes"])

### Additional options to configure

### Default values

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ECORE_INPUT], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT], [eina])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Input])
#### End of Ecore_Input


#### Ecore_Input_Evas
EFL_LIB_START_OPTIONAL([Ecore_Input_Evas], [test "x${build_gui}" = "xyes"])

### Additional options to configure

### Default values

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ECORE_INPUT_EVAS], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [evas])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [eina])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Input_Evas])
#### End of Ecore_Input_Evas


#### Ecore_Cocoa
EFL_LIB_START_OPTIONAL([Ecore_Cocoa], [test "${want_cocoa}" = "yes"])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_COCOA], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_COCOA], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_COCOA], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_COCOA], [eina])

### Checks for header files

EFL_ADD_LIBS([ECORE_COCOA], [-framework Cocoa])

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Cocoa])
#### End of Ecore_Cocoa


#### Ecore_FB
EFL_LIB_START_OPTIONAL([Ecore_FB], [test "${want_fb}" = "yes"])

### Additional options to configure
AC_ARG_ENABLE([tslib],
   [AS_HELP_STRING([--disable-tslib],[disable tslib for touchscreen events.])],
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
EFL_INTERNAL_DEPEND_PKG([ECORE_FB], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_FB], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_FB], [eina])

EFL_OPTIONAL_DEPEND_PKG([ECORE_FB], [${want_tslib}], [TSLIB], [tslib])
EFL_ADD_FEATURE([ECORE_FB], [tslib])

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
EFL_PLATFORM_DEPEND([ECORE_PSL1GHT], [escape])

EFL_INTERNAL_DEPEND_PKG([ECORE_PSL1GHT], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_PSL1GHT], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_PSL1GHT], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_PSL1GHT], [eina])

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
EFL_INTERNAL_DEPEND_PKG([ECORE_SDL], [eo])
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
EFL_INTERNAL_DEPEND_PKG([ECORE_WAYLAND], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_WAYLAND], [eina])

EFL_DEPEND_PKG([ECORE_WAYLAND], [WAYLAND],
   [wayland-client >= 1.3.0 wayland-cursor >= 1.3.0 xkbcommon >= 0.3.0])

EFL_EVAL_PKGS([ECORE_WAYLAND])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions
EFL_LIB_END_OPTIONAL([Ecore_Wayland])
#### End of Ecore_Wayland

#### Ecore_Drm
EFL_LIB_START_OPTIONAL([Ecore_Drm], [test "${want_drm}" = "yes"])

### Additional options to configure
SUID_CFLAGS=-fPIE
SUID_LDFLAGS=-pie
AC_SUBST([SUID_CFLAGS])
AC_SUBST([SUID_LDFLAGS])

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM], [eina])

EFL_DEPEND_PKG([ECORE_DRM], [DRM], [libudev >= 148 libdrm >= 2.4 xkbcommon >= 0.3.0 libsystemd-login >= 192 dbus-1])

EFL_EVAL_PKGS([ECORE_DRM])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Drm])
#### End of Ecore_Drm


#### Ecore_Audio

AC_ARG_ENABLE([audio],
   [AS_HELP_STRING([--disable-audio],[disable audio support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_audio="yes"
    else
       want_audio="no"
       CFOPT_WARNING="xyes"
    fi
   ],
   [want_audio="yes"])

if test "x${build_gui}" = "xno"; then
   want_audio="no"
fi

EFL_LIB_START_OPTIONAL([Ecore_Audio], [test "${want_audio}" = "yes"])

### Additional options to configure

# ALSA support is still not there, thus no option for it yet.
want_alsa="no"

# sndfile is mandatory otherwise it won't read from/write to files.
# TODO: if confirmed sndfile is mandatory, remove this variable
# TODO: and the EFL_OPTIONAL_DEPEND_PKG(), use EFL_DEPEND_PKG()
want_sndfile="yes"

AC_ARG_ENABLE([pulseaudio],
   [AS_HELP_STRING([--disable-pulseaudio],[disable pulseaudio sound support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_pulseaudio="yes"
    else
       want_pulseaudio="no"
       CFOPT_WARNING="xyes"
    fi
   ],
   [want_pulseaudio="yes"])


### Default values

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ECORE_AUDIO], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_AUDIO], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_AUDIO], [eet])
EFL_INTERNAL_DEPEND_PKG([ECORE_AUDIO], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_AUDIO], [eina])

EFL_ADD_LIBS([ECORE_AUDIO], [-lm])

EFL_OPTIONAL_DEPEND_PKG([ECORE_AUDIO], [${want_alsa}], [ALSA], [alsa])
EFL_OPTIONAL_DEPEND_PKG([ECORE_AUDIO], [${want_pulseaudio}], [PULSE], [libpulse])
EFL_OPTIONAL_DEPEND_PKG([ECORE_AUDIO], [${want_sndfile}], [SNDFILE], [sndfile])

EFL_EVAL_PKGS([ECORE_AUDIO])

EFL_ADD_FEATURE([ECORE_AUDIO], [alsa])
EFL_ADD_FEATURE([ECORE_AUDIO], [pulseaudio])
EFL_ADD_FEATURE([ECORE_AUDIO], [sndfile])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Audio])
AM_CONDITIONAL([HAVE_ECORE_AUDIO_PULSE], [test "x${want_pulseaudio}" = "xyes"])
AM_CONDITIONAL([HAVE_ECORE_AUDIO_SNDFILE], [test "x${want_sndfile}" = "xyes"])

#### End of Ecore_Audio

#### Ecore Audio CXX
EFL_LIB_START([Ecore_Audio_Cxx])

EFL_EVAL_PKGS([ECORE_AUDIO_CXX])

EFL_LIB_END([Ecore_Audio_Cxx])
#### End of Ecore Audio CXX

#### Ecore_Win32
EFL_LIB_START_OPTIONAL([Ecore_Win32], [test "${have_win32}" = "yes"])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([ECORE_WIN32], [evil])
EFL_INTERNAL_DEPEND_PKG([ECORE_WIN32], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_WIN32], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_WIN32], [eo])
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

#### Ecore_Avahi

EFL_LIB_START([Ecore_Avahi])

### Default values

### Additional options to configure

want_avahi="yes"

AC_ARG_ENABLE([avahi],
   [AS_HELP_STRING([--disable-avahi],[disable avahi support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_avahi="yes"
    else
       want_avahi="no"
    fi
   ], [
    want_avahi="yes"
   ])

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ECORE_AVAHI], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_AVAHI], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_AVAHI], [eina])
EFL_INTERNAL_DEPEND_PKG([ECORE_AVAHI], [eo])

EFL_OPTIONAL_DEPEND_PKG([ECORE_AVAHI], [${want_avahi}], [AVAHI], [avahi-client], [have_avahi=yes], [have_avahi=no])

EFL_ADD_FEATURE([ECORE_AVAHI], [avahi-client], [${have_avahi}])

# Needed bu example as they use avahi directly in that case
if test "x${have_avahi}" = "xyes"; then
   PKG_CHECK_MODULES([AVAHI_CLIENT], [avahi-client])
fi

EFL_EVAL_PKGS([ECORE_AVAHI])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Ecore_Avahi])

#### End of Ecore_Avahi


#### Ecore_X
EFL_LIB_START_OPTIONAL([Ecore_X], [test "${want_x11_any}" = "yes"])

### Additional options to configure

AC_ARG_ENABLE([gesture],
   [AS_HELP_STRING([--enable-gesture],[enable X11 Gesture extension support])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_gesture="yes"
    else
       want_gesture="no"
    fi
   ],
   [want_gesture="no"])

AC_ARG_ENABLE([xpresent],
   [AS_HELP_STRING([--enable-xpresent],[enable X11 XPresent extension support])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_xpresent="yes"
    else
       want_xpresent="no"
    fi
   ],
   [want_xpresent="no"])

AC_ARG_ENABLE([xinput2],
   [AS_HELP_STRING([--disable-xinput2],[disable X11 XInput v2.x support])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_xinput2="yes"
    else
       want_xinput2="no"
       CFOPT_WARNING="xyes"
    fi
   ],
   [want_xinput2="yes"])

AC_ARG_ENABLE([xinput22],
   [AS_HELP_STRING([--enable-xinput22],[enable X11 XInput v2.2+ support])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_xinput22="yes"
    else
       want_xinput22="no"
    fi
   ],
   [want_xinput22="no"])

AC_ARG_ENABLE([xim],
   [AS_HELP_STRING([--disable-xim],[disable X Input Method.])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_xim="yes"
    else
       want_xim="no"
       CFOPT_WARNING="xyes"
    fi
   ],
   [want_xim="yes"])

AC_ARG_ENABLE([scim],
   [AS_HELP_STRING([--disable-scim],[disable SCIM.])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_scim="yes"
    else
       want_scim="no"
       CFOPT_WARNING="xyes"
    fi
   ],
   [want_scim="yes"])

AC_ARG_ENABLE([ibus],
   [AS_HELP_STRING([--disable-ibus],[disable IBUS.])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_ibus="yes"
    else
       want_ibus="no"
    fi
   ],
   [want_ibus="yes"])

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_X], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_X], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_X], [eo])
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

   PKG_CHECK_EXISTS([xrandr > 1.3.2], [AC_DEFINE([XRANDR_GOOD], [1], [good xrandr])], [])

   if test "${want_xpresent}" = "yes"; then
      ECORE_CHECK_X_EXTENSION([Xpresent], [Xpresent.h], [Xpresent], [XPresentQueryExtension])
   fi
   EFL_ADD_FEATURE([ECORE_X], [xpresent])

   if test "${want_gesture}" = "yes"; then
      ECORE_CHECK_X_EXTENSION([Xgesture], [gesture.h], [Xgesture], [XGestureQueryExtension])
   fi
   EFL_ADD_FEATURE([ECORE_X], [gesture])

   if test "${want_xinput2}" = "yes"; then
      ECORE_CHECK_X_EXTENSION([Xi2], [XInput2.h], [Xi], [XIQueryDevice])
   fi
   EFL_ADD_FEATURE([ECORE_X], [xinput2])

   if test "${want_xinput22}" = "yes"; then
      ECORE_CHECK_X_EXTENSION([Xi2_2], [XInput2.h], [Xi],[XIGrabTouchBegin])
   fi
   EFL_ADD_FEATURE([ECORE_X], [xinput22])

   AC_DEFINE([HAVE_ECORE_X_XLIB], [1], [Defined to 1 if Xlib is enabled.])
   HAVE_ECORE_X_BACKEND="HAVE_ECORE_X_XLIB"

   EFL_EVAL_PKGS([ECORE_X])
   EFL_CHECK_FUNCS([ECORE_X], [dlopen dlsym])
fi

## XCB

if test "${want_x11_xcb}" = "yes"; then
dnl note: added pixman-1 as ecore_xcb_region uses that
   EFL_DEPEND_PKG([ECORE_X], [ECORE_X_XCB],
      [x11-xcb xcb xcb-shm xcb-event xcb-icccm >= 0.3.8 xcb-util >= 0.3.8 xcb-image xcb-keysyms >= 0.3.8 xcb-composite xcb-present xcb-damage xcb-dpms xcb-randr xcb-render xcb-screensaver xcb-shape xcb-sync xcb-xfixes xcb-xinerama xcb-xprint xcb-xtest xcb-renderutil pixman-1])

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

   EFL_OPTIONAL_DEPEND_PKG([ECORE_X], [${want_xpresent}], [ECORE_XCB_XPRESENT],
      [xcb-present])
   AC_DEFINE_IF([ECORE_XCB_XPRESENT], [test "${want_xpresent}" = "yes"],
      [1], [Build support for XCB Present])
   EFL_ADD_FEATURE([ECORE_X], [xpresent])

   EFL_OPTIONAL_DEPEND_PKG([ECORE_X], [${want_gesture}], [ECORE_XCB_GESTURE],
      [xcb-gesture])
   AC_DEFINE_IF([ECORE_XCB_XGESTURE], [test "${want_gesture}" = "yes"],
      [1], [Build support for XCB xgesture])

   EFL_ADD_FEATURE([ECORE_X], [gesture])

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
EFL_LIB_START_OPTIONAL([Ecore_Imf], [test "x${build_gui}" = "xyes"])

### Additional options to configure

### Default values

want_ecore_imf="yes"
want_ecore_imf_xim="no"
want_ecore_imf_scim="no"
want_ecore_imf_ibus="no"
want_ecore_imf_wayland="no"
want_ecore_imf="yes"

if test "${have_windows}" = "no" && test "${have_darwin}" = "no"; then
   want_ecore_imf="yes"
   want_ecore_imf_xim="yes"
   want_ecore_imf_scim="yes"
   want_ecore_imf_ibus="yes"
   if test "${want_wayland}" = "yes"; then
      want_ecore_imf_wayland="yes"
   fi
fi

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ECORE_IMF], [evil])

### Checks for libraries
EFL_PLATFORM_DEPEND([ECORE_IMF], [escape])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF], [eina])

if test "x${want_ecore_imf}" = "xyes" ; then
   AC_DEFINE([HAVE_ECORE_IMF], [1], [Ecore IMF Support])
fi

## modules

# ibus

have_ecore_imf_ibus="no"
if test "x${want_ibus}" = "xyes" && test "x${want_ecore_imf_ibus}" = "xyes" && test "x${have_glib}" = "xyes" ; then
   PKG_CHECK_MODULES([IBUS],
      [ibus-1.0 >= 1.4 glib-2.0],
      [
       have_ecore_imf_ibus="yes"
       AC_DEFINE([BUILD_ECORE_IMF_IBUS], [1], [Ecore Imf IBUS Support])
      ],
      [have_ecore_imf_ibus="no"])
fi

AM_CONDITIONAL([BUILD_ECORE_IMF_IBUS], [test "x${have_ecore_imf_ibus}" = "xyes"])
EFL_ADD_FEATURE([ECORE_IMF], [ibus], [${have_ecore_imf_ibus}])

# scim

have_ecore_imf_scim="no"
if test "x${want_scim}" = "xyes" && test "x${want_ecore_imf_scim}" = "xyes" ; then
   PKG_CHECK_MODULES([SCIM],
      [scim],
      [
       have_ecore_imf_scim="yes"
       AC_DEFINE([BUILD_ECORE_IMF_SCIM], [1], [Ecore Imf SCIM Support])
      ],
      [have_ecore_imf_scim="no"])
fi

AM_CONDITIONAL([BUILD_ECORE_IMF_SCIM], [test "x${have_ecore_imf_scim}" = "xyes"])
EFL_ADD_FEATURE([ECORE_IMF], [scim], [${have_ecore_imf_scim}])

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
EFL_ADD_FEATURE([ECORE_IMF], [xim])

# wayland
if test "x${want_ecore_imf_wayland}" = "xyes" ; then
   PKG_CHECK_MODULES([WAYLAND],
      [wayland-client >= 1.2.0],
      [
       have_ecore_imf_wayland="yes"
       AC_DEFINE([BUILD_ECORE_IMF_WAYLAND], [1], [Ecore Imf Wayland Support])
      ],
      [have_ecore_imf_wayland="no"])
fi

AM_CONDITIONAL([BUILD_ECORE_IMF_WAYLAND], [test "x${have_ecore_imf_wayland}" = "xyes"])
EFL_ADD_FEATURE([ECORE_IMF], [wayland], [${want_ecore_imf_wayland}])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Imf])
if test "x${build_gui}" = "xno"; then
   AM_CONDITIONAL([BUILD_ECORE_IMF_IBUS], [false])
   AM_CONDITIONAL([BUILD_ECORE_IMF_SCIM], [false])
   AM_CONDITIONAL([BUILD_ECORE_IMF_XIM], [false])
   AM_CONDITIONAL([BUILD_ECORE_IMF_WAYLAND], [false])
fi
#### End of Ecore_Imf


#### Ecore_Imf_Evas
EFL_LIB_START_OPTIONAL([Ecore_Imf_Evas], [test "x${build_gui}" = "xyes"])

### Additional options to configure

### Default values

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ECORE_IMF_EVAS], [evil])

### Checks for libraries
EFL_PLATFORM_DEPEND([ECORE_IMF_EVAS], [escape])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF_EVAS], [ecore-imf])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF_EVAS], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF_EVAS], [evas])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF_EVAS], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF_EVAS], [eina])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Imf_Evas])
#### End of Ecore_Imf_Evas


#### Eeze
have_libmount_new="no"
have_libmount_old="no"
have_eeze_mount="no"

EFL_LIB_START_OPTIONAL([Eeze], [test "${have_linux}" = "yes"])

### Additional options to configure
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
EFL_INTERNAL_DEPEND_PKG([EEZE], [eo])
EFL_INTERNAL_DEPEND_PKG([EEZE], [ecore-file])
EFL_INTERNAL_DEPEND_PKG([EEZE], [ecore-con])
EFL_INTERNAL_DEPEND_PKG([EEZE], [eet])

EFL_DEPEND_PKG([EEZE], [UDEV], [libudev >= 148])

AC_ARG_ENABLE([libmount],
   [AS_HELP_STRING([--disable-libmount],[disable libmount support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_libmount="yes"
    else
       want_libmount="no"
       CFOPT_WARNING="xyes"
    fi
   ],
   [want_libmount="yes"])

EFL_OPTIONAL_DEPEND_PKG([EEZE], [${want_libmount}],
   [EEZE_MOUNT], [mount >= 2.18.0])
EFL_ADD_FEATURE([EEZE], [libmount], [${have_eeze_mount}])

PKG_CHECK_EXISTS([mount < 2.19.0],
                 [have_libmount_old="yes"],
                 [have_libmount_old="no"])
AC_MSG_CHECKING([Use old libmount API (before 2.19.0)])
AC_MSG_RESULT([${have_libmount_old}])

PKG_CHECK_EXISTS([mount == 2.19.0],
                 [have_libmount_219="yes"],
                 [have_libmount_219="no"])
AC_MSG_CHECKING([Use libmount 2.19.0 API])
AC_MSG_RESULT([${have_libmount_219}])

PKG_CHECK_EXISTS([mount > 2.19.0],
                 [have_libmount_new="yes"],
                 [have_libmount_new="no"])
AC_MSG_CHECKING([Use new libmount API (newer than 2.19.0)])
AC_MSG_RESULT([${have_libmount_new}])

if test "x${have_libmount_old}" = "xyes"; then
  AC_DEFINE_UNQUOTED([OLD_LIBMOUNT], [1], [using first version of libmount])
fi

## modules
if test "${want_tizen}"  = "yes"; then
  PKG_CHECK_MODULES([TIZEN_SENSOR], [capi-system-sensor >= 0.1.17])
fi
EFL_ADD_FEATURE([EEZE], [tizen])

EFL_EVAL_PKGS([EEZE])

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

AM_CONDITIONAL([EEZE_LIBMOUNT_AFTER_219],
               [test "x${have_libmount_new}" = "xyes"])
AM_CONDITIONAL([EEZE_LIBMOUNT_BEFORE_219],
               [test "x${have_libmount_old}" = "xyes"])
AM_CONDITIONAL([HAVE_EEZE_MOUNT], [test "x${have_eeze_mount}" = "xyes"])
AM_CONDITIONAL([HAVE_EEZE_TIZEN], [test "x${want_tizen}" = "xyes"])
#### End of Eeze


#### Ecore_Evas
EFL_LIB_START_OPTIONAL([Ecore_Evas], [test "x${build_gui}" = "xyes"])

### Additional options to configure

### Default values

want_ecore_evas_software_gdi="${have_evas_engine_software_gdi}"
want_ecore_evas_software_ddraw="${have_evas_engine_software_ddraw}"
want_ecore_evas_gl_cocoa="${have_evas_engine_gl_cocoa}"
want_ecore_evas_wayland_egl="${have_evas_engine_wayland_egl}"
want_ecore_evas_extn="yes"
want_ecore_evas_drm="${have_evas_engine_drm}"

if test "x${have_ecore_ipc}" = "xno" || \
   test "x${efl_func_shm_open}" = "xno" || \
   test "x${have_windows}" = "xyes" ; then
   want_ecore_evas_extn="no"
fi

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ECORE_EVAS], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [ecore-input-evas])
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [eet])
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [evas])
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [eina])

## modules
ECORE_EVAS_MODULE([extn], [${want_ecore_evas_extn}])
ECORE_EVAS_MODULE([ews], [yes])
ECORE_EVAS_MODULE([fb], [${want_fb}])
ECORE_EVAS_MODULE([drm], [${want_drm}], 
  [ EFL_OPTIONAL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [${want_drm}], [ecore-drm])
    EFL_OPTIONAL_INTERNAL_DEPEND_PKG([EVAS], [${want_drm}], [ecore-drm]) ]
)
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

EFL_LIB_END_OPTIONAL([Ecore_Evas])
if test "x${build_gui}" = "xno"; then
   AM_CONDITIONAL([BUILD_ECORE_EVAS_EXTN], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_EWS], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_FB], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_DRM], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_PSL1GHT], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_OPENGL_COCOA], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_SOFTWARE_SDL], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_OPENGL_SDL], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_SDL], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_WAYLAND_SHM], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_WAYLAND_EGL], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_WAYLAND], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_SOFTWARE_GDI], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_SOFTWARE_DDRAW], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_WIN32], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_SOFTWARE_X11], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_OPENGL_X11], [false])
   AM_CONDITIONAL([BUILD_ECORE_EVAS_X11], [false])
fi

#### End of Ecore_Evas

#### Eio
EFL_LIB_START([Eio])

### Additional options to configure

### Default values

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([EIO], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([EIO], [ecore])
EFL_INTERNAL_DEPEND_PKG([EIO], [eet])
EFL_INTERNAL_DEPEND_PKG([EIO], [eo])
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

# Eo Id
if test "x${want_eo_id}" = "xyes" ; then
   AC_DEFINE([HAVE_EO_ID], [1], [Have eo id])
fi

#### Eldbus
EFL_LIB_START([Eldbus])

### Additional options to configure

### Default values

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ELDBUS], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ELDBUS], [ecore])
EFL_INTERNAL_DEPEND_PKG([ELDBUS], [eo])
EFL_INTERNAL_DEPEND_PKG([ELDBUS], [eina])

EFL_DEPEND_PKG([ELDBUS], [DBUS], [dbus-1])

EFL_EVAL_PKGS([ELDBUS])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Eldbus])
#### End of Eldbus


#### Efreet
EFL_LIB_START([Efreet])

### Additional options to configure

### Default values

AC_DEFINE([SLOPPY_SPEC], [1], [Sloppy Spec Compliance])

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([EFREET], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([EFREET], [eet])
EFL_INTERNAL_DEPEND_PKG([EFREET], [ecore])
EFL_INTERNAL_DEPEND_PKG([EFREET], [ecore-file])
EFL_INTERNAL_DEPEND_PKG([EFREET], [eldbus])
EFL_INTERNAL_DEPEND_PKG([EFREET], [eo])
EFL_INTERNAL_DEPEND_PKG([EFREET], [eina])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Efreet])
#### End of Efreet


#### EPhysics
AC_ARG_ENABLE([physics],
   [AS_HELP_STRING([--disable-physics],[disable physics effects and support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_physics="yes"
    else
       CFOPT_WARNING="xyes"
       want_physics="no"
    fi
   ],
   [want_physics="yes"])

if test "x${build_gui}" = "xno"; then
   want_physics="no"
fi

EFL_LIB_START_OPTIONAL([EPhysics], [test "${want_physics}" = "yes"])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([EPHYSICS], [evil])

EFL_INTERNAL_DEPEND_PKG([EPHYSICS], [eina])
EFL_INTERNAL_DEPEND_PKG([EPHYSICS], [evas])
EFL_INTERNAL_DEPEND_PKG([EPHYSICS], [ecore])
EFL_INTERNAL_DEPEND_PKG([EPHYSICS], [eo])

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
EFL_LIB_START_OPTIONAL([Edje], [test "x${build_gui}" = "xyes"])

### Additional options to configure

### Default values
want_multisense="${want_pulseaudio}"
AC_ARG_ENABLE([multisense],
   [AS_HELP_STRING([--enable-multisense],[Enable multisense support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_multisense="yes"
    else
       want_multisense="no"
       CFOPT_WARNING="xyes"
    fi
   ],
   [want_multisense="${want_pulseaudio}"])

if test "x${build_gui}" = "xno"; then
   want_multisense="no"
fi

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
EFL_OPTIONAL_INTERNAL_DEPEND_PKG([EDJE], [${want_multisense}], [ecore-audio])

EFL_ADD_FEATURE([EDJE], [physics])
EFL_ADD_FEATURE([EDJE], [multisense])
EFL_ADD_FEATURE([EDJE], [lua-old])

if test "${want_lua_old}" = "yes"; then
   EFL_CHECK_LUA_OLD([EDJE])
   EFL_CHECK_LUA_OLD([EVAS])
else
   EFL_DEPEND_PKG([EDJE], [LUAJIT], [luajit >= 2.0.0])
   EFL_DEPEND_PKG([EVAS], [LUAJIT], [luajit >= 2.0.0])
fi

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

AC_CHECK_HEADERS([ \
sys/wait.h \
])

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Edje])

if test "x${build_gui}" = "xno"; then
   AC_DEFINE([ENABLE_MULTISENSE], [0], [Use Multisense])
   AM_CONDITIONAL([ENABLE_MULTISENSE], [false])
fi
#### End of Edje


#### Edje CXX
EFL_LIB_START([Edje_Cxx])

EFL_EVAL_PKGS([EDJE_CXX])

EFL_LIB_END([Edje_Cxx])
#### End of Edje CXX

#### Emotion
EFL_LIB_START_OPTIONAL([Emotion], [test "x${build_gui}" = "xyes"])

## Compatibility layers
EFL_PLATFORM_DEPEND([Emotion], [evil])

### Default values
if test "${efl_func_shm_open}" = "yes"; then
   want_emotion_generic="static"
else
   want_emotion_generic="no"
fi

### Additional options to configure
AC_ARG_ENABLE([xine],
   [AS_HELP_STRING([--enable-xine],[enable xine support. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_xine="yes"
    else
       want_xine="no"
    fi
   ],
   [want_xine="no"])

AC_ARG_ENABLE([v4l2],
   [AS_HELP_STRING([--enable-v4l2],[enable v4l2 support.])],
   [
    if test "x${enableval}" = "xyes" ; then
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

EFL_OPTIONAL_INTERNAL_DEPEND_PKG([EMOTION], [${efl_lib_optional_eeze}], [eeze])
EFL_ADD_FEATURE([EMOTION], [v4l2])

## modules

have_gst_xoverlay="no"

EMOTION_MODULE([xine], [${want_xine}])
EMOTION_MODULE([gstreamer], [${want_gstreamer}])
EMOTION_MODULE([gstreamer1], [${want_gstreamer1}])
EMOTION_MODULE([generic], [${want_emotion_generic}])

EFL_ADD_FEATURE([EMOTION], [xine])
EFL_ADD_FEATURE([EMOTION], [gstreamer])
EFL_ADD_FEATURE([EMOTION], [gstreamer1])
EFL_ADD_FEATURE([EMOTION], [generic], [${want_emotion_generic}])

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

EFL_LIB_END_OPTIONAL([Emotion])

if test "x${build_gui}" = "xno"; then
   AM_CONDITIONAL([EMOTION_BUILD_XINE], [false])
   AM_CONDITIONAL([EMOTION_STATIC_BUILD_XINE], [false])
   AM_CONDITIONAL([EMOTION_BUILD_GSTREAMER], [false])
   AM_CONDITIONAL([EMOTION_STATIC_BUILD_GSTREAMER], [false])
   AM_CONDITIONAL([EMOTION_BUILD_GSTREAMER1], [false])
   AM_CONDITIONAL([EMOTION_STATIC_BUILD_GSTREAMER1], [false])
   AM_CONDITIONAL([EMOTION_BUILD_GENERIC], [false])
   AM_CONDITIONAL([EMOTION_STATIC_BUILD_GENERIC], [false])
fi
#### End of Emotion


#### Ethumb
EFL_LIB_START_OPTIONAL([Ethumb], [test "x${build_gui}" = "xyes"])

### Default values

### Additional options to configure

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ETHUMB], [evil])

### Checks for libraries
EFL_PLATFORM_DEPEND([EINA], [evil])

EFL_INTERNAL_DEPEND_PKG([ETHUMB], [eina])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [eet])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [evas])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [eo])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [ecore])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [ecore-evas])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [ecore-file])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [ecore-imf])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [edje])

## modules

EFL_EVAL_PKGS([ETHUMB])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Check availability

EFL_LIB_END_OPTIONAL([Ethumb])
#### End of Ethumb

#### Ethumb_Client
EFL_LIB_START_OPTIONAL([Ethumb_Client], [test "x${build_gui}" = "xyes"])
### Default values

### Additional options to configure

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ETHUMB_CLIENT], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [eina])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [eo])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [eet])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [ecore])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [edje])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [eldbus])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [ethumb])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [evas])

EFL_EVAL_PKGS([ETHUMB_CLIENT])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Check availability

EFL_LIB_END_OPTIONAL([Ethumb_Client])
#### End of Ethumb_Client

#### Elua

have_elua="yes"
if test "${want_lua_old}" = "yes"; then
  have_elua="no"
fi

EFL_LIB_START_OPTIONAL([Elua], [test "${have_elua}" = "yes"])

### Default values

AM_CONDITIONAL([HAVE_ELUA], [test "x${have_elua}" = "xyes"])

### Additional options to configure

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ELUA], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ELUA], [eina])
EFL_INTERNAL_DEPEND_PKG([ELUA], [eo])
EFL_INTERNAL_DEPEND_PKG([ELUA], [ecore])

EFL_DEPEND_PKG([ELUA], [LUAJIT], [luajit >= 2.0.0])

EFL_EVAL_PKGS([ELUA])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Check availability

EFL_LIB_END_OPTIONAL([Elua])
#### End of Elua

AC_ARG_ENABLE([always-build-examples],
   [AS_HELP_STRING([--enable-always-build-examples],[always build examples. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_always_build_examples="yes"
    else
       want_always_build_examples="no"
    fi
   ],
   [want_always_build_examples="no"])
AM_CONDITIONAL([ALWAYS_BUILD_EXAMPLES], [test "${want_always_build_examples}" = "yes"])

BARF_OK="xno"
# Harfbuzz
AC_ARG_ENABLE([i-really-know-what-i-am-doing-and-that-this-will-probably-break-things-and-i-will-fix-them-myself-and-send-patches-aba],
   [ You will be told when this is needed ],
   [
    if test "x${enableval}" = "xyes" ; then
       BARF_OK=""
    else
       BARF_OK="xno"
    fi
   ],
   [ BARF_OK="xno" ])


AC_CONFIG_FILES([
Makefile
data/Makefile
doc/Makefile
doc/Doxyfile
doc/previews/Makefile
src/Makefile
src/benchmarks/eina/Makefile
src/benchmarks/eo/Makefile
src/benchmarks/evas/Makefile
src/examples/eina/Makefile
src/examples/eina_cxx/Makefile
src/examples/eet/Makefile
src/examples/eo/Makefile
src/examples/evas/Makefile
src/examples/ecore/Makefile
src/examples/ecore_avahi/Makefile
src/examples/eio/Makefile
src/examples/eldbus/Makefile
src/examples/ephysics/Makefile
src/examples/edje/Makefile
src/examples/emotion/Makefile
src/examples/ethumb_client/Makefile
src/examples/elua/Makefile
src/examples/eolian_cxx/Makefile
src/lib/eina/eina_config.h
src/lib/ecore_x/ecore_x_version.h
src/lib/efl/Efl_Config.h
spec/efl.spec
pc/evil.pc
pc/escape.pc
pc/eina.pc
pc/eina-cxx.pc
pc/eet.pc
pc/eet-cxx.pc
pc/eo.pc
pc/eo-cxx.pc
pc/eolian.pc
pc/eolian-cxx.pc
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
pc/evas-drm.pc
pc/evas.pc
pc/evas-cxx.pc
pc/ecore.pc
pc/ecore-cxx.pc
pc/ecore-con.pc
pc/ecore-ipc.pc
pc/ecore-file.pc
pc/ecore-input.pc
pc/ecore-input-evas.pc
pc/ecore-cocoa.pc
pc/ecore-drm.pc
pc/ecore-fb.pc
pc/ecore-psl1ght.pc
pc/ecore-sdl.pc
pc/ecore-wayland.pc
pc/ecore-win32.pc
pc/ecore-x.pc
pc/ecore-evas.pc
pc/ecore-imf.pc
pc/ecore-imf-evas.pc
pc/ecore-audio.pc
pc/ecore-audio-cxx.pc
pc/ecore-avahi.pc
pc/embryo.pc
pc/eio.pc
pc/eldbus.pc
pc/efreet.pc
pc/efreet-mime.pc
pc/efreet-trash.pc
pc/eeze.pc
pc/ephysics.pc
pc/edje.pc
pc/edje-cxx.pc
pc/emotion.pc
pc/ethumb.pc
pc/ethumb_client.pc
dbus-services/org.enlightenment.Efreet.service
dbus-services/org.enlightenment.Ethumb.service
systemd-services/efreet.service
systemd-services/ethumb.service
$po_makefile_in
$po_makevars
cmakeconfig/EinaConfig.cmake
cmakeconfig/EinaConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EezeConfig.cmake
cmakeconfig/EezeConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EoConfig.cmake
cmakeconfig/EoConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EolianConfig.cmake
cmakeconfig/EolianConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EolianCxxConfig.cmake
cmakeconfig/EolianCxxConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EinaCxxConfig.cmake
cmakeconfig/EinaCxxConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EoCxxConfig.cmake
cmakeconfig/EoCxxConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EcoreCxxConfig.cmake
cmakeconfig/EcoreCxxConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EvasCxxConfig.cmake
cmakeconfig/EvasCxxConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EetCxxConfig.cmake
cmakeconfig/EetCxxConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EetConfig.cmake
cmakeconfig/EetConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EvasConfig.cmake
cmakeconfig/EvasConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EcoreConfig.cmake
cmakeconfig/EcoreConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EdjeConfig.cmake
cmakeconfig/EdjeConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EldbusConfig.cmake
cmakeconfig/EldbusConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EfreetConfig.cmake
cmakeconfig/EfreetConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EthumbConfig.cmake
cmakeconfig/EthumbConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EthumbClientConfig.cmake
cmakeconfig/EthumbClientConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
cmakeconfig/EmotionConfig.cmake
cmakeconfig/EmotionConfigVersion.cmake:cmakeconfig/EFLConfigVersion.cmake.in
])

AC_OUTPUT

#### Work around bug in automake check macro
## yes it is hugly, but no choice here for now.
$SED -i "s/am__is_gnu_make = test -n '\$(MAKEFILE_LIST)' && test -n '\$(MAKELEVEL)'/ifdef MAKEFILE_LIST\nifdef MAKELEVEL\nam__is_gnu_make = true\nelse\nam__is_gnu_make = false\nendif\nelse\nam__is_gnu_make = false\nendif/" src/Makefile

if test -f $srcdir/config.status; then
   TO="$SED -i \"s/am__is_gnu_make = test -n '\\\\\$(MAKEFILE_LIST)' \&\& test -n '\\\\\$(MAKELEVEL)'/ifdef MAKEFILE_LIST\\\nifdef MAKELEVEL\\\nam__is_gnu_make = true\\\nelse\\\nam__is_gnu_make = false\\\nendif\\\nelse\\\nam__is_gnu_make = false\\\nendif/\" src/Makefile\nas_fn_exit 0"
   $SED -i "s|as_fn_exit 0|$TO|" $srcdir/config.status
fi

#### Info

EFL_ADD_FEATURE([EO], [eo-id], [${want_eo_id}])

case $host_cpu in
  i*86|x86_64|amd64)
    EFL_ADD_FEATURE([cpu], [mmx], [${build_cpu_mmx}])
    EFL_ADD_FEATURE([cpu], [sse3], [${build_cpu_sse3}])
    ;;
  *power* | *ppc*)
    EFL_ADD_FEATURE([cpu], [altivec], [${build_cpu_altivec}])
    ;;
  arm*)
    EFL_ADD_FEATURE([cpu], [neon], [${build_cpu_neon}])
    ;;
esac

if test "${have_linux}" = "yes"; then
   EFL_ADD_FEATURE([system], [inotify])
   EFL_ADD_FEATURE([system], [atfile_source])
elif test "${have_windows}" = "yes"; then
   EFL_ADD_FEATURE([system], [notify_win32])
fi
EFL_ADD_FEATURE([system], [ipv6])

EFL_ADD_FEATURE([thread], [spinlocks], [${efl_have_posix_threads_spinlock}])
EFL_ADD_FEATURE([thread], [barrier], [${efl_have_pthread_barrier}])
EFL_ADD_FEATURE([thread], [affinity], [${efl_have_setaffinity}])

echo
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE_NAME $PACKAGE_VERSION"
echo "------------------------------------------------------------------------"
echo

if test "x${have_windows}" = "xyes" ; then
   osname="${host_os}(${_efl_windows_version})"
else
   osname="${host_os}"
fi

echo "Configuration...: ${COLOR_OTHER}profile=${build_profile} os=${osname}${COLOR_RESET}"
echo "  EFL API Set...: ${efl_api}"
echo "  CPU Extensions: ${host_cpu} (${features_cpu})"
echo "  System Feature: ${features_system}"
echo "  Threads.......: ${efl_have_threads} (${features_thread})"
echo "  Cryptography..: ${build_crypto}"
echo "  X11...........: ${with_x11}"
echo "  OpenGL........: ${with_opengl}"
echo "  C++11.........: ${have_cxx11}"
echo "  GUI libs......: ${build_gui}"
echo "Evas............: ${efl_lib_optional_evas} (${features_evas})"
echo "  Engines.......: ${features_evas_engine}"
echo "  Image Loaders.: ${features_evas_loader}"
if test "x${have_pixman}" = "xyes" ; then
echo "  Pixman........: ${features_evas_pixman}"
fi
echo "Eo..............: yes (${features_eo})"
echo "Eolian..........: yes (${features_eolian})"
echo "Eina............: yes (${features_eina})"
echo "Ecore...........: yes (${features_ecore})"
echo "Ecore_Con.......: yes (${features_ecore_con})"
echo "Ecore_File......: yes"
echo "Ecore_IMF.......: ${efl_lib_optional_ecore_imf} (${features_ecore_imf})"
echo "Ecore_X.........: ${with_x11} (${features_ecore_x})"
echo "Ecore_SDL.......: $want_sdl"
echo "Ecore_Wayland...: $want_wayland"
if test "${have_linux}" = "yes"; then
echo "Ecore_FB........: $want_fb (${features_ecore_fb})"
elif test "${have_ps3}" = "yes"; then
echo "Ecore_PSL1GHT...: $have_ps3"
elif test "${have_darwin}" = "yes"; then
echo "Ecore_Cocoa.....: $efl_lib_optional_ecore_cocoa"
elif test "${have_windows}" = "yes"; then
echo "Ecore_Win32.....: $have_win32"
fi
echo "Ecore_Audio.....: ${efl_lib_optional_ecore_audio} (${features_ecore_audio})"
echo "Ecore_Avahi.....: yes (${features_ecore_avahi})"
echo "Ecore_Evas......: ${efl_lib_optional_ecore_evas} (${features_ecore_evas})"
echo "Eeze............: ${efl_lib_optional_eeze} (${features_eeze})"
echo "EPhysics........: ${efl_lib_optional_ephysics}"
echo "Edje............: ${efl_lib_optional_edje} (${features_edje})"
echo "Emotion.........: ${efl_lib_optional_emotion} (${features_emotion})"
echo "Ethumb..........: ${efl_lib_optional_ethumb}"
echo "Ethumb_Client...: ${efl_lib_optional_ethumb_client}"
echo "Elua............: $have_elua"
if test "${build_tests}" = "none"; then
echo "Tests...........: no"
elif test "${build_tests}" = "auto"; then
echo "Tests...........: make check (inexplicitly enabled)"
elif test "${build_tests}" = "regular"; then
echo "Tests...........: make check"
elif test "${build_tests}" = "coverage"; then
echo "Tests...........: make lcov-check"
fi
echo "Examples........: make examples (make install-examples)"
if test "x${build_doc}" = "xyes"; then
echo "Documentation...: make doc"
else
echo "Documentation...: no"
fi
echo "Compilation.....: make (or gmake)"
echo "  CPPFLAGS......: $CPPFLAGS"
echo "  CFLAGS........: $CFLAGS"
echo "  CXXFLAGS......: $CXXFLAGS"
echo "  LDFLAGS.......: $LDFLAGS"

if test "x${with_binary_edje_cc}" != "x"; then
echo "  edje_cc.......: ${with_binary_edje_cc}"
fi

if test "x${with_binary_eolian_gen}" != "x"; then
echo "  eolian_gen....: ${with_binary_eolian_gen}"
fi

if test "x${with_binary_eolian_cxx}" != "x"; then
echo "  eolian_cxx....: ${with_binary_eolian_cxx}"
fi

if test "x${with_binary_elua_bin}" != "x"; then
echo "  elua..........: ${with_binary_elua_bin}"
fi

echo "  "
echo "Installation....: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix........: $prefix"
echo "  dbus units....: $dbusservicedir"
if test "${have_systemd_user_session}" = "yes"; then
echo "  systemd units.: $USER_SESSION_DIR"
fi
echo

if test "x${have_systemd_pkg}" = "xyes" -a "x${want_systemd}" = "xno"; then
   echo " _________________________________________"
   echo "/ Systemd dependency is available on your \\"
   echo "| system, but you are building without    |"
   echo "| systemd support. Don't forget to        |"
   echo "| --enable-systemd if you want systemd    |"
   echo "\\ integration for EFL.                    /"
   echo " -----------------------------------------"
   echo "        \\   ^__^"
   echo "         \\  (oo)\\_______"
   echo "            (__)\\       )\\/\\"
   echo "                ||----w |"
   echo "                ||     ||"
fi


if test -n "$CFOPT_WARNING"; then
  echo "_____________________________________________________________________"
  echo ""
  echo "==-- WARNING --=="
  echo ""
  echo "_____________________________________________________________________"
  if test "x${with_x11}" = "xxcb"; then
    echo "_____________________________________________________________________"
    echo "You have chosen to use XCB instead of Xlib. It is a myth that XCB"
    echo "is amazingly faster than Xlib (when used sensibly). It can be"
    echo "faster in a few corner cases on startup of an app, but it comes"
    echo "with many downsides. One of those is more complex code inside"
    echo "ecore_x, which is far less tested in XCB mode than Xlib. Also"
    echo "the big catch is that OpenGL support basically requires Xlib anyway"
    echo "so if you want OpenGL in X11, you need Xlib regardless and so you"
    echo "gain nothing really in terms of speed and no savings in memory"
    echo "because Xlib is still linked, loaded and used, BUT instead you"
    echo "have OpenGL drivers working with an hybrid XCB/Xlib (mostly XCB)"
    echo "toolkit and this is basically never tested by anyone working on"
    echo "the OpenGL drivers, so you will have bugs. Do not enable XCB"
    echo "and use OpenGL. XCB is only useful if you wish to shave a few Kb"
    echo "off the memory footprint of a whole system and live with less"
    echo "tested code, and possibly unimplemented features in ecore_x. To"
    echo "remove the XCB setup, remove the --with-x11=xcb option to"
    echo "configure."
    echo "_____________________________________________________________________"
  fi
  if test "x${build_crypto}" = "xgnutls"; then
    echo "_____________________________________________________________________"
    echo "You have chosen gnutls as the crypto back-end. This will have some"
    echo "side-effects that can break set-uid root binaries that happen to"
    echo "link to and/or use EFL. These do actually exist. Gnutls will drop"
    echo "root privs if it detects being setuid, thus breaking these tools"
    echo "and their functionality. Only enable gnutls if you REALLY know"
    echo "what you are doing and are willing to live with broken "
    echo "functionality."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_physics}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "You have chosen to disable physics support. This disables lots of"
    echo "core functionality and is effectively never tested. You are going"
    echo "to find features that suddenly don't work and as a result cause"
    echo "a series of breakages. This is simply not tested so you are on"
    echo "your own in terms of ensuring everything works if you do this"
    echo "_____________________________________________________________________"
  fi
  if test "x${efl_have_threads}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "You have disabled threading support. A lot of code is literally"
    echo "written to need threading. We never test or even build with"
    echo "threading disabled, so doing this is entering uncharted territory."
    echo "There is a very good chance things may not compile at all, or if"
    echo "the do, they will break at runtime in weird and wonderful ways."
    echo "Highly reconsider what you are doing here, or be prepared to deal"
    echo "with the fallout yourself."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_fontconfig}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "You have disabled fontconfig. This is going to make general font"
    echo "searching not work, and only some very direct 'load /path/file.ttf'"
    echo "will work alongside some old-school ttf file path searching. This"
    echo "is very likely not what you want, so highly reconsider turning"
    echo "fontconfig off. Having it off will lead to visual problems like"
    echo "missing text in many UI areas etc."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_fribidi}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "Fribidi is used for handling right-to-left text (like Arabic,"
    echo "Hebrew, Farsi, Persian etc.) and is very likely not a feature"
    echo "you want to disable unless you know for absolute certain you"
    echo "will never encounter and have to display such scripts. Also"
    echo "note that we don't test with fribidi disabled so you may also"
    echo "trigger code paths with bugs that are never normally used."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_pixman}" = "xyes"; then
    echo "_____________________________________________________________________"
    echo "Pixman allows you to replace some rendering paths in Evas with"
    echo "Pixman. Pixman may or may not be faster (probably slower), and"
    echo "the rendering paths with Pixman enabled are not tested often so"
    echo "this may introduce rendering bugs. Do not turn Pixman on unless"
    echo "you wish to deal with these bugs."
    echo "_____________________________________________________________________"
  fi
  if test "x${have_tile_rotate}" = "xyes"; then
    echo "_____________________________________________________________________"
    echo "Tiled rotation code is not tested much, so be aware that you"
    echo "may introduce bugs by enabling this."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_g_main_loop}" = "xyes"; then
    echo "_____________________________________________________________________"
    echo "Using the Glib mainloop as the mainloop in Ecore is not tested"
    echo "regularly, but the glib mainloop integration (on by default) is."
    echo "You can use apps that use glib mainloop constructs by default"
    echo "this way, but the Ecore mainloop is not built on top of glib."
    echo "You have enabled ecore to be built on top of glib and thus you"
    echo "may experience bugs that normally would not be there. Be prepared"
    echo "to fix these if they arise."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_gstreamer}" = "xyes"; then
    echo "_____________________________________________________________________"
    echo "Gstreamer 0.10 is no longer supported, and EFL has moved to use"
    echo "Gstreamer 1.x. The old Gstremaer code is not tested or maintained"
    echo "and will eventually be removed entirely. Don't enable the old"
    echo "Gstreamer support unless you want to deal with the issues yourself."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_gstreamer1}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "You disabled Gstreamer 1.x support. You likely don't want to do"
    echo "this as it will heavily limit your media support options and render"
    echo "some functionality as useless, leading to visible application bugs."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_eo_id}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "Eo's ID abstraction interface is a major safety system that"
    echo "protects code from crashing or misbehaving in many cases. It does"
    echo "come at a slight cost, but the safety and protection is worth it."
    echo "Also by disabling this, you may also introduce security holes in"
    echo "EFL as well as cause all sorts of previously non-existant crashes."
    echo "Seriously reconsider disabling EO ID."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_evas_cserve2}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "Evas Cserve is built and on by default and no testing is done"
    echo "for the old non-cserve2 code paths, so by disabling this you"
    echo "may be introducing bugs. Be aware of this and be prepared to"
    echo "deal with the bugs as a result of this."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_audio}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "You disabled audio support in Ecore. This is not tested and may"
    echo "Create bugs for you due to it creating untested code paths."
    echo "Reconsider disabling audio."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_pulseaudio}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "The only audio output method supported by Ecore right now is via"
    echo "Pulseaudio. You have disabled that and likely have broken a whole"
    echo "bunch of things in the process. Reconsider your configure options."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_xinput2}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "You have disabled xinput2 support. This means a whole lot of input"
    echo "devices in X11 will not work correctly. You likely do not want to"
    echo "do this."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_xim}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "You disabled XIM input method support. This is the most basic and"
    echo "core input method protocol supported in X11 and you almost certainly"
    echo "want the support for it. Input methods allow for complex text input"
    echo "like for Chinese, Japanese and Korean as well as virtual keyboards"
    echo "on touch/mobile devices."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_scim}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "SCIM is a modern and very common input method framework and you"
    echo "disabled support for it. You very likely want the support for"
    echo "complex language input, so please reconsider this. Input methods"
    echo "allow for complex text input like for Chinese, Japanese and Korean"
    echo "as well as virtual keyboards on touch/mobile devices."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_libmount}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "Libmount has been disabled, and it is used heavily inside Eeze"
    echo "for support of removable devices etc. and disabling this will"
    echo "hurt support for Enlightenment and its filemanager."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_multisense}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "Multisense has been disabled. This causes Edje audio suport to"
    echo "Simply not work, and will break applications and libraries"
    echo "that rely on it with users then reporting bugs."
    echo "If you want to mute audio, there are APIs and policies to do"
    echo "that, as opposed to compiling it out."
    echo "_____________________________________________________________________"
  fi
  echo "_____________________________________________________________________"
  echo ""
  echo "==-- WARNING --=="
  echo ""
  echo "_____________________________________________________________________"
  if test -n "$BARF_OK"; then
    echo "Please add the following option to acknowledge this:"
    echo "  --enable-i-really-know-what-i-am-doing-and-that-this-will-probably-break-things-and-i-will-fix-them-myself-and-send-patches-aba"
    echo "_____________________________________________________________________"
    rm -f Makefile
    exit 1
  fi
  sleep 10
fi

if test "x$prefix" != "x/usr"; then
  old=
  path=$dbusservicedir
  while test "x$old" != "x$path"; do
    old=$path
    eval path="\"$path\""
  done
  resolved_dbusservicedir=$path

  old=
  path=$USER_SESSION_DIR
  while test "x$old" != "x$path"; do
    old=$path
    eval path="\"$path\""
  done
  resolved_USER_SESSION_DIR=$path
  base_USER_SESSION_DIR=`echo "$resolved_USER_SESSION_DIR" | sed -e 's:^\(.*\)/systemd/user/*$:\1:g'`

  old=
  path=$datadir
  while test "x$old" != "x$path"; do
    old=$path
    eval path="\"$path\""
  done
  resolved_datadir=$path

  needs_alert_dbus=0
  if test "$resolved_dbusservicedir" = "${HOME}/.local/share/dbus-1/services"; then
    AC_MSG_NOTICE([installing DBus services in user local "$resolved_dbusservicedir". Only accessible to user $USER])
  elif echo "$XDG_DATA_DIRS" | grep -e "$resolved_datadir" >/dev/null 2>/dev/null; then
    AC_MSG_NOTICE([installing DBus services in "$resolved_datadir" set in \$XDG_DATA_DIRS. Every user must have \$XDG_DATA_DIRS containing "$resolved_datadir".])
  elif echo "$resolved_dbusservicedir" | grep -e '^/usr/s' >/dev/null 2>/dev/null; then
    AC_MSG_NOTICE([installing DBus serivces in $resolved_dbusservicedir])
  else
    needs_alert_dbus=1
  fi

  needs_alert_systemd=0
  if test "$have_systemd_user_session" = "yes"; then
    if test "$resolved_USER_SESSION_DIR" = "${HOME}/.config/systemd/user"; then
      AC_MSG_NOTICE([installing systemd services in user local "$resolved_USER_SESSION_DIR". Only accessible to user $USER])
    elif echo "$XDG_DATA_DIRS" | grep -e "$base_USER_SESSION_DIR" >/dev/null 2>/dev/null; then
      AC_MSG_NOTICE([installing systemd services in "$base_USER_SESSION_DIR" set in \$XDG_DATA_DIRS. Every user must have \$XDG_DATA_DIRS containing "$base_USER_SESSION_DIR".])
    elif echo "$XDG_CONFIG_DIRS" | grep -e "$base_USER_SESSION_DIR" >/dev/null 2>/dev/null; then
      AC_MSG_NOTICE([installing systemd services in "$base_USER_SESSION_DIR" set in \$XDG_CONFIG_DIRS. Every user must have \$XDG_CONFIG_DIRS containing "$base_USER_SESSION_DIR".])
    elif echo "$resolved_USER_SESSION_DIR" | grep -e '^/usr/s' >/dev/null 2>/dev/null; then
      AC_MSG_NOTICE([installing systemd serivces in $resolved_USER_SESSION_DIR])
    else
       needs_alert_systemd=1
    fi
  fi

  if test $needs_alert_dbus -eq 1 -o $needs_alert_systemd -eq 1; then
    if test $needs_alert_dbus -eq 1 -a $needs_alert_systemd -eq 1; then
      what_alert="dbus and systemd"
    elif test $needs_alert_dbus -eq 1; then
      what_alert="dbus"
    else
      what_alert="systemd"
    fi

    echo ""
    echo "#-------------------------------------------------------------------#"
    echo "##==--                          ALERT                          --==##"
    echo "#-------------------------------------------------------------------#"
    echo ""
    echo "  Your installation prefix is *NOT* /usr so this means you need"
    echo "to ensure some files are visible to $what_alert otherwise services cannot"
    echo "be started when needed. You will need to do the following:"
    if test $needs_alert_dbus -eq 1; then
      echo ""
      echo "System-wide installation:"
      echo "  ln -s ${resolved_dbusservicedir}/org.enlightenment.Ethumb.service /usr/share/dbus-1/services/org.enlightenment.Ethumb.service"
      echo "  ln -s ${resolved_dbusservicedir}/org.enlightenment.Efreet.service /usr/share/dbus-1/services/org.enlightenment.Efreet.service"
      echo ""
      echo "  or add \"${resolved_datadir}\" to \$XDG_DATA_DIRS"
      echo ""
      echo "User installation:"
      echo "  ln -s ${resolved_dbusservicedir}/org.enlightenment.Ethumb.service ~/.local/share/dbus-1/services/org.enlightenment.Ethumb.service"
      echo "  ln -s ${resolved_dbusservicedir}/org.enlightenment.Efreet.service ~/.local/share/dbus-1/services/org.enlightenment.Efreet.service"
    fi
    if test $needs_alert_systemd -eq 1; then
      echo ""
      echo "System-wide installation:"
      echo "  ln -s ${resolved_USER_SESSION_DIR}/ethumb.service /usr/lib/systemd/user/ethumb.service"
      echo "  ln -s ${resolved_USER_SESSION_DIR}/efreet.service /usr/lib/systemd/user/efreet.service"
      echo ""
      echo "  or add \"${base_USER_SESSION_DIR}\" to \$XDG_DATA_DIRS or \$XDG_CONFIG_DIRS"
      echo ""
      echo "User installation:"
      echo "  ln -s ${resolved_USER_SESSION_DIR}/ethumb.service ~/.config/systemd/user/ethumb.service"
      echo "  ln -s ${resolved_USER_SESSION_DIR}/efreet.service ~/.config/systemd/user/efreet.service"
    fi
    echo ""
    echo "#-------------------------------------------------------------------#"
  fi
fi

if test "x${efl_deprecated_option}" = "xyes"; then
  echo ""
  echo "#-------------------------------------------------------------------#"
  echo "##==--                          ALERT                          --==##"
  echo "#-------------------------------------------------------------------#"
  echo ""
  echo "  Your build script is using a deprecated option. It will get b0rken"
  echo "with the next release of EFL. You better update it now than later."
  echo ""
  echo "#-------------------------------------------------------------------#"
fi

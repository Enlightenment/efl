EFL_VERSION([1], [22], [0], [release])
AC_INIT([efl], [efl_version-alpha1], [enlightenment-devel@lists.sourceforge.net])

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


AM_INIT_AUTOMAKE([1.6 dist-xz no-dist-gzip -Wall color-tests subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_CONDITIONAL([HAVE_AM_16], [test $(echo "${am__api_version}"|cut -d. -f2) -ge 16])
# Due to a bug in automake 1.14 we need to use this after AM_INIT_AUTOMAKE
# http://debbugs.gnu.org/cgi/bugreport.cgi?bug=15981
AC_USE_SYSTEM_EXTENSIONS

CFOPT_WARNING=""
CXXFLAGS="${CXXFLAGS} -Wno-shadow" # No shadow warnings

#### Apply configuring with legacy api's only, eo api's or both.

EFL_API_LEGACY_DEF="#define EFL_NOLEGACY_API_SUPPORT"
EFL_API_EO_DEF="#define EFL_EO_API_SUPPORT"
efl_api="both"

#### For the moment the Eo EFL API is not ready you need to explicitly optin.
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
        CFOPT_WARNING="xyes"
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
eina_log_backtrace="yes"
efl_less_insane="no"
case "${build_profile}" in
   dev|debug)
     prefer_assert="yes"
     eina_log_backtrace="no"
     efl_less_insane="yes"
     ;;
esac

# Enable CRI & ERR backtrace by default for release but not for dev/debug
AC_DEFINE_IF([EINA_LOG_BACKTRACE_ENABLE], [test "x${eina_log_backtrace}" = "xyes"], [1], [Default log level triggering backtraces])

# Additional sanity checks only performed in debug builds
AC_DEFINE_IF([EFL_EXTRA_SANITY_CHECKS], [test "x${efl_less_insane}" = "xyes"], [1], [Extra run time sanity checks])

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
                    @<:@default=auto@:>@])],
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

AC_ARG_WITH([ecore-con-http-test-url],
   [AS_HELP_STRING([--with-ecore-con-http-test-url=http://username:password@example.com],[Url of http server for testing with username and password])],[ECORE_CON_HTTP_TEST_URL=${withval}][AC_DEFINE_UNQUOTED([ECORE_CON_HTTP_TEST_URL],["$withval"],[Http url for testing])])

AC_ARG_WITH([ecore-con-ftp-test-url],
   [AS_HELP_STRING([--with-ecore-con-ftp-test-url=ftp://username:password@ftp.example.com?file=filename&directory=dir],[Url of ftp server for testing with username, password, complete filepath for upload with optional directory])],[ECORE_CON_FTP_TEST_URL=${withval}][AC_DEFINE_UNQUOTED([ECORE_CON_FTP_TEST_URL],["$withval"],[Ftp url for testing])])

dbusservicedir="${datadir}/dbus-1/services"
AC_ARG_WITH([dbus-services],
   [AS_HELP_STRING([--with-dbus-services=DBUS_SERVICES],[specify a directory to store dbus service files.])],
   [dbusservicedir=$withval])
AC_SUBST(dbusservicedir)

efl_deprecated_option="no"
EFL_WITH_BIN([eet], [eet-eet])
EFL_WITH_BIN([edje], [edje-cc])
EFL_WITH_BIN([eolian], [eolian-gen])
EFL_WITH_BIN([eolian_cxx], [eolian-cxx])
EFL_WITH_BIN([eolian_mono], [eolian-mono])
EFL_WITH_BIN([eolian-js], [eolian-js])
EFL_WITH_BIN_SUFFIX([elua], [elua], [_bin])
EFL_WITH_BIN([eldbus], [eldbus_codegen])
EFL_WITH_BIN([elementary], [elementary-codegen])
EFL_WITH_BIN([elementary], [elm-prefs-cc])

#### Default values

requirements_pc_eflall=""
requirements_pc_deps_eflall=""
requirements_libs_eflall=""
requirements_cflags_eflall=""

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
   *solaris*)
      have_solaris="yes"
      MODULE_ARCH="v-v_maj.v_min"
      MODULE_EXT=".so"
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

ELM_UNIX_DEF="#undef"
ELM_WIN32_DEF="#undef"

have_systemd_pkg="no"
have_win32="no"
have_windows="no"
have_freebsd="no"
have_darwin="no"
have_linux="no"
have_ps3="no"
case "$host_os" in
   mingw*|cygwin*)
      # TODO: check cygwin* here
      have_win32="yes"
      have_windows="yes"
      ELM_WIN32_DEF="#define"
      EFLALL_CFLAGS="${EFLALL_CFLAGS} -D__USE_MINGW_ANSI_STDIO"
   ;;
   freebsd*)
      have_freebsd="yes"
      ELM_UNIX_DEF="#define"
   ;;
   darwin*)
      have_darwin="yes"
      ELM_UNIX_DEF="#define"
   ;;
   linux*)
      have_linux="yes"
      have_systemd_pkg="auto"
      want_systemd="yes"
      ELM_UNIX_DEF="#define"
   ;;
   *)
      ELM_UNIX_DEF="#define"
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
AC_DEFINE_UNQUOTED([EXEEXT], ["${EXEEXT}"], [Suffix for binary objects])

# TODO: move me to m4 file that setups the windows related variables
AM_CONDITIONAL([HAVE_WIN32], [test "x${have_win32}" = "xyes"])
AM_CONDITIONAL([HAVE_WINDOWS], [test "x${have_windows}" = "xyes"])
AM_CONDITIONAL([HAVE_PS3], [test "x${have_ps3}" = "xyes"])

AM_CONDITIONAL([HAVE_FREEBSD], [test "x${have_freebsd}" = "xyes"])
AM_CONDITIONAL([HAVE_OSX], [test "x${have_darwin}" = "xyes"])
AM_CONDITIONAL([HAVE_X86_64], [test "x${host_cpu}" = "xx86_64"])

AC_SUBST([ELM_UNIX_DEF])
AC_SUBST([ELM_WIN32_DEF])

#### Checks for programs

### libtool

if test "x${have_windows}" = "xyes" ; then
   lt_cv_deplibs_check_method='pass_all'
fi
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT([win32-dll disable-static pic-only])
EFL_INIT

if test "x${have_win32}" = "xyes" ; then
    # We should be using ${CXX} here, but there is a bug in
    # autotools macro and CXX is always set to g++ even if
    # it's not found. So we are using an internal variable
    # that does the work for now, may get broken in the future.
    if test "x${ac_ct_CXX}" = "x" -a "x${CXX}" = "xg++"; then
       AC_MSG_ERROR([efl requires a C++ compiler got ${ac_ct_CXX} and ${CXX}.])
    fi
else
   CXXFLAGS="${CXXFLAGS} -fPIC -DPIC"
   LDFLAGS="${LDFLAGS} -fPIC -DPIC"
fi

### gettext

AM_GNU_GETTEXT_VERSION([0.18])

m4_ifdef([AC_GNU_GETTEXT], [
AC_GNU_GETTEXT([external])
po_makefile_in=po/Makefile.in
have_po="yes"
],
[
m4_ifdef([AM_GNU_GETTEXT], [
AM_GNU_GETTEXT([external])
po_makefile_in=po/Makefile.in
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

AC_PROG_MKDIR_P
AM_PROG_AS
AC_PROG_CXX
AC_PROG_OBJC
AC_LANG(C)
AC_PROG_CC_C99
AM_PROG_CC_C_O
AC_PROG_SED
AC_PROG_MCS

AM_CONDITIONAL([BUILD_EFL_NATIVE], [test "x${cross_compiling}" = "xno"])

if test "x${ac_cv_prog_cc_c99}" = "xno" ; then
   AC_MSG_ERROR([efl requires a c99-capable compiler])
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
      AC_SEARCH_LIBS([__gcov_init], [gcov])
      EFL_CHECK_COMPILER_FLAGS([EFLALL], [-fprofile-arcs -ftest-coverage])
      EFL_CHECK_COMPILER_FLAGS([EFLALL], [-fprofile-instr-generate -fcoverage-mapping])
      EFL_CHECK_LINKER_FLAGS([EFLALL], [-fprofile-instr-generate -fcoverage-mapping])
      if test "x${prefer_assert}" = "xno"; then
         EFLALL_COV_CFLAGS="${EFLALL_COV_CFLAGS} -DNDEBUG"
      else
         EFLALL_COV_CFLAGS="${EFLALL_COV_CFLAGS} -g -O0"
      fi
   else
      AC_MSG_ERROR([lcov is not found])
   fi
fi

AM_CONDITIONAL([EFL_ENABLE_COVERAGE], [test "${want_coverage}" = "yes"])

#### Checks for libraries

# check unit testing library

if test "${want_tests}" = "yes"; then
   PKG_CHECK_MODULES([CHECK], [check >= 0.9.10])
fi
AM_CONDITIONAL([EFL_ENABLE_TESTS], [test "${want_tests}" = "yes"])


# check for crypto/tls library to use
case "$build_crypto" in
   gnutls)
      EFL_DEPEND_PKG([crypto], [GNUTLS], [gnutls >= 3.3.6])

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


want_liblz4="no"
AC_ARG_ENABLE([liblz4],
   [AS_HELP_STRING([--enable-liblz4],[Enable usage of liblz4 instead of our embedded copy. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
	# Only ships pc file since r120
	PKG_CHECK_MODULES([LIBLZ4], [liblz4])
	EFL_DEPEND_PKG([EET], [LIBLZ4], [liblz4])
	EFL_DEPEND_PKG([EVAS], [LIBLZ4], [liblz4])
       want_liblz4="yes"
    else
       want_liblz4="no"
    fi
   ],
   [want_liblz4="no"])

AM_CONDITIONAL([ENABLE_LIBLZ4], [test "${want_liblz4}" = "yes"])
AC_DEFINE_IF([ENABLE_LIBLZ4], [test "${want_liblz4}" = "yes"], [1], [Use liblz4 external library instead of embedded copy])
AC_SUBST([want_liblz4])
AC_SUBST([ENABLE_LIBLZ4])


want_vnc_server="no"
AC_ARG_ENABLE([vnc-server],
   [AS_HELP_STRING([--enable-vnc-server],[Build Ecore_Evas VNC module. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       PKG_CHECK_MODULES([LIBVNCSERVER], [libvncserver])
       want_vnc_server="yes"
    else
       want_vnc_server="no"
    fi
   ],
   [want_vnc_server="no"])

#### Checks for header files

# Common Checks (keep names sorted for ease of use):
AC_HEADER_DIRENT
AC_HEADER_TIME

AC_CHECK_HEADERS([ \
execinfo.h \
mcheck.h \
sys/signalfd.h \
sys/types.h \
sys/param.h \
sys/mman.h \
netinet/in.h \
sys/auxv.h \
asm/hwcap.h \
])

if test "x${have_linux}" = "xyes" ; then
    AC_CHECK_HEADERS([sys/inotify.h])
    AC_CHECK_HEADERS([sys/epoll.h])
fi

EFL_CHECK_PATH_MAX

#### Checks for defines in headers

EFL_CHECK_DEFINE(TCP_CORK, netinet/tcp.h)
EFL_CHECK_DEFINE(TCP_NOPUSH, netinet/tcp.h)
EFL_CHECK_DEFINE(UDP_CORK, netinet/udp.h)

#### Checks for types

# wchar_t
AC_CHECK_SIZEOF([wchar_t])
EINA_SIZEOF_WCHAR_T=$ac_cv_sizeof_wchar_t
AC_SUBST([EINA_SIZEOF_WCHAR_T])

AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)

AC_CHECK_SIZEOF([uintptr_t])
EINA_SIZEOF_UINTPTR_T=$ac_cv_sizeof_uintptr_t
AC_SUBST([EINA_SIZEOF_UINTPTR_T])

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
EFL_CHECK_COMPILER_FLAGS([EFLALL], [-Wall -Wextra -Wpointer-arith -Wno-missing-field-initializers -fvisibility=hidden -fdata-sections -ffunction-sections])
EFL_CHECK_LINKER_FLAGS([EFLALL], [-fvisibility=hidden -fdata-sections -ffunction-sections -Wl,--gc-sections -fno-strict-aliasing -Wl,--as-needed -Wl,--no-copy-dt-needed-entries])
case "${build_profile}" in
   dev)
     dnl Check if compiler has a dodgy -Wshadow that emits errors when shadowing a global
     AC_MSG_CHECKING([whether -Wshadow generates spurious warnings])
     CFLAGS_save="${CFLAGS}"
     CFLAGS="${CFLAGS} -Werror -Wshadow"
     AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[int x;]], [[int x = 0; (void)x;]])],[AC_MSG_RESULT([no])
        EFL_CHECK_COMPILER_FLAGS([EFLALL], [-Wshadow])],[AC_MSG_RESULT([yes])])
     CFLAGS="${CFLAGS_save}"
     ;;

   debug)
     ;;

   release)
     ;;
esac

EFL_ATTRIBUTE_VECTOR
EFL_ATTRIBUTE_ALWAYS_INLINE

EFLALL_LIBS="${EFLALL_LIBS}"
EFLALL_CFLAGS="${EFLALL_CFLAGS}"

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
       CFLAGS="-mfpu=neon -ftree-vectorize ${CFLAGS}"
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
  aarch64*)
    if test "x${want_neon}" = "xyes"; then
       build_cpu_neon="yes"
       AC_MSG_CHECKING([whether to use NEON instructions])
       AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <arm_neon.h>]], [[volatile uint32x4_t test = vdupq_n_u32(0x1);]])],[
           AC_MSG_RESULT([yes])
           AC_DEFINE([BUILD_NEON], [1], [Build NEON Code])
           AC_DEFINE([BUILD_NEON_INTRINSICS], [1], [Build NEON Intrinsics])
           build_cpu_neon="yes"
           build_cpu_neon_intrinsics="yes"
       	 ],[
	   AC_MSG_RESULT([no])
           build_cpu_neon="no"
       	 ])
    fi
    ;;
esac

AM_CONDITIONAL([BUILD_NEON], [test "${build_cpu_neon}" = "yes"])
AM_CONDITIONAL([BUILD_NEON_INTRINSICS], [test "${build_cpu_neon_intrinsics}" = "yes"])

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
gmtime_r \
pthread_getcpuclockid \
clock_gettime \
clearenv
])

AC_FUNC_ALLOCA
AC_FUNC_MMAP
AC_FUNC_STRERROR_R

EFL_CHECK_FUNCS([EFLALL], [fnmatch gettimeofday dirfd fcntl sched_getcpu])

have_atfile_source="${ac_cv_func_fstatat}"
AC_DEFINE_IF([HAVE_ATFILE_SOURCE],
   [test "x${have_atfile_source}" = "xyes"],
   [1], [Use fstatat and other -at file functions])



######################  EFL  ######################

want_libeeze="yes"
AC_ARG_ENABLE([libeeze],
   [AS_HELP_STRING([--disable-libeeze],[disable Eeze device library @<:@default=enable@:>@])],
   [
    if test "x${enableval}" = "xyes"; then
       want_libeeze="yes"
    else
       want_libeeze="no"
       CFOPT_WARNING="yes"
    fi
   ])

build_libeeze="yes"
if test "x${want_libeeze}" = "xyes" ; then
    if test "x${have_linux}" = "xyes" ; then
        build_libeeze="yes"
    else
        build_libeeze="no"
        want_libeeze="no"
    fi
else
    build_libeeze="no"
fi

AC_ARG_ENABLE([systemd],
   [AS_HELP_STRING([--disable-systemd],[Disable systemd support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_systemd="yes"
    else
       want_systemd="no"
    fi
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
   PKG_CHECK_MODULES([SYSTEMD], [libsystemd >= 209],
   [
     have_systemd_pkg="yes"
     AC_DEFINE_UNQUOTED([HAVE_SYSTEMD_LOGIN_209],[1],[Defined if systemd >= 209])
   ],
   [have_systemd_pkg="no"])
fi

# check for systemd library if requested
if test "x${want_systemd}" = "xyes" -a "x${have_systemd_pkg}" = "xno"; then
   AC_MSG_ERROR([Systemd dependency requested but not found])
fi

AM_CONDITIONAL([WANT_SYSTEMD], [test "${want_systemd}" = "yes"])
AM_CONDITIONAL([HAVE_SYSTEMD], [test "${want_systemd}" = "yes" -a "${have_systemd_pkg}" = "yes"])
#### Platform-dependent

#### Evil
EFL_LIB_START_OPTIONAL([Evil], [test "${have_windows}" = "yes"])

### Default values

### Additional options to configure
EFL_SELECT_WINDOWS_VERSION

### Checks for programs

### Checks for libraries

EFL_ADD_LIBS([EVIL], [-lpsapi -lole32 -lws2_32 -lsecur32 -luuid])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

EVIL_CPPFLAGS="-DPSAPI_VERSION=1"
# TODO: should we have these at EFL (global?)
# Note: these warnings should not be used with C++ code
EVIL_CFLAGS_WRN="-Wdeclaration-after-statement -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls"
EVIL_CXXFLAGS="${EVIL_CXXFLAGS}"

if test "x${have_win32}" = "xyes" ; then
   EVIL_CFLAGS="-DSECURITY_WIN32 ${EVIL_CFLAGS}"
   EVIL_CXXFLAGS="-fno-rtti -fno-exceptions -DSECURITY_WIN32 ${EVIL_CXXFLAGS}"
fi

AC_SUBST([EVIL_CPPFLAGS])
AC_SUBST([EVIL_CFLAGS_WRN])
AC_SUBST([EVIL_CXXFLAGS])

### Checks for linker characteristics

### Checks for library functions

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
     with_max_log_level=""
     have_stringshare_usage="no"
     want_valgrind="no"
     want_debug_malloc="no"
     want_debug_threads="no"
     want_default_mempool="no"
     want_cow_magic="no"
     ;;
esac

EFL_CHECK_LIBUNWIND

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

AC_ARG_ENABLE([debug-threads],
   [AS_HELP_STRING([--enable-debug-threads], [enable debugging of eina threads @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_debug_threads="yes"
    else
       want_debug_threads="no"
    fi
   ])

EINA_CONFIG([MAGIC_DEBUG], [test "x${have_magic_debug}" = "xyes"])

AC_ARG_WITH([xattr-tests-path],
   [AS_HELP_STRING([--with-xattr-tests-path=DIR],[path of xattr enabled directory to create test files])],[XATTR_TEST_DIR=${withval}][AC_DEFINE_UNQUOTED([XATTR_TEST_DIR],["$withval"], [xattr enabled directory])])

evas_dicts_hyphen_dir="/usr/share/hyphen/"
AC_ARG_WITH([dictionaries-hyphen-dir],
   [AS_HELP_STRING([--with-dictionaries-hyphen-dir=DIR],[path of hunspell-compatible hyphen dictionaries])], [evas_dicts_hyphen_dir=$withval])
AC_DEFINE_UNQUOTED([EVAS_DICTS_HYPHEN_DIR],["$evas_dicts_hyphen_dir"], [Hunspell-compatible hyphen dictionaries install directory])

### Checks for programs

### Checks for libraries

# sockets

case "$host_os" in
   mingw*)
      have_socket="no"
      ;;
   *solaris*)
      AC_CHECK_LIB([socket], [connect],
         [
          have_socket="yes"
          requirement_socket="-lsocket"
          requirement_nsl="-lnsl"
         ],
         [have_socket="no"])
      ;;
   darwin*)
      have_socket="yes"
      ;;
   *)
      have_socket="yes"
      ;;
esac

EFL_PLATFORM_DEPEND([EINA], [all])

EFL_ADD_LIBS([EINA], [${requirement_socket} -lm])

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

EFL_OPTIONAL_DEPEND_PKG([EINA], [${want_systemd}], [SYSTEMD], [libsystemd])

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

EFL_CHECK_FUNCS([EINA], [dlopen dladdr iconv shm_open splice setxattr getpagesize prctl])

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

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
       [[ ]],
       [[
        static __thread int a = 0;
       ]])],
   [have_thread_specifier="yes"],
   [have_thread_specifier="no"])
AC_MSG_CHECKING([for __thread specifier])
AC_MSG_RESULT([${have_thread_specifier}])
if test "x${have_thread_specifier}" = "xyes" ; then
    AC_DEFINE([HAVE_THREAD_SPECIFIER], [1], [Have the __thread specifier])
fi

EFL_ADD_PUBLIC_LIBS([EINA], [${EFL_PTHREAD_LIBS}])
EFL_ADD_CFLAGS([EINA], [${EFL_PTHREAD_CFLAGS}])

EINA_CONFIG([HAVE_PTHREAD_BARRIER], [test "x${efl_have_pthread_barrier}" = "xyes"])
EINA_CONFIG([HAVE_PTHREAD_AFFINITY], [test "x${efl_have_setaffinity}" = "xyes"])
EINA_CONFIG([HAVE_PTHREAD_SETNAME], [test "x${efl_have_setname}" = "xyes"])
EINA_CONFIG([HAVE_DEBUG_THREADS], [test "x${want_debug_threads}" = "xyes"])
EINA_CONFIG([HAVE_POSIX_SPINLOCK], [test "x${efl_have_posix_threads_spinlock}" = "xyes"])
EINA_CONFIG([HAVE_OSX_SPINLOCK], [test "x${efl_have_osx_spinlock}" = "xyes"])
EINA_CONFIG([HAVE_OSX_SEMAPHORE], [test "x${have_darwin}" = "xyes"])
EINA_CONFIG([HAVE_WORDS_BIGENDIAN], [test "x${ac_cv_c_bigendian}" = "xyes"])

### Modules

EINA_CHECK_MODULE([chained-pool], [static], [chained pool])
EINA_CHECK_MODULE([pass-through], [static], [pass through])
EINA_CHECK_MODULE([one-big],      [static], [one big])

EFL_ADD_FEATURE([EINA], [systemd-journal], [${want_systemd}])

EFL_LIB_END([Eina])
#### End of Eina

#### Eina CXX
EFL_LIB_START([Eina_Cxx])

AC_ARG_ENABLE([cxx-bindings],
   [AS_HELP_STRING([--disable-cxx-bindings],[disable C++11 bindings. @<:@default=enabled@:>@])],
   [want_cxx11="${enableval}"], [want_cxx11="yes"])

have_cxx11="no"
if test "x${want_cxx11}" = "xyes"; then
  EFL_CXX_COMPILE_STDCXX_11([ext])
  if test "x${HAVE_CXX11}" = "x1" ; then

    # We should be using ${CXX} here, but there is a bug in
    # autotools macro and CXX is always set to g++ even if
    # it's not found. So we are using an internal variable
    # that does the work for now, may get broken in the future.
    if test "x${ac_ct_CXX}" = "x" -a "x${CXX}" = "xg++"; then
       AC_MSG_ERROR([efl requires a C++ compiler got ${ac_ct_CXX} and ${CXX}.])
    fi

    have_cxx11="yes"
  fi
fi

AM_CONDITIONAL([HAVE_CXX], [test "x${have_cxx11}" = "xyes"])

EFL_INTERNAL_DEPEND_PKG([EINA_CXX], [Eina])
EFL_ADD_CFLAGS([EINA_CXX], [${EFL_PTHREAD_CFLAGS}])
EFL_EVAL_PKGS([EINA_CXX])

EFL_LIB_END([Eina_Cxx])
#### End of Eina CXX

AC_ARG_WITH([js],
   [AS_HELP_STRING([--with-js=@<:@nodejs/libv8/libuv/none@:>@],[enable JavaScript bindings using nodejs or libv8/libuv as build dependencies. The libuv option implies libv8. @<:@default=none@:>@])],
   [want_js="${withval}"], [want_js="none"])

EFLJS_CXXFLAGS=""
AC_LANG_PUSH([C++])
case "$want_js" in
     nodejs)
        AC_CHECK_HEADERS([node/v8.h nodejs/deps/v8/v8.h nodejs/deps/v8/include/v8.h nodejs/src/v8.h v8.h],
                         [
                           v8_header=AC_header
                           v8_header_define=AS_TR_CPP([HAVE_]AC_header)
                           EFLJS_CXXFLAGS="$EFLJS_CXXFLAGS -D$v8_header_define"
                           break
                         ]
                         [AC_MSG_ERROR([Could not find v8 include headers from nodejs.])])
        AC_CHECK_HEADERS([node/node.h nodejs/deps/node/node.h nodejs/deps/node/include/node.h nodejs/src/node.h node.h],
                         [
                           node_header_define=AS_TR_CPP([HAVE_]AC_header)
                           EFLJS_CXXFLAGS="$EFLJS_CXXFLAGS -D$node_header_define"
                           break
                         ]
                         [AC_MSG_ERROR([Could not find node include headers from nodejs.])])
        AC_CHECK_HEADERS([node/uv.h nodejs/deps/uv/uv.h nodejs/deps/uv/include/uv.h nodejs/src/uv.h uv.h],
                         [
                           uv_header_define=AS_TR_CPP([HAVE_]AC_header)
                           EFLJS_CXXFLAGS="$EFLJS_CXXFLAGS -D$uv_header_define"
                           break
                         ]
                         [AC_MSG_ERROR([Could not find uv include headers from nodejs.])])
        ;;
     libv8)
        AC_CHECK_HEADERS([v8.h],
                         [
                           v8_header=AC_header
                           break
                         ]
                         [AC_MSG_ERROR([Could not find v8 include headers from libv8.])])
        ;;
     libuv)
        AC_CHECK_HEADERS([v8.h],
                         [
                           v8_header=AC_header
                           break
                         ]
                         [AC_MSG_ERROR([Could not find v8 include headers from libv8.])])
        AC_CHECK_HEADERS([uv.h],
                         [break]
                         [AC_MSG_ERROR([Could not find uv include headers from libuv.])])
        ;;
     none)
        ;;
     *) AC_MSG_ERROR([Invalid javascript dependency (${want_js}): must be none, nodejs, libv8 or libuv]) ;;
esac

have_v8_global="no"
have_v8_create_params="no"
if test "$want_js" != "none" ; then
AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
       [[
#include "$v8_header"
       ]],
       [[
v8::Isolate::CreateParams create_params; create_params.array_buffer_allocator = nullptr;
       ]])
   ],
   [
    AC_DEFINE(HAVE_V8_CREATE_PARAMS, 1, [Define to 1 if you must pass create_params explicitly.])
    have_v8_create_params="yes"
    AC_MSG_NOTICE([checking for v8::Isolate::CreateParams... yes])
   ],
   [
    AC_MSG_NOTICE([checking for v8::Isolate::CreateParams... no])
   ])
AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
       [[
#include "$v8_header"
#include <type_traits>
       ]],
       [[
       static_assert((std::is_same< ::v8::Global<void>, ::v8::UniquePersistent<void>>::value), "");
       ]])
   ],
   [
    AC_DEFINE(HAVE_V8_GLOBAL, 1, [Define to 1 if you must pass create_params explicitly.])
    have_v8_global="yes"
    AC_MSG_NOTICE([checking for v8::Global<T> class... yes])
   ],
   [
    AC_MSG_NOTICE([checking for v8::Global<T> class... no])
   ])
fi
AM_CONDITIONAL([HAVE_V8_CREATE_PARAMS], [test "x${have_v8_create_params}" = "xyes"])
AM_CONDITIONAL([HAVE_V8_GLOBAL], [test "x${have_v8_global}" = "xyes"])
AC_SUBST([HAVE_V8_CREATE_PARAMS])
AC_SUBST([HAVE_V8_GLOBAL])
AC_LANG_POP([C++])

AM_CONDITIONAL([HAVE_NODEJS], [test "x${want_js}" = "xnodejs"])
AC_DEFINE_IF([HAVE_NODEJS], [test "x${want_js}" = "xnodejs"],
  [1], [Using NodeJS])
AC_SUBST([want_js])
AC_SUBST([HAVE_NODEJS])

have_js="no"
if test "x${want_js}" != "xnone" ; then
   have_js="yes"
fi
AM_CONDITIONAL([HAVE_JS], [test "x${have_js}" = "xyes"])
AC_DEFINE_IF([HAVE_JS], [test "x${have_js}" = "xyes"], [1], [Compiling bindings for JavaScript])
AC_SUBST([HAVE_JS])

AM_CONDITIONAL([HAVE_LIBUV], [test "x${want_js}" = "xnodejs" -o "x${want_js}" = "xlibuv"])
AC_DEFINE_IF([HAVE_LIBUV], [test "x${want_js}" = "xnodejs" -o "x${want_js}" = "xlibuv"],
             [1], [Compiling libuv event loop integration])
AC_SUBST([HAVE_LIBUV])

# For efljspack mime handling
AM_CONDITIONAL([HAVE_XDG_DATA_HOME], [test "x${XDG_DATA_HOME}" != "x"])
AM_COND_IF([HAVE_XDG_DATA_HOME], [AC_SUBST([XDG_DATA_HOME])], [AC_SUBST([XDG_DATA_HOME], "$HOME/.local/share")])

#### Eina JS
EFL_LIB_START_OPTIONAL([Eina_Js], [test "x${have_js}" = "xyes"])
EFL_INTERNAL_DEPEND_PKG([EINA_JS], [Eina])
EFL_INTERNAL_DEPEND_PKG([EINA_JS], [Eo])
EFL_ADD_CFLAGS([EINA_JS], [${EFL_PTHREAD_CFLAGS}])
EFL_EVAL_PKGS([EINA_JS])

EFL_LIB_END_OPTIONAL([Eina_Js])
#### End of Eina JS

#### Ecore JS
EFL_LIB_START_OPTIONAL([Ecore_Js], [test "x${have_js}" = "xyes"])

EFL_INTERNAL_DEPEND_PKG([ECORE_JS], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_JS], [ecore_file])
EFL_INTERNAL_DEPEND_PKG([ECORE_JS], [efl])
EFL_ADD_CFLAGS([ECORE_JS], [${EFL_PTHREAD_CFLAGS}])
EFL_EVAL_PKGS([ECORE_JS])

EFL_LIB_END_OPTIONAL([Ecore_Js])
#### End of Ecore JS

#### Eio JS
EFL_LIB_START_OPTIONAL([Eio_Js], [test "x${have_js}" = "xyes"])

EFL_INTERNAL_DEPEND_PKG([EIO_JS], [eio])
EFL_ADD_CFLAGS([EIO_JS], [${EFL_PTHREAD_CFLAGS}])
EFL_EVAL_PKGS([EIO_JS])

EFL_LIB_END_OPTIONAL([Eio_Js])
#### End of Eio JS

#### Ethumb JS
EFL_LIB_START_OPTIONAL([Ethumb_Js], [test "x${have_js}" = "xyes"])

EFL_INTERNAL_DEPEND_PKG([ETHUMB_JS], [ethumb])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_JS], [ethumb_client])
EFL_ADD_CFLAGS([ETHUMB_JS], [${EFL_PTHREAD_CFLAGS}])
EFL_EVAL_PKGS([ETHUMB_JS])

EFL_LIB_END_OPTIONAL([Ethumb_Js])
#### End of Ethumb JS

#### Eldbus JS
EFL_LIB_START_OPTIONAL([Eldbus_Js], [test "x${have_js}" = "xyes"])

EFL_INTERNAL_DEPEND_PKG([ELDBUS_JS], [eldbus])
EFL_ADD_CFLAGS([ELDBUS_JS], [${EFL_PTHREAD_CFLAGS}])
EFL_EVAL_PKGS([ELDBUS_JS])

EFL_LIB_END_OPTIONAL([Eldbus_Js])
#### End of Eldbus JS

#### Eo JS
EFL_LIB_START_OPTIONAL([Eo_Js], [test "x${have_js}" = "xyes"])

EFL_INTERNAL_DEPEND_PKG([EO_JS], [eina])
EFL_ADD_CFLAGS([EO_JS], [${EFL_PTHREAD_CFLAGS}])
EFL_EVAL_PKGS([EO_JS])

EFL_LIB_END_OPTIONAL([Eo_Js])
#### End of Eo JS

#### Efl C Sharp Bindings
want_csharp_beta="no"
AC_ARG_ENABLE([csharp-beta],
  [AS_HELP_STRING([--enable-csharp-beta],[enable C Sharp bindings. @<:@default=disabled@:>@])],
  [
    if test "x${enableval}" = "xyes" ; then
      want_csharp_beta="yes"
    else
      want_csharp_beta="no"
    fi
  ],
  [want_csharp_beta="no"])

want_csharp="no"
AC_ARG_ENABLE([csharp-bindings],
  [AS_HELP_STRING([--enable-csharp-bindings],[enable C Sharp bindings. @<:@default=disabled@:>@])],
  [
    if test "x${enableval}" = "xyes" ; then
      want_csharp="yes"
    else
      want_csharp="no"
    fi
  ],
  [want_csharp="no"])

if test "x${want_csharp}" = "xyes" -a "x${HAVE_MCS}" != "x1"; then
  want_csharp="no"
  AC_MSG_ERROR([C Sharp bindings requested but C Sharp compiler could not be found])
fi

AM_CONDITIONAL([HAVE_CSHARP], [test "x${want_csharp}" = "xyes"])
AC_DEFINE_IF([HAVE_CSHARP], [test "x${want_csharp}" = "xyes"],
  [1], [Compiling bindings for C Sharp])
AC_SUBST([want_csharp])

AM_CONDITIONAL([HAVE_CSHARP_BETA], [test "x${want_csharp_beta}" = "xyes"])
AC_DEFINE_IF([HAVE_CSHARP_BETA], [test "x${want_csharp_beta}" = "xyes"],
  [1], [Compiling bindings for C Sharp with beta methods, properties and classes])
AC_SUBST([want_csharp_beta])

if test "x${have_windows}" = "xyes"; then
  eflmonodlldir="$prefix/bin"
else
  eflmonodlldir="$libdir/efl-mono-${EFL_VERSION_MAJOR}"
fi
AC_SUBST([eflmonodlldir])

# Efl Mono
EFL_LIB_START_OPTIONAL([Efl_Mono], [test "x${want_csharp}" = "xyes"])

EFL_PLATFORM_DEPEND([EFL_MONO], [evil])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Eina])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Eo])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Ecore])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Eet])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Ecore_Evas])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Ecore_Con])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Ecore_Audio])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Efl])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Evas])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Edje])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Emotion])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Eldbus])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Emile])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Ethumb])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Ethumb_Client])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Eio])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Elementary])
EFL_INTERNAL_DEPEND_PKG([EFL_MONO], [Efl_Custom_Exports_Mono])

EFL_EVAL_PKGS([EFL_MONO])

EFL_LIB_END_OPTIONAL([Efl_Mono])

# Efl Custom Exports Mono
EFL_LIB_START_OPTIONAL([Efl_Custom_Exports_Mono], [test "x${want_csharp}" = "xyes"])

EFL_PLATFORM_DEPEND([EFL_CUSTOM_EXPORTS_MONO], [evil])
EFL_INTERNAL_DEPEND_PKG([EFL_CUSTOM_EXPORTS_MONO], [Eina])
EFL_INTERNAL_DEPEND_PKG([EFL_CUSTOM_EXPORTS_MONO], [Eo])
EFL_INTERNAL_DEPEND_PKG([EFL_CUSTOM_EXPORTS_MONO], [Efl])
EFL_INTERNAL_DEPEND_PKG([EFL_CUSTOM_EXPORTS_MONO], [Eldbus])

EFL_EVAL_PKGS([EFL_CUSTOM_EXPORTS_MONO])

EFL_LIB_END_OPTIONAL([Efl_Custom_Exports_Mono])

# Libtool versioning stuff.
# On windows, the version of the library goes into the filename.
# See https://autotools.io/libtool/windows.html#libtool.windows.version
# Based on the current:revision:age string passed to libtool, the generated suffix
# has the value of (current - age).
# To get the current and age, we use the same calculation from m4/efl.m4 (lt_cur, lt_age)
DLIB_PREFIX_MONO=""
DLIB_SUFFIX_MONO=""
if test "x${have_windows}" = "xyes"; then
  DLIB_PREFIX_MONO="lib"
  if test "x${EFL_VERSION_MAJOR}" = "x1"; then
    DLIB_SUFFIX_MONO="-${EFL_VERSION_MAJOR}"
  else
    DLIB_SUFFIX_MONO="-${EFL_VERSION_MAJOR}00"
  fi
fi
AC_SUBST([DLIB_PREFIX_MONO])
AC_SUBST([DLIB_SUFFIX_MONO])

EFL_DL_MONO="${DLIB_PREFIX_MONO}efl${DLIB_SUFFIX_MONO}";
ECORE_DL_MONO="${DLIB_PREFIX_MONO}ecore${DLIB_SUFFIX_MONO}";
EINA_DL_MONO="${DLIB_PREFIX_MONO}eina${DLIB_SUFFIX_MONO}"
EO_DL_MONO="${DLIB_PREFIX_MONO}eo${DLIB_SUFFIX_MONO}"
EVAS_DL_MONO="${DLIB_PREFIX_MONO}evas${DLIB_SUFFIX_MONO}"
if test "x${have_windows}" = "xyes"; then
  EVIL_DL_MONO="${DLIB_PREFIX_MONO}evil${DLIB_SUFFIX_MONO}"
else
  EVIL_DL_MONO="dl"
fi
EDJE_DL_MONO="${DLIB_PREFIX_MONO}edje${DLIB_SUFFIX_MONO}"
ELEMENTARY_DL_MONO="${DLIB_PREFIX_MONO}elementary${DLIB_SUFFIX_MONO}"
ELDBUS_DL_MONO="${DLIB_PREFIX_MONO}eldbus${DLIB_SUFFIX_MONO}"
CUSTOM_EXPORTS_MONO_DL_MONO="${DLIB_PREFIX_MONO}eflcustomexportsmono${DLIB_SUFFIX_MONO}"

AC_SUBST([EFL_DL_MONO])
AC_SUBST([ECORE_DL_MONO])
AC_SUBST([EINA_DL_MONO])
AC_SUBST([EO_DL_MONO])
AC_SUBST([EVAS_DL_MONO])
AC_SUBST([EVIL_DL_MONO])
AC_SUBST([EDJE_DL_MONO])
AC_SUBST([ELEMENTARY_DL_MONO])
AC_SUBST([ELDBUS_DL_MONO])
AC_SUBST([CUSTOM_EXPORTS_MONO_DL_MONO])

# Eos file/library mapping

# Unescaped sed pattern: sed -n 's/src\/lib\/\([a-z0-9_]*\)\/[a-z\/]*\/\([a-z0-9\._]*\)/\2,\1/pg'
AC_CHECK_PROG([have_realpath], [realpath], [yes], [no])
if test "x${have_realpath}" = "xyes" ; then
  _configure_path=`realpath $0`
else
  AC_CHECK_PROG([have_readlink], [readlink], [yes], [no])
  if test "x${have_readlink}" = "xyes" ; then
    _configure_path=`readlink -f $0`
  fi
fi
efl_mono_top_srcdir=`dirname ${_configure_path}`
EFL_MONO_LIBRARY_MAP=`find ${efl_mono_top_srcdir}/src/lib/ -iname "*\.eo" | sed -n 's/.*\/src\/lib\/\(@<:@a-z0-9_@:>@*\)@<:@\/a-z@:>@*\/\(@<:@a-z0-9\._@:>@*\)/\2,\1/pg'`

AC_SUBST([EFL_MONO_LIBRARY_MAP])
AM_SUBST_NOTMAKE([EFL_MONO_LIBRARY_MAP])

#### End of Efl C Sharp Bindings

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

#### Eet CXX
EFL_LIB_START([Eet_Cxx])

EFL_INTERNAL_DEPEND_PKG([EET_CXX], [Eina_Cxx])
EFL_INTERNAL_DEPEND_PKG([EET_CXX], [Eet])

EFL_EVAL_PKGS([EET_CXX])

EFL_LIB_END([Eet_Cxx])
#### End of Eet CXX

#### Emile

EFL_LIB_START([Emile])

### Default values

### Additional options to configure

### Checks for programs

### Checks for libraries

EFL_CHECK_LIBS([EMILE], [libjpeg])

## Compatibility layers
EFL_PLATFORM_DEPEND([EMILE], [evil])

EFL_ADD_LIBS([EMILE], [-lm])

# Cryptography support
if test "$build_crypto" != "none" ; then
   AC_DEFINE([HAVE_CIPHER], [1], [Have cipher support built in emile])
   AC_DEFINE([HAVE_SIGNATURE], [1], [Have signature support in emile])
   EFL_CRYPTO_DEPEND([EMILE])
fi

EFL_CHECK_LIBS([EMILE], [zlib])

EFL_INTERNAL_DEPEND_PKG([EMILE], [eina])

EFL_EVAL_PKGS([EMILE])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Check availability

EFL_ADD_FEATURE([EMILE], [crypto], [${build_crypto}])

EFL_LIB_END([Emile])
#### End of Emile


#### Eet

EFL_LIB_START([Eet])

### Default values

### Additional options to configure

### Checks for programs

### Checks for libraries

EFL_CHECK_LIBS([EET], [libjpeg])

## Compatibility layers
EFL_PLATFORM_DEPEND([EET], [evil])

EFL_ADD_LIBS([EET], [-lm])

# Cryptography support
if test "$build_crypto" != "none" ; then
   AC_DEFINE([HAVE_CIPHER], [1], [Have cipher support built in eet])
   AC_DEFINE([HAVE_SIGNATURE], [1], [Have signature support for eet file])
   EFL_CRYPTO_DEPEND([EET])
fi

EFL_INTERNAL_DEPEND_PKG([EET], [eina])
EFL_INTERNAL_DEPEND_PKG([EET], [emile])

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

EFL_LIB_START_OPTIONAL([Eolian_Js], [test "${have_js}" = "yes"])

EFL_INTERNAL_DEPEND_PKG([EOLIAN_JS], [eina])
EFL_INTERNAL_DEPEND_PKG([EOLIAN_JS], [eolian])

EFL_LIB_END_OPTIONAL([Eolian_Js])
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

#### Efl

EFL_LIB_START([Efl])

EFL_PLATFORM_DEPEND([EFL], [evil])
EFL_INTERNAL_DEPEND_PKG([EFL], [eina])
EFL_INTERNAL_DEPEND_PKG([EFL], [eo])

EFL_ADD_LIBS([EFL], [-lm])

EFL_LIB_END([Efl])
#### End of Efl

#### Ector

EFL_LIB_START([Ector])


### Default values

### Additional options to configure

### Checks for programs

### Checks for libraries

## Compatibility layers

EFL_PLATFORM_DEPEND([ECTOR], [evil])

EFL_INTERNAL_DEPEND_PKG([ECTOR], [eina])
EFL_INTERNAL_DEPEND_PKG([ECTOR], [emile])
EFL_INTERNAL_DEPEND_PKG([ECTOR], [eet])
EFL_INTERNAL_DEPEND_PKG([ECTOR], [eo])
EFL_INTERNAL_DEPEND_PKG([ECTOR], [efl])

EFL_ADD_LIBS([ECTOR], [-lm])

EFL_EVAL_PKGS([ECTOR])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Check availability

EFL_LIB_END([ECTOR])

#### End of Ector


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
AC_ARG_ENABLE([libuv],
   [AS_HELP_STRING([--enable-libuv],[enable ecore_main_loop based on libuv. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_libuv="yes"
       CFOPT_WARNING="xyes"
    else
       want_libuv="no"
    fi
   ],
   [want_libuv="no"])

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

AC_ARG_ENABLE([ecore_wayland],
   [AS_HELP_STRING([--enable-ecore-wayland],[enable legacy Ecore_Wayland support. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_ecore_wayland="yes"
    else
       want_ecore_wayland="no"
    fi
   ],
   [want_ecore_wayland="no"])

AC_ARG_ENABLE([ecore_drm],
   [AS_HELP_STRING([--enable-ecore-drm],[enable legacy Ecore_Drm support. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_ecore_drm="yes"
    else
       want_ecore_drm="no"
    fi
   ],
   [want_ecore_drm="no"])

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
EFL_INTERNAL_DEPEND_PKG([ECORE], [efl])

EFL_ADD_LIBS([ECORE], [-lm])

# glib

EFL_OPTIONAL_DEPEND_PKG([ECORE], [${want_glib}], [GLIB], [glib-2.0 gthread-2.0])
if test "x${have_glib}" = "xno"; then
   want_g_main_loop="no"
fi

EFL_OPTIONAL_DEPEND_PKG([ECORE], [${want_systemd}], [SYSTEMD], [libsystemd])

EFL_ADD_FEATURE([ECORE], [systemd-daemon], [${want_systemd}])
EFL_ADD_FEATURE([ECORE], [glib])
EFL_ADD_FEATURE([ECORE], [g-main-loop])
EFL_ADD_FEATURE([ECORE], [libuv])

want_glib_integration_always=no
if test "x${with_glib}" = "xalways" ; then
  want_glib_integration_always="yes"
  AC_DEFINE([GLIB_INTEGRATION_ALWAYS], [1], [Always integrate glib if support compiled])
fi

if test "x${want_g_main_loop}" = "xyes" ; then
  AC_DEFINE([USE_G_MAIN_LOOP], [1], [Use g_main_loop in ecore])
  if test "x${want_js}" = "xnodejs" -o "x${want_js}" = "xlibuv" ; then
    AC_MSG_ERROR([Can't define glib and libuv integration together])
  fi
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

AM_CONDITIONAL([HAVE_GSTREAMER], [test "x${want_gstreamer}" = "xyes" -o "x${want_gstreamer1}" = "xyes"])
AM_CONDITIONAL([HAVE_GSTREAMER0], [test "x${want_gstreamer}" = "xyes"])
AM_CONDITIONAL([HAVE_GSTREAMER1], [test "x${want_gstreamer1}" = "xyes"])

EFL_EVAL_PKGS([ECORE])

### Checks for header files

AC_HEADER_SYS_WAIT

AC_CHECK_HEADERS([sys/socket.h])

AC_CHECK_HEADERS([ \
arpa/inet.h \
arpa/nameser.h \
langinfo.h \
features.h \
netinet/in.h \
netinet/tcp.h \
netinet/udp.h \
sys/prctl.h \
sys/ioctl.h \
sys/resource.h \
sys/timerfd.h \
sys/un.h \
ieeefp.h \
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
    AC_DEFINE(HAVE_ISFINITE, 1, [Define to 1 if you have 'isfinite', as a function or macro.])
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
   AC_MSG_ERROR([You do not have a working way to implement coroutine. Exiting...])
fi

### Check availability

EFL_LIB_END([Ecore])
#### End of Ecore

#### Evas

EFL_LIB_START([Evas])

### Additional options to configure

# X11
AC_ARG_WITH([x11],
   [AS_HELP_STRING([--with-x11=xlib|none],[Use X11 (Xlib) or not])])

if test "x${have_windows}" = "xyes" || test "x${have_ps3}" = "xyes" || test "x${have_darwin}" = "xyes"; then
   with_x11="none"
elif test "x${with_x11}" = "x"; then
   with_x11="xlib"
fi

want_x11_xlib="no"
want_x11_none="no"
want_x11_any="no"
case "${with_x11}" in
   xlib)
     want_x11_xlib="yes"
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
if test "${with_opengl}" != "none"; then
   want_x11_xlib_opengl="${want_x11_xlib}"
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
   EFL_PKG_CHECK_STRICT([wayland-client >= 1.11.0 wayland-scanner >= 1.11.0 wayland-protocols >= 1.12])
   PKG_CHECK_MODULES([WAYLAND], [wayland-scanner >= 1.11.0],
    [
      AC_ARG_VAR([wayland_scanner], [The wayland-scanner executable])
      AC_PATH_PROG([wayland_scanner], [wayland-scanner])
      if test x$wayland_scanner = x; then
        PKG_CHECK_MODULES(WAYLAND_SCANNER, [wayland-scanner])
        wayland_scanner=`$PKG_CONFIG --variable=wayland_scanner wayland-scanner`
      fi
      ac_wayland_protocols_pkgdatadir=`$PKG_CONFIG --variable=pkgdatadir wayland-protocols`
      AC_SUBST(WAYLAND_PROTOCOLS_DATADIR, $ac_wayland_protocols_pkgdatadir)
    ])
fi
AM_CONDITIONAL([HAVE_WAYLAND_SCANNER], [test x$wayland_scanner != x])

# Wayland IVI-Shell
AC_ARG_ENABLE([wayland-ivi-shell],
   [AS_HELP_STRING([--enable-wayland-ivi-shell],[enable ivi-shell support. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_wayland_ivi_shell="yes"
    else
       want_wayland_ivi_shell="no"
    fi
   ],
   [want_wayland_ivi_shell="no"])

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

# Eglfs
AC_ARG_ENABLE([eglfs],
   [AS_HELP_STRING([--enable-eglfs],[enable hardware accelerated framebuffer access. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_eglfs="yes"
       want_fb="yes"
    else
       want_eglfs="no"
    fi
   ],
   [want_eglfs="no"])

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
   EFL_PKG_CHECK_STRICT([sdl2 >= 2.0.0])
fi

# We only enable SDL with opengl if it is the full version and not ES
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
    [AS_HELP_STRING([--enable-cocoa],[enable Cocoa backend on Mac OS X. @<:@default=enabled@:>@])],
    [
     if test "x${enableval}" = "xyes" ; then
        want_cocoa="yes"
     else
        want_cocoa="no"
     fi
    ],
    [want_cocoa="yes"])

if test "${want_cocoa}" = "yes"; then
   #test cocoa requirements (objc and Cocoa/Cocoa.h)
    cocoa_ldflags=""
    have_cocoa="no"
    m4_ifdef([AC_PROG_OBJC],
      [
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
   ])
fi
AC_SUBST(cocoa_ldflags)

if test "x${have_cocoa}" = "xyes"; then
    #test cocoa requirements (objc and Cocoa/Cocoa.h)
    cocoa_coreservices_ldflags=""
    have_cocoa_coreservices="no"
    m4_ifdef([AC_PROG_OBJC],
      [
          AC_LANG_PUSH([Objective C])
          LIBS_save="$LIBS"
          LIBS="$LIBS -framework CoreServices"
          AC_LINK_IFELSE(
             [AC_LANG_PROGRAM(
                 [[
#include <CoreServices/CoreServices.h>
                 ]],
                 [[
//test function here
                 ]])],
             [
              have_cocoa_coreservices="yes"
              cocoa_coreservices_ldflags="-framework CoreServices"
             ],
             [have_cocoa_coreservices="no"])
          LIBS="$LIBS_save"
          AC_MSG_CHECKING([whether Cocoa CoreServices framework is supported])
          AC_MSG_RESULT([${have_cocoa_coreservices}])
          AC_LANG_POP([Objective C])
   ])
fi
AC_SUBST(cocoa_coreservices_ldflags)

AC_ARG_ENABLE([elput],
   [AS_HELP_STRING([--enable-elput],[enable elput library. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_elput="yes"
    else
       want_elput="no"
    fi
   ],
   [
    if test "x${want_wayland}" = "xyes" ; then
       want_elput="yes"
    else
       want_elput="no"
    fi
   ])

# Drm
AC_ARG_ENABLE([drm],
   [AS_HELP_STRING([--enable-drm],[enable drm engine. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
      if test "x${want_elput}" != "xyes" ; then
        AC_MSG_ERROR([elput is required to build drm support])
      fi
      want_drm="yes"
    else
      want_drm="no"
    fi
   ],
   [want_drm="no"])

AC_ARG_ENABLE([gl-drm],
   [AC_HELP_STRING([--enable-gl-drm],
       [enable gl drm engine. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
      if test "x${want_elput}" != "xyes" ; then
        AC_MSG_ERROR([elput is required to build gl-drm support])
      fi
      want_gl_drm="yes"
    else
      want_gl_drm="no"
    fi
   ],
   [want_gl_drm="no"])

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

# Hyphenation
AC_ARG_ENABLE([hyphen],
   [AS_HELP_STRING([--enable-hyphen],[enable text hyphenation support. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_hyphen="yes"
    else
       want_hyphen="no"
    fi
   ],
   [want_hyphen="no"])

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

# Verify OpenGL + EGL modes match (full+glx or es+egl)
if test "x${want_egl}" = "xyes" && test "x${with_opengl}" != "xes" ; then
  AC_MSG_ERROR([Full OpenGL with EGL is not supported, please add --with-opengl=es to your configure options to switch to EGL + OpenGL ES.])
fi
if test "x${want_egl}" = "xno" && test "x${with_opengl}" = "xes" ; then
  AC_MSG_ERROR([OpenGL ES requires EGL, please add --enable-egl to your configure options to switch to EGL + OpenGL ES.])
fi

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

# Ecore Buffer
AC_ARG_ENABLE([ecore-buffer],
   [AS_HELP_STRING([--enable-ecore-buffer],[enable ecore-buffer. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_ecore_buffer="yes"
    else
       want_ecore_buffer="no"
    fi
   ],
   [want_ecore_buffer="no"])

# Image Loaders
case "$host_os" in
   cegcc*|mingw32ce*)
      AC_MSG_ERROR([ceGCC compiler is not supported anymore. Exiting...])
   ;;
   mingw*)
      want_generic="no"
      want_poppler="no"
      want_spectre="no"
      want_libraw="no"
      want_rsvg="no"
      want_xcf="no"
   ;;
   cygwin*)
      want_generic="no"
      want_poppler="no"
      want_spectre="no"
      want_libraw="no"
      want_rsvg="no"
      want_xcf="no"
   ;;
   *)
      want_generic="static"
      want_poppler="yes"
      want_spectre="yes"
      want_libraw="yes"
      want_rsvg="yes"
      want_xcf="yes"
   ;;
esac

ARG_ENABLE_EVAS_VG_LOADER(SVG, static)
ARG_ENABLE_EVAS_VG_LOADER(EET, static)


ARG_ENABLE_EVAS_IMAGE_LOADER(BMP, static)
ARG_ENABLE_EVAS_IMAGE_LOADER(Eet, static)
ARG_ENABLE_EVAS_IMAGE_LOADER(Generic, [${want_generic}])
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
want_evas_engine_gl_cocoa="${have_cocoa}"
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
EFL_INTERNAL_DEPEND_PKG([EVAS], [efl])
EFL_INTERNAL_DEPEND_PKG([EVAS], [emile])
EFL_INTERNAL_DEPEND_PKG([EVAS], [ector])
EFL_INTERNAL_DEPEND_PKG([EVAS], [ecore])

EFL_ADD_LIBS([EVAS], [-lm])

# Freetype (the version 16.2.10 equals the freetype 2.5.0.1 release)
EFL_DEPEND_PKG([EVAS], [FREETYPE], [freetype2 >= 16.2.10])

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

EVAS_ENGINE([buffer], [static])
EVAS_ENGINE([fb], [${want_fb}], [EVAS_ENGINE_DEP_CHECK_FB])

# XXX TODO:
EVAS_CHECK_ENGINE([gl-cocoa], [${want_evas_engine_gl_cocoa}], [no], [OpenGL Cocoa])
EVAS_CHECK_ENGINE([gl-sdl], [${want_gl_sdl}], [no], [OpenGL SDL])
EVAS_CHECK_ENGINE([software-gdi], [${want_evas_engine_software_gdi}], [no], [Software GDI])
EVAS_CHECK_ENGINE([software-ddraw], [${want_evas_engine_software_ddraw}], [no], [Software DirectDraw])
EVAS_CHECK_ENGINE([wayland-egl], [${want_evas_engine_wayland_egl}], [no], [Wayland Egl])
EVAS_CHECK_ENGINE([wayland-shm], [${want_wayland}], [no], [Wayland Shm])
EVAS_CHECK_ENGINE([drm], [${want_drm}], [no], [Drm])
EVAS_CHECK_ENGINE([gl-drm], [${want_gl_drm}], [no], [OpenGL Drm])
EVAS_CHECK_ENGINE([eglfs], [${want_eglfs}], [no], [OpenGL Fb])


# Software Xlib

have_evas_engine_software_xlib="no"
EVAS_CHECK_ENGINE([software-xlib], [${want_x11_xlib}], [no], [Software Xlib])
AC_MSG_CHECKING([whether to build Software Xlib Engine])
AC_MSG_RESULT([${have_evas_engine_software_xlib}])

# If software_x11 is available, define everything needed for X11

have_evas_engine_software_x11="no"
if test "x${have_evas_engine_software_xlib}" = "xyes" || test "x${have_evas_engine_software_xlib}" = "xstatic"; then
   have_evas_engine_software_x11="yes"
   AC_DEFINE_UNQUOTED([BUILD_ENGINE_SOFTWARE_X11], [1], [Build software X11 engine])
fi
AM_CONDITIONAL([BUILD_ENGINE_SOFTWARE_X11], [test "x${have_evas_engine_software_x11}" = "xyes"])

if test "x${have_evas_engine_software_xlib}" = "xstatic"; then
   AC_DEFINE_UNQUOTED([EVAS_STATIC_BUILD_SOFTWARE_X11], [1], [Build software X11 engine as part of libevas])
fi
AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_X11], [test "x${have_evas_engine_software_xlib}" = "xstatic"])

# Needed for evas-software-x11.pc

AC_SUBST([have_evas_engine_software_xlib])

# Software generic
AC_DEFINE([EVAS_STATIC_BUILD_SOFTWARE_GENERIC], [1], [Build software generic engine as part of libevas])
AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_GENERIC], [true])

# OpenGL

if test "${with_opengl}" = "es" ; then
   AC_DEFINE(GL_GLES, 1, [GLSL runtime shader GLES2 support])
fi

# OpenGL Xlib

have_evas_engine_gl_xlib="no"
EVAS_CHECK_ENGINE([gl-xlib], [${want_x11_xlib_opengl}], [no], [OpenGL XLib])

AC_MSG_CHECKING([whether to build OpenGL Xlib Engine])
AC_MSG_RESULT([${have_evas_engine_gl_xlib}])

# If opengl_x11 is available, define everything needed for X11

have_evas_engine_gl_x11="no"
if test "x${have_evas_engine_gl_xlib}" = "xyes" || test "x${have_evas_engine_gl_xlib}" = "xstatic"; then
   have_evas_engine_gl_x11="yes"
   AC_DEFINE_UNQUOTED([BUILD_ENGINE_GL_X11], [1], [Build OpenGL X11 engine])
fi
AM_CONDITIONAL([BUILD_ENGINE_GL_X11], [test "x${have_evas_engine_gl_x11}" = "xyes"])

if test "x${have_evas_engine_gl_xlib}" = "xstatic"; then
   AC_DEFINE_UNQUOTED([EVAS_STATIC_BUILD_GL_X11], [1], [Build OpenGL X11 engine as part of libevas])
fi
AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_X11], [test "x${have_evas_engine_gl_xlib}" = "xstatic"])

# Needed for evas-opengl-x11.pc

AC_SUBST([have_evas_engine_gl_xlib])

# OpenGL SDL

if test "x$have_evas_engine_gl_sdl" = "xyes" || test "x$have_evas_engine_gl_sdl" = "xstatic" ; then
   AC_CHECK_DECL([SDL_GL_CONTEXT_MAJOR_VERSION],
      [AC_DEFINE([HAVE_SDL_GL_CONTEXT_VERSION], [1], [SDL_GL version attributes present])],,
      [#include <SDL2/SDL_video.h>])
fi

if test "${with_opengl}" = "es"; then
   AC_CHECK_DECL([SDL_OPENGLES],
      [AC_DEFINE([HAVE_SDL_FLAG_OPENGLES], [1], [SDL_OPENGLES flag is present])],,
      [#include <SDL2/SDL_video.h>])
fi

# OpenGL common
evas_engine_gl_common_cflags=""
if test "x${have_egl}" = "xyes"; then
   evas_engine_gl_common_libs="-lEGL -lGLESv2 -lm"
else
   evas_engine_gl_common_libs="-lGL -lm"
fi

# The lines below fix compiling/linking of gl_generic on OSX
if test "x${have_darwin}" = "xyes"; then
   if test "x${have_evas_engine_gl_cocoa}" = "xyes"; then
      # If gl_cocoa is enabled, Apple's Core OpenGL (CGL) should be the default OpenGL.
      # CFLAGS and LIBS are determined by evas-gl_cocoa.
      evas_engine_gl_common_libs="${evas_engine_gl_cocoa_libs}"
      evas_engine_gl_common_cflags="${evas_engine_gl_cocoa_cflags}"
   else
      # If evas-gl_cocoa is disabled, the only supported OpenGL engine on OSX (for now)
      # is evas-gl_x11. Without its CFLAGS and LIBS explicitely set, gl_generic
      # cannot compile (nor link).
      evas_engine_gl_common_libs=""
      evas_engine_gl_common_cflags=""

      if test "x${have_evas_engine_gl_xlib}" = "xyes"; then
         evas_engine_gl_common_libs="${evas_engine_gl_xlib_libs}"
         evas_engine_gl_common_cflags="${evas_engine_gl_xlib_cflags}"
      fi
      ### XXX Check for other supported engines supported on OSX (dec. 2014: NONE)
   fi
fi

AC_SUBST([evas_engine_gl_common_cflags])
AC_SUBST([evas_engine_gl_common_libs])

have_evas_engine_gl_common="no"
have_static_evas_engine_gl_common="no"
if test "x$have_evas_engine_gl_xlib" = "xyes" || \
   test "x$have_evas_engine_gl_sdl" = "xyes" || \
   test "x$have_evas_engine_gl_cocoa" = "xyes" || \
   test "x$have_evas_engine_gl_drm" = "xyes" || \
   test "x$have_evas_engine_eglfs" = "xyes" || \
   test "x$have_evas_engine_wayland_egl" = "xyes"; then
   have_evas_engine_gl_common="yes"
fi
if test "x$have_evas_engine_gl_xlib" = "xstatic" || \
   test "x$have_evas_engine_gl_sdl" = "xstatic" || \
   test "x$have_evas_engine_gl_cocoa" = "xstatic" || \
   test "x$have_evas_engine_gl_drm" = "xstatic" || \
   test "x$have_evas_engine_eglfs" = "xstatic" || \
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

have_evas_engine_wayland_common="no"
if test "x$have_evas_engine_wayland_egl" = "xstatic" || \
   test "x$have_evas_engine_wayland_shm" = "xstatic" || \
   test "x$have_evas_engine_wayland_egl" = "xyes" || \
   test "x$have_evas_engine_wayland_shm" = "xyes"; then
   have_evas_engine_wayland_common="yes"
fi

if test "x$have_evas_engine_wayland_common" = "xyes"; then
   AC_DEFINE([BUILD_ENGINE_WAYLAND_COMMON], [1], [Building any wayland evas engine])
fi
AM_CONDITIONAL([BUILD_ENGINE_WAYLAND_COMMON], [test "x${have_evas_engine_wayland_common}" = "xyes"])

## Vg Loaders

EVAS_CHECK_VG_LOADER([SVG], [${want_evas_vg_loader_svg}])
EVAS_CHECK_VG_LOADER([EET], [${want_evas_vg_loader_eet}])

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

if test "x$want_hyphen" = "xyes" ; then

   EFL_CHECK_LIB_CODE([EVAS], [-lhyphen], [have_fct], [[
   #include <string.h>
   #include <stdlib.h>
   #include <ctype.h>
   #include <stdio.h>
   #include <hyphen.h>
   ]], [[
   HyphenDict *dict;
   ]])

   if test "${have_fct}" = "no"; then
     AC_MSG_ERROR([Cannot find the hyphen library.])
   else
     AC_DEFINE([HAVE_HYPHEN], [1], [have hunspell hyphen support])
   fi
fi

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
EFL_ADD_FEATURE([EVAS], [tile-rotate])
EFL_ADD_FEATURE([EVAS], [dither-mask], [${build_evas_dither_mask}])
EFL_ADD_FEATURE([EVAS], [hyphen])

#### Generic backend

### Options to turn off generic loaders for evas
AC_ARG_ENABLE([poppler],
   [AS_HELP_STRING([--disable-poppler],[disable pdf support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_poppler="yes"
    else
       want_poppler="no"
    fi
   ])

AC_ARG_ENABLE([spectre],
   [AS_HELP_STRING([--disable-spectre],[disable postscript support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_spectre="yes"
    else
       want_spectre="no"
    fi
   ])

AC_ARG_ENABLE([libraw],
   [AS_HELP_STRING([--disable-libraw],[disable libraw support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_libraw="yes"
    else
       want_libraw="no"
    fi
   ])

AC_ARG_ENABLE([librsvg],
   [AS_HELP_STRING([--disable-librsvg],[disable svg support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_rsvg="yes"
    else
       want_rsvg="no"
    fi
   ])

AC_ARG_ENABLE([xcf],
   [AS_HELP_STRING([--disable-xcf],[disable xcf support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_xcf="yes"
    else
       want_xcf="no"
    fi
   ])

have_poppler="no"
have_spectre="no"
have_raw="no"
have_rsvg="no"

### Check needed dependency for each generic loader
if test "x${want_poppler}" = "xyes"; then
   PKG_CHECK_MODULES([POPPLER], [poppler-cpp >= 0.12], [have_poppler="yes"])
fi

if test "x${have_poppler}" = "xyes" ; then
   # We should be using ${CXX} here, but there is a bug in
   # autotools macro and CXX is always set to g++ even if
   # it's not found. So we are using an internal variable
   # that does the work for now, may get broken in the future.
   if test "x${ac_ct_CXX}" = "x" -a "x${CXX}" = "xg++"; then
      AC_MSG_ERROR([efl requires a C++ compiler got ${ac_ct_CXX} and ${CXX}.])
   fi
fi

if test "x${want_spectre}" = "xyes"; then
   PKG_CHECK_MODULES([SPECTRE], [libspectre], [have_spectre="yes"])
fi

if test "x${want_libraw}" = "xyes"; then
   PKG_CHECK_MODULES([LIBRAW], [libraw], [have_raw="yes"])
fi

if test "x${want_rsvg}" = "xyes"; then
   PKG_CHECK_MODULES([RSVG],  [librsvg-2.0 >= 2.14.0 cairo >= 1.0.0], [have_rsvg="yes"])
   PKG_CHECK_MODULES(
      [SVG_2_36], [librsvg-2.0 >= 2.36.0 cairo >= 1.0.0],
      [
       have_rsvg_2_36="yes"
       AC_DEFINE(HAVE_SVG_2_36, 1, [Have librsvg >= 2.36])
      ],
      [have_rsvg_2_36="no"])
fi

AM_CONDITIONAL([HAVE_POPPLER], [test "x${have_poppler}" = "xyes"])
AM_CONDITIONAL([HAVE_SPECTRE], [test "x${have_spectre}" = "xyes"])
AM_CONDITIONAL([HAVE_LIBRAW], [test "x${have_raw}" = "xyes"])
AM_CONDITIONAL([HAVE_RSVG], [test "x${have_rsvg}" = "xyes"])
AM_CONDITIONAL([HAVE_XCF], [test "x${want_xcf}" = "xyes"])

EFL_ADD_FEATURE([EVAS_LOADER], [poppler], [${have_poppler}])
EFL_ADD_FEATURE([EVAS_LOADER], [spectre], [${have_spectre}])
EFL_ADD_FEATURE([EVAS_LOADER], [raw], [${have_raw}])
EFL_ADD_FEATURE([EVAS_LOADER], [rsvg], [${have_rsvg}])
EFL_ADD_FEATURE([EVAS_LOADER], [xcf], [${want_xcf}])

EFL_LIB_END([Evas])
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


#### Ecore CXX
EFL_LIB_START([Ecore_Cxx])

EFL_INTERNAL_DEPEND_PKG([ECORE_CXX], [Eina_Cxx])
EFL_INTERNAL_DEPEND_PKG([ECORE_CXX], [Ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_CXX], [Eina])
EFL_INTERNAL_DEPEND_PKG([ECORE_CXX], [Eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_CXX], [Efl])

EFL_EVAL_PKGS([ECORE_CXX])

EFL_LIB_END([Ecore_Cxx])
#### End of Ecore CXX

#### Ecore_Con

EFL_LIB_START([Ecore_Con])

### Default values

want_ecore_con_local_sockets="yes"
want_ecore_con_abstract_sockets="yes"

if test "${have_win32}" = "yes"; then
   want_ecore_con_abstract_sockets="no"
elif test "${have_darwin}" = "yes"; then
   want_ecore_con_abstract_sockets="yes"
elif test "${have_ps3}" = "yes"; then
   want_ecore_con_local_sockets="no"
   want_ecore_con_abstract_sockets="no"
fi

AC_DEFINE_IF([HAVE_LOCAL_SOCKETS],
   [test "x${want_ecore_con_local_sockets}" = "xyes"],
   [1], [Have local sockets support])
AC_DEFINE_IF([HAVE_ABSTRACT_SOCKETS],
   [test "x${want_ecore_con_abstract_sockets}" = "xyes"],
   [1], [Have abstract sockets namespace])

EFL_NET_CONTROL_BACKEND=none
if test "${have_linux}" = "yes"; then
   EFL_NET_CONTROL_BACKEND=connman
fi

AC_MSG_CHECKING([net-control backend to use])
AC_ARG_WITH([net-control],
   [AS_HELP_STRING([--with-net-control=BACKEND],[Specify which network control backend to be used by Efl.Net.Session and Efl.Net.Control. One of: connman or none])],
   [EFL_NET_CONTROL_BACKEND="${withval}"])

case "${EFL_NET_CONTROL_BACKEND}" in
   connman|none)
     ;;
   *)
     AC_MSG_ERROR([Unknown net-control backend --with-net-control=${EFL_NET_CONTROL_BACKEND}])
     ;;
esac
AC_MSG_RESULT([${EFL_NET_CONTROL_BACKEND}])
AC_SUBST(EFL_NET_CONTROL_BACKEND)

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([ECORE_CON], [all])
if test "$build_crypto" != "none" ; then
   EFL_CRYPTO_DEPEND([ECORE_CON])
fi
EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [efl])
EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [eet])
EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [eina])
EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [emile])

if test "${EFL_NET_CONTROL_BACKEND}" = "connman"; then
   EFL_INTERNAL_DEPEND_PKG([ECORE_CON], [eldbus])
fi
AM_CONDITIONAL([EFL_NET_CONTROL_BACKEND_CONNMAN], [test "${EFL_NET_CONTROL_BACKEND}" = "connman"])
AM_CONDITIONAL([EFL_NET_CONTROL_BACKEND_NONE], [test "${EFL_NET_CONTROL_BACKEND}" = "none"])

EFL_ADD_LIBS([ECORE_CON], [${requirement_nsl} -lm])

EFL_OPTIONAL_DEPEND_PKG([ECORE_CON], [${want_systemd}], [SYSTEMD], [libsystemd])

EFL_ADD_FEATURE([ECORE_CON], [local-sockets], [${want_ecore_con_local_sockets}])
EFL_ADD_FEATURE([ECORE_CON], [abstract-sockets], [${want_ecore_con_abstract_sockets}])
EFL_ADD_FEATURE([ECORE_CON], [systemd-daemon], [${want_systemd}])
EFL_ADD_FEATURE([ECORE_CON], [net-control], [${EFL_NET_CONTROL_BACKEND}])

EFL_EVAL_PKGS([ECORE_CON])

### Checks for header files

AC_CHECK_HEADERS([ws2tcpip.h netdb.h sys/filio.h])

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
AC_CHECK_FUNCS([accept4], ,
   [AC_CHECK_LIB([socket], [accept4],
   [AC_DEFINE([HAVE_ACCEPT4], [1])])])

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
EFL_INTERNAL_DEPEND_PKG([ECORE_IPC], [efl])
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
EFL_INTERNAL_DEPEND_PKG([ECORE_FILE], [efl])
EFL_INTERNAL_DEPEND_PKG([ECORE_FILE], [eina])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Ecore_File])
#### End of Ecore_File


#### Eeze
have_libmount_new="no"
have_libmount_old="no"
have_eeze_mount="no"

EFL_LIB_START_OPTIONAL([Eeze], [test "x${build_libeeze}" = "xyes"])

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
EFL_INTERNAL_DEPEND_PKG([EEZE], [efl])
EFL_INTERNAL_DEPEND_PKG([EEZE], [ecore-file])
EFL_INTERNAL_DEPEND_PKG([EEZE], [ecore-con])
EFL_INTERNAL_DEPEND_PKG([EEZE], [eet])
EFL_INTERNAL_DEPEND_PKG([EEZE], [emile])

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

PKG_CHECK_EXISTS([libudev < 199],
                 [have_libudev_old="yes"],
                 [have_libudev_old="no"])
AC_MSG_CHECKING([Use old libudev API (before 199)])
AC_MSG_RESULT([${have_libudev_old}])

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

if test "x${have_libudev_old}" = "xyes"; then
  AC_DEFINE_UNQUOTED([OLD_LIBUDEV], [1], [using older version of libudev])
fi

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


#### Ecore_Input
EFL_LIB_START([Ecore_Input])

### Additional options to configure

### Default values

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ECORE_INPUT], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT], [efl])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT], [eina])

EFL_OPTIONAL_INTERNAL_DEPEND_PKG([ECORE_INPUT], [${efl_lib_optional_eeze}], [eeze])

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

## Compatibility layers
EFL_PLATFORM_DEPEND([ECORE_INPUT_EVAS], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [evas])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [efl])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [eina])
EFL_INTERNAL_DEPEND_PKG([ECORE_INPUT_EVAS], [emile])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END([Ecore_Input_Evas])
#### End of Ecore_Input_Evas


#### Ecore_Cocoa
EFL_LIB_START_OPTIONAL([Ecore_Cocoa], [test "${have_cocoa}" = "yes"])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_COCOA], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_COCOA], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_COCOA], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_COCOA], [efl])
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
EFL_INTERNAL_DEPEND_PKG([ECORE_FB], [efl])
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
EFL_INTERNAL_DEPEND_PKG([ECORE_SDL], [efl])
EFL_INTERNAL_DEPEND_PKG([ECORE_SDL], [eina])

EFL_DEPEND_PKG([ECORE_SDL], [SDL], [sdl2 >= 2.0.0])

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
EFL_LIB_START_OPTIONAL([Ecore_Wayland], [test "${want_ecore_wayland}" = "yes"])

if test "x${want_wayland_ivi_shell}" = "xyes" ; then
   AC_DEFINE(USE_IVI_SHELL, 1, [Ecore_Wayland IVI-Shell Support])
fi

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_WAYLAND], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_WAYLAND], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_WAYLAND], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_WAYLAND], [efl])
EFL_INTERNAL_DEPEND_PKG([ECORE_WAYLAND], [eina])

EFL_DEPEND_PKG([ECORE_WAYLAND], [WAYLAND],
   [wayland-client >= 1.8.0 wayland-cursor >= 1.8.0 xkbcommon >= 0.5.0 uuid])

EFL_EVAL_PKGS([ECORE_WAYLAND])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Wayland])
#### End of Ecore_Wayland

#### Ecore_Wl2
EFL_LIB_START_OPTIONAL([Ecore_Wl2], [test "${want_wayland}" = "yes"])

if test "x${want_wayland_ivi_shell}" = "xyes" ; then
   AC_DEFINE(USE_IVI_SHELL, 1, [Ecore_Wl2 IVI-Shell Support])
fi

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_WL2], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ECORE_WL2], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_WL2], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_WL2], [efl])
EFL_INTERNAL_DEPEND_PKG([ECORE_WL2], [eina])

EFL_DEPEND_PKG([ECORE_WL2], [WAYLAND],
   [wayland-server >= 1.11.0 wayland-client >= 1.11.0 xkbcommon >= 0.6.0])

EFL_EVAL_PKGS([ECORE_WL2])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Wl2])
#### End of Ecore_Wl2

#### Efl_Wl
EFL_LIB_START_OPTIONAL([Efl_Wl], [test -n "${ECORE_WL2_CFLAGS}"])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([EFL_WL], [ecore-wl2])
EFL_INTERNAL_DEPEND_PKG([EFL_WL], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([EFL_WL], [ecore])
EFL_INTERNAL_DEPEND_PKG([EFL_WL], [ecore-evas])
EFL_INTERNAL_DEPEND_PKG([EFL_WL], [evas])
EFL_INTERNAL_DEPEND_PKG([EFL_WL], [emile])
EFL_INTERNAL_DEPEND_PKG([EFL_WL], [eo])
EFL_INTERNAL_DEPEND_PKG([EFL_WL], [efl])
EFL_INTERNAL_DEPEND_PKG([EFL_WL], [eina])
EFL_OPTIONAL_INTERNAL_DEPEND_PKG([EFL_WL], [${want_x11_any}], [ecore_x])

EFL_DEPEND_PKG([EFL_WL], [WAYLAND],
               [wayland-server >= 1.11.0 xkbcommon >= 0.6.0])
EFL_OPTIONAL_DEPEND_PKG([EFL_WL], [${want_x11_any}], [XKBCOMMONX11], [xkbcommon-x11])

EFL_EVAL_PKGS([EFL_WL])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Efl_Wl])
#### End of Efl_Wl

#### Eldbus
EFL_LIB_START([Eldbus])

### Additional options to configure

### Default values

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ELDBUS], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ELDBUS], [ecore])
EFL_INTERNAL_DEPEND_PKG([ELDBUS], [efl])
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



#### Ecore_Drm
EFL_LIB_START_OPTIONAL([Ecore_Drm], [test "${want_ecore_drm}" = "yes"])

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
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM], [eldbus])
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM], [eeze])
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM], [efl])
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM], [eina])

EFL_DEPEND_PKG([ECORE_DRM], [DRM], [libdrm >= 2.4 xkbcommon >= 0.3.0 gbm])
EFL_DEPEND_PKG([ECORE_DRM], [LIBINPUT], [libinput >= 1.6.0])

EFL_ADD_LIBS([ECORE_DRM], [-lm])

EFL_EVAL_PKGS([ECORE_DRM])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Drm])
#### End of Ecore_Drm


#### Elput
EFL_LIB_START_OPTIONAL([Elput], [test "${want_elput}" = "yes"])

### Additional options to configure
SUID_CFLAGS=-fPIE
SUID_LDFLAGS=-pie
AC_SUBST([SUID_CFLAGS])
AC_SUBST([SUID_LDFLAGS])

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ELPUT], [ecore])
EFL_INTERNAL_DEPEND_PKG([ELPUT], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ELPUT], [eldbus])
EFL_INTERNAL_DEPEND_PKG([ELPUT], [eeze])
EFL_INTERNAL_DEPEND_PKG([ELPUT], [eo])
EFL_INTERNAL_DEPEND_PKG([ELPUT], [eina])

EFL_DEPEND_PKG([ELPUT], [LIBINPUT], [libinput >= 1.7.0 xkbcommon >= 0.3.0 libudev])

AC_ARG_ENABLE([elogind],
   [AS_HELP_STRING([--enable-elogind],[enable elogind support.@<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_elogind="yes"
    else
       want_elogind="no"
    fi
   ], [
    want_elogind="no"
   ])

EFL_OPTIONAL_DEPEND_PKG([ELPUT], [${want_systemd}], [SYSTEMD], [libsystemd])
EFL_OPTIONAL_DEPEND_PKG([ELPUT], [${want_elogind}], [ELOGIND], [libelogind])
EFL_ADD_FEATURE([ELPUT], [systemd-logind], [${want_systemd}])
EFL_ADD_FEATURE([ELPUT], [elogind], [${want_elogind}])

EFL_ADD_LIBS([ELPUT], [-lm])


EFL_EVAL_PKGS([ELPUT])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Elput])
#### End of Elput

#### Ecore_Drm2
EFL_LIB_START_OPTIONAL([Ecore_Drm2], [test "${want_drm}" = "yes"])

### Additional options to configure
SUID_CFLAGS=-fPIE
SUID_LDFLAGS=-pie
AC_SUBST([SUID_CFLAGS])
AC_SUBST([SUID_LDFLAGS])

### Default values

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM2], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM2], [efl])
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM2], [eina])
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM2], [ecore])
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM2], [elput])
EFL_INTERNAL_DEPEND_PKG([ECORE_DRM2], [eeze])

EFL_DEPEND_PKG([ECORE_DRM2], [DRM], [gbm])

EFL_ADD_LIBS([ECORE_DRM2], [-lm])
EFL_ADD_LIBS([ECORE_DRM2], [-ldl])

EFL_EVAL_PKGS([ECORE_DRM2])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Drm2])
#### End of Ecore_Drm2


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

EFL_LIB_START_OPTIONAL([Ecore_Audio], [test "${want_audio}" = "yes"])

### Additional options to configure

# ALSA support is still not there, thus no option for it yet.
want_alsa="no"

# sndfile is mandatory otherwise it won't read from/write to files.
# TODO: if confirmed sndfile is mandatory, remove this variable
# TODO: and the EFL_OPTIONAL_DEPEND_PKG(), use EFL_DEPEND_PKG()
want_sndfile="yes"

want_wasapiaudio="no"

AC_ARG_ENABLE([pulseaudio],
   [AS_HELP_STRING([--disable-pulseaudio],[disable pulseaudio sound support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_pulseaudio="yes"
    else
       want_pulseaudio="no"
       if test "x${have_win32}" = "xyes" ; then
         want_wasapiaudio="yes"
       else
          want_wasapiaudio="no"
          CFOPT_WARNING="xyes"
       fi
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
EFL_INTERNAL_DEPEND_PKG([ECORE_AUDIO], [efl])
EFL_INTERNAL_DEPEND_PKG([ECORE_AUDIO], [eina])
EFL_INTERNAL_DEPEND_PKG([ECORE_AUDIO], [emile])

EFL_ADD_LIBS([ECORE_AUDIO], [-lm])

if test "x${want_alsa}" = "xyes" ; then
  PKG_CHECK_MODULES([ECORE_AUDIO_ALSA], [alsa])
  AC_DEFINE([HAVE_ALSA], [1], [Alsa support])
fi
if test "x${want_pulseaudio}" = "xyes" ; then
  PKG_CHECK_MODULES([ECORE_AUDIO_PULSE], [libpulse])
  AC_DEFINE([HAVE_PULSE], [1], [Pulseaudio support])
fi
if test "x${want_sndfile}" = "xyes" ; then
  PKG_CHECK_MODULES([ECORE_AUDIO_SNDFILE], [sndfile])
  AC_DEFINE([HAVE_SNDFILE], [1], [Sndfile support])
fi
if test "x${want_wasapiaudio}" = "xyes" ; then
  EFL_ADD_LIBS([ECORE_AUDIO], [-luuid -lwinmm -lksuser])
  AC_DEFINE([HAVE_WASAPI], [1], [Wasapiaudio support])
fi

dnl EFL_OPTIONAL_DEPEND_PKG([ECORE_AUDIO_ALSA], [${want_alsa}], [ALSA], [alsa])
dnl EFL_OPTIONAL_DEPEND_PKG([ECORE_AUDIO_PULSE], [${want_pulseaudio}], [PULSE], [libpulse])
dnl EFL_OPTIONAL_DEPEND_PKG([ECORE_AUDIO_SNDFILE], [${want_sndfile}], [SNDFILE], [sndfile])

EFL_EVAL_PKGS([ECORE_AUDIO])

EFL_ADD_FEATURE([ECORE_AUDIO], [alsa])
EFL_ADD_FEATURE([ECORE_AUDIO], [pulseaudio])
EFL_ADD_FEATURE([ECORE_AUDIO], [sndfile])
EFL_ADD_FEATURE([ECORE_AUDIO], [wasapiaudio])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

EFL_LIB_END_OPTIONAL([Ecore_Audio])
AM_CONDITIONAL([HAVE_ECORE_AUDIO_PULSE], [test "x${want_pulseaudio}" = "xyes"])
AM_CONDITIONAL([HAVE_ECORE_AUDIO_SNDFILE], [test "x${want_sndfile}" = "xyes"])
AM_CONDITIONAL([HAVE_ECORE_AUDIO_WASASPI], [test "x${want_wasapiaudio}" = "xyes"])

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
EFL_INTERNAL_DEPEND_PKG([ECORE_WIN32], [efl])
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
EFL_INTERNAL_DEPEND_PKG([ECORE_AVAHI], [efl])

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
EFL_INTERNAL_DEPEND_PKG([ECORE_X], [efl])
EFL_INTERNAL_DEPEND_PKG([ECORE_X], [eina])

## Xlib

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

   EFL_EVAL_PKGS([ECORE_X])
   EFL_CHECK_FUNCS([ECORE_X], [dlopen dlsym])
fi

EFL_ADD_LIBS([ECORE_X], [${ECORE_X_LIBS}])

EFL_LIB_END_OPTIONAL([Ecore_X])

AM_CONDITIONAL([HAVE_ECORE_X_XLIB], [test "${want_x11_xlib}" = "yes"])
#### End of Ecore_X


#### Ecore_Imf
EFL_LIB_START([Ecore_Imf])

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
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF], [efl])
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
      [wayland-client >= 1.11.0],
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

EFL_LIB_END([Ecore_Imf])
#### End of Ecore_Imf


#### Ecore_Imf_Evas
EFL_LIB_START([Ecore_Imf_Evas])

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
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF_EVAS], [efl])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF_EVAS], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF_EVAS], [eina])
EFL_INTERNAL_DEPEND_PKG([ECORE_IMF_EVAS], [emile])

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
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [efl])
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [eina])
EFL_INTERNAL_DEPEND_PKG([ECORE_EVAS], [emile])

## modules
ECORE_EVAS_MODULE([extn], [${want_ecore_evas_extn}])
ECORE_EVAS_MODULE([ews], [yes])
ECORE_EVAS_MODULE([fb], [${want_fb}])
ECORE_EVAS_MODULE([drm], [${want_drm}])
ECORE_EVAS_MODULE([gl-drm], [${want_gl_drm}])

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
if test "x$have_ecore_evas_software_x11" = "xyes" ; then
   have_ecore_evas_software_xlib=${have_evas_engine_software_xlib}
   if test "x${have_ecore_evas_software_xlib}" = "xstatic"; then
      have_ecore_evas_software_xlib="yes"
   fi
   if test "x${have_ecore_evas_software_xlib}" = "xyes"; then
      AC_DEFINE([BUILD_ECORE_EVAS_SOFTWARE_XLIB], [1], [Evas Software Xlib Engine Support])
   fi
fi

# XXX TODO: ecore_evas_opengl_x11

ECORE_EVAS_MODULE([opengl-x11], [${want_x11_any_opengl}])

have_ecore_evas_opengl_xlib="no"
if test "x${have_ecore_evas_opengl_x11}" = "xyes" || test "x${have_ecore_evas_opengl_x11}" = "xstatic" ; then
   have_ecore_evas_opengl_xlib=${have_evas_engine_gl_xlib}
   if test "x${have_ecore_evas_opengl_xlib}" = "xyes" ; then
      AC_DEFINE([BUILD_ECORE_EVAS_OPENGL_XLIB], [1], [OpenGL Xlib rendering backend])
   fi
fi

build_ecore_evas_x11="no"
if test "x$have_ecore_evas_software_x11" = "xyes" || \
   test "x$have_ecore_evas_opengl_x11" = "xyes"; then
   AC_DEFINE([BUILD_ECORE_EVAS_X11], [1], [Support for X Window Engines in Ecore_Evas])
   build_ecore_evas_x11="yes"
fi

AM_CONDITIONAL([BUILD_ECORE_EVAS_X11], [test "${build_ecore_evas_x11}" = "yes"])

build_ecore_evas_vnc="no"
if test "${want_vnc_server}" = "yes" && \
   (test "${build_ecore_evas_x11}" = "yes" || \
   test "${want_fb}" = "yes"); then
   AC_DEFINE([BUILD_ECORE_EVAS_VNC_SERVER], [1], [Build Ecore_Evas VNC module])
   build_ecore_evas_vnc="yes"
fi

AM_CONDITIONAL([BUILD_ECORE_EVAS_VNC_SERVER], [test "${build_ecore_evas_vnc}" = "yes"])
AC_DEFINE_IF([BUILD_ECORE_EVAS_VNC_SERVER], [test "${build_ecore_evas_vnc}" = "yes"], [1], [Build Ecore_Evas VNC module])
EFL_ADD_FEATURE([ECORE_EVAS], [vnc_server], [${build_ecore_evas_vnc}])

EFL_EVAL_PKGS([ECORE_EVAS])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions
if test "x${want_gl_drm}" = "xyes" ; then
   EFL_CHECK_FUNCS([ECORE_EVAS], [dlopen])
fi

EFL_LIB_END([Ecore_Evas])
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
EFL_INTERNAL_DEPEND_PKG([EIO], [ecore-file])
EFL_INTERNAL_DEPEND_PKG([EIO], [eet])
EFL_INTERNAL_DEPEND_PKG([EIO], [eo])
EFL_INTERNAL_DEPEND_PKG([EIO], [eina])
EFL_INTERNAL_DEPEND_PKG([EIO], [efl])
EFL_INTERNAL_DEPEND_PKG([EIO], [emile])
EFL_INTERNAL_DEPEND_PKG([EIO], [efreet-mime])

EFL_ADD_LIBS([EIO], [-lm])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions
have_inotify="${ac_cv_header_sys_inotify_h}"
AM_CONDITIONAL([HAVE_INOTIFY], [test "x${have_inotify}" = "xyes"])

have_notify_win32="${have_win32}"
AC_DEFINE_IF([HAVE_NOTIFY_WIN32],
   [test "x${have_notify_win32}" = "xyes"], [1],
   [File monitoring with Windows notification])
AM_CONDITIONAL([HAVE_NOTIFY_WIN32], [test "x${have_notify_win32}" = "xyes"])

AC_DEFINE_IF([HAVE_NOTIFY_COCOA],
   [test "x${have_darwin}" = "xyes"], [1],
   [File monitoring with fsevent notification])
AM_CONDITIONAL([HAVE_NOTIFY_COCOA], [test "x${have_darwin}" = "xyes"])

AC_CHECK_FUNC([kevent])
have_notify_kevent="${ac_cv_func_kevent}"
AC_DEFINE_IF([HAVE_NOTIFY_KEVENT],
   [test "x${have_notify_kevent}" = "xyes"], [1],
   [File monitoring with kqueue/kevent mechanism])
AM_CONDITIONAL([HAVE_NOTIFY_KEVENT], [test "x${have_notify_kevent}" = "xyes"])


EFL_LIB_END([Eio])
dnl TODO: remove these ifdefs from code!
AC_DEFINE([HAVE_EIO], [1], [Have eio library])
#### End of Eio

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
EFL_INTERNAL_DEPEND_PKG([EFREET], [eo])
EFL_INTERNAL_DEPEND_PKG([EFREET], [efl])
EFL_INTERNAL_DEPEND_PKG([EFREET], [eina])
EFL_INTERNAL_DEPEND_PKG([EFREET], [emile])
EFL_INTERNAL_DEPEND_PKG([EFREET], [ecore-con])
EFL_INTERNAL_DEPEND_PKG([EFREET], [ecore-ipc])

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

if test "x${want_physics}" = "xyes" ; then
  # We should be using ${CXX} here, but there is a bug in
  # autotools macro and CXX is always set to g++ even if
  # it's not found. So we are using an internal variable
  # that does the work for now, may get broken in the future.
  if test "x${ac_ct_CXX}" = "x" -a "x${CXX}" = "xg++"; then
     AC_MSG_ERROR([efl requires a C++ compiler got ${ac_ct_CXX} and ${CXX}.])
  fi
fi

EFL_LIB_START_OPTIONAL([EPhysics], [test "${want_physics}" = "yes"])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([EPHYSICS], [evil])

EFL_INTERNAL_DEPEND_PKG([EPHYSICS], [eina])
EFL_INTERNAL_DEPEND_PKG([EPHYSICS], [evas])
EFL_INTERNAL_DEPEND_PKG([EPHYSICS], [efl])
EFL_INTERNAL_DEPEND_PKG([EPHYSICS], [ecore])
EFL_INTERNAL_DEPEND_PKG([EPHYSICS], [eo])
EFL_INTERNAL_DEPEND_PKG([EPHYSICS], [emile])

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

AC_ARG_ENABLE([multisense],
   [AS_HELP_STRING([--enable-multisense],[Enable multisense support. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_multisense="yes"
    else
       want_multisense="no"
       CFOPT_WARNING="xyes"
    fi
   ],
   [
    if test "x${want_pulseaudio}" = "xyes"; then
       want_multisense="yes"
    else
       if test "x${want_wasapiaudio}" = "xyes"; then
          want_multisense="yes"
       else
          want_multisense="no"
       fi
    fi

   ])

# TODO: should we keep or remove these?
want_edje_program_cache="no"
want_edje_calc_cache="yes"
want_fixed_point="no"

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([EDJE], [evil])

EFL_INTERNAL_DEPEND_PKG([EDJE], [eina])
EFL_INTERNAL_DEPEND_PKG([EDJE], [eo])
EFL_INTERNAL_DEPEND_PKG([EDJE], [efl])
EFL_INTERNAL_DEPEND_PKG([EDJE], [eet])
EFL_INTERNAL_DEPEND_PKG([EDJE], [evas])
EFL_INTERNAL_DEPEND_PKG([EDJE], [ecore])
EFL_INTERNAL_DEPEND_PKG([EDJE], [ecore-evas])
EFL_INTERNAL_DEPEND_PKG([EDJE], [ecore-file])
EFL_INTERNAL_DEPEND_PKG([EDJE], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([EDJE], [ecore-imf])
EFL_INTERNAL_DEPEND_PKG([EDJE], [ecore-imf-evas])
EFL_INTERNAL_DEPEND_PKG([EDJE], [embryo])
EFL_INTERNAL_DEPEND_PKG([EDJE], [emile])
EFL_INTERNAL_DEPEND_PKG([EDJE], [efreet])
EFL_INTERNAL_DEPEND_PKG([EDJE], [eio])

if test "x${want_physics}" = "xyes" ; then
  AC_DEFINE([HAVE_EPHYSICS], [1], [EPhysics Support])
  EDJE_CFLAGS="-I\$(top_srcdir)/src/lib/ephysics ${EDJE_CFLAGS}"
fi
dnl EFL_OPTIONAL_INTERNAL_DEPEND_PKG([EDJE], [${want_physics}], [ephysics])

if test "x${want_lua_old}" = "xyes"; then
   EFL_CHECK_LUA_OLD([EDJE])
else
   PKG_CHECK_MODULES([EDJE_LUA], [luajit >= 2.0.0])
fi

EFL_OPTIONAL_INTERNAL_DEPEND_PKG([EDJE], [${want_multisense}], [ecore-audio])

EFL_ADD_FEATURE([EDJE], [physics])
EFL_ADD_FEATURE([EDJE], [multisense])
EFL_ADD_FEATURE([EDJE], [lua-old])

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

EFL_LIB_END([Edje])
#### End of Edje

#### Edje CXX
EFL_LIB_START([Edje_Cxx])

EFL_EVAL_PKGS([EDJE_CXX])

EFL_LIB_END([Edje_Cxx])
#### End of Edje CXX

#### Emotion
EFL_LIB_START([Emotion])

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

AC_ARG_ENABLE([libvlc],
   [AS_HELP_STRING([--enable-libvlc],[enable libvlc support. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_libvlc="yes"
    else
       want_libvlc="no"
    fi
   ],
   [want_libvlc="no"])

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([EMOTION], [eina])
EFL_INTERNAL_DEPEND_PKG([EMOTION], [eo])
EFL_INTERNAL_DEPEND_PKG([EMOTION], [ecore])
EFL_INTERNAL_DEPEND_PKG([EMOTION], [eet])
EFL_INTERNAL_DEPEND_PKG([EMOTION], [evas])
EFL_INTERNAL_DEPEND_PKG([EMOTION], [efl])
EFL_INTERNAL_DEPEND_PKG([EMOTION], [eio])
EFL_INTERNAL_DEPEND_PKG([EMOTION], [emile])

EFL_OPTIONAL_INTERNAL_DEPEND_PKG([EMOTION], [${efl_lib_optional_eeze}], [eeze])
EFL_ADD_FEATURE([EMOTION], [v4l2])

## modules

have_gst_xoverlay="no"

EMOTION_MODULE([xine], [${want_xine}])
EMOTION_MODULE([gstreamer], [${want_gstreamer}])
EMOTION_MODULE([gstreamer1], [${want_gstreamer1}])
EMOTION_MODULE([libvlc], [${want_libvlc}])
EMOTION_MODULE([generic], [${want_emotion_generic}])

EFL_ADD_FEATURE([EMOTION], [xine])
EFL_ADD_FEATURE([EMOTION], [gstreamer])
EFL_ADD_FEATURE([EMOTION], [gstreamer1])
EFL_ADD_FEATURE([EMOTION], [libvlc])
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

## Generic player
EMOTION_GENERIC_PLAYER([GENERIC_VLC], [no], [libvlc >= 2.0])
EFL_ADD_FEATURE([EMOTION], [generic_vlc], [${with_generic_vlc}])

EFL_LIB_END([Emotion])
#### End of Emotion

#### Ethumb
EFL_LIB_START([Ethumb])

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
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [efl])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [eo])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [ecore])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [ecore-evas])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [ecore-file])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [ecore-imf])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [edje])
EFL_INTERNAL_DEPEND_PKG([ETHUMB], [emile])

## modules

EFL_EVAL_PKGS([ETHUMB])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Check availability

EFL_LIB_END([Ethumb])
#### End of Ethumb

#### Ethumb_Client
EFL_LIB_START([Ethumb_Client])

### Default values

### Additional options to configure

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ETHUMB_CLIENT], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [eina])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [eo])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [efl])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [eet])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [ecore])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [ecore-imf])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [edje])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [eldbus])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [ethumb])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [evas])
EFL_INTERNAL_DEPEND_PKG([ETHUMB_CLIENT], [emile])

EFL_EVAL_PKGS([ETHUMB_CLIENT])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Check availability

EFL_LIB_END([Ethumb_Client])
#### End of Ethumb_Client

#### Elua
AC_ARG_ENABLE([elua],
   [AS_HELP_STRING([--enable-elua],[enable ELUA support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_elua="yes"
    else
       want_elua="no"
    fi
   ],
   [want_elua="yes"])

# turn this into an AC_ARG_ENABLE when bindings are reenabled
want_lua_bindings="no"

have_elua="yes"
have_lua_bindings="yes"

if test "${want_lua_old}" = "yes" -o "x$want_elua" = "xno"; then
  have_elua="no"
fi

if test "x$have_elua" = "xno" -o "x$want_lua_bindings" = "xno"; then
  have_lua_bindings="no"
fi

AM_CONDITIONAL([HAVE_ELUA], [test "x${have_elua}" = "xyes"])
AM_CONDITIONAL([HAVE_LUA_BINDINGS], [test "x${have_lua_bindings}" = "xyes"])

EFL_LIB_START_OPTIONAL([Elua], [test "${have_elua}" = "yes"])

### Default values

### Additional options to configure

### Checks for programs

## Compatibility layers
EFL_PLATFORM_DEPEND([ELUA], [evil])

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ELUA], [eina])
EFL_INTERNAL_DEPEND_PKG([ELUA], [eo])
EFL_INTERNAL_DEPEND_PKG([ELUA], [efl])
EFL_INTERNAL_DEPEND_PKG([ELUA], [ecore])
EFL_INTERNAL_DEPEND_PKG([ELUA], [ecore_file])

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

#### Elocation

EFL_LIB_START([Elocation])

### Default values

### Additional options to configure

### Checks for programs

### Checks for libraries
EFL_PLATFORM_DEPEND([ELOCATION], [evil])
EFL_INTERNAL_DEPEND_PKG([ELOCATION], [eina])
EFL_INTERNAL_DEPEND_PKG([ELOCATION], [eo])
EFL_INTERNAL_DEPEND_PKG([ELOCATION], [efl])
EFL_INTERNAL_DEPEND_PKG([ELOCATION], [ecore])
EFL_INTERNAL_DEPEND_PKG([ELOCATION], [eldbus])

EFL_ADD_LIBS([ELOCATION], [-lm])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Check availability

EFL_LIB_END([Elocation])
#### End of Elocation



#### Efl_Js
EFL_LIB_START_OPTIONAL([Efl_Js], [test "x${have_js}" = "xyes"])

### Default values

### Additional options to configure

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Eina])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Eo])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Ecore])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Eet])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Ecore_Evas])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Ecore_Con])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Ecore_Audio])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Efl])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Evas])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Edje])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Emotion])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Eldbus])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Emile])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Ethumb_Client])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Eio])
EFL_INTERNAL_DEPEND_PKG([EFL_JS], [Elementary])

AM_COND_IF([HAVE_NODEJS], [], [ EFL_ADD_LIBS([EFL_JS], [-lv8]) ])

EFL_EVAL_PKGS([EFL_JS])

AM_COND_IF([HAVE_NODEJS], [EFLJS_CXXFLAGS="$EFLJS_CXXFLAGS -DHAVE_NODEJS"], [])
AM_COND_IF([HAVE_LIBUV], [EFLJS_CXXFLAGS="$EFLJS_CXXFLAGS -DHAVE_LIBUV"], [])
AM_COND_IF([HAVE_JS], [EFLJS_CXXFLAGS="$EFLJS_CXXFLAGS -DHAVE_JS"], [])

AM_COND_IF([HAVE_V8_CREATE_PARAMS], [EFLJS_CXXFLAGS="$EFLJS_CXXFLAGS -DHAVE_V8_CREATE_PARAMS"], [])
AM_COND_IF([HAVE_V8_GLOBAL], [EFLJS_CXXFLAGS="$EFLJS_CXXFLAGS -DHAVE_V8_GLOBAL"], [])
AC_SUBST([EFLJS_CXXFLAGS])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Check availability

EFL_LIB_END_OPTIONAL([Efl_Js])
#### End of Efl_Js

### Add Wayland server library if test is enabled
if test "x${want_tests}" = "xyes" -a "x${want_wayland}" = "xyes"; then
   EFL_DEPEND_PKG([ECORE_WAYLAND_SRV], [WAYLAND], [wayland-server >= 1.11.0])
   EFL_EVAL_PKGS([ECORE_WAYLAND_SRV])
fi

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

#### Ecore_Buffer
build_ecore_buffer_x11_dri2="no"
build_ecore_buffer_x11_dri3="no"
EFL_LIB_START_OPTIONAL([Ecore_Buffer], [test "${want_ecore_buffer}" = "yes"])
### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ECORE_BUFFER], [eina])
EFL_INTERNAL_DEPEND_PKG([ECORE_BUFFER], [eo])
EFL_INTERNAL_DEPEND_PKG([ECORE_BUFFER], [efl])
EFL_INTERNAL_DEPEND_PKG([ECORE_BUFFER], [ecore])
EFL_DEPEND_PKG([ECORE_BUFFER], [WAYLAND],
               [wayland-server >= 1.11.0 wayland-client >= 1.11.0])

PKG_CHECK_MODULES([X11_DRI_COMMON],
   [
      libtbm >= 1.1.0,
      libdrm >= 2.4.35,
   ],
   [have_x11_dri_common_pkgs="yes"],
   [have_x11_dri_common_pkgs="no"]
)

if test "x$have_x11_dri_common_pkgs" = "xyes" ; then
   EFL_INTERNAL_DEPEND_PKG([ECORE_BUFFER], [ecore_x])

   PKG_CHECK_MODULES([X11_DRI2], [libdri2],
                     [have_x11_dri2_pkgs="yes"],
                     [have_x11_dri2_pkgs="no"])
   PKG_CHECK_MODULES([X11_DRI3],
      [
         xshmfence,
         xcb,
         x11-xcb,
         xcb-sync,
         xcb-dri3
      ],
      [have_x11_dri3_pkgs="yes"],
      [have_x11_dri3_pkgs="no"])
fi

if test "x${have_x11_dri2_pkgs}" = "xyes" ; then
   build_ecore_buffer_x11_dri2="yes"
   AC_DEFINE(BUILD_ECORE_BUFFER_X11_DRI2, 1, [Support for X11_DRI2 Backend in Ecore_Buffer])
fi

if test "x${have_x11_dri3_pkgs}" = "xyes" ; then
   build_ecore_buffer_x11_dri3="yes"
   AC_DEFINE(BUILD_ECORE_BUFFER_X11_DRI3, 1, [Support for X11_DRI3 Backend in Ecore_Buffer])
fi
EFL_EVAL_PKGS([ECORE_BUFFER])

EFL_ADD_FEATURE([ECORE_BUFFER], [shm], ["yes"])
EFL_ADD_FEATURE([ECORE_BUFFER], [x11_dri2], [${build_ecore_buffer_x11_dri2}])
EFL_ADD_FEATURE([ECORE_BUFFER], [x11_dri3], [${build_ecore_buffer_x11_dri3}])

EFL_LIB_END_OPTIONAL([Ecore_Buffer])

AM_CONDITIONAL([BUILD_ECORE_BUFFER_X11_DRI2], [test "${build_ecore_buffer_x11_dri2}" = "xyes"])
AM_CONDITIONAL([BUILD_ECORE_BUFFER_X11_DRI3], [test "${build_ecore_buffer_x11_dri3}" = "xyes"])

#### End of Ecore_Buffer


#### Elementary

EFL_LIB_START([Elementary])

### Default values

### Additional options to configure

## Elementary base dir

AC_ARG_WITH([elementary-base-dir],
   [AS_HELP_STRING([--with-elementary-base-dir=PATH], [specify the subdirectory for all elementary data @<:@default=${elementary_base_dir}@:>@])],
   [elementary_base_dir=${withval}],
   [elementary_base_dir=".elementary"])

AC_MSG_NOTICE([ELEMENTARY_BASE_DIR set to ${elementary_base_dir}])
AC_DEFINE_UNQUOTED([ELEMENTARY_BASE_DIR], ["${elementary_base_dir}"], ["subdirectory for all elementary data"])

## Debug mode

AC_ARG_ENABLE([debug],
   [AS_HELP_STRING([--enable-debug], [enable elementary debug support. @<:@default=disabled@:>@])],
   [want_elementary_debug=$enableval],
   [want_elementary_debug="no"])

ELM_DEBUG_DEF="#undef"
if test "x$want_elementary_debug" = "xyes"; then
   AC_DEFINE([HAVE_ELEMENTARY_DEBUG], [1], [Elementary debug.])
   ELM_DEBUG_DEF="#define"
fi
AC_SUBST([ELM_DEBUG_DEF])

## quicklaunch support

AC_ARG_ENABLE([quick-launch],
   [AS_HELP_STRING([--disable-quick-launch], [disable quick-launch support, @<:@default=detect@:>@])],
   [want_quicklaunch=$enableval],
   [want_quicklaunch="auto"])

### Checks for programs

### Checks for libraries

## Compatibility layers

EFL_PLATFORM_DEPEND([ELEMENTARY], [evil])

EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [eina])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [emile])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [eet])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [eo])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [efl])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [evas])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [ecore])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [ecore-evas])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [ecore-file])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [ecore-input])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [edje])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [ethumb])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [ethumb_client])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [emotion])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [ecore-imf])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [ecore-con])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [eldbus])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [efreet])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [efreet-mime])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [efreet-trash])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [eio])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [elocation])

EFL_OPTIONAL_INTERNAL_DEPEND_PKG([ELEMENTARY], [${want_x11_any}], [ecore_x])
EFL_OPTIONAL_INTERNAL_DEPEND_PKG([ELEMENTARY], [${want_fb}], [ecore_fb])
EFL_OPTIONAL_INTERNAL_DEPEND_PKG([ELEMENTARY], [${want_sdl}], [ecore_sdl])
EFL_OPTIONAL_INTERNAL_DEPEND_PKG([ELEMENTARY], [${want_ecore_evas_gl_cocoa}], [ecore_cocoa])
EFL_OPTIONAL_INTERNAL_DEPEND_PKG([ELEMENTARY], [${build_ecore_evas_win32}], [ecore_win32])
EFL_OPTIONAL_INTERNAL_DEPEND_PKG([ELEMENTARY], [${want_drm}], [ecore_drm2])
EFL_OPTIONAL_INTERNAL_DEPEND_PKG([ELEMENTARY], [${build_ecore_evas_wayland}], [ecore_wl2])

EFL_OPTIONAL_DEPEND_PKG([ELEMENTARY], [${want_elput}], [ELPUT], [xkbcommon])
EFL_OPTIONAL_DEPEND_PKG([ELEMENTARY], [${want_wayland}], [WAYLAND], [wayland-client])

dnl Special case deps for ecore_drm
if test "${want_drm}" = "yes"; then
  EFL_DEPEND_PKG([ELEMENTARY], [DRM], [libdrm])
  EFL_INTERNAL_DEPEND_PKG([ELEMENTARY], [eeze])
fi

EFL_ADD_LIBS([ELEMENTARY], [-lm])

ELM_CHECK_BACKEND([X], [${want_x11_any}])
ELM_CHECK_BACKEND([FB], [${want_fb}])
ELM_CHECK_BACKEND([SDL], [${want_sdl}])
ELM_CHECK_BACKEND([Cocoa], [${want_ecore_evas_gl_cocoa}])
ELM_CHECK_BACKEND([Win32], [${build_ecore_evas_win32}])
ELM_CHECK_BACKEND([Wl2], [${build_ecore_evas_wayland}])
ELM_CHECK_BACKEND([DRM], [${want_drm}])

EFL_EVAL_PKGS([ELEMENTARY])

### Checks for header files

ELM_ALLOCA_H_DEF="#undef"
AC_CHECK_HEADER([alloca.h], [ELM_ALLOCA_H_DEF="#define"])
AC_SUBST([ELM_ALLOCA_H_DEF])

ELM_LIBINTL_H_DEF="#undef"
AC_CHECK_HEADER([libintl.h], [ELM_LIBINTL_H_DEF="#define"])
AC_SUBST([ELM_LIBINTL_H_DEF])

ELM_DIRENT_H_DEF="#undef"
AC_CHECK_HEADER([dirent.h], [ELM_DIRENT_H_DEF="#define"])
AC_SUBST([ELM_DIRENT_H_DEF])

AC_CHECK_HEADER([sys/mman.h], [have_mman="yes"], [have_mman="no"])
if test "x${have_mman}" = "xyes"; then
   AC_DEFINE([HAVE_MMAN_H], [1], [Have sys/mman.h header file])
fi

AC_CHECK_HEADERS([locale.h langinfo.h sys/times.h])

case "$host_os" in
   darwin*)
      AC_CHECK_HEADERS([crt_externs.h])
      ;;
esac

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

requirements_libs_elm="${requirement_socket} ${requirements_libs_elm}"

AC_SUBST([requirements_libs_elm])

AM_CONDITIONAL([BUILD_RUN], [test "x$have_socket" = "xyes"])

# Check if we can build binary with quicklaunch support
ELM_QUICKLAUNCH

### Checks for library functions

EFL_CHECK_FUNCS([ELEMENTARY], [dlopen])

AC_FUNC_ALLOCA

AC_CHECK_FUNCS([geteuid getuid getpwent])

AC_CHECK_FUNCS([fork clearenv])

build_quicklaunch="no"
if test "x${ac_cv_func_fork}" = "xyes" -a "x${efl_func_dlopen}" = "xyes"; then
   build_quicklaunch="yes"
fi

AM_CONDITIONAL([BUILD_QUICKLAUNCH], [test "x${build_quicklaunch}" = "xyes"])
EFL_ADD_FEATURE([ELEMENTARY], [quicklaunch], [${build_quicklaunch}])

# environ variable

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
      [[
# define _GNU_SOURCE 1
#include <unistd.h>
      ]],
      [[
extern char **environ;
      ]])
   ],
   [AC_DEFINE([HAVE_ENVIRON], [1], [extern environ exists])])

case "$host_os" in
   darwin*)
      AC_DEFINE([environ], [(*_NSGetEnviron())], ["apple doesn't follow POSIX in this case."])
      ;;
esac

### Check availability

EFL_LIB_END([Elementary])

#### End of Elementary


#### Elementary CXX
EFL_LIB_START([Elementary_Cxx])

EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_CXX], [Eina_Cxx])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_CXX], [Eet_Cxx])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_CXX], [Ecore_Cxx])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_CXX], [Ecore])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_CXX], [Eina])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_CXX], [Eo])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_CXX], [Efl])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_CXX], [Elementary])

EFL_EVAL_PKGS([ELEMENTARY_CXX])

EFL_LIB_END([Elementary_Cxx])
#### End of Ecore CXX


#### Elementary_Js
EFL_LIB_START_OPTIONAL([Elementary_Js], [test "x${have_js}" = "xyes"])

### Default values

### Additional options to configure

### Checks for programs

### Checks for libraries
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Eina])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Eo])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Ecore])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Eet])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Ecore_Evas])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Ecore_Con])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Ecore_Audio])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Efl])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Evas])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Edje])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Emotion])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Eldbus])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Emile])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Ethumb_Client])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Eio])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Elementary])
EFL_INTERNAL_DEPEND_PKG([ELEMENTARY_JS], [Efl_Js])

AM_COND_IF([HAVE_NODEJS], [], [ EFL_ADD_LIBS([ELEMENTARY_JS], [-lv8]) ])

EFL_EVAL_PKGS([ELEMENTARY_JS])

AM_COND_IF([HAVE_NODEJS], [ELEMENTARYJS_CXXFLAGS="$ELEMENTARYJS_CXXFLAGS -DHAVE_NODEJS"], [])
AM_COND_IF([HAVE_LIBUV], [ELEMENTARYJS_CXXFLAGS="$ELEMENTARYJS_CXXFLAGS -DHAVE_LIBUV"], [])
AM_COND_IF([HAVE_JS], [ELEMENTARYJS_CXXFLAGS="$ELEMENTARYJS_CXXFLAGS -DHAVE_JS"], [])

AM_COND_IF([HAVE_V8_CREATE_PARAMS], [ELEMENTARYJS_CXXFLAGS="$ELEMENTARYJS_CXXFLAGS -DHAVE_V8_CREATE_PARAMS"], [])
AM_COND_IF([HAVE_V8_GLOBAL], [ELEMENTARYJS_CXXFLAGS="$ELEMENTARYJS_CXXFLAGS -DHAVE_V8_GLOBAL"], [])
AC_SUBST([ELEMENTARYJS_CXXFLAGS])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

### Check availability

EFL_LIB_END_OPTIONAL([ELEMENTARY_JS])
#### End of Efl_Js

ELM_EDJE_DEFINES=""
AC_ARG_ENABLE([cancel-ok],
              [AS_HELP_STRING([--enable-cancel-ok],[Enable ordering of cancel and ok buttons to be cancel first, then ok instead of ok then cancel. @<:@default=disabled@:>@])],
              [
                if test "x${enableval}" = "xyes" ; then
                    ELM_EDJE_DEFINES="$ELM_EDJE_DEFINES -DELM_CANCEL_OK=1"
                fi
              ],[])
AC_SUBST(ELM_EDJE_DEFINES)

AC_DEFINE([NEED_RUN_IN_TREE], [1], [Need EFL_RUN_IN_TREE=1 trick])

## Disable warning for OS that have a specifc configuration
case "$host_os" in
   mingw*|cygwin*)
      CFOPT_WARNING=""
      ;;
   darwin*)
      CFOPT_WARNING=""
      ;;
esac

AC_CONFIG_FILES([
Makefile
data/Makefile
data/elementary/config/Makefile
data/elementary/config/default/Makefile
data/elementary/config/default/base.src
data/elementary/config/mobile/Makefile
data/elementary/config/mobile/base.src
data/elementary/config/standard/Makefile
data/elementary/config/standard/base.src
doc/Makefile
doc/Doxyfile
doc/previews/Makefile
doc/widgets/Makefile
src/Makefile
src/bindings/mono/efl_mono/efl_libs.cs
src/bindings/mono/efl_mono/efl_libs.csv
src/benchmarks/eina/Makefile
src/benchmarks/eo/Makefile
src/benchmarks/evas/Makefile
src/examples/Makefile
src/lib/eina/eina_config.h
src/lib/efl/Efl_Config.h
src/lib/elementary/Elementary_Options.h
src/scripts/eo/eo_debug
elm_intro.h
spec/efl.spec
pc/evil.pc
pc/escape.pc
pc/eina.pc
pc/eina-cxx.pc
pc/emile.pc
pc/eet.pc
pc/eet-cxx.pc
pc/eo.pc
pc/eo-cxx.pc
pc/eolian.pc
pc/eolian-cxx.pc
pc/eina-js.pc
pc/efl-js.pc
pc/eolian-js.pc
pc/eo-js.pc
pc/efl.pc
pc/efl-cxx.pc
pc/efl-mono.pc
pc/efl-wl.pc
pc/efl-core.pc
pc/efl-net.pc
pc/efl-ui.pc
pc/evas-fb.pc
pc/evas-eglfs.pc
pc/evas-opengl-x11.pc
pc/evas-opengl-sdl.pc
pc/evas-opengl-cocoa.pc
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
pc/ecore-sdl.pc
pc/ecore-wayland.pc
pc/ecore-wl2.pc
pc/ecore-win32.pc
pc/ecore-x.pc
pc/ecore-evas.pc
pc/ecore-imf.pc
pc/ecore-imf-evas.pc
pc/ecore-audio.pc
pc/ecore-audio-cxx.pc
pc/ecore-avahi.pc
pc/ecore-buffer.pc
pc/ector.pc
pc/embryo.pc
pc/eio.pc
pc/eio-cxx.pc
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
pc/elocation.pc
pc/elua.pc
pc/elementary.pc
pc/elementary-cxx.pc
pc/elput.pc
pc/ecore-drm2.pc
dbus-services/org.enlightenment.Ethumb.service
systemd-services/ethumb.service
$po_makefile_in
cmakeconfig/EflConfig.cmake
cmakeconfig/EflConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EinaConfig.cmake
cmakeconfig/EinaConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EioConfig.cmake
cmakeconfig/EioConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EezeConfig.cmake
cmakeconfig/EezeConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EoConfig.cmake
cmakeconfig/EoConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EolianConfig.cmake
cmakeconfig/EolianConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EolianHelper.cmake
cmakeconfig/EolianCxxConfig.cmake
cmakeconfig/EolianCxxConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EinaCxxConfig.cmake
cmakeconfig/EinaCxxConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EoCxxConfig.cmake
cmakeconfig/EoCxxConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EcoreCxxConfig.cmake
cmakeconfig/EcoreCxxConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EvasCxxConfig.cmake
cmakeconfig/EvasCxxConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EetCxxConfig.cmake
cmakeconfig/EetCxxConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EetConfig.cmake
cmakeconfig/EetConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EvasConfig.cmake
cmakeconfig/EvasConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EcoreConfig.cmake
cmakeconfig/EcoreConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EdjeConfig.cmake
cmakeconfig/EdjeConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EldbusConfig.cmake
cmakeconfig/EldbusConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EfreetConfig.cmake
cmakeconfig/EfreetConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EthumbConfig.cmake
cmakeconfig/EthumbConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EthumbClientConfig.cmake
cmakeconfig/EthumbClientConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EmotionConfig.cmake
cmakeconfig/EmotionConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EluaConfig.cmake
cmakeconfig/EluaConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/EmileConfig.cmake
cmakeconfig/EmileConfigVersion.cmake:cmakeconfig/EflConfigVersion.cmake.in
cmakeconfig/ElementaryConfigVersion.cmake
cmakeconfig/ElementaryConfig.cmake
])

AC_OUTPUT

#### Info

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
  aarch64*)
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

if test "x${efl_have_posix_threads_spinlock}" = "xyes" || test "x${efl_have_osx_spinlock}" = "xyes"; then
   efl_have_spinlock="yes"
else
   efl_have_spinlock="no"
fi
EFL_ADD_FEATURE([thread], [spinlocks], [${efl_have_spinlock}])
EFL_ADD_FEATURE([thread], [barrier], [${efl_have_pthread_barrier}])
EFL_ADD_FEATURE([thread], [affinity], [${efl_have_setaffinity}])
EFL_ADD_FEATURE([thread], [setname], [${efl_have_setname}])
EFL_ADD_FEATURE([thread], [__thread], [${have_thread_specifier}])

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

if test "x${want_egl}" = "xyes" ; then
   opengl_egl="(EGL)"
else
   opengl_egl=""
fi

echo "Configuration...: ${COLOR_OTHER}profile=${build_profile} os=${osname}${COLOR_RESET}"
echo "  EFL API Set...: ${efl_api}"
echo "  CPU Extensions: ${host_cpu} (${features_cpu})"
echo "  System Feature: ${features_system}"
echo "  Threads.......: ${efl_have_threads} (${features_thread})"
echo "  Cryptography..: ${build_crypto}"
echo "  X11...........: ${with_x11}"
echo "  OpenGL........: ${with_opengl} ${opengl_egl}"
echo "  C++11.........: ${have_cxx11}"
echo "  C#............: ${want_csharp}"
echo "  JavaScript....: ${want_js}"
echo "  JavaScript flg: $EINA_JS_LIBS"
echo "Eina............: yes (${features_eina} unwind=$have_unwind)"
echo "Eo..............: yes (${features_eo})"
echo "Eolian..........: yes (${features_eolian})"
echo "Emile...........: yes (${features_emile})"
echo "Eet.............: yes"
echo "Evas............: yes (${features_evas})"
echo "  Engines.......: ${features_evas_engine}"
echo "  Vg Loaders    : ${features_evas_vg_loader}"
echo "  Image Loaders.: ${features_evas_loader}"
if test "x${have_pixman}" = "xyes" ; then
echo "  Pixman........: ${features_evas_pixman}"
fi
echo "Ecore...........: yes (${features_ecore})"
echo "Ecore_Con.......: yes (${features_ecore_con})"
echo "Ecore_File......: yes"
echo "Ecore_IMF.......: yes (${features_ecore_imf})"
echo "Ecore_X.........: ${with_x11} (${features_ecore_x})"
echo "Ecore_SDL.......: $want_sdl"
echo "Ecore_Drm.......: $want_ecore_drm"
echo "Ecore_Drm2......: $want_drm"
echo "Ecore_Wayland...: $want_ecore_wayland"
echo "Ecore_Wl2.......: $want_wayland"
echo "IVI-Shell.......: $want_wayland_ivi_shell"
echo "Ecore_Buffer....: $want_ecore_buffer (${features_ecore_buffer})"
if test "${have_linux}" = "yes"; then
echo "Ecore_FB........: $want_fb (${features_ecore_fb})"
elif test "${have_darwin}" = "yes"; then
echo "Ecore_Cocoa.....: $efl_lib_optional_ecore_cocoa"
elif test "${have_windows}" = "yes"; then
echo "Ecore_Win32.....: $have_win32"
fi
echo "Ecore_Audio.....: ${efl_lib_optional_ecore_audio} (${features_ecore_audio})"
echo "Ecore_Avahi.....: yes (${features_ecore_avahi})"
echo "Ecore_Evas......: yes (${features_ecore_evas})"
echo "Elput...........: $want_elput (${features_elput})"
echo "Ector...........: yes"
echo "Eeze............: ${efl_lib_optional_eeze} (${features_eeze})"
echo "EPhysics........: ${efl_lib_optional_ephysics}"
echo "Edje............: yes (${features_edje})"
echo "Emotion.........: yes (${features_emotion})"
echo "Ethumb..........: yes"
echo "Ethumb_Client...: yes"
echo "Elua............: $have_elua"
echo "Elementary......: yes (${features_elementary})"
echo 

echo

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
echo "  EFLJS_CXXFLAGS: $EFLJS_CXXFLAGS"

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

if test -n "$CFOPT_WARNING"; then
  echo "_____________________________________________________________________"
  echo ""
  echo "==-- WARNING --=="
  echo ""
  echo "_____________________________________________________________________"
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
  if test "x${want_audio}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "You disabled audio support in Ecore. This is not tested and may"
    echo "Create bugs for you due to it creating untested code paths."
    echo "Reconsider disabling audio."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_pulseaudio}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "The only audio output method supported by Ecore right now on your"
    echo "system is via Pulseaudio. You have disabled that and likely have"
    echo "broken a whole bunch of things in the process. Reconsider your"
    echo "configure options."
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
  if test "x${efl_api}" = "xeo"; then
    echo "_____________________________________________________________________"
    echo "Using the EO based EFL API only does not work at the moment. We still"
    echo "have a lot code that depends on the legacy interfaces. We provide"
    echo "this option for testing once we are able to migrate to the EO based"
    echo "API."
    echo "If you are not working on this migration please leave the option set"
    echo "to both as it will break your build if set to eo."
    echo "_____________________________________________________________________"
  fi
  if test "x${want_libeeze}" = "xno"; then
    echo "_____________________________________________________________________"
    echo "Libeeze has been disabled, and it is used heavily for support of"
    echo "removable devices etc. and disabling this will hurt support for"
    echo "Enlightenment and its filemanager."
    echo "_____________________________________________________________________"
  fi
  echo "_____________________________________________________________________"
  echo ""
  echo "==-- WARNING --=="
  echo ""
  echo "_____________________________________________________________________"
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
  elif echo "$resolved_dbusservicedir" | grep -e '^/usr/s' >/dev/null 2>/dev/null; then
    AC_MSG_NOTICE([installing DBus serivces in $resolved_dbusservicedir])
  else
    needs_alert_dbus=1
  fi

  needs_alert_systemd=0
  if test "$have_systemd_user_session" = "yes"; then
    if test "$resolved_USER_SESSION_DIR" = "${HOME}/.config/systemd/user"; then
      AC_MSG_NOTICE([installing systemd services in user local "$resolved_USER_SESSION_DIR". Only accessible to user $USER])
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
      echo ""
      echo "  or add \"${resolved_datadir}\" to \$XDG_DATA_DIRS"
      echo ""
      echo "User installation:"
      echo "  ln -s ${resolved_dbusservicedir}/org.enlightenment.Ethumb.service ~/.local/share/dbus-1/services/org.enlightenment.Ethumb.service"
    fi
    if test $needs_alert_systemd -eq 1; then
      echo ""
      echo "System-wide installation:"
      echo "  ln -s ${resolved_USER_SESSION_DIR}/ethumb.service /usr/lib/systemd/user/ethumb.service"
      echo ""
      echo "  or add \"${base_USER_SESSION_DIR}\" to \$XDG_DATA_DIRS or \$XDG_CONFIG_DIRS"
      echo ""
      echo "User installation:"
      echo "  ln -s ${resolved_USER_SESSION_DIR}/ethumb.service ~/.config/systemd/user/ethumb.service"
    fi
    echo ""
    echo "#-------------------------------------------------------------------#"
  fi

elif test "x${have_darwin}" = "xyes"; then
    echo ""
    echo "#-------------------------------------------------------------------#"
    echo "##==--                          ALERT                          --==##"
    echo "#-------------------------------------------------------------------#"
    echo ""
    echo "  If you are running Mac OS X >= 10.11 (starting from El Capitan)"
    echo "  you will have TROUBLE with the prefix you have chosen (/usr)..."
    echo "  Don't make it double and consider to use the default prefix"
    echo "  (/usr/local). This is because of Apple's SIP that provides MAC"
    echo "  and prevents ANYONE modifying the system."
    echo ""
    echo "#-------------------------------------------------------------------#"
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



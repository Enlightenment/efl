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

requirements_libs_evil=""
requirements_libs_eina=""
requirements_libs_eo=""
requirements_libs_eet=""
#requirements_libs_evas=""
#requirements_libs_ecore=""
requirements_libs_embryo=""
#requirements_libs_eio=""
#requirements_libs_edje=""
#requirements_libs_efreet=""
#requirements_libs_e_dbus=""
#requirements_libs_eeze=""
#requirements_libs_emotion=""
#requirements_libs_ethumb=""
#requirements_libs_elementary=""

requirements_libs_deps_evil=""
requirements_libs_deps_eina=""
requirements_libs_deps_eo=""
requirements_libs_deps_eet=""
#requirements_libs_deps_evas=""
#requirements_libs_deps_ecore=""
requirements_libs_deps_embryo=""
#requirements_libs_deps_eio=""
#requirements_libs_deps_edje=""
#requirements_libs_deps_efreet=""
#requirements_libs_deps_e_dbus=""
#requirements_libs_deps_eeze=""
#requirements_libs_deps_emotion=""
#requirements_libs_deps_ethumb=""
#requirements_libs_deps_elementary=""

requirements_pc_eina=""
requirements_pc_eo=""
requirements_pc_eet=""
#requirements_pc_evas=""
#requirements_pc_ecore=""
requirements_pc_embryo=""
#requirements_pc_eio=""
#requirements_pc_edje=""
#requirements_pc_efreet=""
#requirements_pc_e_dbus=""
#requirements_pc_eeze=""
#requirements_pc_emotion=""
#requirements_pc_ethumb=""
#requirements_pc_elementary=""

requirements_pc_deps_eina=""
requirements_pc_deps_eo=""
requirements_pc_deps_eet=""
#requirements_pc_deps_evas=""
#requirements_pc_deps_ecore=""
requirements_pc_deps_embryo=""
#requirements_pc_deps_eio=""
#requirements_pc_deps_edje=""
#requirements_pc_deps_efreet=""
#requirements_pc_deps_e_dbus=""
#requirements_pc_deps_eeze=""
#requirements_pc_deps_emotion=""
#requirements_pc_deps_ethumb=""
#requirements_pc_deps_elementary=""

AC_SUBST([requirements_libs_evil])
AC_SUBST([requirements_libs_eina])
AC_SUBST([requirements_libs_eo])
AC_SUBST([requirements_libs_eet])
#AC_SUBST([requirements_libs_evas])
#AC_SUBST([requirements_libs_ecore])
AC_SUBST([requirements_libs_embryo])
#AC_SUBST([requirements_libs_eio])
#AC_SUBST([requirements_libs_edje])
#AC_SUBST([requirements_libs_efreet])
#AC_SUBST([requirements_libs_e_dbus])
#AC_SUBST([requirements_libs_eeze])
#AC_SUBST([requirements_libs_emotion])
#AC_SUBST([requirements_libs_ethumb])
#AC_SUBST([requirements_libs_elementary])

AC_SUBST([requirements_pc_eina])
AC_SUBST([requirements_pc_eo])
AC_SUBST([requirements_pc_eet])
#AC_SUBST([requirements_pc_evas])
#AC_SUBST([requirements_pc_ecore])
AC_SUBST([requirements_pc_embryo])
#AC_SUBST([requirements_pc_eio])
#AC_SUBST([requirements_pc_edje])
#AC_SUBST([requirements_pc_efreet])
#AC_SUBST([requirements_pc_e_dbus])
#AC_SUBST([requirements_pc_eeze])
#AC_SUBST([requirements_pc_emotion])
#AC_SUBST([requirements_pc_ethumb])
#AC_SUBST([requirements_pc_elementary])


AC_CANONICAL_HOST

# TODO: move me to m4 file that setups module/so related variables
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

# TODO: move me to m4 file that setups the windows related variables
AM_CONDITIONAL([HAVE_WINCE], [test "x${have_wince}" = "xyes"])
AM_CONDITIONAL([HAVE_WIN32], [test "x${have_win32}" = "xyes"])
AM_CONDITIONAL([HAVE_WINDOWS], [test "x${have_windows}" = "xyes"])


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

# TODO: add some build "profile" (server, full, etc...)


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

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])


#### Checks for libraries


#### Checks for header files

# Common Checks (keep names sorted for ease of use):
AC_HEADER_DIRENT
AC_HEADER_TIME

AC_CHECK_HEADERS([ \
execinfo.h \
inttypes.h \
libgen.h \
mcheck.h \
netinet/in.h \
siginfo.h \
stdint.h \
strings.h \
sys/types.h \
unistd.h \
])

EFL_CHECK_PATH_MAX


#### Checks for types

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


#### Checks for structures


#### Checks for compiler characteristics

AC_C_BIGENDIAN
AC_C_INLINE
EFL_CHECK_COMPILER_FLAGS([EFL], [-Wall -Wextra -Wpointer-arith])
case "${build_profile}" in
   dev)
     EFL_CHECK_COMPILER_FLAGS([EFL], [-Wshadow])
     ;;

   debug)
     ;;

   release)
     ;;
esac


#### Checks for linker characteristics


#### Checks for library functions

AC_CHECK_FUNCS([\
backtrace \
backtrace_symbols \
execvp \
fpathconf \
fstatat \
malloc_usable_size \
mtrace \
openat \
strlcpy \
])

AC_FUNC_ALLOCA

EFL_CHECK_FUNCS([EFL], [fnmatch gettimeofday])



######################  EFL  ######################


#### Evil

if test "x${have_windows}" = "xyes" ; then

AC_MSG_NOTICE([Evil checks])

### Default values

### Additional options to configure
EFL_SELECT_WINDOWS_VERSION

### Checks for programs

### Checks for libraries

# TODO: are these needed here? parts of this are replicated for EVERY LIB :-(
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
EVIL_CFLAGS="-Wall -Wextra -Wshadow -Wdeclaration-after-statement -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls -Wpointer-arith -mms-bitfields"
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

have_magic_debug="yes"
have_safety_checks="yes"
want_log="yes"
case "${build_profile}" in
   dev)
     with_max_log_level=""
     have_stringshare_usage="no"
     want_valgrind="no" # TODO: "yes" is not working: relocation R_X86_64_32S against `vgPlain_interim_stack' can not be used when making a shared object; recompile with -fPIC
     want_debug_malloc="no"
     want_debug_threads="no"
     ;;

   debug)
     with_max_log_level=""
     have_stringshare_usage="yes"
     want_valgrind="no" # TODO: "yes" is not working: relocation R_X86_64_32S against `vgPlain_interim_stack' can not be used when making a shared object; recompile with -fPIC
     want_debug_malloc="yes"
     want_debug_threads="yes"
     ;;

   release)
     with_max_log_level="3"
     have_stringshare_usage="no"
     want_valgrind="no"
     want_debug_malloc="no"
     want_debug_threads="no"
     ;;
esac

EINA_CONFIG([MAGIC_DEBUG], [test "x${have_magic_debug}" = "xyes"])
EINA_CONFIG([SAFETY_CHECKS], [test "x${have_safety_checks}" = "xyes"])

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

## Compatibility layers

# Evil library for compilation on Windows

case "$host_os" in
   mingw*)
      requirements_pc_eina="${requirements_pc_eina} evil >= ${PACKAGE_VERSION}"
   ;;
esac

# Escape library for compilation on Playstation 3

case "$host_vendor" in
  ps3*)
# TODO: add back whenever escape is merged into efl single tree
#      PKG_CHECK_EXISTS([escape])
#      AC_DEFINE([HAVE_ESCAPE], [1], [Set to 1 if Escape package is installed])
#      requirements_pc_eina="${requirements_pc_eina} escape"
    ;;
esac

# Exotic library for compilation on Coyote

# TODO: add back whenever escape is merged into efl single tree
#PKG_CHECK_EXISTS([exotic],
#   [
#    enable_exotic="yes"
#    AC_DEFINE([HAVE_EXOTIC_H], [1], [Define to 1 if you have Exotic.])
#    EINA_CONFIGURE_HAVE_EXOTIC="#define EINA_HAVE_EXOTIC"
#    requirements_pc_eina="exotic ${requirements_pc_eina}"
#   ],
#   [enable_exotic="no"])
#
#AM_CONDITIONAL([EINA_HAVE_EXOTIC], [test "x${enable_exotic}" = "xyes"])
#AC_SUBST([EINA_CONFIGURE_HAVE_EXOTIC])

## Options

# Valgrind

if test "x${want_valgrind}" = "xyes" ; then
   PKG_CHECK_MODULES([VALGRIND], [valgrind >= 2.4.0],
      [
       have_valgrind="yes"
       requirements_pc_eina="valgrind >= 2.4.0 ${requirements_pc_eina}"
       requirements_pc_deps_eina="valgrind ${requirements_pc_deps_eina}"
      ],
      [AC_MSG_ERROR([Valgrind >= 2.4.0 is required])])
else
    AC_DEFINE([NVALGRIND], [1], [Valgrind support disabled])
fi

AC_DEFINE_IF([EINA_DEBUG_MALLOC],
   [test "x${ac_cv_func_malloc_usable_size}" = "xyes" && test "x${want_debug_malloc}" = "xyes"],
   [1], [Turn on debugging overhead in mempool])

if ! test "x${requirements_pc_deps_eina}" = "x" ; then
   PKG_CHECK_MODULES([EINA], [${requirements_pc_deps_eina}])
fi

## Examples

# TODO: add once ecore-evas is merged:
#PKG_CHECK_MODULES([ECORE_EVAS],
#   [ecore-evas ecore evas],
#   [build_tiler_example="yes"],
#   [build_tiler_example="no"])

AM_CONDITIONAL([BUILD_TILER_EXAMPLE], [test "x${build_tiler_example}" = "xyes"])

## Tests

prefer_assert="no"
case "$build_profile" in
   dev|debug)
     prefer_assert="yes"
     ;;
esac

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

# sys/mman.h could be provided by evil/escape/exotic so we need to set CFLAGS accordingly
CFLAGS_save="${CFLAGS}"
CFLAGS="${CFLAGS} ${EINA_CFLAGS}"
AC_CHECK_HEADERS([sys/mman.h])
CFLAGS="${CFLAGS_save}"

EINA_CONFIG([HAVE_INTTYPES_H], [test "x${ac_cv_header_inttypes_h}" = "xyes"])
EINA_CONFIG([HAVE_STDINT_H], [test "x${ac_cv_header_stdint_h}" = "xyes"])

### Checks for types

EINA_CONFIG([HAVE_DIRENT_H], [test "x${have_dirent}" = "xyes"])
AC_DEFINE_IF([HAVE_DIRENT_H], [test "x${have_dirent}" = "xyes"],
   [1], [Define to 1 if you have a valid <dirent.h> header file.])

### Checks for structures

### Checks for compiler characteristics

m4_ifdef([v_ver],
   [
    EFL_CHECK_COMPILER_FLAGS([EINA], [-Wall -Wextra])
   ])

EFL_CHECK_COMPILER_FLAGS([EINA], [-Wshadow -Wpointer-arith])

### Checks for linker characteristics

EFL_CHECK_LINKER_FLAGS([EINA], [-fno-strict-aliasing])

### Checks for library functions

EFL_CHECK_FUNCS([EINA], [dirfd dlopen dladdr iconv shm_open setxattr])

enable_log="no"
if test "x${efl_func_fnmatch}" = "xyes" && test "x${want_log}" = "xyes" ; then
   enable_log="yes"
fi

AC_MSG_CHECKING([wether to build Eina_Log infrastructure])
AC_MSG_RESULT([${enable_log}])

EINA_CONFIG([ENABLE_LOG], [test "x${enable_log}" = "xyes"])

EFL_CHECK_THREADS

EINA_CONFIG([HAVE_PTHREAD_BARRIER], [test "x${efl_have_pthread_barrier}" = "xyes"])
EINA_CONFIG([HAVE_PTHREAD_AFFINITY], [test "x${efl_have_setaffinity}" = "xyes"])
EINA_CONFIG([HAVE_DEBUG_THREADS], [test "$want_debug_threads" = "yes"])

### Modules

EINA_CHECK_MODULE([chained-pool], [static], [chained pool])
EINA_CHECK_MODULE([pass-through], [static], [pass through])
EINA_CHECK_MODULE([one-big],      [static], [one big])


#### End of Eina


#### Eet

AC_MSG_NOTICE([Eet checks])

### Default values

### Additional options to configure

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

### Checks for programs

### Checks for libraries

## Compatibility layers

# Evil library for compilation on Windows

case "$host_os" in
   mingw*)
      requirements_pc_eet="evil >= ${PACKAGE_VERSION} ${requirements_pc_eet}"
   ;;
esac

## Secure layer

case "$build_crypto" in
   gnutls)
      PKG_CHECK_MODULES([GNUTLS], [gnutls >= 1.7.6])
      AC_DEFINE([HAVE_GNUTLS], [1], [Have Gnutls support])
      requirements_pc_eet="gnutls >= 1.7.6 ${requirements_pc_eet}"
      requirements_pc_deps_eet="gnutls >= 1.7.6 ${requirements_pc_deps_eet}"

      # TODO: do we need this?
      # libgcrypt
      AC_PATH_GENERIC([libgcrypt], [], [:],
         [AC_MSG_ERROR([libgcrypt required but not found])])
      requirements_libs_eet="${LIBGCRYPT_LIBS} ${requirements_libs_eet}"
      requirements_libs_deps_eet="${LIBGCRYPT_LIBS} ${requirements_libs_deps_eet}"

      # TODO: do we need this? can't we just bump required version?
      # Specific GNUTLS improvement
      CFLAGS_save="${CFLAGS}"
      LIBS_save="${LIBS}"
      CFLAGS="${GNUTLS_CFLAGS}"
      LIBS="${GNUTLS_LIBS}"
      AC_CHECK_LIB([gnutls], [gnutls_x509_crt_verify_hash],
         [AC_DEFINE([EET_USE_NEW_GNUTLS_API], [1], [use gnutls_x509_crt_verify_hash])],
         [AC_MSG_NOTICE([Optional gnutls_x509_crt_verify_hash not present.])])
      CFLAGS="${CFLAGS_save}"
      LIBS="${LIBS_save}"

      # TODO: do we need this? can't we just bump required version?
      CFLAGS_save="${CFLAGS}"
      LIBS_save="${LIBS}"
      CFLAGS="${GNUTLS_CFLAGS}"
      LIBS="${GNUTLS_LIBS}"
      AC_CHECK_LIB([gnutls], [gnutls_privkey_sign_data],
         [AC_DEFINE([EET_USE_NEW_PRIVKEY_SIGN_DATA], [1], [use gnutls_privkey_sign_data])],
         [AC_MSG_NOTICE([Optional gnutls_privkey_sign_data not present.])])
      CFLAGS="${CFLAGS_save}"
      LIBS="${LIBS_save}"

      # TODO: do we need this? can't we just bump required version?
      CFLAGS_save="${CFLAGS}"
      LIBS_save="${LIBS}"
      CFLAGS="${GNUTLS_CFLAGS}"
      LIBS="${GNUTLS_LIBS}"
      AC_CHECK_LIB([gnutls], [gnutls_pubkey_verify_hash],
         [AC_DEFINE([EET_USE_NEW_PUBKEY_VERIFY_HASH], [1], [use gnutls_pubkey_verify_hash])])
      CFLAGS="${CFLAGS_save}"
      LIBS="${LIBS_save}"
      ;;

   openssl)
      PKG_CHECK_MODULES([OPENSSL], [openssl])
      AC_DEFINE([HAVE_OPENSSL], [1], [Have Openssl support])
      requirements_pc_eet="openssl ${requirements_pc_eet}"
      requirements_pc_deps_eet="openssl ${requirements_pc_deps_eet}"
      ;;
esac

# Cryptography support
if test "$build_crypto" != "none" ; then
   AC_DEFINE([HAVE_CIPHER], [1], [Have cipher support built in eet])
   AC_DEFINE([HAVE_SIGNATURE], [1], [Have signature support for eet file])
fi

# libjpeg and zlib

EFL_CHECK_LIBS([EET], [libjpeg zlib])

# TODO: better way to force those instead of is secondary check
if test "x${efl_lib_zlib}" != "xyes" -o "x${efl_lib_libjpeg}" != "xyes" ; then
   AC_MSG_ERROR([Required EET libraries were not found.])
fi


# Eina library

requirements_pc_eet="eina >= ${PACKAGE_VERSION} ${requirements_pc_eet}"

PKG_CHECK_MODULES([EET], [${requirements_pc_deps_eet}])

# Windows

case "$host_os" in
   mingw32ce*)
      requirements_libs_eet="${requirements_libs_eet} -lws2"
      requirements_libs_deps_eet="${requirements_libs_deps_eet} -lws2"
      ;;
   mingw*)
      requirements_libs_eet="${requirements_libs_eet} -lws2_32"
      requirements_libs_deps_eet="${requirements_libs_deps_eet} -lws2_32"
      ;;
esac

EET_LIBS="${EET_LIBS} ${requirements_libs_deps_eet}"

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

case "${host_os}" in
   openbsd*)
   ;;
   *)
      EFL_CHECK_LINKER_FLAGS([EET], [-Wl,--as-needed])
   ;;
esac

### Checks for library functions

### Check availability

#### End of Eet


#### Eo

AC_MSG_NOTICE([Eo checks])

### Default values

### Additional options to configure

### Checks for programs

### Checks for libraries

# Eina library

requirements_pc_eo="eina >= ${PACKAGE_VERSION}"

AC_ARG_VAR([EO_CFLAGS], [preprocessor flags for Eo])
AC_SUBST([EO_CFLAGS])
AC_ARG_VAR([EO_LIBS], [linker flags for Eo])
AC_SUBST([EO_LIBS])

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

#### End of Eo


#### Embryo

AC_MSG_NOTICE([Embryo checks])

### Default values

### Additional options to configure

### Checks for programs

### Checks for libraries

## Compatibility layers

# Evil library for compilation on Windows

case "$host_os" in
   mingw*)
      requirements_pc_embryo="${requirements_pc_embryo} evil >= ${PACKAGE_VERSION}"
   ;;
esac

# Eina library

requirements_pc_embryo="eina >= ${PACKAGE_VERSION} ${requirements_pc_embryo}"

AC_ARG_VAR([EMBRYO_CFLAGS], [preprocessor flags for Embryo])
AC_SUBST([EMBRYO_CFLAGS])
AC_ARG_VAR([EMBRYO_LIBS], [linker flags for Embryo])
AC_SUBST([EMBRYO_LIBS])

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

#### End of Embryo


AC_CONFIG_FILES([
Makefile
data/Makefile
doc/Makefile
doc/Doxyfile
src/Makefile
src/benchmarks/Makefile
src/benchmarks/eina/Makefile
src/benchmarks/eo/Makefile
src/bin/Makefile
src/bin/evil/Makefile
src/bin/eet/Makefile
src/bin/embryo/Makefile
src/examples/Makefile
src/examples/eina/Makefile
src/examples/eet/Makefile
src/examples/eo/Makefile
src/lib/Makefile
src/lib/evil/Makefile
src/lib/eina/Makefile
src/lib/eina/eina_config.h
src/lib/eet/Makefile
src/lib/eo/Makefile
src/lib/embryo/Makefile
src/modules/Makefile
src/modules/eina/Makefile
src/modules/eina/mp/Makefile
src/modules/eina/mp/chained_pool/Makefile
src/modules/eina/mp/pass_through/Makefile
src/modules/eina/mp/one_big/Makefile
src/scripts/Makefile
src/scripts/eina/Makefile
src/tests/Makefile
src/tests/eina/Makefile
src/tests/eet/Makefile
src/tests/eo/Makefile
spec/efl.spec
pc/evil.pc
pc/eina.pc
pc/eet.pc
pc/eo.pc
pc/embryo.pc
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
echo "  Build Profile........: ${build_profile}"
echo "  Threads Type.........: ${efl_have_threads}"
echo "    spinlocks..........: ${efl_have_posix_threads_spinlock}"
echo "    barrier............: ${efl_have_pthread_barrier}"
echo "    affinity...........: ${efl_have_setaffinity}"
echo "  Cryptographic System.: ${build_crypto}"
echo
echo "Tests..................: make check (Coverage: ${_efl_enable_coverage})"
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

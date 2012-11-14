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
requirements_libs_evas=""
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
requirements_libs_deps_evas=""
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
requirements_pc_evas=""
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
requirements_pc_deps_evas=""
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
AC_SUBST([requirements_libs_evas])
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
AC_SUBST([requirements_pc_evas])
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
   cygwin*)
      MODULE_ARCH="$host_os-$host_cpu-v_maj.v_min.v_mic"
      MODULE_EXT=".dll"
   ;;
   *)
      MODULE_ARCH="$host_os-$host_cpu-v_maj.v_min.v_mic"
      MODULE_EXT=".so"
   ;;
esac

AC_SUBST([MODULE_ARCH])
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
AC_PROG_CPP
AC_PROG_CC
AM_PROG_CC_C_O

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
sys/param.h \
sys/mman.h \
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

EFL_ATTRIBUTE_VECTOR
EFL_ATTRIBUTE_ALWAYS_INLINE

EFL_LIBS=
AC_SUBST([EFL_LIBS])

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
    EVIL_LIBS="-lole32 -lws2_32 -lsecur32 -luuid"
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
EVIL_CFLAGS="-Wall -Wextra -Wshadow -Wdeclaration-after-statement -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls -Wpointer-arith"
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


#### Evas

AC_MSG_NOTICE([Evas checks])

### Additional options to configure

# when used, that option makes configure script fails when
# a requirement is selected, but not met.
use_strict="no"
AC_ARG_ENABLE([strict],
   [AC_HELP_STRING([--enable-strict], [enable strict checking mode. @<:@default==disabled@:>@])],
   [use_strict="yes"],
   [use_strict="no"])

# Simple X11 build/link
AC_ARG_ENABLE([simple-x11],
   [AC_HELP_STRING([--enable-simple-x11],
       [enable simple x11 linking. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_evas_simple_x11="yes"
    else
       want_evas_simple_x11="no"
    fi
   ],
   [want_evas_simple_x11="no"])

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
   [AC_HELP_STRING([--disable-harfbuzz],
       [disable complex text shaping and layouting support. @<:@default=auto@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_harfbuzz="yes"
    else
       want_harfbuzz="no"
    fi
   ],
   [want_harfbuzz="auto"])

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
   [want_pixman="yes"])

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

# SSE3
AC_ARG_ENABLE([cpu-sse3],
   [AC_HELP_STRING([--enable-cpu-sse3],
       [enable SSE3 code. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_sse3="yes"
    else
       want_sse3="no"
    fi
   ],
   [want_sse3="yes"])

# Engines

AC_ARG_ENABLE([software-xcb],
   AC_HELP_STRING([--enable-software-xcb], [enable Software XCB Engine. @<:@default=disabled@:>@]),
   [
    if test "x$enableval" = "xyes" ; then
       want_xcb="yes"
    else
       want_xcb="no"
    fi
   ],
   [want_xcb="no"])

AC_ARG_ENABLE([gl-xcb],
   AC_HELP_STRING([--enable-gl-xcb], [enable OpenGL XCB Engine. @<:@default=disabled@:>@]),
   [
    if test "x$enableval" = "xyes" ; then
       want_gl_xcb="yes"
    else
       want_gl_xcb="no"
    fi
   ],
   [want_gl_xcb="no"])

AC_ARG_ENABLE([static-software-generic],
   [AC_HELP_STRING([--enable-static-software-generic], [Build software generic engine as part of libevas. @<:@default=disabled@:>@])],
   [
    if test "x$enableval" = "xyes" ; then
       have_static_software_generic="yes"
    else
       have_static_software_generic="no"
    fi
   ],
   [have_static_software_generic="no"])

AC_ARG_ENABLE([gl-flavor-gles],
   [AC_HELP_STRING([--enable-gl-flavor-gles], [enable gles flavor of gl instead of standard. @<:@default=disabled@:>@])],
   [
    if test "x$enableval" = "xyes" ; then
       gl_flavor_gles="yes"
    else
       gl_flavor_gles="no"
    fi
   ],
   [gl_flavor_gles="no"])

AC_ARG_ENABLE([sdl-primitive],
   [AC_HELP_STRING([--enable-sdl-primitive], [enable SDL primitives. @<:@default=disabled@:>@])],
   [
    if test "x$enableval" = "xyes" ; then
       sdl_primitive="yes"
    else
       sdl_primitive="no"
    fi
   ],
   [sdl_primitive="no"])

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
   [AC_HELP_STRING([--enable-image-loader-svg], [enable SVG image loader. @<:@default=enabled@:>@])],
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
   [want_evas_image_loader_svg="yes"])

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
   [AC_HELP_STRING([--enable-image-loader-webp], [enable WEBP image loader. @<:@default=enabled@:>@])],
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
   [want_evas_image_loader_webp="yes"])

### Default values

want_evas_cserve2="yes"

want_evas_engine_buffer="yes"
want_evas_engine_software_xlib="no"
want_evas_engine_software_xcb="no"
want_evas_engine_software_gdi="no"
want_evas_engine_software_ddraw="no"
want_evas_engine_gl_xlib="no"
want_evas_engine_gl_xcb="no"
want_evas_engine_gl_sdl="no"
want_evas_engine_gl_cocoa="no"
want_evas_engine_direct3d="no"
want_evas_engine_fb="no"
want_evas_engine_directfb="no"
want_evas_engine_psl1ght="no"
want_evas_engine_wayland_shm="no"
want_evas_engine_wayland_egl="no"

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

case "$host_os" in
   mingw32ce*)
      want_fontconfig="no"
      want_evas_image_loader_gif="no"
      want_evas_image_loader_svg="no"
      want_evas_image_loader_tiff="no"
      want_evas_image_loader_webp="no"
      ;;
   mingw*)
      want_evas_engine_software_gdi="yes"
      want_evas_engine_software_ddraw="yes"
      ;;
   darwin*)
      want_evas_engine_software_xlib="auto"
      want_evas_engine_gl_cocoa="auto"
      ;;
   *)
      want_evas_engine_software_xlib="auto"
      want_evas_engine_gl_xlib="auto"
      want_evas_engine_software_xcb="no"
      want_evas_engine_gl_xcb="no"
      want_evas_engine_fb="auto"
      want_evas_engine_wayland_shm="auto"
      want_evas_engine_wayland_egl="auto"
      ;;
esac

EFL_CHECK_FUNC([DLSYM], [dlsym])
requirements_libs_deps_evas="${requirements_libs_deps_evas} ${DLSYM_LIBS}"

if test "x${efl_func_dlsym}" = "xno" ; then
   want_evas_engine_gl_xlib="no"
   want_evas_engine_gl_xcb="no"
   want_evas_engine_gl_sdl="no"
fi

case "$host_vendor" in
  ps3*)
      want_evas_engine_psl1ght="static"
      ;;
esac

### Checks for programs

### Checks for libraries

## Compatibility layers

# Evil library for compilation on Windows

case "$host_os" in
   mingw*)
      requirements_pc_evas="${requirements_pc_evas} evil >= ${PACKAGE_VERSION}"
   ;;
esac

# Eina, Eo and Eet libraries

requirements_pc_evas="eo >= ${PACKAGE_VERSION} eet >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_evas}"

# Freetype
PKG_CHECK_EXISTS([freetype2 >= 9.3.0],
   [requirements_pc_evas="freetype2 >= 9.3.0 ${requirements_pc_evas}"])

requirements_pc_deps_evas="freetype2 >= 9.3.0 ${requirements_pc_deps_evas}"

## optional dependencies

# FontConfig
have_fontconfig="no"
if test "x${want_fontconfig}" = "xyes"; then
   PKG_CHECK_EXISTS([fontconfig >= 2.5.0],
      [
       have_fontconfig="yes"
       AC_DEFINE(HAVE_FONTCONFIG, 1, [have fontconfig searching capabilities])
       requirements_pc_evas="fontconfig >= 2.5.0 ${requirements_pc_evas}"
       requirements_pc_deps_evas="fontconfig >= 2.5.0 ${requirements_pc_deps_evas}"
      ])
fi

# fribidi support
have_fribidi="no"
if test "x${want_fribidi}" = "xyes"; then
   PKG_CHECK_EXISTS([fribidi >= 0.19.2],
      [
       have_fribidi="yes"
       AC_DEFINE([HAVE_FRIBIDI], [1], [have fribidi support])
       requirements_pc_evas="fribidi >= 0.19.2 ${requirements_pc_evas}"
       requirements_pc_deps_evas="fribidi >= 0.19.2 ${requirements_pc_deps_evas}"
       CPPFLAGS_SAVE="$CPPFLAGS"
       CPPFLAGS="$CPPFLAGS $FRIBIDI_CFLAGS"
       AC_CHECK_SIZEOF([FriBidiChar], [], [#include <fribidi/fribidi.h>])
       CPPFLAGS="$CPPFLAGS_SAVE"
      ])
fi

# harfbuzz support
have_harfbuzz="no"
have_harfbuzz_ft="no"
if test "x${want_harfbuzz}" = "xyes" -o "x${want_harfbuzz}" = "xauto" ; then
   PKG_CHECK_EXISTS([harfbuzz >= 0.9.0],
      [
       have_harfbuzz="yes"
       requirements_pc_evas="harfbuzz >= 0.9.0 ${requirements_pc_evas}"
       requirements_pc_deps_evas="harfbuzz >= 0.9.0 ${requirements_pc_deps_evas}"
      ],
      [
       if test "x$want_harfbuzz" = "xyes" && test "x$use_strict" = "xyes" ; then
          AC_MSG_ERROR([Harfbuzz not found (strict dependencies checking)])
       fi
      ])
fi

if test "x${have_harfbuzz}" = "xno"; then
  HAS_HARFBUZZ=0
fi

# Pixman
have_pixman="no"
if test "x${want_pixman}" = "xyes" || test "x${want_pixman}" = "xauto" ; then
   PKG_CHECK_EXISTS([pixman-1],
      [
       have_pixman="yes"
       AC_DEFINE([HAVE_PIXMAN], [1], [have pixman for rendering])
       requirements_pc_evas="pixman-1 ${requirements_pc_evas}"
       requirements_pc_deps_evas="pixman-1 ${requirements_pc_deps_evas}"
      ],
      [
       if test "x${want_pixman}" = "xyes" -a "x${use_strict}" = "xyes" ; then
          AC_MSG_ERROR([Pixman not found (strict dependencies checking)])
       fi
      ])
fi

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

EVAS_CHECK_ENGINE([buffer], [${want_evas_engine_buffer}], [yes], [Buffer])
EVAS_CHECK_ENGINE([direct3d], [${want_evas_engine_direct3d}], [no], [Direct3D])
EVAS_CHECK_ENGINE([directfb], [${want_evas_engine_directfb}], [no], [DirectFB])
EVAS_CHECK_ENGINE([fb], [${want_evas_engine_fb}], [no], [Framebuffer])
EVAS_CHECK_ENGINE([gl-cocoa], [${want_evas_engine_gl_cocoa}], [no], [OpenGL Cocoa])
EVAS_CHECK_ENGINE([gl-sdl], [${want_evas_engine_gl_sdl}], [no], [OpenGL SDL])
EVAS_CHECK_ENGINE([psl1ght], [${want_evas_engine_psl1ght}], [no], [PSL1GHT])
EVAS_CHECK_ENGINE([software-gdi], [${want_evas_engine_software_gdi}], [no], [Software GDI])
EVAS_CHECK_ENGINE([software-ddraw], [${want_evas_engine_software_ddraw}], [no], [Software DirectDraw])
EVAS_CHECK_ENGINE([wayland-shm], [${want_evas_engine_wayland_shm}], [no], [Wayland Shm])
EVAS_CHECK_ENGINE([wayland-egl], [${want_evas_engine_wayland_egl}], [no], [Wayland Egl])

# Software XCB

have_evas_engine_software_xcb="no"
if test "x${want_xcb}" = "xyes" ; then
   EVAS_CHECK_ENGINE([software-xcb], [${want_evas_engine_software_xcb}], [no], [Software XCB])
else
   AM_CONDITIONAL([BUILD_ENGINE_SOFTWARE_XCB], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_XCB], [false])
fi

AC_MSG_CHECKING([whether to build Software XCB Engine])
AC_MSG_RESULT([${have_evas_engine_software_xcb}])

# No Software XCB ? Then try Software Xlib

have_evas_engine_software_xlib="no"
if test "x${have_evas_engine_software_xcb}" = "xno" ; then
   EVAS_CHECK_ENGINE([software-xlib], [${want_evas_engine_software_xlib}], [no], [Software Xlib])
else
   AM_CONDITIONAL([BUILD_ENGINE_SOFTWARE_XLIB], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_X11], [false])
fi

AC_MSG_CHECKING([whether to build Software Xlib Engine])
AC_MSG_RESULT([${have_evas_engine_software_xlib}])

# If software_x11 is available, define everything needed for X11

have_evas_engine_software_x11="no"
if test "x${have_evas_engine_software_xlib}" = "xyes" || test "x${have_evas_engine_software_xcb}" = "xyes" ; then
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

AC_MSG_CHECKING([Whether to build software generic engine as part of libevas])
AC_MSG_RESULT([${have_static_software_generic}])

if test "x${have_static_module}" = "xyes" && test "x${have_static_software_generic}" = "xno"; then
   AC_MSG_WARN([Forcing --enable-static-software-generic as engines must be compiled statically])
   have_static_software_generic="yes"
fi

if test "x${have_static_software_generic}" = "xyes"; then
   AC_DEFINE([EVAS_STATIC_BUILD_SOFTWARE_GENERIC], [1], [Build software generic engine as part of libevas])
fi

AM_CONDITIONAL([EVAS_STATIC_BUILD_SOFTWARE_GENERIC], [test "x${have_static_software_generic}" = "xyes"])

# OpenGL

AC_MSG_CHECKING([whether to build GLES flavor of GL])
AC_MSG_RESULT([${gl_flavor_gles}])

if test "x${gl_flavor_gles}" = "xyes" ; then
   AC_DEFINE(GL_GLES, 1, [GLSL runtime shader GLES2 support])
fi

# OpenGL XCB

have_evas_engine_gl_xcb="no"
if test "x${want_gl_xcb}" = "xyes" ; then
   EVAS_CHECK_ENGINE([gl-xcb], [${want_evas_engine_gl_xcb}], [no], [OpenGL XCB])
else
   AM_CONDITIONAL([BUILD_ENGINE_GL_XCB], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_XCB], [false])
fi

AC_MSG_CHECKING([whether to build OpenGL XCB Engine])
AC_MSG_RESULT([${have_evas_engine_gl_xcb}])

# No openGL XCB ? Then try OpenGL Xlib

have_evas_engine_gl_xlib="no"
if test "x${have_evas_engine_gl_xcb}" = "xno" ; then
   EVAS_CHECK_ENGINE([gl-xlib], [${want_evas_engine_gl_xlib}], [no], [OpenGL XLib])
else
   AM_CONDITIONAL([BUILD_ENGINE_GL_XLIB], [false])
   AM_CONDITIONAL([EVAS_STATIC_BUILD_GL_XLIB], [false])
fi

AC_MSG_CHECKING([whether to build OpenGL Xlib Engine])
AC_MSG_RESULT([${have_evas_engine_gl_xlib}])

# If opengl_x11 is available, define everything needed for X11

have_evas_engine_gl_x11="no"
if test "x${have_evas_engine_gl_xlib}" = "xyes" || test "x${have_evas_engine_gl_xcb}" = "xyes" ; then
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

if test "x${gl_flavor_gles}" = "xyes"; then
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

# SDL

AC_MSG_CHECKING([whether to use SDL primitive when possible])
AC_MSG_RESULT([${sdl_primitive}])

if test "x${sdl_primitive}" = "xyes" ; then
   AC_DEFINE([ENGINE_SDL_PRIMITIVE], [1], [Use SDL primitive when possible])
fi

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
case "$host_os" in
   mingw*)
      ;;
   *)
      if test "x${have_evas_image_loader_jpeg}" = "xyes" -o "x${have_evas_image_loader_jpeg}" = "xstatic"; then
         AC_DEFINE([EVAS_BUILD_SAVER_JPEG], [1], [Build JPEG saver])
         have_evas_image_saver_jpeg="yes"
      fi
      ;;
esac

AM_CONDITIONAL([BUILD_SAVER_JPEG], [test "x${have_evas_image_saver_jpeg}" = "xyes"])

PKG_CHECK_MODULES([EVAS], [${requirements_pc_deps_evas}])

EVAS_LIBS="${EVAS_LIBS} ${requirements_libs_deps_evas}"

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
      [
       if test "x$want_harfbuzz" = "xyes" && test "x$use_strict" = "xyes" ; then
          AC_MSG_ERROR([Harfbuzz-ft not found (strict dependencies checking)])
       fi
       have_harfbuzz="no"
      ])
  CPPFLAGS="$CPPFLAGS_SAVE"
fi

### Checks for types

CPPFLAGS_SAVE="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS -Isrc/lib/eina"
AC_CHECK_SIZEOF([Eina_Unicode], [], [#include <Eina.h>])
CPPFLAGS="$CPPFLAGS_SAVE"

### Checks for structures

AC_CHECK_TYPES([struct sigaction], [], [],
               [[#include <signal.h>]])

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

AC_CHECK_FUNCS([siglongjmp])

res=no
AC_CHECK_LIB([m], [lround], [res=yes], [res=no])
if test "x${res}" = "xyes" ; then
  AC_DEFINE([HAVE_LROUND], [1], [C99 lround function exists])
fi

if test "x${want_evas_image_loader_generic}" = "xyes" || test "x${want_evas_cserve2}" = "xyes" ; then
   EFL_CHECK_FUNC([EVAS], [shm_open])
fi

# cserve2 only works on Linux so far.

if test "x${efl_func_shm_open}" = "xno" ; then
   want_evas_cserve2="no"
fi

if test "x${want_evas_cserve2}" = "xyes" ; then
   AC_CHECK_HEADERS([sys/epoll.h sys/inotify.h sys/signalfd.h],
      [],
      [want_evas_cserve2="no"])
fi

if test "x${want_evas_cserve2}" = "xyes" ; then
  AC_DEFINE([EVAS_CSERVE2], [1], [Shared cache server.])
fi
AM_CONDITIONAL([EVAS_CSERVE2], [test "x${want_evas_cserve2}" = "xyes"])

### Configuration

## Tile rotation

if test "x${have_tile_rotate}" = "xyes" ; then
   AC_DEFINE(TILE_ROTATE, 1, [Enable tiled rotate algorithm])
fi

## CPU architecture specific assembly

build_cpu_mmx="no"
build_cpu_sse3="no"
build_cpu_altivec="no"
build_cpu_neon="no"

case $host_cpu in
  i*86|x86_64|amd64)
    AC_DEFINE([BUILD_MMX], [1], [Build MMX Code])
    build_cpu_mmx="yes"
    build_cpu_sse3="yes"
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

AC_SUBST([altivec_cflags])

# SSE3

if test "x${want_sse3}" = "xyes" && test "x${build_cpu_sse3}" = "xyes" ; then
   AC_CHECK_HEADER([immintrin.h],
      [AC_DEFINE(BUILD_SSE3, 1, [Build SSE3 Code])],
      [build_cpu_sse3="no"])
else
   build_cpu_sse3="no"
fi

AC_MSG_CHECKING([whether to build sse3 code])
AC_MSG_RESULT([${build_cpu_sse3}])

EVAS_SSE3_CFLAGS=" "
EVAS_ALTIVEC_CFLAGS=" "

if test "x$build_cpu_sse3" = "xyes" ; then
    EVAS_SSE3_CFLAGS="-msse3"
fi
if test "x$build_cpu_altivec" = "xyes" ; then
    EVAS_ALTIVEC_CFLAGS="-maltivec"
fi

AC_SUBST([CFLAGS])
AC_SUBST([EVAS_ALTIVEC_CFLAGS])
AC_SUBST([EVAS_SSE3_CFLAGS])

## dither options

# Small dither mask instead of big one (lower quality)
EVAS_CHECK_DITHER([small-dither-mask], [small dither mask], [conv_small_dither="yes"], [conv_small_dither="no"])
# Alternate Line dither mask instead of big one (lower quality - but fastest)
EVAS_CHECK_DITHER([line-dither-mask], [line dither mask], [conv_line_dither="yes"], [conv_line_dither="no"])
# No dither mask at all for 16bpp
EVAS_CHECK_DITHER([no-dither-mask], [conversion to 16bpp without dither mask], [conv_no_dither="yes"], [conv_no_dither="no"])

#### End of Evas


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
src/examples/Makefile
src/examples/eina/Makefile
src/examples/eet/Makefile
src/examples/eo/Makefile
src/examples/evas/Makefile
src/lib/eina/eina_config.h
src/tests/Makefile
src/tests/eina/Makefile
src/tests/eet/Makefile
src/tests/eo/Makefile
src/tests/evas/Makefile
spec/efl.spec
pc/evil.pc
pc/eina.pc
pc/eet.pc
pc/eo.pc
pc/embryo.pc
pc/evas-direct3d.pc
pc/evas-directfb.pc
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
echo "Evas:"
echo
echo "  Engines:"
echo "    Software Memory Buffer.....: $have_evas_engine_buffer"
echo "    Software X11...............: $have_evas_engine_software_x11 (Xlib: $have_evas_engine_software_xlib) (XCB: $have_evas_engine_software_xcb)"
echo $ECHO_N "    OpenGL X11.................: $have_evas_engine_gl_x11 (Xlib: $have_evas_engine_gl_xlib) (XCB: $have_evas_engine_gl_xcb) $ECHO_C"
if test "x$have_evas_engine_gl_x11" = "xyes"; then
  echo "(GLES: $gl_flavor_gles)"
else
  echo
fi
echo "    Software GDI...............: $have_evas_engine_software_gdi"
echo "    Software DirectDraw........: $have_evas_engine_software_ddraw"
echo "    Direct3d...................: $have_evas_engine_direct3d"
echo "    OpenGL SDL.................: $have_evas_engine_gl_sdl $ECHO_C"
if test "x$have_evas_engine_gl_sdl" = "xyes"; then
  echo "(GLES: $gl_flavor_gles)"
else
  echo
fi
echo "    OpenGL Cocoa...............: $have_evas_engine_gl_cocoa"
echo "    Software Framebuffer.......: $have_evas_engine_fb"
echo "    DirectFB...................: $have_evas_engine_directfb"
echo "    PSL1GHT....................: $have_evas_engine_psl1ght"
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
echo "  CPU Specific Extensions:"
echo "    MMX/SSE.................: $build_cpu_mmx"
echo "    SSE3....................: $build_cpu_sse3"
echo "    ALTIVEC.................: $build_cpu_altivec"
echo "    NEON....................: $build_cpu_neon"
echo
echo "  Features:"
echo "    Cache Server 2..........: $want_evas_cserve2"
echo "    Threaded Pipe Rendering.: $build_pipe_render"
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
echo "    Smaller Dither Mask.....: $conv_small_dither"
echo "    Line Dither Mask........: $conv_line_dither"
echo "    No Dither Mask for 16bpp: $conv_no_dither"
echo "    Tiled 32BPP rotate......: $have_tile_rotate"
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

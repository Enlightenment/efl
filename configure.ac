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

AC_USE_SYSTEM_EXTENSIONS

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

m4_ifdef([v_rev], , [m4_define([v_rev], [0])])
AC_DEFINE_UNQUOTED([VMAJ], [v_maj], [Major version])
AC_DEFINE_UNQUOTED([VMIN], [v_min], [Minor version])
AC_DEFINE_UNQUOTED([VMIC], [v_mic], [Micro version])
AC_DEFINE_UNQUOTED([VREV], [v_rev], [Revison])
VMAJ=v_maj
AC_SUBST([VMAJ])


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

AC_ARG_ENABLE([coverage],
   [AC_HELP_STRING([--enable-coverage], [enable unit testing coverage. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_coverage="yes"
    else
       want_coverage="no"
    fi
   ],
   [want_coverage="no"])


#### Default values

requirements_libs_evil=""
requirements_libs_eina=""
requirements_libs_eo=""
requirements_libs_eet=""
requirements_libs_evas=""
requirements_libs_ecore=""
requirements_libs_ecore_con=""
requirements_libs_ecore_ipc=""
requirements_libs_ecore_file=""
requirements_libs_ecore_input=""
requirements_libs_ecore_cocoa=""
requirements_libs_ecore_fb=""
requirements_libs_ecore_psl1ght=""
requirements_libs_ecore_sdl=""
requirements_libs_ecore_wayland=""
requirements_libs_ecore_win32=""
requirements_libs_ecore_wince=""
requirements_libs_ecore_x=""
requirements_libs_ecore_evas=""
requirements_libs_embryo=""
requirements_libs_eio=""
#requirements_libs_edje=""
#requirements_libs_efreet=""
#requirements_libs_e_dbus=""
#requirements_libs_eeze=""
#requirements_libs_emotion=""
#requirements_libs_ethumb=""
#requirements_libs_elementary=""

requirements_pc_eina=""
requirements_pc_eo=""
requirements_pc_eet=""
requirements_pc_evas=""
requirements_pc_ecore=""
requirements_pc_ecore_con=""
requirements_pc_ecore_ipc=""
requirements_pc_ecore_file=""
requirements_pc_ecore_input=""
requirements_pc_ecore_input_evas=""
requirements_pc_ecore_cocoa=""
requirements_pc_ecore_fb=""
requirements_pc_ecore_psl1ght=""
requirements_pc_ecore_sdl=""
requirements_pc_ecore_wayland=""
requirements_pc_ecore_win32=""
requirements_pc_ecore_wince=""
requirements_pc_ecore_x=""
requirements_pc_ecore_evas=""
requirements_pc_embryo=""
requirements_pc_eio=""
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
requirements_pc_deps_ecore=""
requirements_pc_deps_ecore_con=""
requirements_pc_deps_ecore_fb=""
requirements_pc_deps_ecore_sdl=""
requirements_pc_deps_ecore_wayland=""
requirements_pc_deps_ecore_evas=""
requirements_pc_deps_embryo=""
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
AC_SUBST([requirements_libs_ecore])
AC_SUBST([requirements_libs_ecore_con])
AC_SUBST([requirements_libs_ecore_ipc])
AC_SUBST([requirements_libs_ecore_file])
AC_SUBST([requirements_libs_ecore_input])
AC_SUBST([requirements_libs_ecore_cocoa])
AC_SUBST([requirements_libs_ecore_fb])
AC_SUBST([requirements_libs_ecore_psl1ght])
AC_SUBST([requirements_libs_ecore_sdl])
AC_SUBST([requirements_libs_ecore_wayland])
AC_SUBST([requirements_libs_ecore_win32])
AC_SUBST([requirements_libs_ecore_wince])
AC_SUBST([requirements_libs_ecore_x])
AC_SUBST([requirements_libs_ecore_evas])
AC_SUBST([requirements_libs_embryo])
AC_SUBST([requirements_libs_eio])
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
AC_SUBST([requirements_pc_ecore])
AC_SUBST([requirements_pc_ecore_con])
AC_SUBST([requirements_pc_ecore_ipc])
AC_SUBST([requirements_pc_ecore_file])
AC_SUBST([requirements_pc_ecore_input])
AC_SUBST([requirements_pc_ecore_input_evas])
AC_SUBST([requirements_pc_ecore_cocoa])
AC_SUBST([requirements_pc_ecore_fb])
AC_SUBST([requirements_pc_ecore_psl1ght])
AC_SUBST([requirements_pc_ecore_sdl])
AC_SUBST([requirements_pc_ecore_wayland])
AC_SUBST([requirements_pc_ecore_win32])
AC_SUBST([requirements_pc_ecore_wince])
AC_SUBST([requirements_pc_ecore_x])
AC_SUBST([requirements_pc_ecore_evas])
AC_SUBST([requirements_pc_embryo])
AC_SUBST([requirements_pc_eio])
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
have_ps3="no"
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
LT_INIT([win32-dll])
version_info="lt_cur:lt_rev:lt_age"
AC_SUBST([version_info])
release_info="v_rel"
AC_SUBST([release_info])

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

_efl_enable_coverage="no"
if test "x${want_coverage}" = "xyes" ; then
   AC_CHECK_PROG([have_lcov], [lcov], [yes], [no])
   if test "x${have_lcov}" = "xyes" ; then
      EFL_COV_CFLAGS="${EFL_COV_CFLAGS} -fprofile-arcs -ftest-coverage"
      EFL_COV_LIBS="${EFL_COV_LIBS} -lgcov"
      if test "x${prefer_assert}" = "xno"; then
         EFL_COV_CFLAGS="${EFL_COV_CFLAGS} -DNDEBUG"
      else
         EFL_COV_CFLAGS="${EFL_COV_CFLAGS} -g -O0 -DDEBUG"
      fi
      _efl_enable_coverage="yes"
   else
      AC_MSG_WARN([lcov is not found, disable profiling instrumentation])
   fi
fi

AM_CONDITIONAL([EFL_ENABLE_COVERAGE], [test "x${_efl_enable_coverage}" = "xyes"])

AC_SUBST([EFL_COV_CFLAGS])
AC_SUBST([EFL_COV_LIBS])


#### Checks for libraries

# check unit testing library

PKG_CHECK_MODULES([CHECK],
   [check >= 0.9.5],
   [_efl_enable_tests="yes"],
   [_efl_enable_tests="no"])

AM_CONDITIONAL([EFL_ENABLE_TESTS], [test "x${_efl_enable_tests}" = "xyes"])


#### Checks for header files

# Common Checks (keep names sorted for ease of use):
AC_HEADER_DIRENT
AC_HEADER_TIME

AC_CHECK_HEADERS([ \
execinfo.h \
errno.h \
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

AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)

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

AC_SUBST([CFLAGS])
AC_SUBST([ALTIVEC_CFLAGS])
AC_SUBST([SSE3_CFLAGS])

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


#### Escape

if test "x${have_ps3}" = "xyes" ; then

AC_MSG_NOTICE([Escape checks])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries

ESCAPE_LIBS="-llv2"
requirements_libs_escape="${ESCAPE_LIBS}"

AC_SUBST([ESCAPE_LIBS])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

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
      AC_DEFINE([HAVE_ESCAPE], [1], [Set to 1 if Escape package is installed])
      requirements_pc_eina="${requirements_pc_eina} escape >= ${PACKAGE_VERSION}"
    ;;
esac

# Exotic library for compilation on Coyote

# TODO: add back whenever exotic is merged into efl single tree
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
      PKG_CHECK_EXISTS([openssl],
         [
          AC_DEFINE([HAVE_OPENSSL], [1], [Have Openssl support])
          requirements_pc_eet="openssl ${requirements_pc_eet}"
          requirements_pc_deps_eet="openssl ${requirements_pc_deps_eet}"
         ])
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
if test "x${efl_lib_zlib}" != "xyes" || test "x${efl_lib_libjpeg}" != "xyes" ; then
   AC_MSG_ERROR([Required EET libraries were not found.])
fi


# Eina library

requirements_pc_eet="eina >= ${PACKAGE_VERSION} ${requirements_pc_eet}"

PKG_CHECK_MODULES([EET], [${requirements_pc_deps_eet}])

# Windows

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
   [AC_HELP_STRING([--enable-strict], [enable strict checking mode. @<:@default=disabled@:>@])],
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
want_evas_engine_fb="no"
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
requirements_libs_evas="${requirements_libs_evas} ${DLSYM_LIBS}"

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
if test "x${want_harfbuzz}" = "xyes" || test "x${want_harfbuzz}" = "xauto" ; then
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
      if test "x${have_evas_image_loader_jpeg}" = "xyes" || test "x${have_evas_image_loader_jpeg}" = "xstatic"; then
         AC_DEFINE([EVAS_BUILD_SAVER_JPEG], [1], [Build JPEG saver])
         have_evas_image_saver_jpeg="yes"
      fi
      ;;
esac

AM_CONDITIONAL([BUILD_SAVER_JPEG], [test "x${have_evas_image_saver_jpeg}" = "xyes"])

PKG_CHECK_MODULES([EVAS], [${requirements_pc_deps_evas}])

EVAS_LIBS="${EVAS_LIBS} ${requirements_libs_evas}"

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


#### Ecore

AC_MSG_NOTICE([Ecore checks])

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

# thread safety

AC_ARG_ENABLE([thread-safety],
   [AC_HELP_STRING([--enable-thread-safety],
       [enable thread safety. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_thread_safety="yes"
    else
       want_thread_safety="no"
    fi
   ],
   [want_thread_safety="no"])

# epoll

AC_ARG_ENABLE([epoll],
   [AC_HELP_STRING([--disable-epoll],
       [disable epoll support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_epoll="yes"
    else
       want_epoll="no"
    fi
   ],
   [want_epoll="yes"])

### Default values

case "$host_os" in
   mingw32*)
      with_glib="no"
      ;;
esac

want_glib="no"
if test "x${with_glib}" = "xyes" || test "x${with_glib}" = "xalways" ; then
   want_glib="yes"
fi

if test "x${want_thread_safety}" = "xyes" ; then
   AC_DEFINE([HAVE_THREAD_SAFETY], [1], [Define to enable thread safety])
fi

want_ecore_timer_dump="no"
if test "x${build_profile}" = "xdebug" && test "x${ac_cv_func_backtrace}" = "xyes"; then
   want_ecore_timer_dump="yes"
   AC_DEFINE([WANT_ECORE_TIMER_DUMP], [1], [Want Ecore_Timer dump infrastructure])
fi

### Checks for programs

### Checks for libraries

## Compatibility layers

# Evil library for compilation on Windows

if test "x${have_windows}" = "xyes" ; then
   requirements_pc_ecore="${requirements_pc_ecore} evil >= ${PACKAGE_VERSION}"
fi

# Escape library for compilation on PS3

if test "x${have_ps3}" = "xyes" ; then
   requirements_pc_ecore="${requirements_pc_ecore} escape >= ${PACKAGE_VERSION}"
fi

# Eina, Eo libraries

requirements_pc_ecore="eo >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_ecore}"

# glib

have_glib="no"
if test "x${want_glib}" = "xyes" ; then
   PKG_CHECK_EXISTS([glib-2.0], [have_glib="yes"], [have_glib="no"])
fi

if test "x${want_glib}" = "xyes" && test "x${have_glib}" = "xno"; then
   AC_MSG_ERROR([GLib support requested, but no GLib found by pkg-config.])
elif test "x${have_glib}" = "xyes" ; then
   AC_DEFINE([HAVE_GLIB], [1], [Have GLib])
   requirements_pc_ecore="glib-2.0 ${requirements_pc_ecore}"
   requirements_pc_deps_ecore="glib-2.0 ${requirements_pc_deps_ecore}"
fi

if test "x${have_glib}" = "xno"; then
   want_g_main_loop="no"
fi

if test "x${with_glib}" = "xalways" && test "x${want_g_main_loop}" = "xyes"; then
  AC_MSG_ERROR([--with-glib set to always and --enable-glib-main-loop are mutually exclusive])
fi

want_glib_integration_always=no
if test "x${with_glib}" = "xalways" ; then
  want_glib_integration_always="yes"
  AC_DEFINE([GLIB_INTEGRATION_ALWAYS], [1], [Always integrate glib if support compiled])
fi

if test "x${want_g_main_loop}" = "xyes" ; then
  AC_DEFINE([USE_G_MAIN_LOOP], [1], [Use g_main_loop in ecore])
fi

if ! test "x${requirements_pc_deps_ecore}" = "x" ; then
   PKG_CHECK_MODULES([ECORE], [${requirements_pc_deps_ecore}])
fi

case "$host_os" in
   mingw32ce*)
      requirements_libs_ecore="${requirements_libs_ecore} -lws2"
      ;;
   mingw*)
      requirements_libs_ecore="${requirements_libs_ecore} -lws2_32"
      ;;
esac

### Checks for header files

AC_HEADER_SYS_WAIT

AC_CHECK_HEADERS([ \
features.h \
langinfo.h \
locale.h \
signal.h \
sys/time.h \
sys/prctl.h \
sys/resource.h \
sys/select.h \
sys/timerfd.h \
])

if test "x${want_epoll}" = "xyes" ; then
   AC_CHECK_HEADERS([sys/epoll.h])
fi

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
        requirements_libs_ecore="${requirements_libs_ecore} -lrt"
        AC_DEFINE([HAVE_CLOCK_GETTIME], [1], [Have clock_gettime()])
       ])
fi

ECORE_LIBS="${ECORE_LIBS} ${LTLIBINTL} ${requirements_libs_ecore}"

### Check availability

#### End of Ecore


#### Ecore_Con

AC_MSG_NOTICE([Ecore_Con checks])

### Additional options to configure

AC_ARG_ENABLE([ipv6],
   [AC_HELP_STRING([--disable-ipv6],
       [disable ipv6 functionality @<:@default=detect@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_ipv6="yes"
    else
       want_ipv6="no"
    fi],
   [want_ipv6="auto"])

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

AC_ARG_WITH([tls],
   [AC_HELP_STRING([--with-tls=TLS],
                   [select Transport Layer Security, one of: gnutls, openssl, none.
                    @<:@default=gnutls@:>@])],
   [with_tls=${withval}],
   [with_tls="gnutls"])

case "${with_tls}" in
   gnutls|openssl|none)
     ;;
   *)
     AC_MSG_ERROR([Unknown Transport Layer Security --with-tls=${with_tls}])
     ;;
esac

AC_ARG_ENABLE([local-sockets],
   [AC_HELP_STRING([--disable-local-sockets], [disable local sockets.])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_ecore_con_local_sockets="yes"
    else
       want_ecore_con_local_sockets="no"
    fi
   ],
   [want_ecore_con_local_sockets="yes"])

AC_ARG_ENABLE([abstract-sockets],
   [AC_HELP_STRING([--disable-abstract-sockets], [disable abstract sockets.])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_ecore_con_abstract_sockets="yes"
    else
       want_ecore_con_abstract_sockets="no"
    fi
   ],
   [want_ecore_con_abstract_sockets="yes"])

### Default values


case "$host_os" in
   mingw32ce*)
      with_tls="none"
      want_curl="no"
      want_cares="no"
      want_ecore_con_local_sockets="no"
      want_ecore_con_abstract_sockets="no"
      ;;
   mingw*)
      want_cares="yes"
      want_ecore_con_abstract_sockets="no"
      ;;
   darwin*)
      want_ecore_con_abstract_sockets="no"
      ;;
   *)
      want_cares="no"
      ;;
esac

case "$host_vendor" in
  ps3*)
      want_ecore_con_local_sockets="no"
      ;;
esac

if test "x${want_ecore_con_local_sockets}" = "xyes" ; then
   AC_DEFINE([HAVE_LOCAL_SOCKETS], [1], [Have local sockets support])
fi

if test "x${want_ecore_con_abstract_sockets}" = "xyes" ; then
   AC_DEFINE([HAVE_ABSTRACT_SOCKETS], [1], [Have abstract sockets namespace])
fi

### Checks for programs

have_gnutls="no"
if test "x${with_tls}" = "xgnutls" ; then
   AC_PATH_GENERIC([libgcrypt], [], [have_gnutls="yes"], [have_gnutls="no"])
fi

### Checks for libraries

# Evil library for compilation on Windows
case "$host_os" in
   mingw*)
      requirements_pc_ecore_con="evil >= ${PACKAGE_VERSION} ${requirements_pc_ecore_con}"
      ;;
esac

# Eina, Eo, Eet, Ecore

requirements_pc_ecore_con="ecore >= ${PACKAGE_VERSION} eet >= ${PACKAGE_VERSION} eo >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_ecore_con}"

# gnuTLS

if test "x${have_gnutls}" = "xyes" ; then
   PKG_CHECK_EXISTS([gnutls >= 2.10.2],
      [
       have_gnutls="yes"
       AC_DEFINE([USE_GNUTLS], [1], [Use GnuTLS])
       requirements_pc_ecore_con="gnutls >= 2.10.2 ${requirements_pc_ecore_con}"
       requirements_pc_deps_ecore_con="gnutls >= 2.10.2 ${requirements_pc_deps_ecore_con}"
      ],
      [have_gnutls="no"])
fi

# openssl

have_openssl="no"
if test "x${with_tls}" = "xopenssl" ; then
   PKG_CHECK_EXISTS([openssl],
      [
       have_openssl="yes"
       AC_DEFINE([USE_OPENSSL], [1], [Use OpenSSL])
       requirements_pc_ecore_con="openssl ${requirements_pc_ecore_con}"
       requirements_pc_deps_ecore_con="openssl ${requirements_pc_deps_ecore_con}"
      ],
      [have_openssl="no"])
fi

# cURL

have_curl="no"
if test "x${want_curl}" = "xyes" ; then
   PKG_CHECK_EXISTS([libcurl],
      [
       have_curl="yes"
       AC_DEFINE([HAVE_CURL], [1], [Downloading with CURL])
       requirements_pc_ecore_con="libcurl ${requirements_pc_ecore_con}"
       requirements_pc_deps_ecore_con="libcurl ${requirements_pc_deps_ecore_con}"
      ],
      [have_curl="no"])
fi

# c-ares

have_cares="no"
if test "x${want_cares}" = "xyes" ; then
   PKG_CHECK_EXISTS([libcares >= 1.6.1],
     [
      have_cares="yes"
      AC_DEFINE([HAVE_CARES], [1], [Build Ecore_Con_Info with c-ares support])
       requirements_pc_ecore_con="libcares >= 1.6.1 ${requirements_pc_ecore_con}"
       requirements_pc_deps_ecore_con="libcares >= 1.6.1 ${requirements_pc_deps_ecore_con}"
     ],
     [have_cares="no"])
fi

AM_CONDITIONAL([HAVE_CARES], [test "x${have_cares}" = "xyes"])

if test "x${requirements_pc_deps_ecore_con}" != "x" ; then
   PKG_CHECK_MODULES([ECORE_CON], [${requirements_pc_deps_ecore_con}])
fi

case "$host_os" in
   mingw32ce*)
      requirements_libs_ecore_con="${requirements_libs_ecore_con} -lws2"
      ;;
   mingw*)
      requirements_libs_ecore_con="${requirements_libs_ecore_con} -lws2_32"
      ;;
esac

ECORE_CON_CFLAGS="${ECORE_CON_CFLAGS} ${LIBGCRYPT_CFLAGS}"
ECORE_CON_LIBS="${ECORE_CON_LIBS} ${LIBGCRYPT_LIBS}"
requirements_libs_ecore_con="${requirements_libs_ecore_con} ${LIBGCRYPT_LIBS}"

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
if test "x${want_ipv6}" != "xno" ; then
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
fi

if test "x${have_ipv6}" = "xyes" ; then
   AC_DEFINE([HAVE_IPV6], 1, [Define if IPV6 is supported])
fi

AM_CONDITIONAL([HAVE_IPV6], [test "x${have_ipv6}" = "xyes"])

### Checks for structures

### Checks for compiler characteristics

EFL_CHECK_COMPILER_FLAGS([ECORE_CON], [-Wno-override-init -Wno-initializer-overrides])

### Checks for linker characteristics

### Checks for library functions

#### End of Ecore_Con


#### Ecore_Ipc

AC_MSG_NOTICE([Ecore_Ipc checks])

### Default values

### Additional options to configure

### Checks for programs

### Checks for libraries

# Eina, Ecore, Ecore_Con

requirements_pc_ecore_ipc="ecore-con >= ${PACKAGE_VERSION} ecore >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_ecore_ipc}"

### Checks for header files

AC_CHECK_HEADERS([winsock2.h])

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

#### End of Ecore_Ipc


#### Ecore_File

AC_MSG_NOTICE([Ecore_File checks])

### Additional options to configure

AC_ARG_ENABLE([poll],
   [AC_HELP_STRING([--disable-poll],
      [disable poll in the ecore_file module. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_poll="yes"
    else
       want_poll="no"
    fi
   ],
   [want_poll="yes"])

AC_ARG_ENABLE([inotify],
   [AC_HELP_STRING([--disable-inotify],
      [disable inotify in the ecore_file module. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_inotify="yes"
    else
       want_inotify="no"
    fi
   ],
   [want_inotify="yes"])

AC_ARG_ENABLE([atfile-source],
   [AC_HELP_STRING([--disable-atfile-source],
       [disable use of atfile source functions as openat and mkdirat @<:@default=detect@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_atfile_source="yes"
    else
       have_atfile_source="no"
    fi],
   [have_atfile_source=auto])

### Default values

have_poll="no"
if test "x${want_poll}" = "xyes" ; then
   AC_DEFINE([HAVE_POLL], [1], [File monitoring with polling])
   have_poll="yes"
fi

have_notify_win32="no"

case "$host_os" in
  mingw32ce*)
    ;;
  mingw32*)
     have_notify_win32="yes"
     AC_DEFINE([HAVE_NOTIFY_WIN32], [1], [File monitoring with Windows notification])
    ;;
esac

if ! test "x${have_atfile_source}" = "xno" ; then
   AC_CHECK_FUNCS([mkdirat],
      [
       have_atfile_source="yes"
       AC_DEFINE(HAVE_ATFILE_SOURCE, 1, [mkdirat exists])
      ],
      [
       if test "x${have_atfile_source}" = "xyes" ; then
          AC_MSG_ERROR([required atfile-source but no mkdirat()])
       fi
       have_atfile_source="no"
      ])
fi

### Checks for programs

### Checks for libraries

# Escape library for compilation on PS3

if test "x${have_ps3}" = "xyes" ; then
   requirements_pc_ecore_file="${requirements_pc_ecore_file} escape >= ${PACKAGE_VERSION}"
fi

# Eina, Ecore, Ecore_Con

requirements_pc_ecore_file="ecore-con >= ${PACKAGE_VERSION} ecore >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_ecore_file}"

### Checks for header files

have_inotify="no"
if test "x${want_inotify}" = "xyes" ; then
   AC_CHECK_HEADER([sys/inotify.h],
      [
       AC_DEFINE([HAVE_INOTIFY], [1], [File monitoring with Inotify])
       have_inotify="yes"
      ],
      [have_inotify="no"])
fi

AC_MSG_CHECKING([whether inotify is to be used for filemonitoring])
AC_MSG_RESULT(${have_inotify})

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

#### End of Ecore_File


#### Ecore_Input

AC_MSG_NOTICE([Ecore_Input checks])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries

## Eina, Ecore (ecore_input)

requirements_pc_ecore_input="ecore >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_ecore_input}"

## Eina, Evas, Ecore, Ecore_Input (ecore_input_evas)

requirements_pc_ecore_input_evas="ecore-input >= ${PACKAGE_VERSION} ecore >= ${PACKAGE_VERSION} evas >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_ecore_input_evas}"

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

#### End of Ecore_Input


#### Ecore_Cocoa

AC_MSG_NOTICE([Ecore_Cocoa checks])

### Additional options to configure

### Default values

have_ecore_cocoa="no"
case "$host_os" in
   darwin*)
      have_ecore_cocoa="yes"
      ;;
esac

### Checks for programs

### Checks for libraries

## Eina, Ecore, Ecore_Input

requirements_pc_ecore_cocoa="ecore-input >= ${PACKAGE_VERSION} ecore >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_ecore_cocoa}"

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
              ECORE_COCOA_LIBS="-framework Cocoa"
              requirements_libs_ecore_cocoa="-framework Cocoa"
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

#### End of Ecore_Cocoa




#### Ecore_FB

AC_MSG_NOTICE([Ecore_FB checks])

### Additional options to configure

### Default values

want_tslib="no"
case "$host_os" in
   mingw*|darwin*)
      ;;
   *)
      want_tslib="yes"
      ;;
esac

### Checks for programs

### Checks for libraries

## Eina, Ecore

requirements_pc_ecore_fb="ecore >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_ecore_fb}"

## tslib

PKG_CHECK_EXISTS([tslib-1.0],
   [
    have_tslib="yes"
    AC_DEFINE([HAVE_TSLIB], [1], [Build Ecore_FB Touchscreen Code])
    requirements_pc_ecore_fb="tslib-1.0 ${requirements_pc_ecore_fb}"
    requirements_pc_deps_ecore_fb="tslib-1.0 ${requirements_pc_deps_ecore_fb}"
   ],
   [have_tslib="no"])

if test "x${have_tslib}" = "xno" ; then
   PKG_CHECK_EXISTS([tslib],
      [
       have_tslib="yes"
       AC_DEFINE([HAVE_TSLIB], [1], [Build Ecore_FB Touchscreen Code])
       requirements_pc_ecore_fb="tslib ${requirements_pc_ecore_fb}"
       requirements_pc_deps_ecore_fb="tslib-1.0 ${requirements_pc_deps_ecore_fb}"
      ],
      [have_tslib="no"])
fi

if test "x${have_tslib}" = "xno" ; then
   AC_CHECK_HEADER([tslib.h], [have_tslib="yes"], [have_tslib="no"])

   if test "x${have_tslib}" = "xyes" ; then
      AC_CHECK_LIB([ts], [ts_open],
         [
          have_tslib="yes"
          AC_DEFINE([HAVE_TSLIB], [1], [Build Ecore_FB Touchscreen Code])
          requirements_libs_ecore_fb="-lts ${requirements_libs_ecore_fb}"
         ],
         [have_tslib="no"])
   fi

   if test "x${have_tslib}" = "xno" ; then
      AC_CHECK_LIB([tslib], [ts_open],
         [
          have_tslib="yes"
          AC_DEFINE([HAVE_TSLIB], [1], [Build Ecore_FB Touchscreen Code])
          requirements_libs_ecore_fb="-ltslib ${requirements_libs_ecore_fb}"
         ],
         [have_tslib="no"])
   fi
fi

if test "x${requirements_pc_deps_ecore_fb}" = "x" ; then
   ECORE_FB_LIBS=${requirements_libs_ecore_fb}
   AC_SUBST([ECORE_FB_LIBS])
else
   PKG_CHECK_MODULES([ECORE_FB], [${requirements_pc_deps_ecore_fb}])
fi

### Checks for header files

have_ecore_fb="no"
AC_CHECK_HEADER([linux/fb.h],
   [AC_CHECK_HEADER([linux/input.h], [have_ecore_fb="yes"], [have_ecore_fb="no"])])

AM_CONDITIONAL([HAVE_ECORE_FB], [test "x${have_ecore_fb}" = "xyes"])

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

#### End of Ecore_FB


#### Ecore_Psl1ght

if test "x${have_ps3}" = "xyes" ; then

AC_MSG_NOTICE([Ecore_Psl1ght checks])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries

## Escape, Eina, Ecore, Ecore_Input

requirements_pc_ecore_psl1ght="ecore-input >= ${PACKAGE_VERSION} ecore >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} escape >= ${PACKAGE_VERSION} ${requirements_pc_ecore_psl1ght}"

requirements_libs_ecore_psl1ght="-lio -lsysutil -lgem -lcamera -lspurs"

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

fi

#### End of Ecore_Psl1ght


#### Ecore_SDL

AC_MSG_NOTICE([Ecore_SDL checks])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries

# Escape library for compilation on PS3

if test "x${have_ps3}" = "xyes" ; then
   requirements_pc_ecore_sdl="${requirements_pc_ecore_sdl} escape >= ${PACKAGE_VERSION}"
fi

## Eina, Ecore

requirements_pc_ecore_sdl="ecore-input >= ${PACKAGE_VERSION} ecore >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_ecore_sdl}"

## SDL

PKG_CHECK_MODULES([ECORE_SDL],
   [sdl >= 1.2.0],
   [
    have_ecore_sdl="yes"
    requirements_pc_ecore_sdl="sdl >= 1.2.0 ${requirements_pc_ecore_sdl}"
   ],
   [have_ecore_sdl="no"])

if test "x${have_ecore_sdl}" = "xno" ; then
   AC_PATH_GENERIC([sdl], [],
      [
       have_ecore_sdl="yes"
       requirements_libs_ecore_sdl="${SDL_LIBS} ${requirements_libs_ecore_sdl}"
       ECORE_SDL_CFLAGS=${SDL_CFLAGS}
       ECORE_SDL_LIBS=${SDL_LIBS}
      ],
      [have_ecore_sdl="no"])
fi

if test "x${have_ecore_sdl}" = "xyes" ; then
   PKG_CHECK_EXISTS([sdl >= 1.3.0],
      [AC_DEFINE(BUILD_ECORE_EVAS_SDL_130, 1, [Support for SVN SDL])])
fi

AM_CONDITIONAL([HAVE_ECORE_SDL], [test "x${have_ecore_sdl}" = "xyes"])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

#### End of Ecore_SDL


#### Ecore_Wayland

AC_MSG_NOTICE([Ecore_Wayland checks])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries

## Eina, Ecore, Ecore_Input

requirements_pc_ecore_wayland="ecore-input >= ${PACKAGE_VERSION} ecore >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION}"

## Wayland

PKG_CHECK_MODULES([ECORE_WAYLAND],
   [wayland-client wayland-cursor xkbcommon],
   [
    have_ecore_wayland="yes"
    requirements_pc_ecore_wayland="wayland-client wayland-cursor xkbcommon ${requirements_pc_ecore_wayland}"
   ],
   [have_ecore_wayland="no"])

AM_CONDITIONAL([HAVE_ECORE_WAYLAND], [test "x${have_ecore_wayland}" = "xyes"])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

#### End of Ecore_Wayland


#### Ecore_Win32

if test "x${have_win32}" = "xyes" ; then

AC_MSG_NOTICE([Ecore_Win32 checks])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries

## Evil, Eina, Ecore, Ecore_Input

requirements_pc_ecore_win32="ecore-input >= ${PACKAGE_VERSION} ecore >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} evil >= ${PACKAGE_VERSION} ${requirements_pc_ecore_win32}"

ECORE_WIN32_LIBS="-lole32 -lgdi32"
requirements_libs_ecore_win32="-lole32 -lgdi32 ${requirements_libs_ecore_win32}"
AC_SUBST([ECORE_WIN32_LIBS])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

fi

#### End of Ecore_Win32


#### Ecore_WinCE

if test "x${have_wince}" = "xyes" ; then

AC_MSG_NOTICE([Ecore_WinCE checks])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries

## Evil, Eina, Ecore, Ecore_Input

requirements_pc_ecore_wince="ecore-input >= ${PACKAGE_VERSION} ecore >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} evil >= ${PACKAGE_VERSION} ${requirements_pc_ecore_wince}"


### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

fi

#### End of Ecore_WinCE


#### Ecore_X

AC_MSG_NOTICE([Ecore_X checks])

### Additional options to configure

# FIXME: recycle that with evas
AC_ARG_WITH([ecore-x-backend],
   [AC_HELP_STRING([--ecore-x-backend=BACKEND],
                   [use the predefined x11 backends, one of: xlib, xcb and none. @<:@default=xlib@:>@])],
   [ecore_x_backend=${withval}],
   [ecore_x_backend="xlib"])

case "${ecore_x_backend}" in
   xlib)
      have_ecore_x_xlib="yes"
      ;;
   xcb)
      have_ecore_x_xcb="yes"
      ;;
   none)
      have_ecore_x_xcb="no"
      have_ecore_x_xlib="no"
      ;;
   *)
     AC_MSG_ERROR([Unknown x11 backend --with-ecore-x-backend=${ecore_x_backend}])
     ;;
esac

AC_ARG_ENABLE(ecore-x-composite,
   [AC_HELP_STRING([--disable-ecore-x-composite],
       [disable the ecore_x support for Xcomposite extension. @<:@default=detect@:>@])],
   [want_ecore_x_composite=$enableval],
   [want_ecore_x_composite="yes"])

AC_ARG_ENABLE(ecore-x-cursor,
   [AC_HELP_STRING([--disable-ecore-x-cursor],
       [disable the ecore_x support for Xcursor library. @<:@default=detect@:>@])],
   [want_ecore_x_cursor=$enableval],
   [want_ecore_x_cursor="yes"])

AC_ARG_ENABLE(ecore-x-damage,
   [AC_HELP_STRING([--disable-ecore-x-damage],
       [disable the ecore_x support for Xdamage extension. @<:@default=detect@:>@])],
   [want_ecore_x_damage=$enableval],
   [want_ecore_x_damage="yes"])

AC_ARG_ENABLE(ecore-x-dpms,
   [AC_HELP_STRING([--disable-ecore-x-dpms],
       [disable the ecore_x support for Xdpms extension. @<:@default=detect@:>@])],
   [want_ecore_x_dpms=$enableval],
   [want_ecore_x_dpms="yes"])

AC_ARG_ENABLE(ecore-x-dri,
   [AC_HELP_STRING([--disable-ecore-x-dri],
       [disable the ecore_x support for DRI extension. @<:@default=detect@:>@])],
   [want_ecore_x_dri=$enableval],
   [want_ecore_x_dri="yes"])

AC_ARG_ENABLE(ecore-x-gesture,
   [AC_HELP_STRING([--enable-ecore-x-gesture],
       [enable the ecore_x support for Xgesture extension. @<:@default=detect@:>@])],
   [want_ecore_x_gesture=$enableval],
   [want_ecore_x_gesture="yes"])

AC_ARG_ENABLE(ecore-x-input,
   [AC_HELP_STRING([--disable-ecore-x-input],
       [disable the ecore_x support for Xinput/Xinput2 extension. @<:@default=detect@:>@])],
   [want_ecore_x_input=$enableval],
   [want_ecore_x_input="yes"])

AC_ARG_ENABLE(ecore-x-randr,
   [AC_HELP_STRING([--disable-ecore-x-randr],
       [disable the ecore_x support for Xrandr extension. @<:@default=detect@:>@])],
   [want_ecore_x_randr=$enableval],
   [want_ecore_x_randr="yes"])

AC_ARG_ENABLE(ecore-x-render,
   [AC_HELP_STRING([--disable-ecore-x-render],
       [disable the ecore_x support for Xrender extension. @<:@default=detect@:>@])],
   [want_ecore_x_render=$enableval],
   [want_ecore_x_render="yes"])

AC_ARG_ENABLE(ecore-x-screensaver,
   [AC_HELP_STRING([--disable-ecore-x-screensaver],
       [disable the ecore_x support for Xscreensaver extension. @<:@default=detect@:>@])],
   [want_ecore_x_screensaver=$enableval],
   [want_ecore_x_screensaver="yes"])

AC_ARG_ENABLE(ecore-x-shape,
   [AC_HELP_STRING([--disable-ecore-x-shape],
       [disable the ecore_x support for Xshape extension. @<:@default=detect@:>@])],
   [want_ecore_x_shape=$enableval],
   [want_ecore_x_shape="yes"])

AC_ARG_ENABLE(ecore-x-sync,
   [AC_HELP_STRING([--disable-ecore-x-sync],
       [disable the ecore_x support for Xsync extension. @<:@default=detect@:>@])],
   [want_ecore_x_sync=$enableval],
   [want_ecore_x_sync="yes"])

AC_ARG_ENABLE(ecore-x-xfixes,
   [AC_HELP_STRING([--disable-ecore-x-xfixes],
       [disable the ecore_x support for Xfixes extension. @<:@default=detect@:>@])],
   [want_ecore_x_xfixes=$enableval],
   [want_ecore_x_xfixes="yes"])

AC_ARG_ENABLE(ecore-x-xinerama,
   [AC_HELP_STRING([--disable-ecore-x-xinerama],
       [disable the ecore_x support for Xinerama extension. @<:@default=detect@:>@])],
   [want_ecore_x_xinerama=$enableval],
   [want_ecore_x_xinerama="yes"])

AC_ARG_ENABLE(ecore-x-xprint,
   [AC_HELP_STRING([--disable-ecore-x-xprint],
       [disable the ecore_x support for Xprint extension. @<:@default=detect@:>@])],
   [want_ecore_x_xprint=$enableval],
   [want_ecore_x_xprint="yes"])

AC_ARG_ENABLE(ecore-x-xtest,
   [AC_HELP_STRING([--disable-ecore-x-xtest],
       [disable the ecore_x support for Xtest extension. @<:@default=detect@:>@])],
   [want_ecore_x_xtest=$enableval],
   [want_ecore_x_xtest="yes"])

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

have_ecore_x="no"
use_Xcursor="no"

AC_MSG_CHECKING([which X11 backend is to be built])
AC_MSG_RESULT([${ecore_x_backend}])

### Checks for programs

### Checks for libraries

## Eina, Ecore, Ecore_Input

requirements_pc_ecore_x="ecore-input >= ${PACKAGE_VERSION} ecore >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_ecore_x}"

## Xlib
if test "x${have_ecore_x_xlib}" = "xyes" ; then
   AC_PATH_XTRA
fi

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
if test "x${have_ecore_x_xlib}" = "xyes" ; then
   AC_CHECK_HEADERS([X11/X.h],
      [
       if test "x${want_evas_simple_x11}" = "xyes"; then
          x_libs="${x_libs} -lX11 -lXext"
       else
          x_dir=${x_dir:-/usr/X11R6}
          x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
          x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext"
       fi
       have_ecore_x_xlib="yes"
      ])

   if test "x$want_ecore_x_cursor" = "xyes"; then
      CFLAGS_save="${CFLAGS}"
      CFLAGS="$x_cflags $x_includes"
      AC_CHECK_HEADER([X11/Xcursor/Xcursor.h],
         [use_Xcursor="yes"],
         [use_Xcursor="no"],
         [#include <X11/Xlib.h>])
      CFLAGS="$CFLAGS_save"
   fi
fi

## XCB
if test "x${have_ecore_x_xcb}" = "xyes" ; then
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

if test "x${have_ecore_x_xlib}" = "xyes" ; then
   AC_CHECK_LIB([X11], [XCreateImage], [have_ecore_x_xlib="yes"], [have_ecore_x_xlib="no"])
fi

if test "x${have_ecore_x_xlib}" = "xyes" ; then
   AC_CHECK_LIB([Xext], [XShmCreateImage], [have_ecore_x_xlib="yes"], [have_ecore_x_xlib="no"])
   have_ecore_x="yes"
fi

if test "x${have_ecore_x_xlib}" = "xyes" && test "x$want_ecore_x_cursor" = "xyes" ; then
   AC_CHECK_LIB([Xcursor],
      [XcursorImageLoadCursor],
      [
       use_Xcursor="yes"
       AC_DEFINE([ECORE_XCURSOR], 1, [Build support for Xcursor])
       Xcursor_libs="-lXcursor"
      ],
      [use_Xcursor="no"],
      [$x_libs -lXrender])
   ECORE_X_CFLAGS="${ECORE_X_CFLAGS} $x_cflags $x_includes"
   ECORE_X_LIBS="${ECORE_X_LIBS} ${Xcursor_libs}"
fi

if test "x${have_ecore_x_xlib}" = "xyes" ; then
   ECORE_CHECK_X_EXTENSION([Xkb], [XKB.h], [X11], [XkbSetDetectableAutoRepeat], [$want_ecore_x_xkb])
   ECORE_CHECK_X_EXTENSION([Xcomposite], [Xcomposite.h], [Xcomposite], [XCompositeQueryExtension], [$want_ecore_x_composite])
   ECORE_CHECK_X_EXTENSION([Xdamage], [Xdamage.h], [Xdamage], [XDamageSubtract], [$want_ecore_x_damage])
   ECORE_CHECK_X_EXTENSION([Xdpms], [dpms.h], [Xdpms], [DPMSQueryExtension], [$want_ecore_x_dpms])
   if test "x$use_xdpms" = "xno" ; then
      ECORE_CHECK_X_EXTENSION([Xdpms], [dpms.h], [Xext], [DPMSQueryExtension], [$want_ecore_x_dpms])
   fi
   ECORE_CHECK_X_EXTENSION([Xfixes], [Xfixes.h], [Xfixes], [XFixesExpandRegion], [$want_ecore_x_xfixes])
   ECORE_CHECK_X_EXTENSION([Xinerama], [Xinerama.h], [Xinerama], [XineramaQueryScreens], [$want_ecore_x_xinerama])
   ECORE_CHECK_X_EXTENSION([Xprint], [Print.h], [Xp], [XpQueryScreens], [$want_ecore_x_xprint])
   ECORE_CHECK_X_EXTENSION([Xrandr], [Xrandr.h], [Xrandr], [XRRGetScreenResourcesCurrent], [$want_ecore_x_randr])
   ECORE_CHECK_X_EXTENSION([Xgesture], [gesture.h], [Xgesture], [XGestureQueryExtension], [$want_ecore_x_gesture])
   ECORE_CHECK_X_EXTENSION([Xrender], [Xrender.h], [Xrender], [XRenderFindVisualFormat], [$want_ecore_x_render])
   ECORE_CHECK_X_EXTENSION([Xtest], [XTest.h], [Xtst], [XTestFakeKeyEvent], [$want_ecore_x_xtest])
   ECORE_CHECK_X_EXTENSION([Xss], [scrnsaver.h], [Xss], [XScreenSaverSelectInput], [$want_ecore_x_screensaver])
   ECORE_CHECK_X_EXTENSION([Xi2], [XInput2.h], [Xi], [XIQueryDevice], [$want_ecore_x_input])
   ECORE_CHECK_X_EXTENSION([Xi2_2], [XInput2.h], [Xi], [XIGrabTouchBegin], [$want_ecore_x_input])
   
   AC_DEFINE([HAVE_ECORE_X_XLIB], [1], [Defined to 1 if Xlib is enabled.])
fi

## XCB

if test "x${have_ecore_x_xcb}" = "xyes" ; then

   PKG_CHECK_EXISTS([x11-xcb xcb xcb-shm xcb-event xcb-icccm >= 0.3.8 xcb-util >= 0.3.8 xcb-image xcb-keysyms >= 0.3.8],
      [
       have_ecore_x_xcb="yes"
       have_ecore_x="yes"
       requirements_pc_ecore_x="${requirements_pc_ecore_x} x11-xcb xcb xcb-shm xcb-event xcb-icccm >= 0.3.8 xcb-util >= 0.3.8 xcb-image xcb-keysyms >= 0.3.8"
       requirements_pc_deps_ecore_x="${requirements_pc_deps_ecore_x} x11-xcb xcb xcb-shm xcb-event xcb-icccm >= 0.3.8 xcb-util >= 0.3.8 xcb-image xcb-keysyms >= 0.3.8"
      ],
      [AC_MSG_ERROR([Ecore_X with XCB backend enabled but XCB not available. Exiting...])])
fi

if test "x${have_ecore_x_xcb}" = "xyes" ; then
   if test "x${have_pixman}" = "xyes" ; then
      requirements_pc_ecore_x="${requirements_pc_ecore_x} pixman-1"
      requirements_pc_deps_ecore_x="${requirements_pc_deps_ecore_x} pixman-1"
   fi

   if test "x$want_ecore_x_composite" != "xno"; then
      PKG_CHECK_EXISTS([xcb-composite],
         [
          have_ecore_x_xcb_composite="yes"
          requirements_pc_ecore_x="xcb-composite ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-composite ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_COMPOSITE], [1], [Build support for XCB composite])
         ],
         [have_ecore_x_xcb_composite="no"])
   else
      have_ecore_x_xcb_composite="no"
      AC_MSG_NOTICE([composite extension explicitly disabled])
   fi

   if test "x$want_ecore_x_damage" != "xno"; then
      PKG_CHECK_EXISTS([xcb-damage],
         [
          have_ecore_x_xcb_damage="yes"
          requirements_pc_ecore_x="xcb-damage ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-damage ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_DAMAGE], [1], [Build support for XCB damage])
         ],
         [have_ecore_x_xcb_damage="no"])
   else
      have_ecore_x_xcb_damage="no"
      AC_MSG_NOTICE([damage extension explicitly disabled])
   fi

   if test "x$want_ecore_x_dpms" != "xno"; then
      PKG_CHECK_EXISTS([xcb-dpms],
         [
          have_ecore_x_xcb_dpms="yes"
          requirements_pc_ecore_x="xcb-dpms ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-dpms ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_DPMS], [1], [Build support for XCB dpms])
         ],
         [have_ecore_x_xcb_dpms="no"])
   else
      have_ecore_x_xcb_dpms="no"
      AC_MSG_NOTICE([dpms extension explicitly disabled])
   fi

   if test "x$want_ecore_x_randr" != "xno"; then
      PKG_CHECK_EXISTS([xcb-randr],
         [
          have_ecore_x_xcb_randr="yes"
          requirements_pc_ecore_x="xcb-randr ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-randr ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_RANDR], [1], [Build support for XCB randr])
         ],
         [have_ecore_x_xcb_randr="no"])
   else
      have_ecore_x_xcb_randr="no"
      AC_MSG_NOTICE([randr extension explicitly disabled])
   fi

   if test "x$want_ecore_x_render" != "xno"; then
      PKG_CHECK_EXISTS([xcb-render],
         [
          have_ecore_x_xcb_render="yes"
          requirements_pc_ecore_x="xcb-render ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-render ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_RENDER], [1], [Build support for XCB render])
         ],
         [have_ecore_x_xcb_render="no"])
   else
      have_ecore_x_xcb_render="no"
      AC_MSG_NOTICE([render extension explicitly disabled])
   fi

   if test "x$want_ecore_x_screensaver" != "xno"; then
      PKG_CHECK_EXISTS([xcb-screensaver],
         [
          have_ecore_x_xcb_screensaver="yes"
          requirements_pc_ecore_x="xcb-screensaver ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-screensaver ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_SCREENSAVER], [1], [Build support for XCB screensaver])
         ],
         [have_ecore_x_xcb_screensaver="no"])
   else
      have_ecore_x_xcb_screensaver="no"
      AC_MSG_NOTICE([screensaver extension explicitly disabled])
   fi

   if test "x$want_ecore_x_shape" != "xno"; then
      PKG_CHECK_EXISTS([xcb-shape],
         [
          have_ecore_x_xcb_shape="yes"
          requirements_pc_ecore_x="xcb-shape ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-shape ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_SHAPE], [1], [Build support for XCB shape])
         ],
         [have_ecore_x_xcb_shape="no"])
   else
      have_ecore_x_xcb_shape="no"
      AC_MSG_NOTICE([shape extension explicitly disabled])
   fi

   if test "x$want_ecore_x_gesture" != "xno"; then
      PKG_CHECK_EXISTS([xcb-gesture],
         [
          have_ecore_x_xcb_gesture="yes"
          requirements_pc_ecore_x="xcb-gesture ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-gesture ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_XGESTURE], [1], [Build support for XCB xgesture])
         ],
         [have_ecore_x_xcb_gesture="no"])
   else
      have_ecore_x_xcb_gesture="no"
      AC_MSG_NOTICE([gesture extension explicitly disabled])
   fi

   if test "x$want_ecore_x_sync" != "xno"; then
      PKG_CHECK_EXISTS([xcb-sync],
         [
          have_ecore_x_xcb_sync="yes"
          requirements_pc_ecore_x="xcb-sync ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-sync ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_SYNC], [1], [Build support for XCB sync])
         ],
         [have_ecore_x_xcb_sync="no"])
   else
      have_ecore_x_xcb_sync="no"
      AC_MSG_NOTICE([sync extension explicitly disabled])
   fi

   if test "x$want_ecore_x_xfixes" != "xno"; then
      PKG_CHECK_EXISTS([xcb-xfixes],
         [
          have_ecore_x_xcb_xfixes="yes"
          requirements_pc_ecore_x="xcb-xfixes ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-xfixes ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_XFIXES], [1], [Build support for XCB xfixes])
         ],
         [have_ecore_x_xcb_xfixes="no"])
   else
      have_ecore_x_xcb_xfixes="no"
      AC_MSG_NOTICE([xfixes extension explicitly disabled])
   fi

   if test "x$want_ecore_x_xinerama" != "xno"; then
      PKG_CHECK_EXISTS([xcb-xinerama],
         [
          have_ecore_x_xcb_xinerama="yes"
          requirements_pc_ecore_x="xcb-xinerama ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-xinerama ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_XINERAMA], [1], [Build support for XCB xinerama])
         ],
         [have_ecore_x_xcb_xinerama="no"])
   else
      have_ecore_x_xcb_xinerama="no"
      AC_MSG_NOTICE([xinerama extension explicitly disabled])
   fi

   if test "x$want_ecore_x_xprint" != "xno"; then
      PKG_CHECK_EXISTS([xcb-xprint],
         [
          have_ecore_x_xcb_xprint="yes"
          requirements_pc_ecore_x="xcb-xprint ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-xprint ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_XPRINT], [1], [Build support for XCB xprint])
         ],
         [have_ecore_x_xcb_xprint="no"])
   else
      have_ecore_x_xcb_xprint="no"
      AC_MSG_NOTICE([xprint extension explicitly disabled])
   fi

   if test "x$want_ecore_x_xtest" != "xno"; then
      PKG_CHECK_EXISTS([xcb-xtest],
         [
          have_ecore_x_xcb_xtest="yes"
          requirements_pc_ecore_x="xcb-xtest ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-xtest ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_XTEST], [1], [Build support for XCB xtest])
         ],
         [have_ecore_x_xcb_xtest="no"])
   else
      have_ecore_x_xcb_xtest="no"
      AC_MSG_NOTICE([xtest extension explicitly disabled])
   fi

# input extension disabled currently in xcb as xcb-input has some issues
   want_ecore_x_input="no"
   if test "x$want_ecore_x_input" != "xno"; then
      PKG_CHECK_EXISTS([xcb-xinput],
         [
          have_ecore_x_xcb_xinput="yes"
          requirements_pc_ecore_x="xcb-xinput ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-xinput ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_XINPUT], [1], [Build support for XCB input])
         ],
         [have_ecore_x_xcb_xinput="no"])
   else
      have_ecore_x_xcb_xinput="no"
      AC_MSG_NOTICE([input extension explicitly disabled])
   fi

   if test "x$want_ecore_x_cursor" != "xno"; then
      PKG_CHECK_EXISTS([xcb-render xcb-renderutil],
         [
          have_ecore_x_xcb_cursor="yes"
          requirements_pc_ecore_x="xcb-render xcb-renderutil ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-render xcb-renderutil ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_CURSOR], [1], [Build support for XCB cursor])
         ],
         [have_ecore_x_xcb_cursor="no"])
   else
      have_ecore_x_xcb_cursor="no"
      AC_MSG_NOTICE([cursor extension explicitly disabled])
   fi

# dri extension disabled currently in xcb
   want_ecore_x_dri="no"
   if test "x$want_ecore_x_dri" != "xno"; then
      PKG_CHECK_EXISTS([xcb-dri2],
         [
          have_ecore_x_xcb_dri="yes"
          requirements_pc_ecore_x="xcb-dri2 ${requirements_pc_ecore_x}"
          requirements_pc_deps_ecore_x="xcb-dri2 ${requirements_pc_deps_ecore_x}"
          AC_DEFINE([ECORE_XCB_DRI], [1], [Build support for XCB dri])
         ],
         [have_ecore_x_xcb_dri="no"])
   else
      have_ecore_x_xcb_dri="no"
      AC_MSG_NOTICE([dri extension explicitly disabled])
   fi

   PKG_CHECK_MODULES([ECORE_X], [${requirements_pc_deps_ecore_x}])
   EFL_CHECK_FUNCS([ECORE_X], [dlopen iconv])
   AC_DEFINE([HAVE_ECORE_X_XCB], [1], [Defined to 1 if XCB is enabled.])
   have_ecore_x_xcb_define="-DHAVE_ECORE_X_XCB"
   AC_SUBST([have_ecore_x_xcb_define])
fi

requirements_libs_ecore_x="${requirements_libs_ecore_x} ${ECORE_X_LIBS}"

AC_SUBST([ECORE_X_CFLAGS])
AC_SUBST([ECORE_X_LIBS])

AM_CONDITIONAL([HAVE_ECORE_X_XLIB], [test "x${have_ecore_x_xlib}" = "xyes"])
AM_CONDITIONAL([HAVE_ECORE_X_XCB], [test "x${have_ecore_x_xcb}" = "xyes"])
AM_CONDITIONAL([HAVE_ECORE_X], [test "x${have_ecore_x}" = "xyes"])

#### End of Ecore_X


#### Ecore_Imf

AC_MSG_NOTICE([Ecore_Imf checks])

### Additional options to configure

### Default values

want_ecore_imf="yes"
want_ecore_imf_xim="no"
want_ecore_imf_scim="no"
want_ecore_imf_ibus="no"

case "$host_os" in
   mingw32ce*)
      want_ecore_imf="no"
      ;;
   mingw*)
      want_ecore_imf="yes"
      ;;
   darwin*)
      want_ecore_imf="yes"
      ;;
   *)
      want_ecore_imf="yes"
      want_ecore_imf_xim="yes"
      want_ecore_imf_scim="yes"
      want_ecore_imf_ibus="yes"
      ;;
esac

### Checks for programs

### Checks for libraries

# Escape library for compilation on PS3

if test "x${have_ps3}" = "xyes" ; then
   requirements_pc_ecore_imf="${requirements_pc_ecore_imf} escape >= ${PACKAGE_VERSION}"
   requirements_pc_ecore_imf_evas="${requirements_pc_ecore_imf_evas} escape >= ${PACKAGE_VERSION}"
fi

## Eina, Ecore (ecore_imf)

requirements_pc_ecore_imf="ecore >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_ecore_imf}"

## Eina, Evas, Ecore, Ecore_Imf (ecore_imf_evas)

requirements_pc_ecore_imf_evas="ecore_imf >= ${PACKAGE_VERSION} ecore >= ${PACKAGE_VERSION} evas >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_ecore_imf_evas}"

## modules

# ibus

have_ecore_imf_ibus="no"
if test "x${want_ecore_imf_ibus}" = "xyes" && test "x${have_glib}" = "xyes" ; then
   PKG_CHECK_MODULES([IBUS],
      [ibus-1.0 >= 1.4],
      [
       have_ecore_imf_ibus="yes"
       AC_DEFINE([BUILD_ECORE_IMF_IBUS], [1], [Ecore Imf IBUS Support])
      ],
      [have_ecore_imf_ibus="no"])
   IBUS_CFLAGS="${GLIB_CFLAGS} ${IBUS_CFLAGS}"
   IBUS_LIBS="${GLIB_LIBS} ${IBUS_LIBS}"
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
   have_ecore_imf_xim="yes"
   AC_DEFINE([ENABLE_XIM], [1], [Enable X Input Method])
fi

AM_CONDITIONAL([BUILD_ECORE_IMF_XIM], [test "x${have_ecore_imf_xim}" = "xyes"])

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

#### End of Ecore_Imf


#### Ecore_Evas

AC_MSG_NOTICE([Ecore_Evas checks])

### Additional options to configure

### Default values

have_ecore_evas="yes"
have_extn="yes"

want_ecore_evas_software_buffer="yes"
want_ecore_evas_software_x11="no"
want_ecore_evas_opengl_x11="no"
want_ecore_evas_software_xcb="no"
want_ecore_evas_software_gdi="no"
want_ecore_evas_software_ddraw="no"
want_ecore_evas_software_sdl="no"
want_ecore_evas_gl_sdl="no"
want_ecore_evas_gl_cocoa="no"
want_ecore_evas_fb="no"
want_ecore_evas_ews="yes"
want_ecore_evas_extn="yes"
want_ecore_evas_wayland_shm="no"
want_ecore_evas_wayland_egl="no"

case "$host_os" in
   mingw32ce*)
      want_ecore_evas_extn="no"
      ;;
   mingw*)
      want_ecore_evas_software_gdi="yes"
      want_ecore_evas_software_ddraw="yes"
      want_ecore_evas_software_sdl="yes"
      want_ecore_evas_gl_sdl="yes"
      want_ecore_evas_extn="no"
      ;;
   darwin*)
      want_ecore_evas_software_sdl="yes"
      want_ecore_evas_gl_sdl="yes"
      want_ecore_evas_gl_cocoa="yes"
      ;;
   *)
      want_ecore_evas_software_x11="yes"
      want_ecore_evas_opengl_x11="yes"
      want_ecore_evas_software_xcb="no"
      want_ecore_evas_software_sdl="yes"
      want_ecore_evas_gl_sdl="yes"
      want_ecore_evas_gl_cocoa="no"
      want_ecore_evas_fb="yes"
      want_ecore_evas_wayland_shm="yes"
      want_ecore_evas_wayland_egl="yes"
      ;;
esac

### Checks for programs

### Checks for libraries

## Eina, Evas, Ecore, Ecore_Input

requirements_pc_ecore_evas="ecore-input-evas >= ${PACKAGE_VERSION} ecore-input >= ${PACKAGE_VERSION} ecore >= ${PACKAGE_VERSION} evas >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_ecore_evas}"

## modules

# ecore_evas_buffer

ECORE_EVAS_CHECK_MODULE_FULL([software-buffer],
   [${have_evas_engine_buffer}],
   [${want_ecore_evas_software_buffer}],
   [Software Buffer],
   [yes])

# ecore_evas_cocoa

ECORE_EVAS_CHECK_MODULE_FULL([opengl-cocoa],
   [${have_evas_engine_gl_cocoa}],
   [${want_ecore_evas_gl_cocoa}],
   [OpenGL Cocoa],
   [${have_ecore_cocoa}],
   [requirements_pc_ecore_evas="ecore-cocoa >= ${PACKAGE_VERSION} ${requirements_pc_ecore_evas}"])

# ecore_evas_ews

ECORE_EVAS_CHECK_MODULE_FULL([ews],
   [${have_evas_engine_buffer}],
   [${want_ecore_evas_ews}],
   [Ecore Evas Single Process Windowing System],
   [yes], [])

# ecore_evas_extn

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
   requirements_pc_ecore_evas="ecore-ipc >= ${PACKAGE_VERSION} ${requirements_pc_ecore_evas}"
fi

# ecore_evas_fb

ECORE_EVAS_CHECK_MODULE_FULL([fb],
   [${have_evas_engine_fb}],
   [${want_ecore_evas_fb}],
   [Linux Framebuffer],
   [${have_ecore_fb}],
   [requirements_pc_ecore_evas="ecore-fb >= ${PACKAGE_VERSION} ${requirements_pc_ecore_evas}"])

# ecore_evas_psl1ght

ECORE_EVAS_CHECK_MODULE_FULL([psl1ght],
   [${have_evas_engine_psl1ght}],
   [${want_ecore_evas_psl1ght}],
   [PSL1GHT],
   [${have_ecore_psl1ght}],
   [requirements_pc_ecore_evas="ecore-psl1ght >= ${PACKAGE_VERSION} ${requirements_pc_ecore_evas}"])

# ecore_evas_sdl

ECORE_EVAS_CHECK_MODULE_FULL([software-sdl],
   [${have_evas_engine_software_sdl}],
   [${want_ecore_evas_software_sdl}],
   [Software SDL],
   [${have_ecore_sdl}],
   [requirements_ecore_evas="ecore-sdl >= ${PACKAGE_VERSION} ${requirements_ecore_evas}"])

ECORE_EVAS_CHECK_MODULE_FULL([opengl-sdl],
   [${have_evas_engine_gl_sdl}],
   [${want_ecore_evas_gl_sdl}],
   [OpenGL SDL],
   [${have_ecore_sdl}],
   [requirements_ecore_evas="ecore-sdl >= ${PACKAGE_VERSION} ${requirements_ecore_evas}"])

# ecore_evas_wayland

ECORE_EVAS_CHECK_MODULE_FULL([wayland-shm],
   [${have_evas_engine_wayland_shm}],
   [${want_ecore_evas_wayland_shm}],
   [Wayland Shm],
   [${have_ecore_wayland}],
   [requirements_pc_ecore_evas="ecore-wayland >= ${PACKAGE_VERSION} ${requirements_pc_ecore_evas}"])

ECORE_EVAS_CHECK_MODULE_FULL([wayland-egl],
   [${have_evas_engine_wayland_egl}],
   [${want_ecore_evas_wayland_egl}],
   [Wayland Egl],
   [${have_ecore_wayland}],
   [
    PKG_CHECK_MODULES([WAYLAND_EGL], [wayland-egl], [have_wayland_egl="yes"], [have_wayland_egl="no"])
    if test "x${have_wayland_egl}" = "xyes" ; then
       requirements_pc_ecore_evas="wayland-egl egl >= 7.10 ecore-wayland >= ${PACKAGE_VERSION} ${requirements_pc_ecore_evas}"
    fi
   ])

# ecore_evas_win32
ECORE_EVAS_CHECK_MODULE_FULL([software-gdi],
   [${have_evas_engine_software_gdi}],
   [${want_ecore_evas_software_gdi}],
   [Software GDI],
   [${have_win32}])

ECORE_EVAS_CHECK_MODULE_FULL([software-ddraw],
   [${have_evas_engine_software_ddraw}],
   [${want_ecore_evas_software_ddraw}],
   [Software DirectDraw],
   [${have_win32}])

if test "x${have_ecore_evas_software_gdi}" = "xyes" || \
   test "x${have_ecore_evas_software_ddraw}" = "xyes" ; then
   AC_DEFINE(BUILD_ECORE_EVAS_WIN32, 1, [Support for Win32 Engine in Ecore_Evas])
   requirements_pc_ecore_evas="ecore-win32 >= ${PACKAGE_VERSION} ${requirements_pc_ecore_evas}"
fi

# ecore_evas_x11

ECORE_EVAS_CHECK_MODULE_FULL([software-x11],
   [${have_evas_engine_software_x11}],
   [${want_ecore_evas_software_x11}],
   [Software X11],
   [${have_ecore_x}])

have_ecore_evas_software_xlib="no"
have_ecore_evas_software_xcb="no"
if test "x$have_ecore_evas_software_x11" = "xyes" ; then
   have_ecore_evas_software_xlib=`${PKG_CONFIG} --variable=Xlib evas-software-x11`
   if test "x${have_ecore_evas_software_xlib}" = "xstatic"; then
      have_ecore_evas_software_xlib="yes"
   fi
   if test "x${have_ecore_evas_software_xlib}" = "xyes" && test "x${have_ecore_x_xlib}" = "xyes" ; then
      AC_DEFINE([BUILD_ECORE_EVAS_SOFTWARE_XLIB], [1], [Evas Software Xlib Engine Support])
   fi
   have_ecore_evas_software_xcb=`${PKG_CONFIG} --variable=XCB evas-software-x11`
   if test "x$have_ecore_evas_software_xcb" = "xstatic"; then
      have_ecore_evas_software_xcb="yes"
   fi
   if test "x$have_ecore_evas_software_xcb" = "xyes" && test "x${have_ecore_x_xcb}" = "xyes" ; then
      AC_DEFINE([BUILD_ECORE_EVAS_SOFTWARE_XCB], [1], [Evas Software XCB Engine Support])
   fi
fi

# ecore_evas_opengl_x11

ECORE_EVAS_CHECK_MODULE_FULL([opengl-x11],
   [${have_evas_engine_gl_x11}],
   [${want_ecore_evas_opengl_x11}],
   [OpenGL Xlib],
   [${have_ecore_x}])

have_ecore_evas_opengl_xlib="no"
have_ecore_evas_opengl_xcb="no"
if test "x${have_ecore_evas_opengl_x11}" = "xyes" || test "x${have_ecore_evas_opengl_x11}" = "xstatic" ; then
   have_ecore_evas_opengl_xlib=`${PKG_CONFIG} --variable=Xlib evas-opengl-x11`
   if test "x${have_ecore_evas_opengl_xlib}" = "xyes" ; then
      AC_DEFINE([BUILD_ECORE_EVAS_OPENGL_XLIB], [1], [OpenGL Xlib rendering backend])
   fi

# opengl does not work with xcb (yet)
   have_ecore_evas_opengl_xcb=`${PKG_CONFIG} --variable=XCB evas-opengl-x11`
   if test "x${have_ecore_evas_opengl_xcb}" = "xstatic"; then
      have_ecore_evas_opengl_xcb="yes"
   fi
   if test "x${have_ecore_evas_opengl_xcb}" = "xyes" && test "x${have_ecore_x_xcb}" = "xyes" ; then
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

if test "x$have_ecore_evas_software_x11" = "xyes" || \
   test "x$have_ecore_evas_opengl_x11" = "xyes" || \
   test "x$have_ecore_evas_software_xcb" = "xyes"; then
   AC_DEFINE([BUILD_ECORE_EVAS_X11], [1], [Support for X Window Engines in Ecore_Evas])
   requirements_pc_ecore_evas="ecore-x >= ${PACKAGE_VERSION} ${requirements_pc_ecore_evas}"
fi

if test "x${requirements_pc_deps_ecore_evas}" = "x" ; then
   ECORE_EVAS_CFLAGS=""
   ECORE_EVAS_LIBS=""
   AC_SUBST([ECORE_EVAS_CFLAGS])
   AC_SUBST([ECORE_EVAS_LIBS])
else
   PKG_CHECK_MODULES([ECORE_EVAS], [${requirements_pc_deps_ecore_evas}])
fi

### Checks for header files

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

#### End of Ecore_Evas


#### Eio

AC_MSG_NOTICE([Eio checks])

### Additional options to configure

### Default values

### Checks for programs

### Checks for libraries

requirements_pc_eio="ecore >= ${PACKAGE_VERSION} eet >= ${PACKAGE_VERSION} eina >= ${PACKAGE_VERSION} ${requirements_pc_eio}"

requirements_libs_eio="${requirements_libs_eio} -lm"
EIO_LIBS=${requirements_libs_eio}
AC_SUBST([EIO_LIBS])

### Checks for header files

AC_CHECK_HEADERS([grp.h pwd.h])

### Checks for types

### Checks for structures

### Checks for compiler characteristics

### Checks for linker characteristics

### Checks for library functions

AC_CHECK_FUNCS([fchmod chown getpwnam getgrnam])

# Check for splice system call

AC_MSG_CHECKING([whether to use splice for file copy])
AC_TRY_LINK(
   [
#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif
#include <fcntl.h>
   ],
   [
long ret = splice(0,0,1,0,400,0);
   ],
   [have_splice="yes"],
   [have_splice="no"])
AC_MSG_RESULT([${have_splice}])

if test "x${have_splice}" = "xyes" ; then
   AC_DEFINE([HAVE_SPLICE], [1], [Define to mention that splice syscall is supported])
fi

# Check for lstat

AC_MSG_CHECKING([whether lstat is available])
AC_TRY_LINK(
   [
#include <sys/types.h>
#include <sys/stat.h>
#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif
   ],
   [
struct stat st;
lstat("/tmp", &st);
   ],
   [have_lstat="yes"],
   [have_lstat="no"])
AC_MSG_RESULT([${have_lstat}])

if test "x${have_lstat}" = "xyes" ; then
   AC_DEFINE([HAVE_LSTAT], [1], [Define to mention that lstat syscall is supported])
fi

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

# Check for inotify specificity
have_inotify="no"
have_notify_win32="no"
EIO_CHECK_INOTIFY([have_inotify="yes"], [have_inotify="no"])
EIO_CHECK_NOTIFY_WIN32([have_notify_win32="yes"], [have_notify_win32="no"])

AM_CONDITIONAL([EIO_HAVE_INOTIFY], [test "x${have_inotify}" = "xyes"])
AM_CONDITIONAL([EIO_HAVE_WINCHANGE], [test "x${have_notify_win32}" = "xyes"])

#### End of Eio


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
src/lib/eina/eina_config.h
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
pc/embryo.pc
pc/eio.pc
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
echo $ECHO_N "    OpenGL SDL.................: $have_evas_engine_gl_sdl $ECHO_C"
if test "x$have_evas_engine_gl_sdl" = "xyes"; then
  echo "(GLES: $gl_flavor_gles)"
else
  echo
fi
echo "    OpenGL Cocoa...............: $have_evas_engine_gl_cocoa"
echo "    Software Framebuffer.......: $have_evas_engine_fb"
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
echo "    Smaller Dither Mask.....: $conv_small_dither"
echo "    Line Dither Mask........: $conv_line_dither"
echo "    No Dither Mask for 16bpp: $conv_no_dither"
echo "    Tiled 32BPP rotate......: $have_tile_rotate"
echo

echo "Ecore:"
echo "  Thread safety..............: $want_thread_safety"
echo "  GLib support...............: $with_glib"
echo "  Use g_main_loop............: $want_g_main_loop"
echo "  Gathering memory statistic.: $ac_cv_func_mallinfo"
echo "  Gathering timer allocation.: $want_ecore_timer_dump"
echo

echo "Ecore_Con:"
if test "x${have_gnutls}" = "xyes" || test "x${have_openssl}" = "xyes" ; then
echo "  TLS........................: $with_tls"
else
echo "  TLS........................: no"
fi
echo "  IPv6.......................: $have_ipv6"
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

echo "Ecore_Ipc:"
echo "  TLS........................: $with_tls"
echo

echo "Ecore_File:"
echo "  Inotify....................: $have_inotify"
echo "  Windows notification.......: $have_notify_win32"
echo "  Poll.......................: $have_poll"
echo "  CURL.......................: $have_curl"
echo

echo "Ecore_IMF....................: $have_ecore_imf"
echo "  XIM........................: $have_ecore_imf_xim"
echo "  SCIM.......................: $have_ecore_imf_scim"
echo "  IBUS.......................: $have_ecore_imf_ibus"
echo "Ecore_IMF_Evas...............: $have_ecore_imf_evas"
echo

echo "Ecore Graphic systems:"
echo

if test "x$have_ecore_x" = "xyes" ; then
  if test "x$have_ecore_x_xcb" = "xyes" ; then
    echo "  Ecore_X (XCB backend)........: $have_ecore_x_xcb"
    echo "    Xprint.....................: $have_ecore_x_xcb_xprint"
    echo "    Xinerama...................: $have_ecore_x_xcb_xinerama"
    echo "    Xrandr.....................: $have_ecore_x_xcb_randr"
    echo "    Xscreensaver...............: $have_ecore_x_xcb_screensaver"
    echo "    Xshape.....................: $have_ecore_x_xcb_shape"
    echo "    Xgesture...................: $have_ecore_x_xcb_gesture"
    echo "    Xsync......................: $have_ecore_x_xcb_sync"
    echo "    Xrender....................: $have_ecore_x_xcb_render"
    echo "    Xcomposite.................: $have_ecore_x_xcb_composite"
    echo "    Xfixes.....................: $have_ecore_x_xcb_xfixes"
    echo "    Xdamage....................: $have_ecore_x_xcb_damage"
    echo "    Xdpms......................: $have_ecore_x_xcb_dpms"
    echo "    Xtest......................: $have_ecore_x_xcb_xtest"
  else
    echo "  Ecore_X (Xlib backend).......: $have_ecore_x"
    echo "    Xcursor....................: $use_Xcursor"
    echo "    Xkb........................: $use_xkb"
    echo "    Xprint.....................: $use_xprint"
    echo "    Xinerama...................: $use_xinerama"
    echo "    Xrandr.....................: $use_xrandr"
    echo "    Xgesture...................: $use_xgesture"
    echo "    Xscreensaver...............: $use_xss"
    echo "    Xrender....................: $use_xrender"
    echo "    Xcomposite.................: $use_xcomposite"
    echo "    Xfixes.....................: $use_xfixes"
    echo "    Xdamage....................: $use_xdamage"
    echo "    Xdpms......................: $use_xdpms"
    echo "    Xtest......................: $use_xtest"
    echo "    XIM........................: $want_xim"
    echo "    Xi2........................: $use_xi2"
    echo "    Xi2.2......................: $use_xi2_2"
  fi
else
  echo "  Ecore_X....................: $have_ecore_x"
fi
echo "  Ecore_Win32................: $have_win32"
echo "  Ecore_Cocoa................: $have_ecore_cocoa"
echo "  Ecore_SDL..................: $have_ecore_sdl"
echo "  Ecore_FB...................: $have_ecore_fb"
if test "x${have_ecore_fb}" = "xyes" ; then
  echo "    Touchscreen..............: $have_tslib"
fi
echo "  Ecore_WinCE................: $have_wince"
echo "  Ecore_PSL1GHT..............: $have_ps3"
echo "  Ecore_Wayland..............: $have_ecore_wayland"

echo "  Ecore_Evas...................: $have_ecore_evas"
#if test "x${have_ecore_evas}" = "xyes" ; then
  echo "    Software Memory Buffer.....: $have_ecore_evas_software_buffer"
  if test "x${have_ecore_evas_software_x11}" = "xyes" ; then
    echo "    Software X11...............: $have_ecore_evas_software_x11 (Xlib=${have_ecore_evas_software_xlib}) (XCB=${have_ecore_evas_software_xcb})"
  else
    echo "    Software X11...............: $have_ecore_evas_software_x11"
  fi
  if test "x${have_ecore_evas_opengl_x11}" = "xyes" ; then
    echo "    OpenGL X11.................: $have_ecore_evas_opengl_x11 (Xlib=${have_ecore_evas_opengl_xlib}) (XCB=${have_ecore_evas_opengl_xcb})"
  else
    echo "    OpenGL X11.................: $have_ecore_evas_opengl_x11"
  fi
  echo "    Software GDI...............: $have_ecore_evas_software_gdi"
  echo "    Software DirectDraw........: $have_ecore_evas_software_ddraw"
  echo "    Software SDL...............: $have_ecore_evas_software_sdl"
  echo "    OpenGL SDL.................: $have_ecore_evas_opengl_sdl"
  echo "    OpenGL Cocoa...............: $have_ecore_evas_opengl_cocoa"
  echo "    PSL1GHT....................: $have_ecore_evas_psl1ght"
  echo "    Software Framebuffer.......: $have_ecore_evas_fb"
  echo "    Sing.Proc. Windowing System: $have_ecore_evas_ews"
  echo "    Wayland Shm................: $have_ecore_evas_wayland_shm"
  echo "    Wayland Egl................: $have_ecore_evas_wayland_egl"
  echo "    Extn (Plug/socket Extn)....: $have_ecore_evas_extn"
#fi
echo
echo "Eio"
echo "  Inotify..............: ${have_inotify}"
echo "  Windows notification.: ${have_notify_win32}"
echo
echo "Tests..................: ${_efl_enable_tests}"
if test "x${_efl_enable_tests}" = "xyes"; then
echo "    Coverage...........: ${_efl_enable_coverage}"
echo "    Build and run......: make check"
fi
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

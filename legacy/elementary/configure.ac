EFL_VERSION([1], [11], [99], [dev])
AC_INIT([elementary], [efl_version], [enlightenment-devel@lists.sourceforge.net])

AC_PREREQ(2.63)
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_CONFIG_HEADERS([elementary_config.h])
AH_TOP([
#ifndef EFL_CONFIG_H__
#define EFL_CONFIG_H__
])
AH_BOTTOM([
#endif /* EFL_CONFIG_H__ */
])

AC_USE_SYSTEM_EXTENSIONS

AM_INIT_AUTOMAKE([1.11 dist-bzip2 dist-xz -Wall])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LT_INIT([win32-dll disable-static pic-only])
EFL_INIT


### Additional options to configure

# C++ bindings

AC_ARG_ENABLE([cxx-bindings],
   [AS_HELP_STRING([--enable-cxx-bindings],
       [enable C++11 bindings. @<:@default=disabled@:>@])],
   [want_cxx11="${enableval}"],
   [want_cxx11="no"])

# Tests

AC_ARG_WITH([tests],
   [AS_HELP_STRING([--with-tests=none|regular],
                   [choose elementary testing method: regular or none.(coverage will be supported)@<:@default=none@:>@])],
   [build_tests=${withval}],
   [build_tests=none])

# Elementary base dir

AC_ARG_WITH([elementary-base-dir],
   [AS_HELP_STRING([--with-elementary-base-dir=PATH], [specify the subdirectory for all elementary data @<:@default=${elementary_base_dir}@:>@])],
   [elementary_base_dir=${withval}],
   [elementary_base_dir=".elementary"])

AC_MSG_NOTICE([ELEMENTARY_BASE_DIR set to ${elementary_base_dir}])
AC_DEFINE_UNQUOTED([ELEMENTARY_BASE_DIR], ["${elementary_base_dir}"], ["subdirectory for all elementary data"])

# Build and install examples

EFL_CHECK_BUILD_EXAMPLES(
   [enable_build_examples="yes"],
   [enable_build_examples="no"])

EFL_CHECK_INSTALL_EXAMPLES(
   [enable_install_examples="yes"],
   [enable_install_examples="no"])

# Debug mode

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

# quicklaunch support

AC_ARG_ENABLE([quick-launch],
   [AS_HELP_STRING([--disable-quick-launch], [disable quick-launch support, @<:@default=detect@:>@])],
   [want_quicklaunch=$enableval],
   [want_quicklaunch="auto"])


### Default options with respect to host and default values

ELM_VERSION_MAJOR="v_maj"
ELM_VERSION_MINOR="v_min"
AC_SUBST([ELM_VERSION_MAJOR])
AC_SUBST([ELM_VERSION_MINOR])

MODULE_ARCH="v-v_maj.v_min.v_mic"
AC_SUBST([MODULE_ARCH])
AC_DEFINE_UNQUOTED([MODULE_ARCH], ["$MODULE_ARCH"], ["Module architecture"])

ELM_UNIX_DEF="#undef"
ELM_WIN32_DEF="#undef"
have_windows="no"
have_freebsd="no"

case "$host_os" in
   mingw*)
      AC_DEFINE([HAVE_EVIL], [1], [Set to 1 if evil package is installed.])
      ELM_WIN32_DEF="#define"
      have_windows="yes"
      ;;
   *solaris*)
      ELM_UNIX_DEF="#define"
      ;;
   freebsd*)
      ELM_UNIX_DEF="#define"
      have_freebsd="yes"
      ;;
   darwin*)
      ELM_UNIX_DEF="#define"
      ;;
   *)
      ELM_UNIX_DEF="#define"
      ;;
esac

AM_CONDITIONAL([ELEMENTARY_WINDOWS_BUILD], [test "x${have_windows}" = "xyes"])
AM_CONDITIONAL([HAVE_FREEBSD], [test "x${have_freebsd}" = "xyes"])
AC_SUBST([ELM_UNIX_DEF])
AC_SUBST([ELM_WIN32_DEF])

### Checks for programs

# compilers
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_CXX

EFL_CXX_COMPILE_STDCXX_11([ext])

# pkg-config
PKG_PROG_PKG_CONFIG

# gettext
m4_ifdef([AM_GNU_GETTEXT_VERSION], [
AM_GNU_GETTEXT_VERSION([0.18])
])

m4_ifdef([AM_GNU_GETTEXT], [
AM_GNU_GETTEXT([external])
po_makefile_in=po/Makefile.in
have_po="yes"
],[
have_po="no"
])
AC_SUBST([LTLIBINTL])

if test "x${POSUB}" = "x" ; then
   have_po="no"
fi

AM_CONDITIONAL([HAVE_PO], [test "x${have_po}" = "xyes"])

# doxygen
EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])

# EFL binaries
EFL_WITH_BIN([eet], [eet-eet], [eet])
EFL_WITH_BIN([edje], [edje-cc], [edje_cc])
EFL_WITH_BIN([elementary], [elementary-codegen], [elementary_codegen])
EFL_WITH_BIN([elementary], [elm-prefs-cc], [elm_prefs_cc])
EFL_WITH_BIN([eolian], [eolian-gen], [eolian_gen])
EFL_WITH_BIN([eolian-cxx], [eolian-cxx], [eolian_cxx])
# Force the helper to try external eolian generators
AM_CONDITIONAL([HAVE_EOLIAN_GEN], [true])
AM_CONDITIONAL([HAVE_EOLIAN_CXX], [true])

EFL_ENABLE_BIN([elementary-test])
EFL_ENABLE_BIN([elementary-codegen])
EFL_ENABLE_BIN([elm-prefs-cc])
EFL_ENABLE_BIN([elementary-config])


### Checks for libraries

requirement_elm_pc=""

case "$host_os" in
   mingw*)
      requirement_elm_pc="evil >= efl_version"
      ;;
esac

requirement_elm_pc="\
    ${requirement_elm_pc} \
    efl >= efl_version \
    eina >= efl_version \
    eet >= efl_version \
    evas >= efl_version \
    ecore >= efl_version \
    ecore-evas >= efl_version \
    ecore-file >= efl_version \
    ecore-input >= efl_version \
    edje >= efl_version \
    eo >= efl_version \
    ethumb_client >= efl_version \
    emotion >= efl_version \
    ecore-imf >= efl_version \
    ecore-con >= efl_version \
    eio >= efl_version \
    eldbus >= efl_version \
    efreet >= efl_version \
    efreet-mime >= efl_version \
    efreet-trash >= efl_version \
    eio >= efl_version \
    "

ELM_CHECK_BACKEND([X])
ELM_CHECK_BACKEND([FB])
ELM_CHECK_BACKEND([PSL1GHT])
ELM_CHECK_BACKEND([SDL])
ELM_CHECK_BACKEND([Cocoa])
ELM_CHECK_BACKEND([Win32])
ELM_CHECK_BACKEND([Wayland])
ELM_CHECK_BACKEND([DRM])

ELM_CHECK_OPTION([elocation], [0.1.0])
ELM_CHECK_OPTION([eweather], [0.2.0])
# emap is in svn : trunk/PROTO
ELM_CHECK_OPTION([emap], [0.1.0])
ELM_CHECK_OPTION([eweb])

PKG_CHECK_MODULES([ELEMENTARY], [${requirement_elm_pc}])

EFL_ENABLE_EO_API_SUPPORT
EFL_ENABLE_BETA_API_SUPPORT


if test "x${HAVE_CXX11}" = "x1" -a "x${want_cxx11}" = "xyes"; then
   PKG_CHECK_MODULES([ELEMENTARY_CXX],
      [
        eo-cxx >= efl_version
        eina-cxx >= efl_version
        evas-cxx >= efl_version
      ],
      [
        requirement_elm_pc="${requirement_elm_pc} \
          eo-cxx >= efl_version \
          eina-cxx >= efl_version \
          evas-cxx >= efl_version \
        "
        have_cxx11="yes"
        LDFLAGS="${LDFLAGS} -fPIC -DPIC"
        CXXFLAGS="${CXXFLAGS} -fPIC -DPIC"
      ],
      [have_cxx11="no"]
   )
else
   have_cxx11="no"
fi
AM_CONDITIONAL([HAVE_CXX11], [test "x${have_cxx11}" = "xyes"])


MODULE_EDJE=`${PKG_CONFIG} --variable=module_arch edje`
AC_SUBST([MODULE_EDJE])
AC_DEFINE_UNQUOTED([MODULE_EDJE], ["$MODULE_EDJE"], ["Edje module architecture"])

# Needs to be moved into a macro, and also, needs a way to automatically fetch
# from all the dependencies using the Requires.
DEPS_EOLIAN_FLAGS=`${PKG_CONFIG} --variable=eolian_flags eo evas edje ecore efl`
AC_SUBST([DEPS_EOLIAN_FLAGS])

requirement_elm_libs="-lm"

# sockets

case "$host_os" in
   mingw*)
      have_socket="no"
      ;;
   *solaris*)
      AC_CHECK_LIB([socket], [connect],
         [
          have_socket="yes"
          requirement_elm_libs="-lsocket ${requirement_elm_libs}"
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

AM_CONDITIONAL([BUILD_RUN], [test "x$have_socket" = "xyes"])

# dlopen

have_dlopen="no"
case "$host_os" in
   mingw*)
dnl managed by evil
      AC_DEFINE([HAVE_DLADDR], [1], [Define to 1 if you have the `dladdr' function.])
      have_dlopen="yes"
      requirement_elm_libs="-ldl ${requirement_elm_libs}"
      ;;
   *solaris*)
      have_dlopen="yes"
      ;;
   darwin*)
      have_dlopen="yes"
      ;;
   *)
      AC_CHECK_LIB([dl], [dlopen], [res=yes], [res=no])
      if test "x$res" = "xyes"; then
         AC_CHECK_LIB([dl], [dladdr], AC_DEFINE([HAVE_DLADDR], [1], [Define to 1 if you have the `dladdr' function.]))
         requirement_elm_libs="-ldl ${requirement_elm_libs}"
         have_dlopen="yes"
      fi
      ;;
esac

AM_CONDITIONAL([BUILD_MODULES], [test "x$have_dlopen" = "xyes"])

ELEMENTARY_LIBS="${ELEMENTARY_LIBS} ${requirement_elm_libs}"

AC_SUBST([requirement_elm_pc])

# check unit testing library
if test "${build_tests}" = "regular"; then
   PKG_CHECK_MODULES([CHECK], [check >= 0.9.5])
fi
AM_CONDITIONAL([ENABLE_ELEMENTARY_TESTS], [test "${build_tests}" = "regular"])


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

AC_C_BIGENDIAN

m4_ifdef([v_mic],
   [
    EFL_COMPILER_FLAG([-Wshadow])
    EFL_COMPILER_FLAG([-Wall])
    EFL_COMPILER_FLAG([-W])
   ])

EFL_COMPILER_FLAG([-Wall -Wextra -Wpointer-arith -Wno-missing-field-initializers -fvisibility=hidden -fdata-sections -ffunction-sections])

EFL_CHECK_PATH_MAX


### Checks for linker characteristics

EFL_LINKER_FLAG([-fvisibility=hidden -fdata-sections -ffunction-sections -Wl,--gc-sections -fno-strict-aliasing -Wl,--as-needed -Wl,--no-copy-dt-needed-entries])

# Check if we can build binary with quicklaunch support
ELM_QUICKLAUNCH


### Checks for library functions

AC_FUNC_ALLOCA

AC_CHECK_FUNCS([geteuid getuid])

if test "x${want_quicklaunch}" != "xno"; then
   AC_CHECK_FUNCS([fork clearenv])
fi

if test "x${have_dlopen}" = "xno"; then
   AC_CHECK_FUNCS([dlopen dladdr])
fi

if test "x${ac_cv_func_dlopen}" = "xyes"; then
   have_dlopen="yes"
fi

AM_CONDITIONAL([BUILD_QUICKLAUNCH], [test "x${ac_cv_func_fork}" = "xyes" && test "x${have_dlopen}" = "xyes"])

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


AC_CONFIG_FILES([
Makefile
elementary.spec
doc/Makefile
doc/Doxyfile
doc/widgets/Makefile
src/Makefile
src/lib/Makefile
src/lib/Elementary.h
src/lib/elm_intro.h
src/bin/Makefile
src/modules/Makefile
src/modules/prefs/Makefile
src/modules/access_output/Makefile
src/modules/datetime_input_ctxpopup/Makefile
src/modules/test_entry/Makefile
src/modules/test_map/Makefile
src/edje_externals/Makefile
src/examples/Makefile
src/tests/Makefile
src/imported/Makefile
src/imported/atspi/Makefile
data/Makefile
data/themes/Makefile
data/images/Makefile
data/objects/Makefile
data/desktop/Makefile
data/edje_externals/Makefile
config/Makefile
config/default/Makefile
config/default/base.src
config/standard/Makefile
config/standard/base.src
config/mobile/Makefile
config/mobile/base.src
$po_makefile_in
cmakeconfig/ElementaryConfig.cmake
cmakeconfig/ElementaryConfigVersion.cmake
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
echo "Configuration Options Summary:"
echo
echo "  Engines:"
echo "    X11....................: ${have_elementary_x}"
echo "    Framebuffer............: ${have_elementary_fb}"
echo "    DRM....................: ${have_elementary_drm}"
echo "    PSL1GHT................: ${have_elementary_psl1ght}"
echo "    SDL....................: ${have_elementary_sdl}"
echo "    Cocoa..................: ${have_elementary_cocoa}"
echo "    Windows XP.............: ${have_elementary_win32}"
echo "    Wayland................: ${have_elementary_wayland}"
echo
echo "  Features:"
echo "    EWeather...............: ${have_elementary_eweather}"
echo "    EMap...................: ${have_elementary_emap}"
echo "    Web (WebKit)...........: ${elementary_web}"
echo "    Quick Launch...........: ${ac_cv_func_fork}"
echo "    Elocation..............: ${have_elementary_elocation}"
echo
echo "  eet......................: ${eet_eet}"
echo "  edje_cc..................: ${edje_cc}"
echo "  elementary_codegen.......: ${elementary_codegen}"
echo "  elm_prefs_cc.............: ${elm_prefs_cc}"
echo "  eolian_gen...............: ${eolian_gen}"
echo "  eolian_cxx...............: ${eolian_cxx}"
echo
echo "  Build elementary_test....: ${have_elementary_test}"
echo "  Build elementary_codegen.: ${have_elementary_codegen}"
echo "  Build elm_prefs_cc.......: ${have_elm_prefs_cc}"
echo "  Examples.................: ${enable_build_examples}"
echo "  Examples installed.......: ${enable_install_examples}"
echo "  Build elementary_config..: ${have_elementary_config}"
echo "  Enable debug.............: ${want_elementary_debug}"
echo "  Enable C++...............: ${have_cxx11}"
echo
echo "Compilation................: make (or gmake)"
echo "  CPPFLAGS.................: $CPPFLAGS"
echo "  CFLAGS...................: $CFLAGS"
if test "x${have_cxx11}" = "xyes"; then
echo "  CXXFLAGS.................: $CXXFLAGS"
fi
echo "  LDFLAGS..................: $LDFLAGS"
echo
if test "${build_tests}" = "none"; then
echo "Tests......................: no"
else
echo "Tests......................: make check"
fi
if test "${build_doc}" = "no"; then
echo "Docs.......................: no (doxygen not found)"
else
echo "Docs.......................: make doc"
fi
echo
echo "Installation...............: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...................: $prefix"
echo

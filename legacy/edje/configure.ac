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

AC_INIT([edje], [v_ver], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.52])
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

AC_DEFINE_UNQUOTED([SHARED_LIB_SUFFIX], ["$shrext_cmds"], [Suffix for shared objects])

### Default options with respect to host
AC_CANONICAL_BUILD
AC_CANONICAL_HOST


case "$host_os" in
   mingw32ce* | cegcc*)
      MODULE_ARCH="$host_os-$host_cpu"
      install_vim="no"
      have_edje_cc="no"
      ;;
   *)
      MODULE_ARCH="$host_os-$host_cpu-v_maj.0.0"
      install_vim="yes"
      have_edje_cc="yes"
      ;;
esac

requirement_edje=""

AC_SUBST(MODULE_ARCH)
AC_DEFINE_UNQUOTED(MODULE_ARCH, "${MODULE_ARCH}", "Module architecture")

### Additional options to configure

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
AM_CONDITIONAL([EDJE_AMALGAMATION], [test "x${do_amalgamation}" = "xyes"])

EFL_ENABLE_BIN([edje-cc])
EFL_ENABLE_BIN([edje-decc])
EFL_ENABLE_BIN([edje-recc])
EFL_ENABLE_BIN([edje-player])
EFL_ENABLE_BIN([edje-inspector])
EFL_ENABLE_BIN([edje-external-inspector])

# Optional EDJE_PROGRAM_CACHE (use much more ram, but increase speed in some cases)
want_edje_program_cache="no"

AC_ARG_ENABLE([edje-program-cache],
   [AC_HELP_STRING(
       [--enable-edje-program-cache],
       [enable EDJE_PROGRAM_CACHE support. [[default=disabled]]]
    )],
   [want_edje_program_cache=$enableval]
)
AM_CONDITIONAL([EDJE_PROGRAM_CACHE], [test "x${want_edje_program_cache}" = "xyes"])

if test "x${want_edje_program_cache}" = "xyes" ; then
   AC_DEFINE([EDJE_PROGRAM_CACHE], [1], [Cache result of program glob matches - this uses up extra ram with the gain of faster program matching])
fi

# Optional EDJE_CALC_CACHE (use more ram, but increase speed by reducing the need to recalculate static stuff)
want_edje_calc_cache="yes"

AC_ARG_ENABLE([edje-calc-cache],
   [AC_HELP_STRING(
       [--enable-edje-calc-cache],
       [enable EDJE_CALC_CACHE support. [[default=disabled]]]
    )],
   [want_edje_calc_cache=$enableval]
)
AM_CONDITIONAL([EDJE_CALC_CACHE], [test "x${want_edje_calc_cache}" = "xyes"])

if test "x${want_edje_calc_cache}" = "xyes" ; then
   AC_DEFINE([EDJE_CALC_CACHE], [1], [Cache result of edje_part_recalc - this uses up extra ram with the gain of reducing CPU usage when edje object are not resized])
fi

# Enable Fixed Point use
want_fixed_point="no"

AC_ARG_ENABLE([fixed-point],
   [AC_HELP_STRING(
       [--enable-fixed-point],
       [reduce use of FPU by using Fixed Point provided by Eina and Eet, [[default=disabled]]]
    )],
    [want_fixed_point=$enableval]
)
AM_CONDITIONAL([BUILD_EDJE_FP], [test "x${want_fixed_point}" = "xyes"])

if test "x${want_fixed_point}" = "xyes" ; then
   AC_DEFINE([BUILD_EDJE_FP], [1], [Use Fixed Point instead of FPU])
fi

install_vim="yes"
AC_ARG_WITH([vim],
    [AC_HELP_STRING([--with-vim=DIR], [Location of Vim data files [[autodetect]]])],
    [
     if test -d "${withval}"; then
        vimdir="${withval}"
     fi
    ]
)

if test "x${vimdir}" = "x" ; then
   if test -d "${prefix}/share/vim"; then
      vimdir="${prefix}/share/vim"
   elif test -d "/usr/share/vim"; then
      vimdir="/usr/share/vim"
   elif test -d "/usr/local/share/vim"; then
      vimdir="/usr/local/share/vim"
   elif test -d "/opt/share/vim"; then
      vimdir="/opt/share/vim"
   else
      install_vim="no"
   fi
fi

AC_MSG_CHECKING([for location of Vim data files])

if test "${install_vim}" = "yes"; then
   AC_MSG_RESULT([$vimdir])
else
   AC_MSG_RESULT([Not found, EDC syntax file will not be installed])
fi

AC_SUBST(vimdir)

### Checks for programs
AC_PROG_CC

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])

# python

AM_PATH_PYTHON([2.5], , [:])
AM_CONDITIONAL([HAVE_PYTHON], [test "$PYTHON" != : ])

# pkg-config

PKG_PROG_PKG_CONFIG

# Check whether pkg-config supports Requires.private

if $PKG_CONFIG --atleast-pkgconfig-version 0.22; then
   pkgconfig_requires_private="Requires.private"
else
   pkgconfig_requires_private="Requires"
fi
AC_SUBST(pkgconfig_requires_private)


### Checks for libraries

# Evil library for compilation on Windows

EFL_EDJE_BUILD=""
case "$host_os" in
   mingw* | cegcc*)
      PKG_CHECK_MODULES([EVIL], [evil])
      AC_DEFINE([HAVE_EVIL], [1], [Set to 1 if Evil library is installed])
      requirement_edje="evil ${requirement_edje}"
      EFL_EDJE_BUILD="-DEFL_EDJE_BUILD"
   ;;
esac
AC_SUBST(EFL_EDJE_BUILD)

# Dependencies for the library

# Lua pkg-config hack for different naming conventions
have_lua="yes"
requirement_lua=""
lua_libs=""
PKG_CHECK_MODULES([LUA], [lua >= 5.1.0], requirement_lua="lua",
   [PKG_CHECK_MODULES([LUA], [lua5.1 >= 5.1.0], requirement_lua="lua5.1",
      [PKG_CHECK_MODULES([LUA], [lua-5.1 >= 5.1.0], requirement_lua="lua-5.1",
         [PKG_CHECK_MODULES([LUA], [lua51 >= 5.1.0], requirement_lua="lua51",
            [have_lua="no"])])])])

if test "x${have_lua}" = "xno" ; then
   AC_MSG_CHECKING([whether lua_newstate() is in liblua])
   AC_CHECK_LIB([lua], [lua_newstate],
      [
       have_lua="yes"
       lua_libs="-llua"
      ],
      [have_lua="no"])
   AC_MSG_RESULT([${have_lua}])
fi
AC_SUBST(lua_libs)

if test "x${have_lua}" = "xno" ; then
   AC_MSG_ERROR([Cannot find lua library. Be sure that you have installed it and set CFLAGS and LDFLAGS correctly])
fi

PKG_CHECK_MODULES([MINIMAL],
   [
    eina >= 1.0.0
    eet >= 1.4.0
   ])

PKG_CHECK_MODULES([EDJE],
   [
    eina >= 1.0.0
    eet >= 1.4.0
    evas >= 1.0.999
    ecore >= 1.0.0
    ecore-evas >= 1.0.0
    ecore-file >= 1.0.0
    embryo >= 1.0.0
   ])

EDJE_LIBS="${EDJE_LIBS} ${LUA_LIBS} ${lua_libs}"
EDJE_CFLAGS="${EDJE_CFLAGS} ${LUA_CFLAGS}"

requirement_edje="embryo >= 1.0.0 ecore-evas >= 1.0.0 ecore >= 1.0.0 evas >= 1.0.0 eet >= 1.4.0 eina >= 1.0.0 ${requirement_edje}"
requirement_edje="${requirement_lua} ${requirement_edje}"

have_ecore_imf="no"
PKG_CHECK_MODULES([ECORE_IMF],
   [
    ecore-imf >= 1.0.0
    ecore-imf-evas >= 1.0.0
   ],
   [
    AC_DEFINE([HAVE_ECORE_IMF], [1], [Input Method Support for Edje Entry])
    have_ecore_imf="yes"
    requirement_edje="ecore-imf-evas >= 1.0.0 ecore-imf >= 1.0.0 ${requirement_edje}"
   ],
   [have_ecore_imf="no"])

# Enable Multisense use
want_multisense="yes"
AC_ARG_ENABLE([multisense],
   [AC_HELP_STRING(
       [--enable-multisense],
       [multisense provides sound. tone and haptic effects support, [[default=disabled]]]
    )],
    [want_multisense=$enableval]
)
AM_CONDITIONAL([ENABLE_MULTISENSE], [test "x${want_multisense}" = "xyes"])

##sndfile library
have_sndfile="no"
want_sndfile="auto"
AC_ARG_ENABLE([sndfile],
   [AC_HELP_STRING([--disable-sndfile], [disable sndfile support. @<:@default=detect@:>@])],
   [want_sndfile=$enableval], [])

if test "x${want_multisense}" = "xyes" -a  "x$want_sndfile" != "xno"; then

	   PKG_CHECK_MODULES([SNDFILE],
	      [sndfile >= 1.0.22],
	      [
	       AC_DEFINE(HAVE_LIBSNDFILE, 1, [sndfile support for Edje])
	       have_sndfile="yes"
	       requirement_edje="sndfile >= 1.0.22 ${requirement_edje}"
	       ],
	      [have_sndfile="no"; want_multisense="no"]
	   )
	
	if test "x$want_sndfile" = "xyes" -a "x$have_sndfile" = "xno"; then
	    AC_MSG_ERROR([sndfile support requested, but not found by pkg-config.])
	fi
fi
AM_CONDITIONAL([HAVE_LIBSNDFILE], [test "x${have_sndfile}" = "xyes"])

##libremix library
have_libremix="no"
want_libremix="auto"
AC_ARG_ENABLE([remix],
   [AC_HELP_STRING([--disable-remix], [disable remix support. @<:@default=detect@:>@])],
   [want_libremix=$enableval], [])

if test "x${want_multisense}" = "xyes" -a "x$want_libremix" != "xno"; then
   PKG_CHECK_MODULES([REMIX],
      [remix >= 0.2.4],
      [
       AC_DEFINE(HAVE_LIBREMIX, 1, [remix support for Edje])
       have_libremix="yes"
       requirement_edje="remix >= 0.2.4 ${requirement_edje}"
       AC_DEFINE(__REMIX_PLUGIN__, 1, "Set to REMIX Plugin type")
       REMIX_PLUGIN_DIR=`pkg-config --variable=libdir remix`"/remix"
       AC_SUBST(REMIX_PLUGIN_DIR)
       AC_DEFINE_UNQUOTED(REMIX_PLUGIN_DIR, "$REMIX_PLUGIN_DIR", [Set the remix plugin directory])
       ],
      [have_libremix="no"; want_multisense="no"]
   )

	if test "x$want_libremix" = "xyes" -a "x$have_libremix" = "xno"; then
	    AC_MSG_ERROR([remix support requested, but not found by pkg-config.])
	fi
fi
AM_CONDITIONAL([HAVE_LIBREMIX], [test "x${have_libremix}" = "xyes"])

##vorbis/ogg library
have_vorbis="no"
want_vorbis="auto"
AC_ARG_ENABLE([vorbisenc],
   [AC_HELP_STRING([--disable-vorbis], [disable ogg-vorbis support. @<:@default=detect@:>@])],
   [want_vorbis=$enableval], [])

if test "x${want_multisense}" = "xyes" -a "x$want_vorbis" != "xno"; then
   PKG_CHECK_MODULES([VORBISENC],
      [
       ogg >= 1.1.4
       vorbis >= 1.2.3
       vorbisenc >= 1.2.3
      ],
      [
       AC_DEFINE(HAVE_VORBIS, 1, [vorbis support for Edje])
       have_vorbis="yes"
       requirement_edje="ogg >= 1.1.4 vorbis >= 1.2.3 vorbisenc >= 1.2.3 ${requirement_edje}"
      ],
      [have_vorbis="no"; want_multisense="no"]
   )

	if test "x$want_vorbis" = "xyes" -a "x$have_vorbis" = "xno"; then
	    AC_MSG_ERROR([vorbisenc support requested, but not found by pkg-config.])
	fi
fi

##alsa library
have_alsa_lib="no"
want_alsa_lib="auto"
AC_ARG_ENABLE([flac],
   [AC_HELP_STRING([--disable-alsa], [disable alsa support. @<:@default=detect@:>@])],
   [want_alsa_lib=$enableval], [])

if test "x${want_multisense}" = "xyes" -a "x$want_alsa_lib" != "xno"; then
   PKG_CHECK_MODULES([ALSA],
      [
       alsa >= 1.0.21a
      ],
      [
       AC_DEFINE(HAVE_LIBALSA, 1, [ALSA support for Edje])
       have_alsa_lib="yes"
       requirement_edje="alsa >= 1.0.21a ${requirement_edje}"
      ],
      [have_alsa_lib="no"; want_multisense="no"]
   )

	if test "x$want_alsa_lib" = "xyes" -a "x$have_alsa_lib" = "xno"; then
	    AC_MSG_ERROR([alsa support requested, but not found by pkg-config.])
	fi
fi	
AM_CONDITIONAL([HAVE_LIBALSA], [test "x${have_alsa_lib}" = "xyes"])


##flac library
have_flac_lib="no"
want_flac_lib="auto"
AC_ARG_ENABLE([flac],
   [AC_HELP_STRING([--disable-flac], [disable flac support. @<:@default=detect@:>@])],
   [want_flac_lib=$enableval], [])

if test "x${want_multisense}" = "xyes" -a "x$want_flac_lib" != "xno"; then
   PKG_CHECK_MODULES([FLAC],
      [
       flac >= 1.2.1
      ],
      [
       AC_DEFINE(HAVE_LIBFLAC, 1, [flac support for Edje])
       have_flac_lib="yes"
       # disabled because this adds flac include dirs to cflags... and
       # FLAC has an assert.h in its include dirs that ends up
       # overriding the system assert.h and thus causes all sorts of
       # mayhem
       #requirement_edje="flac >= 1.2.1 ${requirement_edje}"
      ],
      [have_flac_lib="no"; want_multisense="no"]
   )
	if test "x$want_flac_lib" = "xyes" -a "x$have_flac_lib" = "xno"; then
	    AC_MSG_ERROR([flac support requested, but not found by pkg-config.])
	fi
fi

if test "x${want_multisense}" = "xyes" ; then
   AC_DEFINE([ENABLE_MULTISENSE], [1], [Use Multisense])
fi

# Dependencies for the binaries

if test "x$have_edje_cc" = "xyes"; then
   PKG_CHECK_MODULES([EDJE_CC], [ecore-file >= 1.0.0 ecore-evas >= 1.0.0 ecore >= 1.0.0 evas >= 1.0.0 eet >= 1.0.0 eina >= 1.0.0])
   EDJE_CC_LIBS="${EDJE_CC_LIBS} ${LUA_LIBS} ${lua_libs}"
fi

AM_CONDITIONAL([BUILD_EPP], [test "x$have_edje_cc" = "xyes"])

if test "x$have_edje_decc" = "xyes"; then
   PKG_CHECK_MODULES([EDJE_DECC], [ecore-file >= 1.0.0 ecore-evas >= 1.0.0 ecore >= 1.0.0 evas >= 1.0.0 eet >= 1.0.0 eina >= 1.0.0])
fi

if test "x$have_edje_player" = "xyes"; then
   PKG_CHECK_MODULES([EDJE_PLAYER], [ecore-evas >= 1.0.0 ecore >= 1.0.0 evas >= 1.0.0 eina >= 1.0.0])
fi

if test "x$have_edje_inspector" = "xyes"; then
   PKG_CHECK_MODULES([EDJE_INSPECTOR], [ecore-evas >= 1.0.0 ecore >= 1.0.0 evas >= 1.0.0 eina >= 1.0.0])
fi

if test "x$have_edje_external_inspector" = "xyes"; then
   PKG_CHECK_MODULES([EDJE_EXTERNAL_INSPECTOR], [ecore >= 1.0.0 evas >= 1.0.0 eina >= 1.0.0])
fi


### Checks for header files
AC_CHECK_HEADERS([locale.h sys/resource.h])

EFL_CHECK_PATH_MAX


### Checks for types


### Checks for structures


### Checks for compiler characteristics
AM_PROG_CC_C_O
AC_C_CONST
AC_PROG_CC_STDC
AC_HEADER_STDC
AC_C___ATTRIBUTE__

case "$host_os" in
   mingw32ce*)
      EDJE_CFLAGS="${EDJE_CFLAGS} -D_WIN32_WCE=0x0420"
      ;;
   cegcc*)
      EDJE_CFLAGS="${EDJE_CFLAGS} -mwin32 -D_WIN32_WCE=0x0420"
      ;;
esac


### Checks for linker characteristics

lt_enable_auto_import=""
case "$host_os" in
   mingw* | cegcc*)
      lt_enable_auto_import="-Wl,--enable-auto-import"
      ;;
esac
AC_SUBST(lt_enable_auto_import)


### Checks for library functions

AC_ISC_POSIX

AC_FUNC_ALLOCA

case "$host_os" in
   mingw* | cegcc*)
      AC_DEFINE([HAVE_REALPATH], [1], [Define to 1 if you have the `realpath' function.])
      ;;
   *)
      AC_CHECK_FUNCS([realpath])
      ;;
esac

### Unit tests, coverage and benchmarking

EFL_CHECK_TESTS([enable_tests="yes"], [enable_tests="no"])

EFL_CHECK_COVERAGE([${enable_tests}], [enable_coverage="yes"], [enable_coverage="no"])
EDJE_CFLAGS="${EDJE_CFLAGS} ${EFL_COVERAGE_CFLAGS}"
EDJE_LIBS="${EDJE_LIBS} ${EFL_COVERAGE_LIBS}"
if test "x$enable_coverage" = "xyes" ; then
   EDJE_CFLAGS="${EDJE_CFLAGS} ${EFL_DEBUG_CFLAGS}"
fi

## Examples

install_examples="yes"
AC_ARG_ENABLE([install-examples],
   AC_HELP_STRING([--disable-install-examples],
                  [disable installing examples (compiled or just source).
                   @<:@default==enabled@:>@]),
   [
    if test "x${enableval}" = "xyes" ; then
       install_examples="yes"
    else
       install_examples="no"
    fi
   ],
   [install_examples="yes"])
AM_CONDITIONAL([INSTALL_EXAMPLES], [test "x${install_examples}" = "xyes"])

build_examples="no"
AC_ARG_ENABLE([build-examples],
   AC_HELP_STRING([--enable-build-examples],
                  [Enable building examples. This requires extra denpendencies. If you don't have them installed yet, don't build with this option or it will fail. Build again when you have it, overriding the previous installation). @<:@default==disabled@:>@]),
   [
    if test "x${enableval}" = "xyes" ; then
       build_examples="yes"
       edje_cc=$($PKG_CONFIG --variable=prefix edje)/bin/edje_cc
       # put in here the dependencies for Edje' examples. They are
       # meant to be 'real world' usage examples, thus one will be
       # using higher level libraries on these programs
       AC_SUBST(edje_cc)
    else
       build_examples="no"
    fi
   ],
   [build_examples="no"])
AM_CONDITIONAL([BUILD_EXAMPLES], [test "x${build_examples}" = "xyes"])

AC_SUBST(requirement_edje)

AC_OUTPUT([
edje.pc
edje.spec
Makefile
data/Makefile
data/include/Makefile
doc/Makefile
doc/Doxyfile
src/Makefile
src/lib/Makefile
src/bin/Makefile
src/bin/epp/Makefile
src/modules/Makefile
src/modules/alsa_snd_player/Makefile
src/modules/eet_snd_reader/Makefile
src/modules/multisense_factory/Makefile
src/tests/Makefile
utils/Makefile
src/examples/Makefile
])


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
echo "  Amalgamation.........: ${do_amalgamation}"
echo "  Ecore IMF............: $have_ecore_imf"
echo "  Multisense...........: $want_multisense"

if test "x${want_multisense}" = "xyes" ; then
echo "  LibRemix.............: $have_libremix"
echo "  Libsndfile...........: $have_sndfile"
echo "  Ogg/Vorbis...........: $have_vorbis"
echo "  LibFLAC..............: $have_flac_lib"
echo "  LibALSA..............: $have_alsa_lib"
fi

echo "  EDJE_PROGRAM_CACHE...: $want_edje_program_cache"
echo "  EDJE_CALC_CACHE......: $want_edje_calc_cache"
echo "  Fixed point..........: $want_fixed_point"
echo "  Documentation........: ${build_doc}"
echo "  Tests................: ${enable_tests}"
echo "  Coverage.............: ${enable_coverage}"
echo "  Examples.............: install:${install_examples} build:${build_examples}"
echo
echo "Programs:"
echo
echo "  Build edje_cc................: $have_edje_cc"
echo "  Build edje_decc..............: $have_edje_decc"
echo "  Build edje_recc..............: $have_edje_recc"
echo "  Build edje_player............: $have_edje_player"
echo "  Build edje_inspector.........: $have_edje_inspector"
echo "  Build edje_external_inspector: $have_edje_external_inspector"
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo

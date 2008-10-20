# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT([eina], [0.0.1], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.60])
AC_CONFIG_SRCDIR(configure.ac)
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_ISC_POSIX

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

VMAJ=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $1);}'`
VMIN=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $2);}'`
VMIC=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $3);}'`
SNAP=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $4);}'`
version_info=`expr $VMAJ + $VMIN`":$VMIC:$VMIN"
AC_SUBST(VMAJ)
AC_SUBST(version_info)


### Needed information

MODULE_ARCH="$host_os-$host_cpu"
AC_SUBST(MODULE_ARCH)
AC_DEFINE_UNQUOTED(MODULE_ARCH, "${MODULE_ARCH}", "Module architecture")

EFL_CHECK_CPU_MMX([have_mmx="yes"], [have_mmx="no"])
EFL_CHECK_CPU_SSE([have_sse="yes"], [have_sse="no"])
EFL_CHECK_CPU_SSE2([have_sse2="yes"], [have_sse2="no"])
EFL_CHECK_CPU_ALTIVEC([have_altivec="yes"], [have_altivec="no"])

EFL_CHECK_PTHREAD([have_pthread="yes"], [have_pthread="no"])


### Additional options to configure

# Magic debug
AC_ARG_ENABLE([magic-debug],
   [AC_HELP_STRING([--enable-magic-debug], [enable magic debug of eina structure])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_magic_debug="yes"
    else
       enable_magic_debug="no"
    fi
   ],
   [enable_magic_debug="yes"]
)
AC_MSG_CHECKING([whether magic debug is enable])
AC_MSG_RESULT([${enable_magic_debug}])

EINA_MAGIC_DEBUG=0
if test "x${enable_magic_debug}" = "xyes" ; then
   EINA_MAGIC_DEBUG=1
fi
AC_SUBST(EINA_MAGIC_DEBUG)

# Choose best memory pool
AC_ARG_ENABLE([default-mempool],
   [AC_HELP_STRING([--enable-default-mempool], [Default memory allocator could be faster for some computer])],
   [
    if test "x${enableval}" = "xyes"; then
       enable_default_mempool="yes"
    else
       enable_default_mempool="no"
    fi
   ],
   [enable_default_mempool="no"]
)
AC_MSG_CHECKING([whether to use default mempool allocator])
AC_MSG_RESULT([${enable_default_mempool}])

EINA_DEFAULT_MEMPOOL=0
if test "x${enable_default_mempool}" = "xyes" ; then
   EINA_DEFAULT_MEMPOOL=1
fi
AC_SUBST(EINA_DEFAULT_MEMPOOL)

# Report stringshare usage
AC_ARG_ENABLE([stringshare-usage],
   [AC_HELP_STRING([--enable-stringshare-usage], [Report stringshare usage on stringshare shutdown])],
   [
    if test "x${enableval}" = "xyes"; then
       enable_stringshare_usage="yes"
    else
       enable_stringshare_usage="no"
    fi
   ],
   [enable_stringshare_usage="no"]
)
AC_MSG_CHECKING([whether to report stringshare usage])
AC_MSG_RESULT([${enable_stringshare_usage}])

if test "x${enable_stringshare_usage}" = "xyes"; then
   AC_DEFINE(EINA_STRINGSHARE_USAGE, 1, [Report Eina stringshare usage pattern])
fi

# Ememoa memory pool

AC_ARG_ENABLE([ememoa],
   [AC_HELP_STRING([--enable-ememoa], [build ememoa memory pool module @<:@default=yes@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_ememoa="yes"
    else
       enable_ememoa="no"
    fi
   ],
   [enable_ememoa="yes"]
)
AC_MSG_CHECKING([whether to use ememoa for memory pool])
AC_MSG_RESULT([$enable_ememoa])

# Static linking

EINA_CHECK_STATIC([chained-pool], [chained pool])
EINA_CHECK_STATIC([ememoa-fixed], [ememoa fixed])
EINA_CHECK_STATIC([ememoa-unknown], [ememoa unknown])
EINA_CHECK_STATIC([pass-through], [pass through])
EINA_CHECK_STATIC([fixed-bitmap], [fixed bitmap])

# Unit tests, coverage and benchmarking

EFL_CHECK_TESTS([enable_tests="yes"], [enable_tests="no"])
EFL_CHECK_COVERAGE([${enable_tests}], [enable_coverage="yes"], [enable_coverage="no"])
EFL_CHECK_BENCHMARK([enable_benchmark="yes"], [enable_benchmark="no"])


### Checks for libraries

PKG_PROG_PKG_CONFIG

# Evil library for compilation on Windows CE

case "$host_os" in
   mingw* | cegcc*)
   PKG_CHECK_MODULES([EVIL], [evil])
   AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if evil package is installed])
   ;;
esac

# Glib library for comparison in the benchmark

have_glib="no"
if test "x${enable_benchmark}" = "xyes" ; then
   PKG_CHECK_MODULES([GLIB],
      [glib-2.0],
      [have_glib="yes"],
      [have_glib="no"]
   )
fi

AM_CONDITIONAL(EINA_HAVE_GLIB, test "x${have_glib}" = "xyes")

# Evas library for comparison in the benchmark

have_evas="no"
if test "x${enable_benchmark}" = "xyes" ; then
   PKG_CHECK_MODULES([EVAS],
      [evas],
      [have_evas="yes"],
      [have_evas="no"]
   )
fi

AM_CONDITIONAL(EINA_HAVE_EVAS, test "x${have_evas}" = "xyes")

# Ecore library for comparison in the benchmark

have_ecore="no"
if test "x${enable_benchmark}" = "xyes" ; then
   PKG_CHECK_MODULES([ECORE],
      [ecore],
      [have_ecore="yes"],
      [have_ecore="no"]
   )
fi

AM_CONDITIONAL(EINA_HAVE_ECORE, test "x${have_ecore}" = "xyes")

# Check if we want to benchmark on real data
enable_benchmark_e17="no"
AC_ARG_ENABLE([e17],
   [AC_HELP_STRING([--enable-e17], [disable heavy benchmark @<:@default=no@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_benchmark_e17="yes"
    else
       enable_benchmark_e17="no"
    fi
   ],
   [enable_benchmark_e17="no"]
)
AC_MSG_CHECKING([whether e17 real data benchmark are built])
AC_MSG_RESULT([${enable_benchmark_e17}])
AM_CONDITIONAL(EINA_ENABLE_BENCHMARK_E17, test "x${enable_benchmark_e17}" = "xyes")

# Check ememoa memory pool library

if test "x${enable_ememoa}" = "xyes" ; then
   PKG_CHECK_MODULES([EMEMOA],
      [ememoa >= 0.0.26 ],
      [enable_ememoa="yes"],
      [enable_ememoa="no"]
   )
fi

AM_CONDITIONAL(EINA_ENABLE_EMEMOA, test "x${enable_ememoa}" = "xyes")
if test "x${enable_ememoa}" = "xyes"; then
   AC_DEFINE(EINA_EMEMOA_SUPPORT, 1, [Use by the test suite to try ememoa mempool])
fi

### Checks for header files
AC_HEADER_ASSERT
AC_HEADER_DIRENT
AC_HEADER_TIME


### Checks for programs
AC_PROG_CC

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])


### Checks for types


### Checks for structures


### Checks for compiler characteristics
AC_C_CONST
AC_C_BIGENDIAN
AC_C_INLINE
AC_C___ATTRIBUTE__
AC_PROG_CC_STDC

EINA_CFLAGS="${EFL_COVERAGE_CFLAGS}"
case "${host_os}" in
   cegcc*)
      EINA_CFLAGS="${EINA_CFLAGS} ${EVIL_CFLAGS} -mwin32"
   ;;
   mingw*)
      EINA_CFLAGS="${EINA_CFLAGS} ${EVIL_CFLAGS}"
   ;;
esac

if test "x$enable_coverage" = "xyes" ; then
   EINA_CFLAGS="${EINA_CFLAGS} ${EFL_DEBUG_CFLAGS}"
fi

if ! test "${VMIC}" = "x" ; then
   EINA_CFLAGS="${EINA_CFLAGS} -Wall -W -Wextra" # -Werror
fi

AC_SUBST(EINA_CFLAGS)


### Checks for linker characteristics
EINA_LIBS="${EFL_COVERAGE_LIBS}"
lt_enable_auto_import=""
case "${host_os}" in
   mingw* | cegcc*)
      AC_DEFINE(EFL_EINA_BUILD, 1, [Define to mention that eina is built])
      EINA_LIBS="${EINA_LIBS} ${EVIL_LIBS} -lm"
      lt_enable_auto_import="-Wl,--enable-auto-import"
   ;;
   *)
      EINA_LIBS="${EINA_LIBS} -ldl -lrt -lm"
   ;;
esac
AC_SUBST(EINA_LIBS)
AC_SUBST(lt_enable_auto_import)


### Checks for library functions
AC_FUNC_ALLOCA


### Create the .pc.in file according to the major version
cat > ${srcdir}/eina-${VMAJ}.pc.in << EOF
prefix=@prefix@
exec_prefix=@exec_prefix@
libdir=@libdir@
includedir=@includedir@

Name: Eina
Description: A Library that implements fast data types and miscellaneous tools
Requires:
Version: @VERSION@
Libs: -L${libdir} -leina
Libs.private: -ldl
Cflags: -I${includedir}/eina-@VMAJ@ -I${includedir}/eina-@VMAJ@/eina
EOF


### Make the debug preprocessor configurable

AC_CONFIG_FILES([
Makefile
eina-0.pc
doc/Makefile
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
src/tests/Makefile
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
echo "  Documentation........: ${build_doc}"
echo "  Tests................: ${enable_tests}"
echo "  Coverage.............: ${enable_coverage}"
echo "  Benchmark............: ${enable_benchmark}"
if test "x${enable_benchmark}" = "xyes" ; then
echo "    Glib...............: ${have_glib}"
echo "    Evas...............: ${have_evas}"
echo "    Ecore..............: ${have_ecore}"
echo "    E17 real data......: ${enable_benchmark_e17}"
fi
echo
echo "CPU Specific Extensions:"
echo
echo "  MMX..................: ${have_mmx}"
echo "  SSE..................: ${have_sse}"
echo "  SSE2.................: ${have_sse2}"
echo "  ALTIVEC..............: ${have_altivec}"
echo
echo "  Thread Support.......: ${have_pthread}"
echo
echo "  Magic debug..........: ${enable_magic_debug}"
echo
echo "  Report string usage..: ${enable_stringshare_usage}"
echo
echo "  Default mempool......: ${enable_default_mempool}"
echo
echo "  Memory pool:"
echo "    Ememoa.............: ${enable_ememoa}"
echo
echo "  Static build of memory pools:"
echo "    Chained pool.......: ${enable_static_chained_pool}"
echo "    Ememoa fixed.......: ${enable_static_ememoa_fixed}"
echo "    Ememoa unknown.....: ${enable_static_ememoa_unknown}"
echo "    Pass through.......: ${enable_static_pass_through}"
echo
echo "  Installation.........: make install"
echo
echo "    prefix.............: $prefix"
echo
echo "Now type 'make' ('gmake' on some systems) to compile $PACKAGE,"
echo "and then afterwards as root (or the user who will install this), type"
echo "'make install'. Change users with 'su' or 'sudo' appropriately."
echo

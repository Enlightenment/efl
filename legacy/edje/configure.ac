dnl Process this file with autoconf to produce a configure script.

# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT([edje], [0.9.9.050], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
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
AC_SUBST(version_info)


### Default options with respect to host

case "$host_os" in
   mingw32ce* | cegcc*)
      install_vim="no"
      have_edje_cc="no"
      ;;
   *)
      install_vim="yes"
      have_edje_cc="yes"
      ;;
esac

want_edje_program_cache="no"

requirement_edje=""


### Additional options to configure

AC_ARG_ENABLE([edje-cc],
   [AC_HELP_STRING([--disable-edje-cc], [disable building of edje_cc])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_edje_cc="yes"
    else
       have_edje_cc="no"
    fi
   ]
)
AC_MSG_CHECKING(whether to build edje_cc)
AC_MSG_RESULT([${have_edje_cc}])

AM_CONDITIONAL(BUILD_EDJE_CC, test "x${have_edje_cc}" = "xyes")
AM_CONDITIONAL(BUILD_EDJE_DECC, test "x${have_edje_cc}" = "xyes")

EDJE_CC_PRG=""
EDJE_DECC_PRG=""
EDJE_RECC_PRG=""
if test "x${have_edje_cc}" = "xyes"; then
   EDJE_CC_PRG="edje_cc"
   EDJE_DECC_PRG="edje_decc"
   EDJE_RECC_PRG="edje_recc"
fi
AC_SUBST(EDJE_CC_PRG)
AC_SUBST(EDJE_DECC_PRG)
AC_SUBST(EDJE_RECC_PRG)

# Optional EDJE_PROGRAM_CACHE (use much more ram, but increase speed in some cases)
AC_ARG_ENABLE([edje-program-cache],
   [AC_HELP_STRING(
       [--enable-edje-program-cache],
       [enable EDJE_PROGRAM_CACHE support. [[default=disabled]]]
    )],
   [want_edje_program_cache=$enableval]
)
AM_CONDITIONAL(EDJE_PROGRAM_CACHE, test "x${want_edje_program_cache}" = "xyes")

if test "x${want_edje_program_cache}" = "xyes" ; then
   AC_DEFINE(EDJE_PROGRAM_CACHE, 1, [Cache result of program glob matches - this uses up extra ram with the gain of faster program matching])
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


### Checks for libraries

PKG_PROG_PKG_CONFIG

# Evil library for compilation on Windows

EFL_EDJE_BUILD=""
case "$host_os" in
   mingw* | cegcc*)
      PKG_CHECK_MODULES([EVIL], [evil])
      AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if Evil library is installed])
      requirement_edje="evil ${requirement_edje}"
      EFL_EDJE_BUILD="-DEFL_EDJE_BUILD"
   ;;
esac
AC_SUBST(EFL_EDJE_BUILD)

# Dependencies for the library

PKG_CHECK_MODULES([EDJE],
   [
    eina-0
    eet >= 1.0.1
    evas >= 0.9.9
    ecore >= 0.9.9
    ecore-job >= 0.9.9
    embryo >= 0.9.1
   ]
)

requirement_edje="embryo ecore-job ecore evas eet eina-0 ${requirement_edje}"

# Dependencies for the binaries

if test "x$have_edje_cc" = "xyes"; then
   PKG_CHECK_MODULES([ECORE_FILE], [ecore-file >= 0.9.9])
   PKG_CHECK_MODULES([ECORE_EVAS], [ecore-evas >= 0.9.9])
fi


### Checks for header files
AC_CHECK_HEADERS([locale.h])


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
AC_FUNC_ALLOCA

case "$host_os" in
   mingw* | cegcc*)
      AC_DEFINE(HAVE_REALPATH, 1, [Define to 1 if you have the `realpath' function.])
      ;;
   *)
      AC_CHECK_FUNCS([realpath])
      ;;
esac


AC_SUBST(requirement_edje)

AC_OUTPUT([
edje.pc
edje.spec
Makefile
data/Makefile
data/include/Makefile
doc/Makefile
doc/edje.dox
src/Makefile
src/lib/Makefile
src/bin/Makefile
utils/Makefile
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
echo "  EDJE_PROGRAM_CACHE...: $want_edje_program_cache"
echo
echo "  Build binaries.......: $have_edje_cc"
echo
echo "  Documentation........: ${enable_doc}"
echo
echo "  Compilation..........: make"
echo
echo "  Installation.........: make install"
echo
echo "    prefix.............: $prefix"
echo

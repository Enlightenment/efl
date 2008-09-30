dnl Process this file with autoconf to produce a configure script.

# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT(edje, 0.9.9.050, enlightenment-devel@lists.sourceforge.net)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(configure.ac)
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_ISC_POSIX

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AM_PROG_CC_STDC
AM_PROG_CC_C_O
AC_HEADER_STDC
AC_C_CONST

AM_PATH_PYTHON([2.5], , [:])
AM_CONDITIONAL([HAVE_PYTHON], [test "$PYTHON" != : ])

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

PKG_PROG_PKG_CONFIG

AC_FUNC_ALLOCA

WIN32_CFLAGS=""
lt_no_undefined=""
lt_enable_auto_import=""
case "$host_os" in
	mingw*|cegcc)
		PKG_CHECK_MODULES([EVIL], [evil])
		AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if evil package is installed])
		AC_DEFINE(HAVE_REALPATH, 1, [Define to 1 if you have the `realpath' function.])
		dnl needed for correct definition of EAPI
		AC_DEFINE(EFL_EDJE_BUILD, 1, [Define to mention that embryo is built])
		if test "x$host_os" = "xcegcc" ; then
			WIN32_CFLAGS="-mwin32"
			lt_enable_auto_import="-Wl,--enable-auto-import"
		fi
		lt_no_undefined="-no-undefined"
		;;
	*)
		AC_CHECK_FUNCS(realpath)
		;;
esac
AC_SUBST(WIN32_CFLAGS)
AC_SUBST(lt_no_undefined)
AC_SUBST(lt_enable_auto_import)

AC_MSG_CHECKING(whether to build edje_cc)
have_edje_cc="yes"
AC_ARG_ENABLE(edje-cc, [  --disable-edje-cc       disable building of edje_cc ], [
     if test x"$enableval" = x"yes" ; then
       AC_MSG_RESULT(yes)
       have_edje_cc="yes"
     else
       AC_MSG_RESULT(no)
       have_edje_cc="no"
     fi
   ], [
      AC_MSG_RESULT($have_edje_cc)
   ]
)

PKG_CHECK_MODULES(EDJE, [
  evas >= 0.9.9
  ecore >= 0.9.9
  ecore-job >= 0.9.9
  eet >= 1.0.1
  embryo >= 0.9.1
])

PKG_CHECK_MODULES(ECORE_FILE, [
  ecore-file >= 0.9.9
])

PKG_CHECK_MODULES(ECORE_EVAS, [
  ecore-evas >= 0.9.9
])

dnl Optional EDJE_PROGRAM_CACHE (use much more ram, but increase speed in some case)
want_edje_program_cache="no"
AC_ARG_ENABLE(edje-program-cache,
  AC_HELP_STRING(
    [--enable-edje-program-cache],
    [enable EDJE_PROGRAM_CACHE support. [[default=disabled]]]
  ),
  [ want_edje_program_cache=$enableval ]
)
AM_CONDITIONAL(EDJE_PROGRAM_CACHE, test "x$want_edje_program_cache" = "xyes")
if test "x$want_edje_program_cache" = "xyes"; then
  AC_DEFINE(EDJE_PROGRAM_CACHE, 1, [Cache result of program glob matches - this uses up extra ram with the gain of faster program matching])
fi

AC_MSG_CHECKING([for location of Vim data files])
install_vim="yes";
AC_ARG_WITH(vim,
    [  --with-vim=DIR          Location of Vim data files [[autodetect]]],
    [if test -d "${withval}"; then
       vimdir="${withval}"
    fi])

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

if test "${install_vim}" = "yes"; then
   AC_MSG_RESULT([$vimdir])
else
   AC_MSG_RESULT([Not found, EDC syntax file will not be installed])
fi

AC_SUBST(vimdir)

EDJE_CC_PRG=""
EDJE_DECC_PRG=""
EDJE_RECC_PRG=""
if test "x$have_edje_cc" = "xyes"; then
  AM_CONDITIONAL(BUILD_EDJE_CC, true)
  AM_CONDITIONAL(BUILD_EDJE_DECC, true)
  EDJE_CC_PRG="edje_cc"
  EDJE_DECC_PRG="edje_decc"
  EDJE_RECC_PRG="edje_recc"
else
  AM_CONDITIONAL(BUILD_EDJE_CC, false)
  AM_CONDITIONAL(BUILD_EDJE_DECC, false)
fi
AC_SUBST(EDJE_CC_PRG)
AC_SUBST(EDJE_DECC_PRG)
AC_SUBST(EDJE_RECC_PRG)

AC_OUTPUT([
edje.pc
edje.spec
Makefile
data/Makefile
data/include/Makefile
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
echo "  Compilation..........: make"
echo
echo "  Installation.........: make install"
echo
echo "    prefix.............: $prefix"
echo

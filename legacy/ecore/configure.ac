dnl Process this file with autoconf to produce a configure script.

AC_INIT
AC_CONFIG_SRCDIR([configure.in])
AM_INIT_AUTOMAKE(ecore, 0.0.2)
AM_CONFIG_HEADER(config.h)

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AM_PROG_LIBTOOL

AM_WITH_DMALLOC

dnl Set PACKAGE_LOCALE_DIR in config.h.
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "${ac_default_prefix}/${DATADIRNAME}/locale")
else
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "${prefix}/${DATADIRNAME}/locale")
fi

dnl Set PACKAGE_DATA_DIR in config.h.
if test "x${datadir}" = 'x${prefix}/share'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}")
  else
    AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/${PACKAGE}")
  fi
else
  AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${datadir}/${PACKAGE}")
fi

dnl Set PACKAGE_SOURCE_DIR in config.h.
packagesrcdir=`cd $srcdir && pwd`
AC_DEFINE_UNQUOTED(PACKAGE_SOURCE_DIR, "${packagesrcdir}")

dnl Use -Wall if we have gcc.
changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi
changequote([,])dnl

AC_PATH_XTRA

if test "x$x_includes" = "x"; then
  x_includes="/usr/include"
fi

x_cflags="$X_CFLAGS"
x_ldflags="$X_LDFLAGS"
x_libs="$X_LIBS $X_EXTRA_LIBS"

AC_CHECK_LIB(X11, XOpenDisplay, x_libs="$x_libs",
  AC_MSG_ERROR([ERROR: libX11 not found.]); exit,
  $X_LDFLAGS $X_EXTRA_LIBS $X_LIBS)
AC_CHECK_LIB(Xext, XShmAttach, x_libs="-lXext $x_libs",
             AC_CHECK_LIB(XextSam, XShmAttach,
                          x_libs="-lXextSam -lXext $x_libs",
                          AC_MSG_ERROR([ERROR: XShm not found.]); exit,
                          $x_libs),
             $x_libs)
AC_CHECK_LIB(Xext, XShapeCombineMask, x_libs="-lXext $x_libs",
             AC_MSG_ERROR([ERROR: XShape not found.]); exit,
	     $x_libs)
AC_CHECK_LIB(Xrender, XRenderCreatePicture,
             [ x_libs=" -lXrender -lXext $x_libs";
	       AC_DEFINE(HAVE_RENDER)
	     ],
	     [],
	     $x_libs)
								   
AC_SUBST(x_cflags)
AC_SUBST(x_includes)
AC_SUBST(x_ldflags)
AC_SUBST(x_libs)

AC_CONFIG_FILES([
Makefile
ecore-config
src/Makefile
debian/Makefile
])
AC_CONFIG_COMMANDS([default],[[
chmod +x ecore-config

]],[[]])
AC_OUTPUT

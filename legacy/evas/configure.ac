# SETUP
AC_INIT(src/Evas.h)
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(evas, 0.6.0)

IMLIB2_CONFIG_IN_PATH=xyes
AC_PROGRAM_CHECK(IMLIB2_CONFIG_IN_PATH, imlib2-config, xyes, xno)
if test $IMLIB2_CONFIG_IN_PATH = xno; then
  echo "ERROR:"
  echo "The imlib2-config development script was not found in your execute"
  echo "path. This may mean one of several things"
  echo "1. You may not have installed the Imlib2-devel (or Imlib2-dev)"
  echo "   packages."
  echo "2. You may have Imlib2 installed somewhere not covered by your path."
  echo ""
  echo "If this is the case make sure you have the packages installed, AND"
  echo "that the imlib2-config script is in your execute path (see your"
  echo "shell's manual page on setting the \$PATH environment variable)."
  AC_MSG_ERROR([Fatal Error: no imlib2-config detected.])
  exit;
fi

AC_ARG_WITH(ttf,
[  --with-ttf=DIR          use DIR where freetype was installed (eg /opt/freetype)],
[
CPPFLAGS="$CPPFLAGS -I$withval/include";
LDFLAGS="$LDFLAGS -L$withval/lib";
LIBS_TTF="-L"$withval"/lib"
])

gl=yes
AC_ARG_ENABLE(gl,
[  --disable-gl            disable building with opengl support],
[
  if test x$enableval = xyes; then
    gl=yes
  else
    gl=no
  fi
]
)

AC_ARG_WITH(gl,
[  --with-gl=DIR           use DIR where opengl was installed (eg /usr/X11R6)],
[
  if test x$gl = xyes; then
    CPPFLAGS="$CPPFLAGS -I$withval/include";
    LDFLAGS="$LDFLAGS -L$withval/lib";
    LIBS_GL="-L"$withval"/lib"
  fi
])

AC_C_BIGENDIAN
AC_PROG_CC
AM_PROG_CC_STDC
AM_WITH_DMALLOC
AC_C_CONST
AM_ENABLE_SHARED
AM_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_MAKE_SET

if test X"$enable_shared" = Xyes; then
  DLLDFLAGS=-export-dynamic
  AC_SUBST(DLLDFLAGS)
fi
        
AC_PATH_X
AC_PATH_XTRA

if test "x$x_includes" = "x"; then
  x_includes="/usr/include"
fi

x_cflags="$X_CFLAGS"
x_ldflags="$X_LDFLAGS"
x_libs="$X_LIBS $X_EXTRA_LIBS -lX11"

AC_CHECK_LIB(
X11,
XOpenDisplay,
x_libs="$x_libs",
AC_MSG_ERROR([ERROR: libX11 not found.]); exit,
$X_LDFLAGS $X_EXTRA_LIBS $X_LIBS
)
AC_CHECK_LIB(
Xext,
XShmAttach,
x_libs="-lXext $x_libs",
  AC_CHECK_LIB(
  XextSam,
  XShmAttach,
  x_libs="-lXextSam -lXext $x_libs",
  AC_MSG_ERROR([ERROR: XShm not found.]); exit, 
  $x_libs),
$x_libs)
AC_CHECK_LIB(
Xext,
XShapeCombineMask,
x_libs="-lXext $x_libs",
AC_MSG_ERROR([ERROR: XShape not found.]); exit,
$x_libs)

x_libs="$x_libs -lX11"

AC_CHECK_LIB(
Xrender, 
XRenderCreatePicture, 
[
x_libs=" -lXrender -lXext $x_libs";
AC_DEFINE(HAVE_RENDER)
],
[],
$x_libs)

AC_CHECK_LIB(
ttf,
TT_Init_FreeType,
TTF_LIBS="-lttf "$LIBS_TTF,
[
echo "ERROR: Evas needs a system with libttf.so (TrueType Font Library)"
echo "You can obtain it from:"
echo "ftp://ftp.enlightenment.org/pub/enlightenment/"
echo "You can also obtain it from:"
echo "http://www.freetype.org/"
echo "You can obtain some truetype fonts from:"
echo "http://voyager.cns.ohiou.edu/~sadkins/web_library/fonts/font_archive/"
echo ""
AC_MSG_ERROR([Fatal Error: no FreeType detected.])
],
$LIBS_TTF -lm)

AC_CHECK_HEADERS(freetype.h freetype/freetype.h, [ break ], )
AC_CHECK_HEADER(freetype.h, [ ftype=yes ], [ ftype=no ])
AC_CHECK_HEADER(freetype/freetype.h, [ ftype_ftype=yes ], [ ftype_ftype=no ])
if test x$ftype = xno; then
if test x$ftype_ftype = xno; then
echo ""
echo "ERROR: freetype.h not found in freetype.h or freetype/freetype.h include"
echo "paths. (ie /usr/include, /usr/local/include etc.). You need the freetype"
echo "header files to build Evas. This may mean you have to install the"
echo "freetype-devel packages for your distribution or if all else fails"
echo "find the source for freetype and install that somewhere where Evas"
echo "can find the header files."
echo ""
echo "You can obtain it from:"
echo "ftp://ftp.enlightenment.org/pub/enlightenment/"
echo "You can also obtain it from:"
echo "http://www.freetype.org/"
echo "You can obtain some truetype fonts from:"
echo "http://voyager.cns.ohiou.edu/~sadkins/web_library/fonts/font_archive/"
echo ""
AC_MSG_ERROR([Fatal Error: no FreeType header files detected.])
fi
fi

ttf_includes="$CPPFLAGS"
ttf_libs="$LIBS_TTF -lm -lttf"

if test x$gl = xyes ; then
gl_includes=""
PREV_CPPFLAGS=$CPPFLAGS
CPPFLAGS=$CPPFLAGS" -I/usr/include -I/usr/local/include "$x_cflags

header_gl_h=xno
header_glu_h=xno
AC_TRY_CPP(
[
#include <GL/gl.h>
],
[
echo "checking for GL/gl.h... yes";
gl_includes=$CPPFLAGS;
header_gl_h=xyes;
]
,
[
echo "checking for GL/gl.h... no";
header_gl_h=xno;
])

if test $header_gl_h = xyes; then
AC_TRY_CPP(
[
#include <GL/gl.h>
#include <GL/glu.h>
],
[
echo "checking for GL/glu.h... yes";
glu_includes=$CPPFLAGS;
header_glu_h=xyes;
]
,
[
echo "checking for GL/glu.h... no";
header_glu_h=xno;
])
fi

have_gl=no
have_glu=no

if test $header_gl_h = xyes; then
AC_CHECK_LIB(
GL,
glBindTexture,
have_gl=yes,
[],
-L/usr/local/lib -lGL $x_libs $x_ldflags $x_cflags $LIBS_GL -lm)
fi
if test "x$have_gl" = "xyes"; then
gl_includes=$gl_includes
gl_ldflags="$gl_ldflags -L/usr/local/lib -lGL $x_libs $x_ldflags $x_cflags $LIBS_GL -lm";
gl_libs=$gl_libs" -lGL"
AC_DEFINE(HAVE_GL)
fi

if test $header_glu_h = xyes; then
AC_CHECK_LIB(
GLU,
gluBuild2DMipmaps,
have_glu=yes,
echo "WARNING:.......";
echo "no libGLU was found. This means filtered (anti-aliased) scaling down";
echo "of images will be disabled.";
echo "Please read the config.log file for more information as to why this library";
echo "was not found.";
,
$gl_ldflags -lGL -lGLU)
AC_CHECK_LIB(
GLU,
gluTessBeginPolygon,
have_glu=yes,
echo "";
echo "ERROR: evas needs a system with libGLU 1.3";
echo "You can obtain it from:";
echo "http://www.mesa3d.org/downloads/sgi.html";
echo "You will need to obtain the SGI'd libGLU.";
echo "debian: download the rpm and use alien to convert it";
echo "";
have_glu = no;
,
$gl_ldflags -lGL -lGLU)
fi

if test have_gl = no; then
  echo "WARNING:......."
  echo "no OpenGL libraries /  headers found. This means no GL support will be"
  echo "built into Evas. Ifapplications request 3D Hardware rendering from"
  echo "evas, they will be forcibly moved back to using software alpha"
  echo "rendering."
  echo "Please read the config.log file for more information as to why this library"
  echo "was not found."
fi
fi
CPPFLAGS=$PREV_CPPFLAGS

imlib2_includes=`imlib2-config --cflags`
imlib2_libs=`imlib2-config --libs`

AC_SUBST(x_cflags)
AC_SUBST(x_includes)
AC_SUBST(x_ldflags)
AC_SUBST(x_libs)
AC_SUBST(gl_includes)
AC_SUBST(gl_ldflags)
AC_SUBST(gl_libs)
AC_SUBST(imlib2_includes)
AC_SUBST(imlib2_libs)
AC_SUBST(ttf_includes)
AC_SUBST(ttf_libs)

AC_OUTPUT([
Makefile
src/Makefile
test/Makefile
test/img/Makefile
test/fnt/Makefile
evas-config
doc/Makefile
debian/Makefile
], [
chmod +x evas-config
])

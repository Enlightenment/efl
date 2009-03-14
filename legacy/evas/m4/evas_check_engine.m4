
dnl use: EVAS_CHECK_ENGINE_DEP_BUFFER(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_BUFFER],
[

have_dep="yes"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_X11(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_X11],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADER([X11/X.h], [have_dep="yes"])

if test "x${have_dep}" = "xyes" ; then
   if test "x$2" = "xyes" ; then
      x_libs="${x_libs} -lX11 -lXext"
   else
      x_dir=${x_dir:-/usr/X11R6}
      x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
      x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext"
   fi
   evas_engine_[]$1[]_cflags="${x_cflags}"
   evas_engine_[]$1[]_libs="${x_libs}"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_XRENDER_X11(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_XRENDER_X11],
[

evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADERS([X11/X.h X11/extensions/Xrender.h],
   [have_dep="yes"],
   [have_dep="no"])

if test "x${have_dep}" = "xyes" ; then
   if test "x$2" = "xyes" ; then
      x_libs="${x_libs} -lX11 -lXext"
   else
      x_dir=${x_dir:-/usr/X11R6}
      x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
      x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext"
   fi
   evas_engine_[]$1[]_cflags="${x_cflags}"
   evas_engine_[]$1[]_libs="${x_libs} -lXrender"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_GL_X11(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_GL_X11],
[

evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADERS([GL/gl.h GL/glu.h X11/X.h],
   [have_dep="yes"],
   [have_dep="no"])

if test "x${have_dep}" = "xyes" ; then
   if test "x$2" = "xyes" ; then
      x_libs="${x_libs} -lX11 -lXext"
   else
      x_dir=${x_dir:-/usr/X11R6}
      x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
      x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext"
   fi
   evas_engine_[]$1[]_cflags="-I/usr/include ${x_cflags}"
   evas_engine_[]$1[]_libs="${x_libs} -lGL -lGLU -lpthread"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_GLITZ_X11(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_GLITZ_X11],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADER([X11/X.h],
   [PKG_CHECK_MODULES([GLITZ],
       [glitz glitz-glx],
       [have_dep="yes"]
    )]
)

if test "x${have_dep}" = "xyes" ; then
   if test "x$2" = "xyes" ; then
      x_libs="${x_libs} -lX11 -lXext"
   else
      x_dir=${x_dir:-/usr/X11R6}
      x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
      x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext"
   fi
   evas_engine_[]$1[]_cflags="${GLITZ_CFLAGS} ${x_cflags}"
   evas_engine_[]$1[]_libs="${GLITZ_LIBS} ${x_libs}"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_CAIRO_X11(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_CAIRO_X11],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADER([X11/X.h],
   [PKG_CHECK_MODULES([CAIRO],
       [cairo >= 1.0.0],
       [have_dep="yes"]
    )]
)

if test "x${have_dep}" = "xyes" ; then
   if test "x$2" = "xyes" ; then
      x_libs="${x_libs} -lX11 -lXext"
   else
      x_dir=${x_dir:-/usr/X11R6}
      x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
      x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext"
   fi
   evas_engine_[]$1[]_cflags="${CAIRO_CFLAGS} ${x_cflags}"
   evas_engine_[]$1[]_libs="${CAIRO_LIBS} ${x_libs}"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_XCB(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_XCB],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([XCB],
   [xcb xcb-shm xcb-image >= 0.2.1 pixman-1],
   [
    have_dep="yes"
    evas_engine_[]$1[]_cflags="${XCB_CFLAGS}"
    evas_engine_[]$1[]_libs="${XCB_LIBS}"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_XRENDER_XCB(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_XRENDER_XCB],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([XCBRENDER],
   [xcb xcb-shm xcb-render xcb-image >= 0.2.1 pixman-1],
   [
    have_dep="yes"
    evas_engine_[]$1[]_cflags="${XCBRENDER_CFLAGS}"
    evas_engine_[]$1[]_libs="${XCBRENDER_LIBS}"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_DDRAW(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_DDRAW],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADER([ddraw.h],
   [
    have_dep="yes"
    evas_engine_[]$1[]_libs="-lddraw"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_DIRECT3D(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_DIRECT3D],
[

evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADERS([d3d9.h d3dx9.h],
   [
    have_dep="yes"
    evas_engine_[]$1[]_libs="-ld3d9 -ld3dx9 -lgdi32"
   ],
   [have_dep="no"]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_QUARTZ(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_QUARTZ],
[

evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADERS([/System/Library/Frameworks/Cocoa.framework/Headers/Cocoa.h],
   [have_dep="yes"],
   [have_dep="no"]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_GL_GLEW(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_GL_GLEW],
[

evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADERS([GL/gl.h GL/glu.h GL/glew.h],
   [
    have_dep="yes"
    evas_engine_[]$1[]_libs="-lglu32 -lglew32 -lopengl32 -lgdi32"
   ],
   [have_dep="no"]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_SDL(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_SDL],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([SDL],
   [sdl >= 1.2.0],
   [
    have_dep="yes"
    evas_engine_[]$1[]_cflags="${SDL_CFLAGS}"
    evas_engine_[]$1[]_libs="${SDL_LIBS}"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_FB(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_FB],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADER([linux/fb.h], [have_dep="yes"])

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_DIRECTFB(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_DIRECTFB],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([DIRECTFB],
   [directfb >= 0.9.16],
   [
    have_dep="yes"
    evas_engine_[]$1[]_cflags="${DIRECTFB_CFLAGS}"
    evas_engine_[]$1[]_libs="${DIRECTFB_LIBS}"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_QTOPIA(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_QTOPIA],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""
evas_engine_[]$1[]_moc=""

AC_ARG_WITH([qtdir],
   [AC_HELP_STRING([--with-qtdir=QT_DIR], [use qt directory specified])],
   [
    qt_dir=$withval;
    echo "using "$qt_dir" for qt directory.";
   ],
   [qt_dir="/opt/Qtopia"]
)

AC_LANG_PUSH(C++)

AC_CHECK_HEADER([qdirectpainter_qws.h],
   [have_dep="yes"],
   [have_dep="no"],
   [
#include <qwidget.h>
#include <qnamespace.h>
#include <qbrush.h>
#include <qpainter.h>
   ]
)

AC_LANG_POP(C++)

if test "x${have_dep}" = "xyes" ; then
   evas_engine_[]$1[]_cflags="-fno-exceptions -fno-rtti -I${qt_dir}/include"
   evas_engine_[]$1[]_libs="-L${qt_dir}/lib -lqte -lqpe"
   evas_engine_[]$1[]_moc="${qt_dir}/bin/moc"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])
AC_SUBST([evas_engine_$1_moc])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_X11(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_X11],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADER([X11/X.h], [have_dep="yes"])

if test "x${have_dep}" = "xyes" ; then
   if test "x$2" = "xyes" ; then
      x_libs="${x_libs} -lX11 -lXext"
   else
      x_dir=${x_dir:-/usr/X11R6}
      x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
      x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext"
   fi
   evas_engine_[]$1[]_cflags="${x_cflags}"
   evas_engine_[]$1[]_libs="${x_libs}"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_DDRAW(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_DDRAW],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADER([ddraw.h],
   [
    have_dep="yes"
    evas_engine_[]$1[]_libs="-lddraw -lgdi32"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_WINCE(engine, simple[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_WINCE],
[

have_dep="yes"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$3], [:])
else
  m4_default([$4], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE(engine, want_engine, simple, description)


AC_DEFUN([EVAS_CHECK_ENGINE],
[

m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

want_engine="$2"
have_evas_engine_[]DOWN="no"

AC_ARG_ENABLE([$1],
   [AC_HELP_STRING([--enable-$1], [enable $4 rendering backend])],
   [want_engine=${enableval}]
)

AC_MSG_CHECKING([whether to enable $4 rendering backend])
AC_MSG_RESULT([${want_engine}])

if test "x${want_engine}" = "xyes" -o "x${want_engine}" = "xauto"; then
   m4_default([EVAS_CHECK_ENGINE_DEP_]m4_defn([UP]))(DOWN, $3, [have_evas_engine_[]DOWN="yes"], [have_evas_engine_[]DOWN="no"])
fi

if test "x${have_evas_engine_[]DOWN}" = "xno" -a "x${want_engine}" = "xyes" -a "x${use_strict}" = "xyes" ; then
   AC_MSG_ERROR([$4 dependencies not found (strict dependencies checking)])
fi

AC_MSG_CHECKING([whether $4 rendering backend will be built])
AC_MSG_RESULT([${have_evas_engine_[]DOWN}])

if test "x${have_evas_engine_[]DOWN}" = "xyes" ; then
   AC_DEFINE(BUILD_ENGINE_[]UP, [1], [$4 rendering backend])
fi

AM_CONDITIONAL(BUILD_ENGINE_[]UP, [test "x${have_evas_engine_[]DOWN}" = "xyes"])

m4_popdef([UP])
m4_popdef([DOWN])

])

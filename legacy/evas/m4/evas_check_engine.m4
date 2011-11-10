
dnl use: EVAS_CHECK_ENGINE_DEP_BUFFER(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_BUFFER],
[

have_dep="yes"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_XLIB(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_XLIB],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADER([X11/X.h], [have_dep="yes"])

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([X11], [XCreateImage], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([Xext], [XShmCreateImage], [have_dep="yes"], [have_dep="no"])
fi

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
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_GL_XLIB(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_GL_XLIB],
[

evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADER([GL/gl.h],
   [have_dep="yes"],
   [have_dep="no"],
   [
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xresource.h>
   ])

gl_pt_lib="";
have_gl_pt="no"

AC_MSG_CHECKING([whether pthread_create() is supported])
CFLAGS_save="${CFLAGS}"
CFLAGS="${CFLAGS} -pthread"
LIBS_save="${LIBS}"
LIBS="${LIBS} -pthread"
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM([[
#include <pthread.h>
                   ]],
                   [[
pthread_create(NULL, NULL, NULL, NULL);
                   ]])],
   [have_gl_pt="yes"],
   [have_gl_pt="no"])
CFLAGS=${CFLAGS_save}
LIBS=${LIBS_save}
AC_MSG_RESULT([${have_gl_pt}])

if test "x$have_gl_pt" = "xyes" ; then
   gl_pt_lib=" -pthread"
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([X11], [XCreateColormap], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([Xrender], [XRenderCreatePicture], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([GL], [glXCreateContext], [have_dep="yes"], [have_dep="no"], -lX11 -lXext -lXrender -lm $gl_pt_lib)
fi

if test "x$gl_flavor_gles" = "xyes" ; then
  have_dep=no
fi

if test "x${have_dep}" = "xyes" ; then
   PKG_CHECK_MODULES([GL_EET], [eet >= 1.4.0], [have_dep="yes"], [have_dep="no"])
   if test "x${have_dep}" = "xyes" ; then
      if test "x$2" = "xyes" ; then
         x_libs="${x_libs} -lX11 -lXext -lXrender"
      else
         x_dir=${x_dir:-/usr/X11R6}
         x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
         x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext -lXrender"
      fi
   evas_engine_[]$1[]_cflags="-I/usr/include ${x_cflags}"
   evas_engine_[]$1[]_libs="${x_libs} -lGL $gl_pt_lib"
   evas_engine_gl_common_libs="-lGL $gl_pt_lib"
   fi
else
   if test "x$2" = "xyes" ; then
      x_libs="${x_libs} -lX11 -lXext -lXrender"
   else
      x_dir=${x_dir:-/usr/X11R6}
      x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
      x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext -lXrender"
   fi
   AC_CHECK_HEADER([GLES2/gl2.h],
      [have_egl="yes"],
      [have_egl="no"],
      [
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xresource.h>
      ])
   if test "x${have_egl}" = "xyes" ; then
      AC_CHECK_LIB(GLESv2, glTexImage2D, [have_glesv2="yes"], , -lEGL ${x_libs} -lm $gl_pt_lib)
      if test "x${have_glesv2}" = "xyes" ; then
         PKG_CHECK_MODULES([GL_EET], [eet >= 1.4.0], [have_dep="yes"], [have_dep="no"])
         if test "x${have_dep}" = "xyes" ; then
            evas_engine_[]$1[]_cflags="${x_cflags}"
            evas_engine_[]$1[]_libs="${x_libs} -lGLESv2 -lEGL -lm $gl_pt_lib"
            evas_engine_gl_common_libs="-lGLESv2 -lm $gl_pt_lib"
            have_dep="yes"
            gl_flavor_gles="no"
            AC_DEFINE(GLES_VARIETY_SGX, 1, [Imagination SGX GLES2 support])
            gles_variety_sgx="yes"
         fi
      fi
   fi
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])


if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_XCB(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_XCB],
[

requirement=""
have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([XCB],
   [xcb xcb-shm xcb-image >= 0.2.1 pixman-1],
   [
    have_dep="yes"
    requirement="xcb xcb-shm xcb-image pixman-1"
    evas_engine_[]$1[]_cflags="${XCB_CFLAGS}"
    evas_engine_[]$1[]_libs="${XCB_LIBS}"
   ],[
    have_dep="no"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x$3" = "xstatic" ; then
   requirement_evas="${requirement} ${requirement_evas}"
fi

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])


dnl use: EVAS_CHECK_ENGINE_DEP_GL_XCB(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_GL_XCB],
[

evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADER([GL/gl.h],
   [have_dep="yes"],
   [have_dep="no"],
   [
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
   ])

gl_pt_lib="";
have_gl_pt="no"

AC_MSG_CHECKING([whether pthread_create() is supported])
CFLAGS_save="${CFLAGS}"
CFLAGS="${CFLAGS} -pthread"
LIBS_save="${LIBS}"
LIBS="${LIBS} -pthread"
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM([[
#include <pthread.h>
                   ]],
                   [[
pthread_create(NULL, NULL, NULL, NULL);
                   ]])],
   [have_gl_pt="yes"],
   [have_gl_pt="no"])
CFLAGS=${CFLAGS_save}
LIBS=${LIBS_save}
AC_MSG_RESULT([${have_gl_pt}])

if test "x$have_gl_pt" = "xyes" ; then
   gl_pt_lib=" -pthread"
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([X11], [XCreateColormap], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([Xrender], [XRenderCreatePicture], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([GL], [glXCreateContext], [have_dep="yes"], [have_dep="no"], -lX11 -lXext -lXrender -lm $gl_pt_lib)
fi

PKG_CHECK_MODULES([XCB_GL],
   [x11-xcb xcb xcb-glx xcb-render xcb-renderutil],
   [
    have_dep="yes"
    requirement="x11-xcb xcb xcb-glx xcb-render xcb-renderutil"
    evas_engine_[]$1[]_cflags="${XCB_GL_CFLAGS}"
    evas_engine_[]$1[]_libs="${XCB_GL_LIBS}"
   ],[
    have_dep="no"
   ]
)

if test "x$gl_flavor_gles" = "xyes" ; then
  have_dep=no
fi

if test "x${have_dep}" = "xyes" ; then
   PKG_CHECK_MODULES([GL_EET], [eet >= 1.4.0], [have_dep="yes"], [have_dep="no"])
   if test "x${have_dep}" = "xyes" ; then
      if test "x$2" = "xyes" ; then
         x_libs="${x_libs} -lX11 -lXext -lXrender"
      else
         x_dir=${x_dir:-/usr/X11R6}
         x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
         x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext -lXrender"
      fi
   evas_engine_[]$1[]_cflags="-I/usr/include ${XCB_GL_CFLAGS} ${x_cflags}"
   evas_engine_[]$1[]_libs="${XCB_GL_LIBS} ${x_libs} -lGL $gl_pt_lib"
   evas_engine_gl_common_libs="-lGL $gl_pt_lib"
   fi
else
   if test "x$2" = "xyes" ; then
      x_libs="${x_libs} -lX11 -lXext -lXrender"
   else
      x_dir=${x_dir:-/usr/X11R6}
      x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
      x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext -lXrender"
   fi
   AC_CHECK_HEADER([GLES2/gl2.h],
      [have_egl="yes"],
      [have_egl="no"],
      [
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
      ])
   if test "x${have_egl}" = "xyes" ; then
      AC_CHECK_LIB(GLESv2, glTexImage2D, [have_glesv2="yes"], , -lEGL ${x_libs} -lm $gl_pt_lib)
      if test "x${have_glesv2}" = "xyes" ; then
         PKG_CHECK_MODULES([GL_EET], [eet >= 1.4.0], [have_dep="yes"], [have_dep="no"])
         if test "x${have_dep}" = "xyes" ; then
            evas_engine_[]$1[]_cflags="${XCB_GL_CFLAGS} ${x_cflags}"
            evas_engine_[]$1[]_libs="${XCB_GL_LIBS} ${x_libs} -lGLESv2 -lEGL -lm $gl_pt_lib"
            evas_engine_gl_common_libs="-lGLESv2 -lm $gl_pt_lib"
            have_dep="yes"
            gl_flavor_gles="no"
            AC_DEFINE(GLES_VARIETY_SGX, 1, [Imagination SGX GLES2 support])
            gles_variety_sgx="yes"
         fi
      fi
   fi
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])


dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_GDI(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_GDI],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADER([windows.h],
   [
    have_dep="yes"
    evas_engine_[]$1[]_libs="-lgdi32"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_DDRAW(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

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
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_DIRECT3D(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_DIRECT3D],
[

evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADERS([d3d9.h d3dx9.h],
   [
    have_dep="yes"
    evas_engine_[]$1[]_libs="-ld3d9 -ld3dx9 -lgdi32"
   ],
   [have_dep="no"; break]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])


dnl use: EVAS_CHECK_ENGINE_DEP_GL_COCOA(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_GL_COCOA],
[

evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

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
    have_dep="yes"
    evas_engine_[]$1[]_libs="-framework Cocoa"
   ],
   [have_dep="no"])
LIBS="$LIBS_save"

AC_LANG_POP([Objective C])

if test "x${have_dep}" = "xyes" ; then
   PKG_CHECK_MODULES([GL_EET], [eet >= 1.4.0], [have_dep="yes"], [have_dep="no"])
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])


dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_SDL(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_SDL],
[

requirement=""
have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([SDL],
   [sdl >= 1.2.0],
   [
    have_dep="yes"
    requirement="sdl"
    evas_engine_[]$1[]_cflags="${SDL_CFLAGS}"
    evas_engine_[]$1[]_libs="${SDL_LIBS}"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x$3" = "xstatic" ; then
   requirement_evas="${requirement} ${requirement_evas}"
fi

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])


dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_SDL(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_SDL],
[

requirement=""
have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([SDL],
   [sdl >= 1.2.0],
   [
    have_dep="yes"
    requirement="sdl"
    evas_engine_[]$1[]_cflags="${SDL_CFLAGS}"
    evas_engine_[]$1[]_libs="${SDL_LIBS}"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x$3" = "xstatic" ; then
   requirement_evas="${requirement} ${requirement_evas}"
fi

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_GL_SDL(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_GL_SDL],
[

requirement=""
have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([SDL],
   [sdl >= 1.2.0],
   [
    have_dep="yes"
    requirement="sdl"
    evas_engine_[]$1[]_cflags="${SDL_CFLAGS}"
    evas_engine_[]$1[]_libs="${SDL_LIBS}"
   ]
)

gl_pt_lib="";
have_gl_pt="no"

AC_MSG_CHECKING([whether pthread_create() is supported])
CFLAGS_save="${CFLAGS}"
CFLAGS="${CFLAGS} -pthread"
LIBS_save="${LIBS}"
LIBS="${LIBS} -pthread"
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM([[
#include <pthread.h>
                   ]],
                   [[
pthread_create(NULL, NULL, NULL, NULL);
                   ]])],
   [have_gl_pt="yes"],
   [have_gl_pt="no"])
CFLAGS=${CFLAGS_save}
LIBS=${LIBS_save}
AC_MSG_RESULT([${have_gl_pt}])

if test "x$have_gl_pt" = "xyes" ; then
   gl_pt_lib=" -pthread"
fi

AC_CHECK_HEADER([GL/gl.h],
   [have_dep="yes"],
   [have_dep="no"],
   [
#include <GL/gl.h>
#include <GL/glext.h>
   ])

if test "x$gl_flavor_gles" = "xyes" ; then
  have_dep=no
fi

if test "x${have_dep}" = "xyes" ; then
   evas_engine_[]$1[]_cflags="${SDL_CFLAGS}"
   evas_engine_[]$1[]_libs="${SDL_LIBS} -lGL -lm $gl_pt_lib"
   evas_engine_gl_common_libs="-lGL -lm $gl_pt_lib"
else
   AC_CHECK_HEADER([SDL/SDL_opengles.h],
      [have_egl="yes"],
      [have_egl="no"],
      [
#include <SDL/SDL_opengles.h>
#include <EGL/egl.h>
      ])
   if test "x${have_egl}" = "xyes" ; then
      AC_CHECK_LIB(GLESv2, glTexImage2D, [have_glesv2="yes"], , -lEGL -lm $gl_pt_lib)
      if test "x${have_glesv2}" = "xyes" ; then
         evas_engine_[]$1[]_cflags="${SDL_CFLAGS}"
         evas_engine_[]$1[]_libs="${SDL_LIBS} -lGLESv2 -lEGL -lm $gl_pt_lib"
         evas_engine_gl_common_libs="-lGLESv2 -lm $gl_pt_lib"
         have_dep="yes"
         gl_flavor_gles="no"
         AC_DEFINE(GLES_VARIETY_SGX, 1, [Imagination SGX GLES2 support])
         gles_variety_sgx="yes"
      fi
   fi
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x$3" = "xstatic" ; then
   requirement_evas="${requirement} ${requirement_evas}"
fi

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_FB(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_FB],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADER([linux/fb.h], [have_dep="yes"])

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_DIRECTFB(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_DIRECTFB],
[

requirement=""
have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([DIRECTFB],
   [directfb >= 0.9.16],
   [
    have_dep="yes"
    requirement="directfb"
    evas_engine_[]$1[]_cflags="${DIRECTFB_CFLAGS}"
    evas_engine_[]$1[]_libs="${DIRECTFB_LIBS}"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x$3" = "xstatic" ; then
   requirement_evas="${requirement} ${requirement_evas}"
fi

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_8_X11(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_8_X11],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([XCB],
   [xcb xcb-shm xcb-image >= 0.2.1 pixman-1],
   [
    have_dep="yes"
    requirement="xcb xcb-shm xcb-image pixman-1"
    evas_engine_[]$1[]_cflags="${XCB_CFLAGS}"
    evas_engine_[]$1[]_libs="${XCB_LIBS}"
   ],[
    have_dep="no"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x$3" = "xstatic" ; then
   requirement_evas="${requirement} ${requirement_evas}"
fi

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_X11(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_X11],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADER([X11/X.h], [have_dep="yes"])

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([X11], [XCreateImage], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([Xext], [XShmCreateImage], [have_dep="yes"], [have_dep="no"])
fi

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
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_DDRAW(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

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
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_WINCE(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_WINCE],
[

have_dep="yes"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_PSL1GHT(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_PSL1GHT],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADER([rsx/rsx.h], [have_dep="yes"])

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE(engine, want_engine, simple, description)


AC_DEFUN([EVAS_CHECK_ENGINE],
[

m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

want_engine="$2"
want_static_engine="no"
have_engine="no"
have_evas_engine_[]DOWN="no"

AC_ARG_ENABLE([$1],
   [AC_HELP_STRING([--enable-$1], [enable $4 rendering backend])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_engine="yes"
    else
       if test "x${enableval}" = "xstatic" ; then
          want_engine="static"
       else
          want_engine="no"
       fi
    fi
   ])

AC_MSG_CHECKING([whether to enable $4 rendering backend])
AC_MSG_RESULT([${want_engine}])

if test "x${want_engine}" = "xyes" -o "x${want_engine}" = "xstatic" -o "x${want_engine}" = "xauto" ; then
   m4_default([EVAS_CHECK_ENGINE_DEP_]m4_defn([UP]))(DOWN, $3, ${want_engine}, [have_engine="yes"], [have_engine="no"])
fi

if test "x${have_engine}" = "xno" -a "x${want_engine}" = "xyes" -a "x${use_strict}" = "xyes" ; then
   AC_MSG_ERROR([$4 dependencies not found (strict dependencies checking)])
fi

AC_MSG_CHECKING([whether $4 rendering backend will be built])
AC_MSG_RESULT([${have_engine}])

if test "x${have_engine}" = "xyes" ; then
   if test "x${want_engine}" = "xstatic" ; then
      have_evas_engine_[]DOWN="static"
      want_static_engine="yes"
   else
      have_evas_engine_[]DOWN="yes"
   fi
fi

if test "x${have_engine}" = "xyes" ; then
   AC_DEFINE(BUILD_ENGINE_[]UP, [1], [$4 rendering backend])
fi

AM_CONDITIONAL(BUILD_ENGINE_[]UP, [test "x${have_engine}" = "xyes"])

if test "x${want_static_engine}" = "xyes" ; then
   AC_DEFINE(EVAS_STATIC_BUILD_[]UP, [1], [Build $1 engine inside libevas])
   have_static_module="yes"
fi

AM_CONDITIONAL(EVAS_STATIC_BUILD_[]UP, [test "x${want_static_engine}" = "xyes"])

m4_popdef([UP])
m4_popdef([DOWN])

])

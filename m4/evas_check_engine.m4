AC_DEFUN([REQUIRED_WAYLAND_VERSION], [1.11.0])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_XLIB(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_XLIB],
[

EFL_FIND_X(evas_engine_[]$1,
  [X11/X.h], [X11 XCreateImage Xext XShmCreateImage],
  [
    if test "x$3" = "xstatic"; then
      requirements_libs_evas="$evas_engine_[]$1[]_libs $requirements_libs_evas"
    fi
    ifelse([$4], , :, [$4])
  ],[
    ifelse([$5], , :, [$5])
  ])
])

dnl use: EVAS_CHECK_ENGINE_DEP_GL_XLIB(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_GL_XLIB],
[

EFL_FIND_X(evas_engine_[]$1,
  [X11/Xlib.h X11/Xatom.h X11/Xutil.h X11/extensions/Xrender.h X11/Xresource.h],
  [X11 XCreateColormap Xrender XRenderCreatePicture],
  [
    CFLAGS_save="$CFLAGS"
    CFLAGS="$evas_engine_[]$1[]_cflags $CFLAGS"
    CPPFLAGS_save="$CPPFLAGS"
    CPPFLAGS="$evas_engine_[]$1[]_cflags $CPPFLAGS"

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

    gl_pt_lib=""
    have_gl_pt="no"

    AC_MSG_CHECKING([whether pthread_create() is supported])
    CFLAGS_pt_save="$CFLAGS"
    CFLAGS="$CFLAGS -pthread"
    LIBS_pt_save="$LIBS"
    LIBS="$LIBS -pthread"
    AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([[
#include <pthread.h>
                       ]],
                       [[
pthread_create(NULL, NULL, NULL, NULL);
                       ]])],
      [have_gl_pt="yes"],
      [have_gl_pt="no"])
    CFLAGS=$CFLAGS_pt_save
    LIBS=$LIBS_pt_save
    AC_MSG_RESULT([$have_gl_pt])

    if test "x$have_gl_pt" = "xyes" ; then
      gl_pt_lib=" -pthread"
    fi

    if test "x$have_dep" = "xyes"; then
      LIBS_save="$LIBS"
      LIBS="$LIBS $evas_engine_[]$1[]_libs"
      AC_CHECK_LIB([GL], [glXCreateContext], [have_dep="yes"], [have_dep="no"], [-lm $gl_pt_lib])
      LIBS="$LIBS_save"
    fi

    if test "x${with_opengl}" = "xes" ; then
      have_dep=no
    fi

    if test "x$have_dep" = "xyes" ; then
      evas_engine_[]$1[]_libs="$evas_engine_[]$1[]_libs -lGL $gl_pt_lib"
      evas_engine_gl_common_libs="$evas_engine_[]$1[]_libdirs -lGL $gl_pt_lib"
    else
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
        AC_CHECK_LIB(GLESv2, glTexImage2D, [have_glesv2="yes"], , -lEGL -lm $gl_pt_lib)
        if test "x${have_glesv2}" = "xyes" ; then
          evas_engine_[]$1[]_libs="$evas_engine_[]$1[]_libs -lGLESv2 -lEGL -lm $gl_pt_lib"
          evas_engine_gl_common_libs="$evas_engine_[]$1[]_libdirs -lGLESv2 -lm $gl_pt_lib"
          have_dep="yes"
          AC_DEFINE(GL_GLES, 1, [GLSL runtime shader GLES2 support])
          gles_variety_sgx="yes"
        fi
      fi
    fi

    CPPFLAGS="$CPPFLAGS_save"
    CFLAGS="$CFLAGS_save"

    if test "x$3" = "xstatic" && test "x${have_dep}" = "xyes" ; then
      requirements_libs_evas="$evas_engine_[]$1[]_libs $requirements_libs_evas"
    fi

    AS_IF([test "x${have_dep}" = "xyes"], [$4], [$5])
  ],[
    ifelse([$5], , :, [$5])
  ])
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
   ])

if test "x$3" = "xstatic"  && test "x${have_dep}" = "xyes" ; then
   requirements_libs_evas="${evas_engine_[]$1[]_libs} ${requirements_libs_evas}"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$4], [$5])

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
   ])

if test "x$3" = "xstatic" && test "x${have_dep}" = "xyes" ; then
   requirements_libs_evas="${evas_engine_[]$1[]_libs} ${requirements_libs_evas}"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$4], [$5])

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
    evas_engine_[]$1[]_libs="-framework Cocoa -framework OpenGL"
    evas_engine_gl_common_libs="$evas_engine_[]$1[]_libdirs"
   ],
   [have_dep="no"])
LIBS="$LIBS_save"

AC_LANG_POP([Objective C])

if test "x$3" = "xstatic"  && test "x${have_dep}" = "xyes" ; then
   requirements_libs_evas="${evas_engine_[]$1[]_libs} ${requirements_libs_evas}"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$4], [$5])

])

dnl use: EVAS_CHECK_ENGINE_DEP_GL_SDL(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_GL_SDL],
[

requirement=""
have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_EXISTS([sdl2 >= 2.0.0],
   [
    have_dep="yes"
    requirement="sdl2 >= 2.0.0"
   ],
   [have_dep="no"])

if test "x${have_dep}" = "xyes" ; then
   if test "x$3" = "xstatic" ; then
      requirements_pc_evas="${requirement} ${requirements_pc_evas}"
      requirements_pc_deps_evas="${requirement} ${requirements_pc_deps_evas}"
   else
      PKG_CHECK_MODULES([SDL], [${requirement}])
      evas_engine_[]$1[]_cflags="${SDL_CFLAGS}"
      evas_engine_[]$1[]_libs="${SDL_LIBS}"
   fi
fi

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

if test "x${with_opengl}" = "xes" ; then
  have_dep=no
fi

if test "x${have_dep}" = "xyes" ; then
   evas_engine_[]$1[]_libs="${evas_engine_[]$1[]_libs} -lGL -lm $gl_pt_lib"
   evas_engine_gl_common_libs="-lGL -lm $gl_pt_lib"
else
   AC_CHECK_HEADER([SDL2/SDL_opengles.h],
      [have_egl="yes"],
      [have_egl="no"],
      [
#include <SDL2/SDL_opengles.h>
#include <EGL/egl.h>
      ])
   if test "x${have_egl}" = "xyes" ; then
      AC_CHECK_LIB(GLESv2, glTexImage2D, [have_glesv2="yes"], , -lEGL -lm $gl_pt_lib)
      if test "x${have_glesv2}" = "xyes" ; then
         evas_engine_[]$1[]_libs="${evas_engine_[]$1[]_libs} -lGLESv2 -lEGL -lm $gl_pt_lib"
         evas_engine_gl_common_libs="-lGLESv2 -lm $gl_pt_lib"
         have_dep="yes"
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

AS_IF([test "x${have_dep}" = "xyes"], [$4], [$5])

])


dnl use: EVAS_CHECK_ENGINE_DEP_WAYLAND_SHM(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_WAYLAND_SHM],
[

requirement=""
have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_EXISTS([wayland-client >= REQUIRED_WAYLAND_VERSION],
   [
    have_dep="yes"
    requirement="wayland-client"
   ],
   [have_dep="no"])

if test "x${have_dep}" = "xyes" ; then
   if test "x$3" = "xstatic" ; then
      requirements_pc_evas="${requirement} ${requirements_pc_evas}"
      requirements_pc_deps_evas="${requirement} ${requirements_pc_deps_evas}"
   else
      PKG_CHECK_MODULES([WAYLAND_SHM], [${requirement}])
      evas_engine_[]$1[]_cflags="${WAYLAND_SHM_CFLAGS}"
      evas_engine_[]$1[]_libs="${WAYLAND_SHM_LIBS}"
   fi
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$4], [$5])

])

dnl use: EVAS_CHECK_ENGINE_DEP_WAYLAND_EGL(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_WAYLAND_EGL],
[

requirement=""
have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

if test "x${with_opengl}" = "xes" ; then
    gl_library="glesv2"
else
    gl_library="gl"
fi

PKG_CHECK_EXISTS([egl ${gl_library} wayland-client >= REQUIRED_WAYLAND_VERSION wayland-egl],
   [
    have_dep="yes"
    requirement="egl ${gl_library} wayland-client wayland-egl"
   ],
   [have_dep="no"])

if test "x${have_dep}" = "xyes" ; then
   if test "${gl_library}" != "gl" ; then
      have_egl="yes"
   fi
   if test "x$3" = "xstatic" ; then
      requirements_pc_evas="${requirement} ${requirements_pc_evas}"
      requirements_pc_deps_evas="${requirement} ${requirements_pc_deps_evas}"
   else
      PKG_CHECK_MODULES([WAYLAND_EGL], [${requirement}])
      evas_engine_[]$1[]_cflags="${WAYLAND_EGL_CFLAGS}"
      evas_engine_[]$1[]_libs="${WAYLAND_EGL_LIBS}"
      evas_engine_gl_common_libs="$evas_engine_[]$1[]_libdirs -lGLESv2 -lm -lEGL"
   fi
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$4], [$5])

])


dnl use: EVAS_CHECK_ENGINE_DEP_DRM(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_DRM],
[

requirement=""
have_dep="no"
have_hw_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_EXISTS([libdrm],
  [
    have_dep="yes"
    requirement="libdrm"
  ], [have_dep="no"])

if test "x${have_dep}" = "xyes" ; then
   if test "x$3" = "xstatic" ; then
      requirements_pc_evas="${requirement} ${requirements_pc_evas}"
      requirements_pc_deps_evas="${requirement} ${requirements_pc_deps_evas}"
   else
      PKG_CHECK_MODULES([DRM], [${requirement}])
      evas_engine_[]$1[]_cflags="${DRM_CFLAGS}"
      evas_engine_[]$1[]_libs="${DRM_LIBS}"
   fi
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$4], [$5])

])

dnl use: EVAS_CHECK_ENGINE_DEP_GL_DRM(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_GL_DRM],
[

requirement=""
have_dep="no"
have_hw_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

if test "x${with_opengl}" = "xes" ; then
   gl_library="glesv2"
else
   AC_MSG_ERROR([We currently do not support GL DRM without OpenGL ES. Please consider OpenGL ES if you want to use it.])
fi

PKG_CHECK_EXISTS([egl ${gl_library} libdrm gbm wayland-client >= REQUIRED_WAYLAND_VERSION],
   [
    have_dep="yes"
    requirement="egl ${gl_library} libdrm gbm wayland-client >= REQUIRED_WAYLAND_VERSION"
   ],
   [have_dep="no"])

if test "x${have_dep}" = "xyes" ; then
   if test "x$3" = "xstatic" ; then
      requirements_pc_evas="${requirement} ${requirements_pc_evas}"
      requirements_pc_deps_evas="${requirement} ${requirements_pc_deps_evas}"
   else
      PKG_CHECK_MODULES([GL_DRM], [${requirement}])
      evas_engine_[]$1[]_cflags="${GL_DRM_CFLAGS}"
      evas_engine_[]$1[]_libs="${GL_DRM_LIBS}"
      evas_engine_gl_common_libs="$evas_engine_[]$1[]_libdirs -lGLESv2 -lm -lEGL"
   fi
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$4], [$5])

])

dnl use: EVAS_CHECK_ENGINE_DEP_EGLFS(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_EGLFS],
[

requirement=""
have_dep="no"
have_hw_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

if test "x${with_opengl}" = "xes" ; then
   gl_library="glesv2"
else
   AC_MSG_ERROR([We do not support Eglfs without OpenGL ES. Please consider OpenGL ES if you want to use it.])
fi

PKG_CHECK_EXISTS([egl >= 7.10 ${gl_library}],
   [
    have_dep="yes"
    requirement="egl >= 7.10 ${gl_library}"
   ],
   [have_dep="no"])

if test "x${have_dep}" = "xyes" ; then
   if test "x$3" = "xstatic" ; then
      requirements_pc_evas="${requirement} ${requirements_pc_evas}"
      requirements_pc_deps_evas="${requirement} ${requirements_pc_deps_evas}"
   else
      PKG_CHECK_MODULES([EGLFS], [${requirement}])
      evas_engine_[]$1[]_cflags="${EGLFS_CFLAGS}"
      evas_engine_[]$1[]_libs="${EGLFS_LIBS}"
      evas_engine_gl_common_libs="$evas_engine_[]$1[]_libdirs -lGLESv2 -lm -lEGL"
   fi
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$4], [$5])

])


dnl use: EVAS_ENGINE(name, want_engine, [DEPENDENCY-CHECK-CODE])
dnl
dnl defines BUILD_ENGINE_NAME if it should be built
dnl defines BUILD_STATIC_BUILD_NAME if should be built statically
dnl
dnl will call DEPENDENCY-CHECK-CODE if it should be built,
dnl if some dependency fail just call AC_MSG_ERROR() to abort.

AC_DEFUN([EVAS_ENGINE],
[dnl
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

want_engine="$2"
want_static_engine="no"
have_engine="no"
have_evas_engine_[]DOWN="no"

evas_engine_[]m4_defn([DOWN])[]_cflags=""
evas_engine_[]m4_defn([DOWN])[]_libs=""

if test "x${want_engine}" = "xyes" -o "x${want_engine}" = "xstatic"; then
   $3

   have_engine="yes"
   if test "x${want_engine}" = "xstatic" ; then
      have_evas_engine_[]DOWN="static"
      want_static_engine="yes"
   else
      have_evas_engine_[]DOWN="yes"
   fi
fi

AC_DEFINE_IF(BUILD_ENGINE_[]UP, [test "${have_engine}" = "yes"],
  [1], [Build $1 Evas engine])
AM_CONDITIONAL(BUILD_ENGINE_[]UP, [test "${have_engine}" = "yes"])

AC_DEFINE_IF(EVAS_STATIC_BUILD_[]UP, [test "${want_static_engine}" = "yes"],
  [1], [Build $1 Evas engine inside libevas])
AM_CONDITIONAL(EVAS_STATIC_BUILD_[]UP, [test "${want_static_engine}" = "yes"])

AC_SUBST([evas_engine_]m4_defn([DOWN])[_cflags])
AC_SUBST([evas_engine_]m4_defn([DOWN])[_libs])

EFL_ADD_FEATURE([EVAS_ENGINE], [$1], [${have_evas_engine_]DOWN[}])dnl
m4_popdef([UP])dnl
m4_popdef([DOWN])dnl
])



dnl use: EVAS_CHECK_ENGINE(engine, want_engine, simple, description)
AC_DEFUN([EVAS_CHECK_ENGINE],
[dnl
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

want_engine="$2"
want_static_engine="no"
have_engine="no"
have_evas_engine_[]DOWN="no"

AC_MSG_CHECKING([whether to enable $4 rendering backend])
AC_MSG_RESULT([${want_engine}])

if test "x${want_engine}" = "xyes" -o "x${want_engine}" = "xstatic"; then
   m4_default([EVAS_CHECK_ENGINE_DEP_]m4_defn([UP]))(DOWN, $3, ${want_engine}, [have_engine="yes"], [have_engine="no"])
fi

if test "x${have_engine}" = "xno" -a "x${want_engine}" = "xyes"; then
   AC_MSG_ERROR([$4 dependencies not found])
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

EFL_ADD_FEATURE([EVAS_ENGINE], [$1], [${have_evas_engine_]DOWN[}])
AM_CONDITIONAL(EVAS_STATIC_BUILD_[]UP, [test "x${want_static_engine}" = "xyes"])dnl
m4_popdef([UP])dnl
m4_popdef([DOWN])dnl
])

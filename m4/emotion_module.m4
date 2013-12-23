dnl use: EMOTION_MODULE_DEP_CHECK_XINE(want_engine)
dnl where want_engine = yes or static
AC_DEFUN([EMOTION_MODULE_DEP_CHECK_XINE],
[dnl
   requirements="libxine >= 1.1.1"
   if test "$1" = "static"; then
      EFL_DEPEND_PKG([EMOTION], [EMOTION_MODULE_XINE], [${requirements}])
   else
      PKG_CHECK_MODULES([EMOTION_MODULE_XINE], [${requirements}])
   fi
])

dnl use: EMOTION_MODULE_DEP_CHECK_GSTREAMER(want_static)
dnl where want_engine = yes or static
AC_DEFUN([EMOTION_MODULE_DEP_CHECK_GSTREAMER],
[dnl
   GST_VER=0.10.2
   requirements="gstreamer-0.10 >= ${GST_VER} gstreamer-plugins-base-0.10 >= ${GST_VER} gstreamer-video-0.10 >= ${GST_VER} gstreamer-interfaces-0.10 >= ${GST_VER}"
   have_gst_xoverlay="no"
   if test "$1" = "static"; then
      EFL_DEPEND_PKG([EMOTION], [EMOTION_MODULE_GSTREAMER], [${requirements}])
   else
      PKG_CHECK_MODULES([EMOTION_MODULE_GSTREAMER], [${requirements}])
   fi

   if test "${want_x11_any}" = "yes"; then
      if test "$1" = "static"; then # we need gstreamer cflags and libs to test xoverlay support
         PKG_CHECK_MODULES([EMOTION_MODULE_GSTREAMER], [${requirements}])
      fi
      CFLAGS_save="${CFLAGS}"
      CFLAGS="${CFLAGS} ${EMOTION_MODULE_GSTREAMER_CFLAGS}"
      AC_CHECK_HEADER([gst/interfaces/xoverlay.h],
         [have_gst_xoverlay="old" # will check for "new" later with AC_CHECK_LIB()
          AC_DEFINE(HAVE_XOVERLAY_H, 1, [Build with Gstreamer Xoverlay support])],
         [AC_MSG_WARN([Building Gstreamer with X11 but no gst/interfaces/xoverlay.h found])],
         [#include <gst/gst.h>])
      CFLAGS="${CFLAGS_save}"

      LDFLAGS_save=${LDFLAGS}
      LDFLAGS="${LDFLAGS} ${EMOTION_MODULE_GSTREAMER_LIBS}"
      AC_CHECK_LIB([gstinterfaces-0.10], [gst_x_overlay_set_window_handle],
         [have_gst_xoverlay="new"
          AC_DEFINE([HAVE_X_OVERLAY_SET], [1], [Use gst_x_overlay_set_window_handle instead of old deprecated gst_x_overlay_set_xwindow_id])])
      LDFLAGS="${LDFLAGS_save}"
   fi
])

dnl use: EMOTION_MODULE_DEP_CHECK_GSTREAMER_1(want_static)
dnl where want_engine = yes or static
AC_DEFUN([EMOTION_MODULE_DEP_CHECK_GSTREAMER1],
[dnl
   GST_VER=1.0
   requirements="gstreamer-1.0 >= ${GST_VER} gstreamer-plugins-base-1.0 >= ${GST_VER} gstreamer-video-1.0 >= ${GST_VER} gstreamer-audio-1.0 >= ${GST_VER} gstreamer-tag-1.0 >= ${GST_VER}"
   if test "$1" = "static"; then
      EFL_DEPEND_PKG([EMOTION], [EMOTION_MODULE_GSTREAMER1], [${requirements}])
   else
      PKG_CHECK_MODULES([EMOTION_MODULE_GSTREAMER1], [${requirements}])
   fi
])

dnl use: EMOTION_MODULE_DEP_CHECK_GENERIC(want_static)
dnl where want_engine = yes or static
AC_DEFUN([EMOTION_MODULE_DEP_CHECK_GENERIC],
[dnl
   if test "$1" = "static"; then
      EFL_ADD_LIBS([EMOTION], [${requirements_libs_shm}])
   else
      EMOTION_MODULE_GENERIC_CFLAGS=""
      EMOTION_MODULE_GENERIC_LIBS="${requirements_libs_shm}"
      AC_SUBST([EMOTION_MODULE_GENERIC_CFLAGS])
      AC_SUBST([EMOTION_MODULE_GENERIC_LIBS])
   fi
])

dnl use: EMOTION_MODULE(name, want_engine)
dnl
dnl defines EMOTION_BUILD_NAME if it should be built
dnl defines EMOTION_STATIC_BUILD_NAME if should be built statically
dnl
AC_DEFUN([EMOTION_MODULE],
[
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

want_engine="$2"
want_static_engine="no"
have_engine="no"
have_emotion_module_[]DOWN="no"

EMOTION_MODULE_[]m4_defn([UP])[]_CFLAGS=""
EMOTION_MODULE_[]m4_defn([UP])[]_LIBS=""

if test "x${want_engine}" = "xyes" -o "x${want_engine}" = "xstatic"; then

   m4_default([EMOTION_MODULE_DEP_CHECK_]m4_defn([UP]))([${want_engine}])

   have_engine="yes"
   if test "x${want_engine}" = "xstatic" ; then
      have_emotion_module_[]DOWN="static"
      want_static_engine="yes"
   else
      have_emotion_module_[]DOWN="yes"
   fi
fi

AC_DEFINE_IF(EMOTION_BUILD_[]UP, [test "${have_engine}" = "yes"],
  [1], [Build $1 Evas engine])
AM_CONDITIONAL(EMOTION_BUILD_[]UP, [test "${have_engine}" = "yes"])

AC_DEFINE_IF(EMOTION_STATIC_BUILD_[]UP, [test "${want_static_engine}" = "yes"],
  [1], [Build $1 Evas engine inside libevas])
AM_CONDITIONAL(EMOTION_STATIC_BUILD_[]UP, [test "${want_static_engine}" = "yes"])

AC_SUBST([EMOTION_MODULE_]m4_defn([UP])[_CFLAGS])
AC_SUBST([EMOTION_MODULE_]m4_defn([UP])[_LIBS])

m4_popdef([UP])
m4_popdef([DOWN])
])

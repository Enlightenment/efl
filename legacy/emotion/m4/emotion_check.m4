
dnl use: EMOTION_CHECK_DEP_XINE(want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EMOTION_CHECK_DEP_XINE],
[

requirement=""

PKG_CHECK_MODULES([XINE],
   [libxine >= 1.1.1 evas >= 0.9.9],
   [
    have_dep="yes"
    requirement="libxine"
   ],
   [have_dep="no"])

if test "x$1" = "xstatic" ; then
   requirement_emotion="${requirement} ${requirement_emotion}"
fi

AS_IF([test "x$have_dep" = "xyes"], [$2], [$3])

])

dnl use: EMOTION_CHECK_DEP_GSTREAMER(want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EMOTION_CHECK_DEP_GSTREAMER],
[

GST_REQS=0.10.2
GSTPLUG_REQS=0.10.1
GST_MAJORMINOR=0.10

requirement=""
PKG_CHECK_MODULES([GSTREAMER],
   [gstreamer-$GST_MAJORMINOR >= $GST_REQS gstreamer-plugins-base-$GST_MAJORMINOR >= $GSTPLUG_REQS gstreamer-video-$GST_MAJORMINOR >= $GSTPLUG_REQS evas >= 0.9.9],
   [
    have_dep="yes"
    requirement="gstreamer-$GST_MAJORMINOR gstreamer-plugins-base-$GST_MAJORMINOR"
   ],
   [have_dep="no"])

if test "x$1" = "xstatic" ; then
   requirement_emotion="${requirement} ${requirement_emotion}"
fi

AS_IF([test "x$have_dep" = "xyes"], [$2], [$3])

])

dnl use: EMOTION_CHECK_DEP_VLC(want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EMOTION_CHECK_DEP_VLC],
[

requirement=""

PKG_CHECK_MODULES([VLC],
   [libvlc >= 0.9 evas >= 0.9.9],
   [
    have_dep="yes"
    requirement="libvlc"
   ],
   [have_dep="no"])

if test "x$1" = "xstatic" ; then
   requirement_emotion="${requirement} ${requirement_emotion}"
fi

AS_IF([test "x$have_dep" = "xyes"], [$2], [$3])

])

dnl use: EMOTION_CHECK_MODULE(description, want_module[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([EMOTION_CHECK_MODULE],
[
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

want_module="$2"

AC_ARG_ENABLE([DOWN],
   [AC_HELP_STRING([--enable-]DOWN, [enable build of $1 module @<:@default=yes@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_module="yes"
    else
       if test "x${enableval}" = "xstatic" ; then
          enable_module="static"
       else
          enable_module="no"
       fi
    fi
   ],
   [enable_module="yes"])

if test "x${enable_module}" = "xyes" || test "x${enable_module}" = "xstatic" ; then
   want_module="yes"
fi

have_module="no"
if test "x${want_module}" = "xyes" && (test "x${enable_module}" = "xyes" || test "x${enable_module}" = "xstatic") ; then
   m4_default([EMOTION_CHECK_DEP_]m4_defn([UP]))(${enable_module}, [have_module="yes"], [have_module="no"])
fi

AC_MSG_CHECKING([whether to enable $1 module built])
AC_MSG_RESULT([${have_module}])

static_module="no"
if test "x${have_module}" = "xyes" && test "x${enable_module}" = "xstatic" ; then
   static_module="yes"
fi

AM_CONDITIONAL(EMOTION_BUILD_[]UP, [test "x${have_module}" = "xyes"])
AM_CONDITIONAL(EMOTION_STATIC_BUILD_[]UP, [test "x${static_module}" = "xyes"])

if test "x${static_module}" = "xyes" ; then
   AC_DEFINE(EMOTION_STATIC_BUILD_[]UP, 1, [Set to 1 if $1 is statically built])
   have_static_module="yes"
fi

enable_[]DOWN="no"
if test "x${have_module}" = "xyes" ; then
   enable_[]DOWN=${enable_module}
   AC_DEFINE(EMOTION_BUILD_[]UP, 1, [Set to 1 if $1 is built])
fi

AS_IF([test "x$have_module" = "xyes"], [$3], [$4])

m4_popdef([UP])
m4_popdef([DOWN])
])

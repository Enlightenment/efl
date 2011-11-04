##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_define([v_maj], [0])
m4_define([v_min], [2])
m4_define([v_mic], [0])
m4_define([v_rev], m4_esyscmd([(svnversion "${SVN_REPO_PATH:-.}" | grep -v export || echo 0) | awk -F : '{printf("%s\n", $1);}' | tr -d ' :MSP\n']))
m4_if(v_rev, [0], [m4_define([v_rev], m4_esyscmd([git log 2> /dev/null | (grep -m1 git-svn-id || echo 0) | sed -e 's/.*@\([0-9]*\).*/\1/' | tr -d '\n']))])
##--   When released, remove the dnl on the below line
dnl m4_undefine([v_rev])
##--   When doing snapshots - change soname. remove dnl on below line
m4_define([relname], [ver-pre-svn-07])
m4_define([v_rel], [-release relname])
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], [m4_define([v_ver], [v_maj.v_min.v_mic.v_rev])],
[m4_define([v_ver], [v_maj.v_min.v_mic])])
m4_define([lt_rev], m4_eval(v_maj + v_min))
m4_define([lt_cur], v_mic)
m4_define([lt_age], v_min)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##

AC_INIT([emotion], [v_ver], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.60])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_ISC_POSIX

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
AM_CONFIG_HEADER([config.h])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], , [m4_define([v_rev], [0])])
m4_ifdef([v_rel], , [m4_define([v_rel], [])])
AC_DEFINE_UNQUOTED(VMAJ, [v_maj], [Major version])
AC_DEFINE_UNQUOTED(VMIN, [v_min], [Minor version])
AC_DEFINE_UNQUOTED(VMIC, [v_mic], [Micro version])
AC_DEFINE_UNQUOTED(VREV, [v_rev], [Revison])
version_info="lt_rev:lt_cur:lt_age"
release_info="v_rel"
AC_SUBST(version_info)
AC_SUBST(release_info)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
VMAJ=v_maj
AC_SUBST(VMAJ)

want_generic="yes"
case "$host_os" in
   mingw* | cegcc*)
      want_xine="no"
      want_gstreamer="yes"
      want_generic_vlc="no"
      ;;
   *)
      want_xine="yes"
      want_gstreamer="yes"
      want_generic_vlc="yes"
      ;;
esac

requirement_emotion=""

### Additional options to configure

EFL_ENABLE_BIN([emotion-test])

# edje_cc

AC_ARG_WITH([edje-cc],
   [AC_HELP_STRING([--with-edje-cc=PATH], [specify a specific path to edje_cc])],
   [
    v=$withval;
    edje_cc=$v
    echo "  Enlightenment edje_cc explicitly set to "$edje_cc;
   ],
   [edje_cc=$(pkg-config --variable=prefix edje)/bin/edje_cc])
AC_SUBST(edje_cc)


### Checks for programs
AC_PROG_CC

# pkg-config
PKG_PROG_PKG_CONFIG

# Check whether pkg-config supports Requires.private
if $PKG_CONFIG --atleast-pkgconfig-version 0.22; then
   pkgconfig_requires_private="Requires.private"
else
   pkgconfig_requires_private="Requires"
fi
AC_SUBST(pkgconfig_requires_private)

# doxygen program for documentation building

EFL_CHECK_DOXYGEN([build_doc="yes"], [build_doc="no"])


### Checks for libraries

PKG_CHECK_MODULES(EMOTION, [eina >= 1.0.0 evas >= 1.0.0 ecore >= 1.0.0])
requirement_emotion="ecore >= 1.0.0 evas >= 1.0.0 eina >= 1.0.0"

if test "x${have_emotion_test}" = "xyes" ; then
   PKG_CHECK_MODULES(EMOTION_BIN, [eina >= 1.0.0 evas >= 1.0.0 ecore >= 1.0.0 ecore-evas >= 1.0.0 edje >= 1.0.0])
fi

PKG_CHECK_MODULES(ECORE_X, [ecore-x >= 1.0.0 ecore-evas >= 1.0.0], [have_ecore_x="yes"], [have_ecore_x="no"])

PKG_CHECK_MODULES(ECORE_FB, [ecore-fb >= 1.0.0], [have_ecore_fb="yes"], [have_ecore_fb="no"])

if test "x${have_ecore_x}" = "xyes" ; then
   AC_DEFINE(EMOTION_HAVE_ECORE_X, 1, [Define to 1 if Ecore_X is enabled])  
fi

if test "x${have_ecore_fb}" = "xyes" ; then
   AC_DEFINE(EMOTION_HAVE_ECORE_FB, 1, [Define to 1 if Ecore_Fb is enabled])  
fi

PKG_CHECK_EXISTS([evas-software-x11 >= 1.0.0], [AC_DEFINE(HAVE_EVAS_SOFWARE_X11, 1, [Evas Software Xlib Engine Support])])

PKG_CHECK_EXISTS([evas-xrender-x11 >= 1.0.0], [AC_DEFINE(HAVE_EVAS_XRENDER_X11, 1, [Evas Software Xlib Engine Support])])

PKG_CHECK_EXISTS([evas-opengl-x11 >= 1.0.0], [AC_DEFINE(HAVE_EVAS_OPENGL_X11, 1, [Evas OpenGL Xlib Engine Support])])

PKG_CHECK_EXISTS([evas-fb >= 1.0.0], [AC_DEFINE(HAVE_EVAS_FB, 1, [Evas Framebuffer Engine Support])])

EFL_EDJE_EXTERNAL([have_edje_external="yes"], [have_edje_external="no"])

PKG_CHECK_MODULES(EEZE, [eeze >= 1.0.99], [have_eeze="yes"], [have_eeze="no"])

if test "x${have_eeze}" = "xyes"; then
   AC_DEFINE(EMOTION_HAVE_EEZE, 1, [Define to 1 if Eeze is available])
   requirement_emotion="${requirement_emotion} eeze >= 1.0.99"
fi

### Checks for header files
AC_HEADER_STDC

### Checks for types


### Checks for structures


### Checks for compiler characteristics
AC_C_CONST
AC_C_BIGENDIAN
AC_PROG_CC_STDC
AC_C___ATTRIBUTE__

EFL_EMOTION_BUILD=""
case "$host_os" in
   mingw32ce* | cegcc*)
      EMOTION_CPPFLAGS="-D_WIN32_WCE=0x0420"
      EFL_EMOTION_BUILD="-DEFL_EMOTION_BUILD"
      ;;
   mingw*)
      EMOTION_CPPFLAGS="-D_WIN32_WINNT=0x0501"
      EFL_EMOTION_BUILD="-DEFL_EMOTION_BUILD"
      ;;
esac
AC_SUBST(EFL_EMOTION_BUILD)
AC_SUBST(EMOTION_CPPFLAGS)


### Checks for linker characteristics

# use --enable-auto-import on Windows

lt_enable_auto_import=""
case "$host_os" in
   mingw* | cegcc*)
      lt_enable_auto_import="-Wl,--enable-auto-import"
      ;;
esac
AC_SUBST(lt_enable_auto_import)

### Check for video4linux

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM(
       [[
#include <stdio.h>
#include <linux/videodev2.h>
       ]],
       [[
fprintf(stdout, "%i\n", V4L2_CAP_VIDEO_CAPTURE);
       ]])],
   [
     AC_DEFINE(HAVE_V4L2, 1, [Define to 1 if you have Video4Linux 2 available])
     have_v4l2="yes"
   ],
   [have_v4l2="no"])
AC_MSG_CHECKING([for V4L2])
AC_MSG_RESULT([${have_v4l2}])

### Modules

EMOTION_CHECK_MODULE([Xine], [${want_xine}])
EMOTION_CHECK_MODULE([Gstreamer], [${want_gstreamer}])
EMOTION_CHECK_MODULE([Generic], [${want_generic}])

EMOTION_CHECK_GENERIC_PLAYER([VLC], [${want_generic_vlc}])

if test "x${enable_xine}" = "xno" && test "x${enable_gstreamer}" = "xno" && test "x${enable_generic}" = "xno" ; then
   AC_MSG_ERROR([Xine, Gstreamer or Generic backends must be selected to build Emotion])
fi


PKG_CHECK_MODULES([EIO],
	[eio],
	[
	 have_eio="yes"
	 AC_DEFINE(HAVE_EIO, 1, [Use EIO for asynchronous file access])
	 requirement_emotion="eio ${requirement_emotion}"
	],
	[have_eio="no"])

AM_CONDITIONAL([HAVE_EIO], [test "x${have_eio}" = "xyes"])

## Check Ecore-X availability, used for XV, VAAPI, VDPAU output.

have_ecore_x="no"
want_ecore_x="auto"
AC_ARG_ENABLE([ecore-x],
   [AC_HELP_STRING([--disable-ecore-x], [disable ecore-x support. @<:@default=detect@:>@])],
   [want_ecore_x=$enableval], [])

if test "x$want_ecore_x" != "xno"; then
   PKG_CHECK_MODULES([ECORE_X],
      [ecore-x >= 1.0.0],
      [
       AC_DEFINE(HAVE_ECORE_X, 1, [X11 support for Ecore])
       have_ecore_x="yes"
       requirement_elm="ecore-x >= 1.0.0 ${requirement_elm}"
      ],
      [have_ecore_x="no"]
   )
else
    have_ecore_x="no"
fi
if test "x$want_ecore_x" = "xyes" -a "x$have_ecore_x" = "xno"; then
    AC_MSG_ERROR([ecore-x support requested, but not found by pkg-config.])
fi

## Check if gstreamer X Overlay is available
build_xoverlay="no"
if test "x$enable_gstreamer" = "xyes"; then
   PKG_CHECK_MODULES([GSTREAMER_INTERFACE],
	[ gstreamer-interfaces-0.10 >= 0.10.34 ],
	[ build_xoverlay="yes" ],
	[ build_xoverlay="no" ]
   )
   if test "x${build_xoverlay}" = "xyes"; then
      save_CPPFLAGS=$CPPFLAGS
      CPPFLAGS="$CPPFLAGS ${GSTREAMER_CFLAGS} ${GSTREAMER_INTERFACE_CFLAGS}"
      AC_CHECK_HEADER([gst/interfaces/xoverlay.h],
	[
	  AC_DEFINE(HAVE_XOVERLAY_H, 1, [Build with Gstreamer Xoverlay support])
	  build_xoverlay="yes"
	], [
     	  build_xoverlay="no"
     	]
      )
      CPPFLAGS=$save_CPPFLAGS

      save_LDFLAGS=$LDFLAGS
      LDFLAGS="$LDFLAGS ${GSTREAMER_INTERFACES}"
      AC_CHECK_LIB(gstinterfaces-0.10, gst_x_overlay_set_window_handle, AC_DEFINE(HAVE_X_OVERLAY_SET, 1, [Use gst_x_overlay_set_window_handle instead of old deprecated gst_x_overlay_set_xwindow_id]))
      LDFLAGS=$sava_LDFLAGS
   fi
fi

### install and build examples

EFL_CHECK_BUILD_EXAMPLES([enable_build_examples="yes"], [enable_build_examples="no"])
EFL_CHECK_INSTALL_EXAMPLES([enable_install_examples="yes"], [enable_install_examples="no"])

AC_SUBST(requirement_emotion)

AC_OUTPUT([
Makefile
emotion.pc
emotion.spec
src/Makefile
src/lib/Makefile
src/modules/Makefile
src/modules/xine/Makefile
src/modules/gstreamer/Makefile
src/modules/generic/Makefile
src/edje_external/Makefile
src/generic_players/Makefile
src/generic_players/vlc/Makefile
src/bin/Makefile
src/examples/Makefile
doc/Makefile
doc/Doxyfile
data/Makefile
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
echo "  Modules:"
echo "    Xine...............: ${enable_xine}"
echo "    Gstreamer..........: ${enable_gstreamer} (X: ${have_ecore_x}/${build_xoverlay})"
echo "    Generic............: ${enable_generic}"

if test "x${enable_generic}" = "xyes" || test "x${enable_generic}" = "xstatic"; then
echo
echo "  Generic Players:"
echo "    VLC................: ${enable_generic_vlc}"
fi

echo
echo "  Build emotion_test...: $have_emotion_test"
echo "  edje_cc..............: ${edje_cc}"
echo
echo "  Edje EXTERNAL support: ${have_edje_external}"
echo
echo "  Eio..................: ${have_eio}"
echo "  V4L2.................: ${have_v4l2}"
echo
echo "Documentation..........: ${build_doc}"
echo "Examples.............: ${enable_build_examples}"
echo "Examples installed...: ${enable_install_examples}"
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo
if test "x${have_static_module}" = "xyes" ; then
echo -e "\0033\01331;31mWarning\0033\01331;0m: You are trying to link statically one or more modules to Emotion."
echo "         You must know what you are doing, or else you will have a lot of problems."
echo "         And asparagus will disappear from the earth."
echo "         Think about that."
echo
fi

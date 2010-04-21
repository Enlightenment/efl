# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT([emotion], [0.1.0.043], [enlightenment-devel@lists.sourceforge.net])
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

VMAJ=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $1);}'`
VMIN=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $2);}'`
VMIC=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $3);}'`
SNAP=`echo $PACKAGE_VERSION | awk -F. '{printf("%s", $4);}'`
version_info=`expr $VMAJ + $VMIN`":$VMIC:$VMIN"
AC_SUBST(version_info)


### Needed information

case "$host_os" in
   mingw* | cegcc*)
      want_xine="no"
      want_gstreamer="yes"
      want_vlc="no"
      ;;
   *)
      want_xine="yes"
      want_gstreamer="yes"
      want_vlc="yes"
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

PKG_CHECK_MODULES(EMOTION, [eina-0 evas >= 0.9.9 ecore >= 0.9.9])
requirement_emotion="ecore evas eina-0"

if test "x${have_emotion_test}" = "xyes" ; then
   PKG_CHECK_MODULES(EMOTION_BIN, [evas >= 0.9.9 ecore >= 0.9.9 ecore-evas >= 0.9.9 edje >= 0.5.0])
fi

PKG_CHECK_MODULES(ECORE_X, [ecore-x >= 0.9.9], [have_ecore_x="yes"], [have_ecore_x="no"])

PKG_CHECK_MODULES(ECORE_FB, [ecore-fb >= 0.9.9], [have_ecore_fb="yes"], [have_ecore_fb="no"])

if test "x${have_ecore_x}" = "xyes" ; then
   AC_DEFINE(EMOTION_HAVE_ECORE_X, 1, [Define to 1 if Ecore_X is enabled])  
fi

if test "x${have_ecore_fb}" = "xyes" ; then
   AC_DEFINE(EMOTION_HAVE_ECORE_FB, 1, [Define to 1 if Ecore_Fb is enabled])  
fi

PKG_CHECK_EXISTS([evas-software-x11], [AC_DEFINE(HAVE_EVAS_SOFWARE_X11, 1, [Evas Software Xlib Engine Support])])

PKG_CHECK_EXISTS([evas-xrender-x11], [AC_DEFINE(HAVE_EVAS_XRENDER_X11, 1, [Evas Software Xlib Engine Support])])

PKG_CHECK_EXISTS([evas-opengl-x11], [AC_DEFINE(HAVE_EVAS_OPENGL_X11, 1, [Evas OpenGL Xlib Engine Support])])

PKG_CHECK_EXISTS([evas-fb], [AC_DEFINE(HAVE_EVAS_FB, 1, [Evas Framebuffer Engine Support])])

EFL_EDJE_EXTERNAL([have_edje_external="yes"], [have_edje_external="no"])


### Checks for header files
AC_HEADER_STDC

### Checks for types


### Checks for structures


### Checks for compiler characteristics
AC_C_CONST
AC_C_BIGENDIAN
AM_PROG_CC_STDC
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


### Modules

EMOTION_CHECK_MODULE([Xine], [${want_xine}])
EMOTION_CHECK_MODULE([Gstreamer], [${want_gstreamer}])
EMOTION_CHECK_MODULE([VLC], [${want_vlc}])

if test "x${enable_xine}" = "xno" && test "x${enable_gstreamer}" = "xno" && test "x${enable_vlc}" = "xno" ; then
   AC_MSG_ERROR([Xine, Gstreamer or VLC backends must be selected to build Emotion])
fi

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
src/modules/vlc/Makefile
src/edje_external/Makefile
src/bin/Makefile
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
echo "    Vlc................: ${enable_vlc}"
echo "    Gstreamer..........: ${enable_gstreamer}"
echo "    Gstreamer FFmpeg...: ${have_gst_ffmpeg}"
echo "    Gstreamer CDDA.....: ${have_gst_cdiocddasrc}"
echo
echo "  Build emotion_test...: $have_emotion_test"
echo "  edje_cc..............: ${edje_cc}"
echo
echo "  Edje EXTERNAL support: ${have_edje_external}"
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

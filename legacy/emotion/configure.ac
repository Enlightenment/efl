dnl Process this file with autoconf to produce a configure script.

# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT(emotion, 0.1.0.042, enlightenment-devel@lists.sourceforge.net)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(configure.ac)

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

AC_C_BIGENDIAN
AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST

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

PKG_CHECK_MODULES(EVAS, [evas >= 0.9.9])
PKG_CHECK_MODULES(EDJE, [edje >= 0.5.0])
PKG_CHECK_MODULES(ECORE, [ecore >= 0.9.9 ecore-evas >= 0.9.9 ecore-job >= 0.9.9])

save_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$CPPFLAGS $ECORE_CFLAGS"
AC_CHECK_HEADERS(Ecore_X.h Ecore_Fb.h)
CPPFLAGS=$save_CPPFLAGS

AC_ARG_ENABLE(vlc,
	AC_HELP_STRING([--disable-vlc],[disable vlc support @<:@default=auto@:>@]),
	[enable_vlc=$enableval],[enable_vlc=auto])
if test "$enable_vlc" != "no" ; then
	PKG_CHECK_MODULES(VLC, [libvlc >= 0.9], [HAVE_VLC="yes"], [HAVE_VLC="no"])
fi
if test "$enable_vlc$HAVE_VLC" = "yesno" ; then
	AC_MSG_RESULT(vlc not found)
fi

AC_ARG_ENABLE(xine,
	AC_HELP_STRING([--disable-xine],[disable xine support @<:@default=auto@:>@]),
	[enable_xine=$enableval],[enable_xine=auto])
HAVE_XINE="no"
if test "$enable_xine" != "no" ; then
	PKG_CHECK_MODULES(XINE, [libxine >= 1.1.1], [HAVE_XINE="yes"])
fi
if test "$enable_xine$HAVE_XINE" = "yesno" ; then
	AC_MSG_RESULT(xine not found)
fi

AC_ARG_ENABLE(gstreamer,
	AC_HELP_STRING([--disable-gstreamer],[disable gstreamer support @<:@default=auto@:>@]),
	[enable_gstreamer=$enableval],[enable_gstreamer=auto])
HAVE_GSTREAMER="no"
HAVE_GSTPLUG="no"
if test "$enable_gstreamer" != "no" ; then

	dnl Gstreamer version requirement
	GST_REQS=0.10.2
	GSTPLUG_REQS=0.10.1
	GST_MAJORMINOR=0.10

	PKG_CHECK_MODULES(GST, \
	   gstreamer-$GST_MAJORMINOR >= $GST_REQS,
	   HAVE_GSTREAMER="yes",
	   HAVE_GSTREAMER="no")

	PKG_CHECK_MODULES(GSTPLUG, \
	   gstreamer-plugins-base-$GST_MAJORMINOR >= $GSTPLUG_REQS,
	   HAVE_GSTPLUG="yes",
	   HAVE_GSTPLUG="no")

	AM_GST_ELEMENT_CHECK(
	   [ffmpeg],
	   [HAVE_GSTFFMPEG="yes"],
	   [HAVE_GSTFFMPEG="no"])

	AM_GST_ELEMENT_CHECK(
	   [cdiocddasrc],
	   [HAVE_GSTCDIOCDDASRC="yes"],
	   [HAVE_GSTCDIOCDDASRC="no"])
fi
if test "$enable_gstreamer$HAVE_GSTREAMER" = "yesno" ; then
	AC_MSG_RESULT(gstreamer not found)
fi
if test "$HAVE_GSTPLUG" = "no"; then
	if test "$HAVE_GSTREAMER" = "no" ; then
		GSTPLUG_MSG="no"
	else
		AC_MSG_WARN([You should install gstreamer plugins and gst-ffmpeg to properly decode your video and audio files])
		GSTPLUG_MSG="no (you should install gst-plugins-base and gst-ffmpeg)"
	fi
else
	if test "$HAVE_GSTFFMPEG" = "no"; then
		if test "$HAVE_GSTREAMER" = "no" ; then
			GSTPLUG_MSG="no"
		else
			AC_MSG_WARN([You should install gstreamer plugins and gst-ffmpeg to properly decode your video and audio files])
			GSTPLUG_MSG="no (you should install gst-plugins-base and gst-ffmpeg)"
		fi
		GSTPLUG_MSG="yes"
	else
		GSTPLUG_MSG="yes"
	fi
	if test "$HAVE_GSTCDIOCDDASRC" = "no"; then
		AC_MSG_WARN([You should install gstreamer-plugins-good and make sure that the cdiocddasrc element is installed])
		GSTCDDA_MSG="no (you should install gst-plugins-good and the cdiocddasrc element)"
	else
		GSTCDDA_MSG="yes"
	fi
fi

if test "$HAVE_XINE$HAVE_GSTREAMER" = "nono" ; then
	AC_MSG_ERROR([Xine or Gstreamer must be installed to build emotion])
fi

AM_CONDITIONAL([BUILD_VLC_MODULE], [test "$HAVE_VLC" = yes])
AM_CONDITIONAL([BUILD_XINE_MODULE], [test "$HAVE_XINE" = yes])
AM_CONDITIONAL([BUILD_GSTREAMER_MODULE], [test "$HAVE_GSTREAMER" = yes])

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
echo "    Xine...............: $HAVE_XINE"
echo "    Vlc................: $HAVE_VLC"
echo "    Gstreamer..........: $HAVE_GSTREAMER"
echo "    Gstreamer plugins..: $GSTPLUG_MSG"
echo "    Gstreamer FFmpeg...: $HAVE_GSTFFMPEG"
echo "    Gstreamer CDDA.....: $GSTCDDA_MSG"
echo
echo "  Compilation..........: make"
echo
echo "  Installation.........: make install"
echo
echo "    prefix.........: $prefix"
echo

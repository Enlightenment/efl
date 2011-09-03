
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EMOTION_CPPFLAGS@ \
@EMOTION_CFLAGS@ \
@EIO_CFLAGS@ \
@EFL_EMOTION_BUILD@ \
@EEZE_CFLAGS@

if EMOTION_STATIC_BUILD_XINE
AM_CPPFLAGS += @XINE_CFLAGS@
endif

if EMOTION_STATIC_BUILD_GSTREAMER
AM_CPPFLAGS += @GSTREAMER_CFLAGS@
endif

#if EMOTION_STATIC_BUILD_VLC
#AM_CPPFLAGS += @VLC_CFLAGS@
#endif

lib_LTLIBRARIES      = libemotion.la
includes_HEADERS      = Emotion.h
includesdir = $(includedir)/emotion-@VMAJ@

libemotion_la_SOURCES = emotion_smart.c emotion_main.c

if EMOTION_STATIC_BUILD_XINE
libemotion_la_SOURCES += \
$(top_srcdir)/src/modules/xine/emotion_xine.c \
$(top_srcdir)/src/modules/xine/emotion_xine_vo_out.c
endif

if EMOTION_STATIC_BUILD_GSTREAMER
libemotion_la_SOURCES += \
$(top_srcdir)/src/modules/gstreamer/emotion_gstreamer.c \
$(top_srcdir)/src/modules/gstreamer/emotion_sink.c
endif

#if EMOTION_STATIC_BUILD_VLC
#libemotion_la_SOURCES += \
#$(top_srcdir)/src/modules/vlc/emotion_vlc.c
#endif

libemotion_la_LIBADD = @EMOTION_LIBS@ @EIO_LIBS@ @EEZE_LIBS@

if EMOTION_STATIC_BUILD_XINE
libemotion_la_LIBADD += @XINE_LIBS@
endif

if EMOTION_STATIC_BUILD_GSTREAMER
libemotion_la_LIBADD += @GSTREAMER_LIBS@
endif

#if EMOTION_STATIC_BUILD_VLC
#libemotion_la_LIBADD += @VLC_LIBS@
#endif

libemotion_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@

EXTRA_DIST = emotion_private.h

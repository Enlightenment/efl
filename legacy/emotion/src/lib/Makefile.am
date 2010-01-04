
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EMOTION_CPPFLAGS@ \
@EMOTION_CFLAGS@ \
@EFL_EMOTION_BUILD@

if EMOTION_STATIC_BUILD_XINE
AM_CPPFLAGS += @XINE_CFLAGS@
endif

if EMOTION_STATIC_BUILD_GSTREAMER
AM_CPPFLAGS += @GSTREAMER_CFLAGS@
endif

if EMOTION_STATIC_BUILD_VLC
AM_CPPFLAGS += @VLC_CFLAGS@
endif

lib_LTLIBRARIES      = libemotion.la
include_HEADERS      = Emotion.h
libemotion_la_SOURCES = emotion_smart.c

if EMOTION_STATIC_BUILD_XINE
libemotion_la_SOURCES += \
$(top_srcdir)/src/modules/xine/emotion_xine.c \
$(top_srcdir)/src/modules/xine/emotion_xine_vo_out.c
endif

if EMOTION_STATIC_BUILD_GSTREAMER
libemotion_la_SOURCES += \
$(top_srcdir)/src/modules/gstreamer/emotion_gstreamer.c \
$(top_srcdir)/src/modules/gstreamer/emotion_gstreamer_pipeline.c \
$(top_srcdir)/src/modules/gstreamer/emotion_gstreamer_pipeline_cdda.c \
$(top_srcdir)/src/modules/gstreamer/emotion_gstreamer_pipeline_dvd.c \
$(top_srcdir)/src/modules/gstreamer/emotion_gstreamer_pipeline_file.c \
$(top_srcdir)/src/modules/gstreamer/emotion_gstreamer_pipeline_uri.c \
$(top_srcdir)/src/modules/gstreamer/emotion_gstreamer_pipeline_v4l.c
endif

if EMOTION_STATIC_BUILD_VLC
libemotion_la_SOURCES += \
$(top_srcdir)/src/modules/vlc/emotion_vlc.c
endif

libemotion_la_LIBADD = @EMOTION_LIBS@

if EMOTION_STATIC_BUILD_XINE
libemotion_la_LIBADD += @XINE_LIBS@
endif

if EMOTION_STATIC_BUILD_GSTREAMER
libemotion_la_LIBADD += @GSTREAMER_LIBS@
endif

if EMOTION_STATIC_BUILD_VLC
libemotion_la_LIBADD += @VLC_LIBS@
endif

libemotion_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@

EXTRA_DIST = emotion_private.h

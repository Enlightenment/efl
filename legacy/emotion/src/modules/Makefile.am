
MAINTAINERCLEANFILES = Makefile.in

if BUILD_XINE_MODULE
XINE_LIB_NAME = xine.la
endif

if BUILD_GSTREAMER_MODULE
GSTREAMER_LIB_NAME = gstreamer.la
endif

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/modules \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVAS_CFLAGS@ \
@ECORE_CFLAGS@ \
@XINE_CFLAGS@ \
@GST_CFLAGS@

pkgdir = $(libdir)/emotion

pkg_LTLIBRARIES = $(XINE_LIB_NAME) $(GSTREAMER_LIB_NAME)

if BUILD_XINE_MODULE
xine_la_SOURCES      = \
emotion_xine.c \
emotion_xine.h \
emotion_xine_vo_out.c
xine_la_LIBADD       = @EVAS_LIBS@ @ECORE_LIBS@ @XINE_LIBS@ $(top_builddir)/src/lib/libemotion.la -lpthread
xine_la_LDFLAGS      = -module -avoid-version
xine_la_DEPENDENCIES = $(top_builddir)/config.h
endif

if BUILD_GSTREAMER_MODULE
gstreamer_la_SOURCES = \
emotion_gstreamer.c \
emotion_gstreamer.h \
emotion_gstreamer_pipeline.c \
emotion_gstreamer_pipeline.h
gstreamer_la_LIBADD  = @EVAS_LIBS@ @ECORE_LIBS@ @GST_LIBS@ $(top_builddir)/src/lib/libemotion.la
gstreamer_la_LDFLAGS = -module -avoid-version
gstreamer_la_DEPENDENCIES = $(top_builddir)/config.h
endif

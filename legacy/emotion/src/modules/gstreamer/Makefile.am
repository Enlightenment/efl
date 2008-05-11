
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVAS_CFLAGS@ \
@ECORE_CFLAGS@ \
@GST_CFLAGS@

if BUILD_GSTREAMER_MODULE

pkgdir = $(libdir)/emotion

pkg_LTLIBRARIES = gstreamer.la
gstreamer_la_SOURCES = \
emotion_gstreamer.c \
emotion_gstreamer.h \
emotion_gstreamer_pipeline.c \
emotion_gstreamer_pipeline.h \
emotion_gstreamer_pipeline_cdda.c \
emotion_gstreamer_pipeline_dvd.c \
emotion_gstreamer_pipeline_file.c \
emotion_gstreamer_pipeline_uri.c
gstreamer_la_LIBADD  = @EVAS_LIBS@ @ECORE_LIBS@ @GST_LIBS@ $(top_builddir)/src/lib/libemotion.la
gstreamer_la_LDFLAGS = -module -avoid-version
gstreamer_la_LIBTOOLFLAGS = --tag=disable-static
gstreamer_la_DEPENDENCIES = $(top_builddir)/config.h

endif
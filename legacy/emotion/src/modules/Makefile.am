## Process this file with automake to produce Makefile.in

if BUILD_XINE_MODULE
XINE_LIB_NAME=emotion_decoder_xine.la
endif

if BUILD_GSTREAMER_MODULE
GSTREAMER_LIB_NAME = emotion_decoder_gstreamer.la
endif

#AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = -I$(top_srcdir) \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/modules \
		       @EVAS_CFLAGS@ @ECORE_CFLAGS@ @XINE_CFLAGS@ @GST_CFLAGS@

pkgdir                               = $(libdir)/emotion

pkg_LTLIBRARIES                      = $(XINE_LIB_NAME) $(GSTREAMER_LIB_NAME)

if BUILD_XINE_MODULE
emotion_decoder_xine_la_SOURCES      = \
emotion_xine.c \
emotion_xine.h \
emotion_xine_vo_out.c
emotion_decoder_xine_la_LIBADD       = @EVAS_LIBS@ @ECORE_LIBS@ @XINE_LIBS@ -lpthread
emotion_decoder_xine_la_LDFLAGS      = -module -avoid-version \
				       -L$(top_builddir)/src/lib -L$(top_builddir)/src/lib/.libs
emotion_decoder_xine_la_DEPENDENCIES = $(top_builddir)/config.h
endif

if BUILD_GSTREAMER_MODULE
emotion_decoder_gstreamer_la_SOURCES = \
emotion_gstreamer.c \
emotion_gstreamer.h \
emotion_gstreamer_pipeline.c \
emotion_gstreamer_pipeline.h
emotion_decoder_gstreamer_la_LIBADD  = @EVAS_LIBS@ @ECORE_LIBS@ @GST_LIBS@
emotion_decoder_gstreamer_la_LDFLAGS = -module -avoid-version \
				       -L$(top_builddir)/src/lib -L$(top_builddir)/src/lib/.libs
emotion_decoder_gstreamer_la_DEPENDENCIES = $(top_builddir)/config.h
endif

## Process this file with automake to produce Makefile.in

SUBDIRS = xine

#AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = -I$(top_srcdir) \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/modules \
		       @EVAS_CFLAGS@ @ECORE_CFLAGS@ @XINE_CFLAGS@

pkgdir               = $(libdir)/emotion

pkg_LTLIBRARIES      = emotion_decoder_xine.la
emotion_decoder_xine_la_SOURCES  = \
emotion_xine.c \
emotion_xine.h
emotion_decoder_xine_la_LIBADD       = @EVAS_LIBS@ @ECORE_LIBS@ @XINE_LIBS@
emotion_decoder_xine_la_LDFLAGS      = -module -avoid-version \
				       -L$(top_builddir)/src/lib -L$(top_builddir)/src/lib/.libs
emotion_decoder_xine_la_DEPENDENCIES = $(top_builddir)/config.h

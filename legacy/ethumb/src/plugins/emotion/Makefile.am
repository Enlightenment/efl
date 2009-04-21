MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/plugins/emotion \
	@EVAS_CFLAGS@ @EINA_CFLAGS@ @EMOTION_CFLAGS@ @ECORE_CFLAGS@

pkgdir = $(pluginsdir)
pkg_LTLIBRARIES = emotion.la
emotion_la_SOURCES = emotion.c
emotion_la_DEPENDENCIES = $(top_builddir)/config.h
emotion_la_LIBADD = $(top_builddir)/src/lib/libethumb.la \
	@EVAS_LIBS@ @EINA_LIBS@ @EMOTION_LIBS@ @ECORE_LIBS@
emotion_la_LDFLAGS = -module -avoid-version

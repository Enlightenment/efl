MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = edje_cc
EDJE_FLAGS = -v

filesdir = $(pluginsdir)/data
files_DATA = emotion_template.edj

EXTRA_DIST= \
      template.edc

AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/plugins/emotion \
	@EVAS_CFLAGS@ @EINA_CFLAGS@ @EMOTION_CFLAGS@ @ECORE_CFLAGS@ \
	@EDJE_CFLAGS@

pkgdir = $(pluginsdir)
pkg_LTLIBRARIES = emotion.la
emotion_la_SOURCES = emotion.c
emotion_la_DEPENDENCIES = $(top_builddir)/config.h
emotion_la_LIBADD = $(top_builddir)/src/lib/libethumb.la \
	@EVAS_LIBS@ @EINA_LIBS@ @EMOTION_LIBS@ @ECORE_LIBS@ @EDJE_LIBS@
emotion_la_LDFLAGS = -module -avoid-version


emotion_template.edj: Makefile $(EXTRADIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
		$(top_srcdir)/src/plugins/emotion/template.edc \
		$(top_builddir)/src/plugins/emotion/emotion_template.edj

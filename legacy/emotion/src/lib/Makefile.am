
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVAS_CFLAGS@ \
@ECORE_CFLAGS@

lib_LTLIBRARIES      = libemotion.la
include_HEADERS      = Emotion.h
libemotion_la_SOURCES  = \
emotion_smart.c \
emotion_private.h

libemotion_la_LIBADD       = @EVAS_LIBS@ @ECORE_LIBS@
libemotion_la_DEPENDENCIES = $(top_builddir)/config.h
libemotion_la_LDFLAGS      = -version-info @version_info@


MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVAS_CFLAGS@ \
@ECORE_CFLAGS@ \
@EDJE_CFLAGS@

bin_PROGRAMS = \
emotion_test

emotion_test_SOURCES = \
emotion_test_main.c

emotion_test_LDADD = \
@EVAS_LIBS@ @ECORE_LIBS@ @EDJE_LIBS@ \
$(top_builddir)/src/lib/libemotion.la

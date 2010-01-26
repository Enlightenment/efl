
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EMOTION_CPPFLAGS@ \
@EMOTION_BIN_CFLAGS@ \
@ECORE_X_CFLAGS@ \
@ECORE_FB_CFLAGS@

bin_PROGRAMS = @EMOTION_TEST_PRG@
EXTRA_PROGRAMS = emotion_test

emotion_test_SOURCES = emotion_test_main.c

emotion_test_LDADD = @ECORE_X_CFLAGS@ @ECORE_FB_CFLAGS@ @EMOTION_BIN_LIBS@ $(top_builddir)/src/lib/libemotion.la

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EMOTION_CPPFLAGS@ \
@EMOTION_CFLAGS@ \
@EFL_EMOTION_BUILD@ \
@EDJE_EXTERNAL_CFLAGS@

pkgdir = $(libdir)/edje/modules/$(MODULE_EDJE)
pkg_LTLIBRARIES = emotion.la

emotion_la_SOURCES = emotion.c
emotion_la_LIBADD = $(top_builddir)/src/lib/libemotion.la @EDJE_EXTERNAL_LIBS@
emotion_la_LDFLAGS = -no-undefined -module -avoid-version -shared -fPIC

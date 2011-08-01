MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EMOTION_CFLAGS@ \
@EMOTION_CPPFLAGS@ \
@EFL_EMOTION_BUILD@ \
@GSTREAMER_CFLAGS@

if EMOTION_BUILD_GSTREAMER
if !EMOTION_STATIC_BUILD_GSTREAMER

pkgdir = $(libdir)/emotion

pkg_LTLIBRARIES = gstreamer.la
gstreamer_la_SOURCES = \
emotion_gstreamer.c \
emotion_sink.c \
emotion_alloc.c
gstreamer_la_LIBADD  = @GSTREAMER_LIBS@ $(top_builddir)/src/lib/libemotion.la
gstreamer_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
gstreamer_la_LIBTOOLFLAGS = --tag=disable-static
gstreamer_la_DEPENDENCIES = $(top_builddir)/config.h

endif
endif

EXTRA_DIST = emotion_gstreamer.h

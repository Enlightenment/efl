MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/modules \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EMOTION_CFLAGS@ \
@EMOTION_CPPFLAGS@ \
@EFL_EMOTION_BUILD@ \
@XINE_CFLAGS@

if EMOTION_BUILD_XINE
if !EMOTION_STATIC_BUILD_XINE

pkgdir = $(libdir)/emotion

pkg_LTLIBRARIES = xine.la

xine_la_SOURCES = \
emotion_xine.c \
emotion_xine.h \
emotion_xine_vo_out.c
xine_la_LIBADD = @EMOTION_LIBS@ @XINE_LIBS@ $(top_builddir)/src/lib/libemotion.la -lpthread
xine_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
xine_la_LIBTOOLFLAGS = --tag=disable-static

endif
endif

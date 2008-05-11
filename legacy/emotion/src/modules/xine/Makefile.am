
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/modules \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVAS_CFLAGS@ \
@ECORE_CFLAGS@ \
@XINE_CFLAGS@

if BUILD_XINE_MODULE

pkgdir = $(libdir)/emotion

pkg_LTLIBRARIES = xine.la

xine_la_SOURCES      = \
emotion_xine.c \
emotion_xine.h \
emotion_xine_vo_out.c
xine_la_LIBADD       = @EVAS_LIBS@ @ECORE_LIBS@ @XINE_LIBS@ $(top_builddir)/src/lib/libemotion.la -lpthread
xine_la_LDFLAGS      = -module -avoid-version
xine_la_LIBTOOLFLAGS = --tag=disable-static
xine_la_DEPENDENCIES = $(top_builddir)/config.h

endif

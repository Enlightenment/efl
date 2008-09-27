
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
@VLC_CFLAGS@

if BUILD_VLC_MODULE

pkgdir = $(libdir)/emotion

pkg_LTLIBRARIES = vlc.la

vlc_la_SOURCES      = \
emotion_vlc.c \
emotion_vlc.h
vlc_la_LIBADD       = @EVAS_LIBS@ @ECORE_LIBS@ @VLC_LIBS@ $(top_builddir)/src/lib/libemotion.la -lpthread
vlc_la_LDFLAGS      = -module -avoid-version
vlc_la_LIBTOOLFLAGS = --tag=disable-static
vlc_la_DEPENDENCIES = $(top_builddir)/config.h

endif

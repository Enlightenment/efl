MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/modules \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EMOTION_CPPFLAGS@ \
@VLC_CFLAGS@

if EMOTION_BUILD_VLC
if !EMOTION_STATIC_BUILD_VLC

pkgdir = $(libdir)/emotion

pkg_LTLIBRARIES = vlc.la

vlc_la_SOURCES = \
emotion_vlc.c \
emotion_vlc.h
vlc_la_LIBADD = @VLC_LIBS@ $(top_builddir)/src/lib/libemotion.la -lpthread
vlc_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
vlc_la_LIBTOOLFLAGS = --tag=disable-static

endif
endif


MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I. -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVIL_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@EVAS_GENERAL_CFLAGS@

noinst_LTLIBRARIES      = libevas_cache.la
libevas_cache_la_SOURCES  = \
evas_cache_image.c \
evas_cache_engine_image.c \
evas_preload.c \
evas_cache.h

libevas_cache_la_LIBAD = @EVIL_LIBS@

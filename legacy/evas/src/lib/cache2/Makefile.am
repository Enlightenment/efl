
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS	     = -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       -I$(top_srcdir)/src/lib/cserve2 \
		       -DPACKAGE_BIN_DIR=\"$(bindir)\" \
		       -DPACKAGE_LIB_DIR=\"$(libdir)\" \
		       -DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
		       @EVIL_CFLAGS@ \
		       @FREETYPE_CFLAGS@ \
                       @PIXMAN_CFLAGS@ \
		       @EVAS_GENERAL_CFLAGS@ \
                       @PIXMAN_CFLAGS@ \
                       @FRIBIDI_CFLAGS@

if EVAS_CSERVE2

noinst_LTLIBRARIES      = libevas_cache2.la
libevas_cache2_la_SOURCES  = \
evas_cache2.c \
evas_cache2.h

libevas_cache2_la_LIBAD = @EVIL_LIBS@

endif

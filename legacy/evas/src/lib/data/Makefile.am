
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS	     = -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       -DPACKAGE_BIN_DIR=\"$(bindir)\" \
		       -DPACKAGE_LIB_DIR=\"$(libdir)\" \
		       -DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
		       @FREETYPE_CFLAGS@

noinst_LTLIBRARIES      = libevas_data.la
libevas_data_la_SOURCES  = \
evas_hash.c \
evas_list.c \
evas_array.c \
evas_object_list.c \
evas_stringshare.c \
evas_array_hash.c \
evas_mempool.c

libevas_data_la_DEPENDENCIES = $(top_builddir)/config.h

EXTRA_DIST = evas_mempool.h


MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS	     = -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       -DPACKAGE_BIN_DIR=\"$(bindir)\" \
		       -DPACKAGE_LIB_DIR=\"$(libdir)\" \
		       -DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
		       @FREETYPE_CFLAGS@ \
		       @EET_CFLAGS@ \
		       @EINA_CFLAGS@

noinst_LTLIBRARIES      = libevas_imaging.la
libevas_imaging_la_SOURCES  = \
evas_imaging.c

libevas_imaging_la_DEPENDENCIES = $(top_builddir)/config.h

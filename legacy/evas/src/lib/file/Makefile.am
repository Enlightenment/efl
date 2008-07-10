
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@FREETYPE_CFLAGS@ \
@EVIL_CFLAGS@

AM_CFLAGS = @WIN32_CFLAGS@

noinst_LTLIBRARIES      = libevas_file.la
libevas_file_la_SOURCES  = \
evas_path.c \
evas_module.c
libevas_file_la_LIBADD = @EVIL_LIBS@
libevas_file_la_DEPENDENCIES = $(top_builddir)/config.h

EXTRA_DIST = evas_module.h evas_path.h

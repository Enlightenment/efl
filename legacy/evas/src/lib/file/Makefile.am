
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@EVIL_CFLAGS@ \
@WIN32_CPPFLAGS@

AM_CFLAGS = @WIN32_CFLAGS@

noinst_LTLIBRARIES      = libevas_file.la
libevas_file_la_SOURCES  = \
evas_path.c \
evas_module.c
libevas_file_la_LIBADD = @EINA_LIBS@ @EVIL_LIBS@

EXTRA_DIST = evas_module.h evas_path.h

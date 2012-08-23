
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@FREETYPE_CFLAGS@ \
@EVAS_GENERAL_CFLAGS@ \
@EVIL_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@PIXMAN_CFLAGS@

noinst_LTLIBRARIES      = libevas_file.la
libevas_file_la_SOURCES  = \
evas_path.c \
evas_module.c \
evas_module.h \
evas_path.h

libevas_file_la_LIBADD = @EVAS_GENERAL_LIBS@ @EVIL_LIBS@


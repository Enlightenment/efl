MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(REMIX_PLUGIN_DIR)

AM_CPPFLAGS = \
-I. \
-DPACKAGE_LIB_DIR=\"$(libdir)/remix\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/remix\" \
@EDJE_CFLAGS@ \
@REMIX_CFLAGS@

pkgdir = $(REMIX_PLUGIN_DIR)
pkg_LTLIBRARIES =  libeet_sndfile_reader.la

libeet_sndfile_reader_la_SOURCES = eet_snd_reader.c
libeet_sndfile_reader_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version @EDJE_LIBS@ @REMIX_LIBS@
libeet_sndfile_reader_la_LIBTOOLFLAGS = --tag=disable-static

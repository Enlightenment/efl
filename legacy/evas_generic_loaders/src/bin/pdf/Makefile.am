MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/bin/common \
-I$(top_srcdir)/src/bin/pdf \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CFLAGS@ \
@POPPLER_CFLAGS@

bin_PROGRAMS = evas_image_loader.pdf
bindir = $(libdir)/evas/utils

evas_image_loader_pdf_SOURCES = \
main.cpp \
$(top_srcdir)/src/bin/common/shmfile.c
evas_image_loader_pdf_CFLAGS =
evas_image_loader_pdf_LDADD = @POPPLER_LIBS@ @EINA_LIBS@ @SHM_OPEN_LIBS@
evas_image_loader_pdf_LDFLAGS =

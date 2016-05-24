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

EXTRA_DIST = evas_generic_pdf_loader.libreoffice

bin_SCRIPTS = evas_generic_pdf_loader.libreoffice
bin_PROGRAMS = evas_image_loader.pdf
bindir = $(libdir)/evas/utils

evas_image_loader_pdf_SOURCES = \
main.cpp \
$(top_srcdir)/src/bin/common/shmfile.c \
$(top_srcdir)/src/bin/common/timeout.c
evas_image_loader_pdf_CFLAGS =
evas_image_loader_pdf_LDADD = @POPPLER_LIBS@ @EINA_LIBS@ @SHM_OPEN_LIBS@
evas_image_loader_pdf_LDFLAGS =

install-exec-hook:
	ln -sf evas_image_loader.pdf $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.ppt
	ln -sf evas_image_loader.pdf $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.pptx
	ln -sf evas_image_loader.pdf $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.odp
	ln -sf evas_image_loader.pdf $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.xls
	ln -sf evas_image_loader.pdf $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.xlsx
	ln -sf evas_image_loader.pdf $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.ods
	ln -sf evas_image_loader.pdf $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.doc
	ln -sf evas_image_loader.pdf $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.docx
	ln -sf evas_image_loader.pdf $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.odt
	ln -sf evas_image_loader.pdf $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.rtf
	ln -sf evas_generic_pdf_loader.libreoffice $(DESTDIR)$(libdir)/evas/utils/evas_generic_pdf_loader.ppt
	ln -sf evas_generic_pdf_loader.libreoffice $(DESTDIR)$(libdir)/evas/utils/evas_generic_pdf_loader.pptx
	ln -sf evas_generic_pdf_loader.libreoffice $(DESTDIR)$(libdir)/evas/utils/evas_generic_pdf_loader.odp
	ln -sf evas_generic_pdf_loader.libreoffice $(DESTDIR)$(libdir)/evas/utils/evas_generic_pdf_loader.xls
	ln -sf evas_generic_pdf_loader.libreoffice $(DESTDIR)$(libdir)/evas/utils/evas_generic_pdf_loader.xlsx
	ln -sf evas_generic_pdf_loader.libreoffice $(DESTDIR)$(libdir)/evas/utils/evas_generic_pdf_loader.ods
	ln -sf evas_generic_pdf_loader.libreoffice $(DESTDIR)$(libdir)/evas/utils/evas_generic_pdf_loader.doc
	ln -sf evas_generic_pdf_loader.libreoffice $(DESTDIR)$(libdir)/evas/utils/evas_generic_pdf_loader.docx
	ln -sf evas_generic_pdf_loader.libreoffice $(DESTDIR)$(libdir)/evas/utils/evas_generic_pdf_loader.odt
	ln -sf evas_generic_pdf_loader.libreoffice $(DESTDIR)$(libdir)/evas/utils/evas_generic_pdf_loader.rtf

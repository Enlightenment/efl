MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/bin/common \
-I$(top_srcdir)/src/bin/raw \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CFLAGS@

bin_PROGRAMS = evas_image_loader.raw
bindir = $(libdir)/evas/utils

evas_image_loader_raw_SOURCES = main.c $(top_srcdir)/src/bin/common/shmfile.c
evas_image_loader_raw_CFLAGS = @LIBRAW_CFLAGS@
evas_image_loader_raw_LDADD = @SHM_OPEN_LIBS@ @LIBRAW_LIBS@ -lm
evas_image_loader_raw_LDFLAGS =

install-data-hook:
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.arw
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.cr2
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.crw
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.dcr
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.dng
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.k25
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.kdc
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.erf
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.mrw
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.nef
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.nrf
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.nrw
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.orf
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.rw2
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.pef
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.raf
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.sr2
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.srf
	ln -sf evas_image_loader.raw $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.x3f

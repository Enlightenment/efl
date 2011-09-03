MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/bin/common \
-I$(top_srcdir)/src/bin/xcf \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CFLAGS@

bin_PROGRAMS = evas_image_loader.xcf
bindir = $(libdir)/evas/utils

evas_image_loader_xcf_SOURCES = \
main.c pixelfuncs.c common.h \
$(top_srcdir)/src/bin/common/shmfile.c
evas_image_loader_xcf_CFLAGS = @EINA_CFLAGS@
evas_image_loader_xcf_LDADD = @EINA_LIBS@ -lz -lm @SHM_OPEN_LIBS@
evas_image_loader_xcf_LDFLAGS =

install-data-hook:
	ln -sf evas_image_loader.xcf $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.xcf.gz

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/bin/common \
-I$(top_srcdir)/src/bin/svg \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CFLAGS@

bin_PROGRAMS = evas_image_loader.svg
bindir = $(libdir)/evas/utils

evas_image_loader_svg_SOURCES = \
main.c \
$(top_srcdir)/src/bin/common/shmfile.c \
$(top_srcdir)/src/bin/common/timeout.c
evas_image_loader_svg_CFLAGS = @SVG_CFLAGS@
evas_image_loader_svg_LDADD = @SVG_LIBS@ @SHM_OPEN_LIBS@ -lm
evas_image_loader_svg_LDFLAGS =


install-exec-hook:
	ln -sf evas_image_loader.svg $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.svgz
	ln -sf evas_image_loader.svg $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.svg.gz

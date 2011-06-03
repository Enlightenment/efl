MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/bin/common \
-I$(top_srcdir)/src/bin/gst \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CFLAGS@ \
@GSTREAMER_CFLAGS@

bin_PROGRAMS = evas_image_loader.gst

evas_image_loader_gst_SOURCES = \
main.cpp \
$(top_srcdir)/src/bin/common/shmfile.c
evas_image_loader_gst_CFLAGS =
evas_image_loader_gst_LDADD = @GSTREAMER_LIBS@ @EINA_LIBS@ @SHM_OPEN_LIBS@
evas_image_loader_gst_LDFLAGS =

install-data-hook:
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.3g2
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.3gp
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.asf
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.avi
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.mkv
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.mov
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.mp4
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.mpg
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.mpeg
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.mxf
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.nut
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.ogg
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.ogm
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.rm
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.webm
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.weba
	ln -sf evas_image_loader.gst $(DESTDIR)$(bindir)/evas_image_loader.wmv

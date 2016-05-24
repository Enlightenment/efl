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
bindir = $(libdir)/evas/utils

if HAVE_GST1

evas_image_loader_gst_SOURCES = \
main.c \
$(top_srcdir)/src/bin/common/shmfile.c \
$(top_srcdir)/src/bin/common/timeout.c

else

evas_image_loader_gst_SOURCES = \
main_0_10.c \
$(top_srcdir)/src/bin/common/shmfile.c \
$(top_srcdir)/src/bin/common/timeout.c

endif

evas_image_loader_gst_CFLAGS =
evas_image_loader_gst_LDADD = @GSTREAMER_LIBS@ @EINA_LIBS@ @SHM_OPEN_LIBS@
evas_image_loader_gst_LDFLAGS =

## good list of extensions:
## http://www.fileinfo.com/filetypes/video
## should check which ones gst does support and make sure they are here
install-exec-hook:
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.264
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.3g2
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.3gp
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.3gp2
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.3gpp
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.3gpp2
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.3p2
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.asf
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.avi
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.bdm
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.bdmv
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.clpi
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.cpi
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.dv
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.fla
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.flv
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.m1v
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.m2t
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.m2v
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.m4v
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.mkv
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.mov
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.mp2
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.mp2ts
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.mp4
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.mpe
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.mpeg
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.mpg
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.mpl
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.mpls
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.mts
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.mxf
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.nut
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.nuv
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.ogg
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.ogm
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.ogv
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.qt
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.rm
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.rmj
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.rmm
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.rms
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.rmx
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.rmvb
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.rv
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.swf
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.ts
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.webm
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.weba
	ln -sf evas_image_loader.gst $(DESTDIR)$(libdir)/evas/utils/evas_image_loader.wmv

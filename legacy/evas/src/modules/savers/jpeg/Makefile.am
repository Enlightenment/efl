
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS	       = -I. \
			 -I$(top_srcdir)/src/lib \
			 -I$(top_srcdir)/src/lib/include \
			 @FREETYPE_CFLAGS@ @jpeg_cflags@

pkgdir                 = $(libdir)/evas/modules/savers/jpeg/$(MODULE_ARCH)

pkg_LTLIBRARIES        = module.la

module_la_SOURCES      = evas_image_save_jpeg.c

module_la_LIBADD       = @jpeg_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS      = @create_shared_lib@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static
module_la_DEPENDENCIES = $(top_builddir)/config.h

EXTRA_DIST             = evas_image_save_jpeg.c

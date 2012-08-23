
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/lib/cserve2 \
-I$(top_srcdir)/src/bin \
@EVAS_GENERAL_CFLAGS@ \
@evas_image_loader_jpeg_cflags@ \
@EVIL_CFLAGS@

if BUILD_LOADER_JPEG
#if !EVAS_STATIC_BUILD_JPEG

pkgdir = $(libdir)/evas/cserve2/loaders/jpeg/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = evas_image_load_jpeg.c

module_la_LIBADD = @EVAS_GENERAL_LIBS@ @EVIL_LIBS@ @evas_image_loader_jpeg_libs@
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

#else

#noinst_LTLIBRARIES = libevas_loader_jpeg.la
#libevas_loader_jpeg_la_SOURCES = evas_image_load_jpeg.c
#libevas_loader_jpeg_la_LIBADD = @evas_image_loader_jpeg_libs@

#endif
endif

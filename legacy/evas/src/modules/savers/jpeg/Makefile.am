
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS= \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@evas_image_loader_jpeg_cflags@

AM_CFLAGS = @WIN32_CFLAGS@

pkgdir = $(libdir)/evas/modules/savers/jpeg/$(MODULE_ARCH)

if !EVAS_STATIC_BUILD_JPEG

pkg_LTLIBRARIES = module.la

module_la_SOURCES = evas_image_save_jpeg.c

module_la_LIBADD = @EINA_LIBS@ @evas_image_loader_jpeg_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_saver_jpeg.la
libevas_saver_jpeg_la_SOURCES = evas_image_save_jpeg.c
libevas_saver_jpeg_la_LIBADD = @evas_image_loader_jpeg_libs@

endif

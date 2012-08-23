
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@EVAS_GENERAL_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@evas_image_loader_tiff_cflags@ \
@EVIL_CFLAGS@

if BUILD_LOADER_TIFF
if !EVAS_STATIC_BUILD_TIFF

pkgdir = $(libdir)/evas/modules/loaders/tiff/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = evas_image_load_tiff.c

module_la_LIBADD = @EVAS_GENERAL_LIBS@ @EVIL_LIBS@ @evas_image_loader_tiff_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_loader_tiff.la

libevas_loader_tiff_la_SOURCES = evas_image_load_tiff.c
libevas_loader_tiff_la_LIBADD = @evas_image_loader_tiff_libs@

endif
endif

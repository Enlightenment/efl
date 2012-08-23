
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@EVAS_GENERAL_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@evas_image_loader_png_cflags@ \
@EVIL_CFLAGS@

if BUILD_LOADER_PNG
if !EVAS_STATIC_BUILD_PNG

pkgdir = $(libdir)/evas/modules/loaders/png/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = evas_image_load_png.c

module_la_LIBADD = @EVAS_GENERAL_LIBS@ @evas_image_loader_png_libs@ @EVIL_LIBS@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_loader_png.la
libevas_loader_png_la_SOURCES = evas_image_load_png.c
libevas_loader_png_la_LIBADD = @evas_image_loader_png_libs@

endif
endif

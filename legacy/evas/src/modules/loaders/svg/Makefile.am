
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@evas_image_loader_svg_cflags@

AM_CFLAGS = @WIN32_CFLAGS@

pkgdir = $(libdir)/evas/modules/loaders/svg/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la

module_la_SOURCES = evas_image_load_svg.c

module_la_LIBADD = @EINA_LIBS@ @evas_image_loader_svg_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

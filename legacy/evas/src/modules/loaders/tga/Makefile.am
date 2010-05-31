
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@EVIL_CFLAGS@ \
@WIN32_CPPFLAGS@

if BUILD_LOADER_TGA
if !EVAS_STATIC_BUILD_TGA

pkgdir = $(libdir)/evas/modules/loaders/tga/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = evas_image_load_tga.c

module_la_LIBADD = @EINA_LIBS@ @EVIL_LIBS@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_loader_tga.la
libevas_loader_tga_la_SOURCES = evas_image_load_tga.c
libevas_loader_tga_la_LIBADD = 

endif
endif

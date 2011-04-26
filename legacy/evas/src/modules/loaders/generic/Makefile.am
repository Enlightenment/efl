
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@evas_image_loader_generic_cflags@ \
@EINA_CFLAGS@ \
@EVIL_CFLAGS@ \
@WIN32_CPPFLAGS@

AM_CFLAGS = @WIN32_CFLAGS@


if BUILD_LOADER_GENERIC
if !EVAS_STATIC_BUILD_GENERIC

pkgdir = $(libdir)/evas/modules/loaders/generic/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = evas_image_load_generic.c

module_la_LIBADD = @EINA_LIBS@ @evas_image_loader_generic_libs@ @SHM_OPEN_LINK@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_loader_generic.la

libevas_loader_generic_la_SOURCES = evas_image_load_generic.c
libevas_loader_generic_la_LIBADD = @evas_image_loader_generic_libs@ @SHM_OPEN_LINK@

endif
endif

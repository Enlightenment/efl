
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@FREETYPE_CFLAGS@ \
@evas_image_loader_edb_cflags@ \
@EINA_CFLAGS@

pkgdir                 = $(libdir)/evas/modules/loaders/edb/$(MODULE_ARCH)

if !EVAS_STATIC_BUILD_EDB

pkg_LTLIBRARIES        = module.la

module_la_SOURCES      = evas_image_load_edb.c

module_la_LIBADD       = @evas_image_loader_edb_libs@ @EINA_LIBS@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS      = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_loader_edb.la

libevas_loader_edb_la_SOURCES = evas_image_load_edb.c
libevas_loader_edb_la_LIBADD = @evas_image_loader_edb_libs@

endif

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS	       = -I. \
			 -I$(top_srcdir)/src/lib \
			 -I$(top_srcdir)/src/lib/include \
			 @FREETYPE_CFLAGS@ @evas_image_loader_edb_cflags@ \
			 @EINA_CFLAGS@

if BUILD_LOADER_EDB
if !EVAS_STATIC_BUILD_EDB

pkgdir                 = $(libdir)/evas/modules/savers/edb/$(MODULE_ARCH)
pkg_LTLIBRARIES        = module.la

module_la_SOURCES      = evas_image_save_edb.c

module_la_LIBADD       = @EINA_LIBS@ @evas_image_loader_edb_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS      = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_saver_edb.la

libevas_saver_edb_la_SOURCES = evas_image_save_edb.c
libevas_saver_edb_la_LIBADD = @evas_image_loader_edb_libs@

endif
endif

EXTRA_DIST             = evas_image_save_edb.c


MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@EET_CFLAGS@

AM_CFLAGS = @WIN32_CFLAGS@

pkgdir = $(libdir)/evas/modules/savers/eet/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la

module_la_SOURCES = evas_image_save_eet.c

module_la_LIBADD = @EINA_LIBS@ @EET_LIBS@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = @lt_no_undefined@ @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static
module_la_DEPENDENCIES = $(top_builddir)/config.h


MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@EVAS_GENERAL_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@evas_image_loader_eet_cflags@

if BUILD_LOADER_EET
if !EVAS_STATIC_BUILD_EET

pkgdir = $(libdir)/evas/modules/savers/eet/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = evas_image_save_eet.c

module_la_LIBADD = @EVAS_GENERAL_LIBS@ @evas_image_loader_eet_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_saver_eet.la

libevas_saver_eet_la_SOURCES = evas_image_save_eet.c
libevas_saver_eet_la_LIBADD = @evas_image_loader_eet_libs@

endif
endif

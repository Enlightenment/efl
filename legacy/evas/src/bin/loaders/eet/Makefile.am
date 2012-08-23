
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/lib/cserve2 \
-I$(top_srcdir)/src/bin \
@evas_image_loader_eet_cflags@ \
@EVAS_GENERAL_CFLAGS@


if BUILD_LOADER_EET
#if !EVAS_STATIC_BUILD_EET

pkgdir = $(libdir)/evas/cserve2/loaders/eet/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = evas_image_load_eet.c

module_la_LIBADD = @EVAS_GENERAL_LIBS@ @evas_image_loader_eet_libs@
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

#else

#noinst_LTLIBRARIES = libevas_loader_eet.la

#libevas_loader_eet_la_SOURCES = evas_image_load_eet.c
#libevas_loader_eet_la_LIBADD = @evas_image_loader_eet_libs@

#endif
endif

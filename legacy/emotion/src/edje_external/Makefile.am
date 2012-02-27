MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EMOTION_CPPFLAGS@ \
@EMOTION_CFLAGS@ \
@EDJE_EXTERNAL_CFLAGS@ \
@EFL_EMOTION_BUILD@

pkgdir = $(libdir)/edje/modules/emotion/$(MODULE_EDJE)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = emotion.c
module_la_LIBADD = @EMOTION_LIBS@ $(top_builddir)/src/lib/libemotion.la @EDJE_EXTERNAL_LIBS@
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

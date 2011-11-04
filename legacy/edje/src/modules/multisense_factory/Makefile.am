MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/modules/multisense_factory

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DPACKAGE_EXAMPLES_DIR=\"$(datadir)/$(PACKAGE)/multisense_factory\" \
@EDJE_CFLAGS@ \
@REMIX_CFLAGS@

pkgdir = $(libdir)/edje/modules/multisense_factory/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = multisense_factory.c
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version $(top_builddir)/src/lib/libedje.la @EDJE_LIBS@ @REMIX_LIBS@
module_la_LIBTOOLFLAGS = --tag=disable-static

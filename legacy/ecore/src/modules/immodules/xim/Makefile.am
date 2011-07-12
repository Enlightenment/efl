MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_x \
-I$(top_srcdir)/src/lib/ecore_imf \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore_x \
-I$(top_builddir)/src/lib/ecore_imf \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CFLAGS@

pkgdir = $(libdir)/ecore/immodules

pkg_LTLIBRARIES = xim.la
xim_la_SOURCES = \
ecore_imf_xim.c
xim_la_LIBADD  = $(top_builddir)/src/lib/ecore_imf/libecore_imf.la
xim_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
xim_la_LIBTOOLFLAGS = --tag=disable-static
xim_la_DEPENDENCIES = $(top_builddir)/config.h

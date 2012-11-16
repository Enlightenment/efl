MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_x \
-I$(top_srcdir)/src/lib/ecore_imf \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore_x \
-I$(top_builddir)/src/lib/ecore_imf \
-I$(top_builddir)/src/lib/ecore_evas \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@IBUS_CFLAGS@ \
@EVAS_CFLAGS@ \
@EINA_CFLAGS@

pkgdir = $(libdir)/ecore/immodules

pkg_LTLIBRARIES = ibus.la
ibus_la_SOURCES = \
ibus_module.c \
ibus_imcontext.c \
ibus_imcontext.h

ibus_la_LIBADD = \
$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
$(top_builddir)/src/lib/ecore_imf/libecore_imf.la \
$(top_builddir)/src/lib/ecore_x/libecore_x.la \
@IBUS_LIBS@ \
@EVAS_LIBS@ \
@EINA_LIBS@
ibus_la_LDFLAGS = -no-undefined -module -avoid-version
ibus_la_LIBTOOLFLAGS = --tag=disable-static

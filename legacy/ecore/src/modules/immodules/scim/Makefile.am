MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
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
@SCIM_CFLAGS@ \
@EVAS_CFLAGS@ \
@EINA_CFLAGS@

pkgdir = $(libdir)/ecore/immodules

pkg_LTLIBRARIES = scim.la
scim_la_SOURCES = \
scim_imcontext.cpp \
scim_module.cpp \
scim_imcontext.h

scim_la_LIBADD = \
	 $(top_builddir)/src/lib/ecore_imf/libecore_imf.la \
	 $(top_builddir)/src/lib/ecore_x/libecore_x.la \
	@SCIM_LIBS@ \
	@EVAS_LIBS@ \
	@EINA_LIBS@
scim_la_LDFLAGS = -no-undefined -module -avoid-version
scim_la_LIBTOOLFLAGS = --tag=disable-static

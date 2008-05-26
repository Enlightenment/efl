MAINTAINERCLEANFILES = Makefile.in

if BUILD_ECORE_IMF_EVAS
AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_imf \
@EVAS_CFLAGS@

lib_LTLIBRARIES = libecore_imf_evas.la
include_HEADERS = \
Ecore_IMF_Evas.h

libecore_imf_evas_la_SOURCES = \
ecore_imf_evas.c

libecore_imf_evas_la_LIBADD = \
$(top_builddir)/src/lib/ecore_imf/libecore_imf.la \
@EVAS_LIBS@

libecore_imf_evas_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore_imf/libecore_imf.la

libecore_imf_evas_la_LDFLAGS = @lt_no_undefined@ @lt_enable_auto_import@ -version-info @version_info@
endif

EXTRA_DIST = \
Ecore_IMF_Evas.h \
ecore_imf_evas.c

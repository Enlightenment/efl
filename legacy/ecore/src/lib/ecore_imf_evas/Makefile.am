MAINTAINERCLEANFILES = Makefile.in

if BUILD_ECORE_IMF_EVAS
AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_imf \
@EFL_ECORE_IMF_BUILD@ \
@EVAS_CFLAGS@ \
@EINA_CFLAGS@

lib_LTLIBRARIES = libecore_imf_evas.la
include_HEADERS = \
Ecore_IMF_Evas.h

libecore_imf_evas_la_SOURCES = \
ecore_imf_evas.c

libecore_imf_evas_la_LIBADD = \
$(top_builddir)/src/lib/ecore_imf/libecore_imf.la \
@EVAS_LIBS@ \
@EINA_LIBS@

libecore_imf_evas_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@
endif

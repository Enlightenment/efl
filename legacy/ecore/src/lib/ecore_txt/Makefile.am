MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
@iconv_cflags@

if BUILD_ECORE_TXT

lib_LTLIBRARIES = libecore_txt.la

include_HEADERS = \
Ecore_Txt.h

libecore_txt_la_SOURCES = \
ecore_txt.c

libecore_txt_la_LIBADD = @iconv_libs@ \
@EINA_LIBS@

libecore_txt_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@


endif

EXTRA_DIST = ecore_txt_private.h

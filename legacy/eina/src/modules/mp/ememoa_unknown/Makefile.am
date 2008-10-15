MAINTAINERCLEANFILES = \
Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/include \
-I$(top_builddir)/src/include \
@EINA_CFLAGS@ \
@EMEMOA_CFLAGS@

if EINA_ENABLE_EMEMOA

if !EINA_STATIC_BUILD_EMEMOA_UNKNOWN

controllerdir = $(libdir)/eina/mp/
controller_LTLIBRARIES = eina_ememoa_unknown.la

eina_ememoa_unknown_la_SOURCES = \
eina_ememoa_unknown.c

eina_ememoa_unknown_la_LIBADD       = $(top_builddir)/src/lib/libeina.la @EMEMOA_LIBS@ @EINA_LIBS@
eina_ememoa_unknown_la_LDFLAGS      = -no-undefined @lt_enable_auto_import@ -module -avoid-version
eina_ememoa_unknown_la_LIBTOOLFLAGS = --tag=disable-static

endif

endif

clean-local:
	rm -rf *.gcno

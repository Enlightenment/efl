MAINTAINERCLEANFILES = \
Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/include \
@COVERAGE_CFLAGS@ \
@EMEMOA_CFLAGS@

if EINA_ENABLE_EMEMOA

controllerdir = $(libdir)/eina/chained_pool/
controller_LTLIBRARIES = eina_ememoa_fixed.la

eina_ememoa_fixed_la_SOURCES = \
eina_ememoa_fixed.c

eina_ememoa_fixed_la_LIBADD       = $(top_builddir)/src/lib/libeina.la @EMEMOA_LIBS@ @COVERAGE_LIBS@
eina_ememoa_fixed_la_LDFLAGS      = -no-undefined @lt_enable_auto_import@ -module -avoid-version
eina_ememoa_fixed_la_DEPENDENCIES = $(top_builddir)/src/lib/libeina.la

endif

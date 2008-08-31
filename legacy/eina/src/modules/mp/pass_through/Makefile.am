MAINTAINERCLEANFILES = \
Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/include \
@COVERAGE_CFLAGS@ \
@EMEMOA_CFLAGS@

controllerdir = $(libdir)/eina/pass_through/
controller_LTLIBRARIES = pass_through.la

pass_through_la_SOURCES = \
pass_through.c

pass_through_la_LIBADD       = $(top_builddir)/src/lib/libeina.la @COVERAGE_LIBS@
pass_through_la_LDFLAGS      = -no-undefined @lt_enable_auto_import@ -module -avoid-version
pass_through_la_DEPENDENCIES = $(top_builddir)/src/lib/libeina.la

clean-local:
	rm -rf *.gcno


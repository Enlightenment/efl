MAINTAINERCLEANFILES = \
Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/include \
-I$(top_builddir)/src/include \
@EINA_CFLAGS@

if !EINA_STATIC_BUILD_PASS_THROUGH

controllerdir = $(libdir)/eina/mp/
controller_LTLIBRARIES = pass_through.la

pass_through_la_SOURCES = \
pass_through.c

pass_through_la_LIBADD       = $(top_builddir)/src/lib/libeina.la @EINA_LIBS@
pass_through_la_LDFLAGS      = -no-undefined @lt_enable_auto_import@ -module -avoid-version
pass_through_la_LIBTOOLFLAGS = --tag=disable-static

endif

clean-local:
	rm -rf *.gcno


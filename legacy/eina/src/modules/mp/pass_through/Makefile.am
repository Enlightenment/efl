MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/include \
-I$(top_builddir)/src/include \
@EINA_CPPFLAGS@ \
@EINA_CFLAGS@ \
@EFL_EINA_BUILD@

if EINA_BUILD_PASS_THROUGH
if !EINA_STATIC_BUILD_PASS_THROUGH

controllerdir = $(libdir)/eina/mp
controller_LTLIBRARIES = eina_pass_through.la

eina_pass_through_la_SOURCES = \
eina_pass_through.c

eina_pass_through_la_LIBADD = $(top_builddir)/src/lib/libeina.la @EINA_LIBS@
eina_pass_through_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
eina_pass_through_la_LIBTOOLFLAGS = --tag=disable-static

endif
endif

clean-local:
	rm -rf *.gcno


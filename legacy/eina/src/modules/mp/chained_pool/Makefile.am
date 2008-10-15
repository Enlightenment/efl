MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/include \
-I$(top_builddir)/src/include \
@EINA_CFLAGS@

if !EINA_STATIC_BUILD_CHAINED_POOL

controllerdir = $(libdir)/eina/mp/
controller_LTLIBRARIES = eina_chained_mempool.la

eina_chained_mempool_la_SOURCES = \
eina_chained_mempool.c

eina_chained_mempool_la_LIBADD = $(top_builddir)/src/lib/libeina.la @EINA_LIBS@
eina_chained_mempool_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
eina_chained_mempool_la_LIBTOOLFLAGS = --tag=disable-static

endif

clean-local:
	rm -rf *.gcno

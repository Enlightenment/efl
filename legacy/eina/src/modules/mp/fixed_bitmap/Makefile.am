MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/include \
-I$(top_builddir)/src/include \
@EINA_CFLAGS@

if !EINA_STATIC_BUILD_FIXED_BITMAP

controllerdir = $(libdir)/eina/mp/
controller_LTLIBRARIES = eina_fixed_bitmap.la

eina_fixed_bitmap_la_SOURCES = \
eina_fixed_bitmap.c

eina_fixed_bitmap_la_LIBADD = $(top_builddir)/src/lib/libeina.la @EINA_LIBS@
eina_fixed_bitmap_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
eina_fixed_bitmap_la_LIBTOOLFLAGS = --tag=disable-static
eina_fixed_bitmap_la_DEPENDENCIES = $(top_builddir)/src/lib/libeina.la

endif

clean-local:
	rm -rf *.gcno

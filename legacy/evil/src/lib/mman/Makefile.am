
MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libmman.la

nobase_include_HEADERS = sys/mman.h
libmman_la_SOURCES = mman.c

libmman_la_CFLAGS = @win32_cflags@
libmman_la_LIBADD = $(top_builddir)/src/lib/libevil.la
libmman_la_DEPENDENCIES = $(top_builddir)/config.h $(top_builddir)/src/lib/libevil.la
libmman_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@


MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libdl.la

include_HEADERS = dlfcn.h
libdl_la_SOURCES = dlfcn.c

libdl_la_CFLAGS = @win32_cflags@
libdl_la_LIBADD = $(top_builddir)/src/lib/libevil.la
libdl_la_DEPENDENCIES = $(top_builddir)/src/lib/libevil.la
libdl_la_LDFLAGS = -no-undefined -Wl,--enable-auto-import -version-info @version_info@


MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libdl.la

include_HEADERS = dlfcn.h
libdl_la_SOURCES = dlfcn.c

libdl_la_CPPFLAGS = -DEFL_EVIL_DLFCN_BUILD
libdl_la_CFLAGS = @win32_cflags@
libdl_la_LIBADD = $(top_builddir)/src/lib/libevil.la $(EFL_MPATROL_LIBS)
libdl_la_LDFLAGS = -no-undefined -Wl,--enable-auto-import -version-info @version_info@

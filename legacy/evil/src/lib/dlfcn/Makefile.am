
MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libdl.la


stdheadersdir = $(includedir)/evil-@VMAJ@
dist_stdheaders_DATA = dlfcn.h

libdl_la_SOURCES = dlfcn.c

libdl_la_CPPFLAGS = @win32_cppflags@ -DEFL_EVIL_DLFCN_BUILD -DPSAPI_VERSION=1
libdl_la_CFLAGS = @win32_cflags@
libdl_la_LIBADD = $(top_builddir)/src/lib/libevil.la $(EFL_MPATROL_LIBS) @win32_dl_libs@ @win32_libs@
libdl_la_LDFLAGS = -no-undefined -Wl,--enable-auto-import -version-info @version_info@

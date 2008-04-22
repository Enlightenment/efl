
MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libmman.la

nobase_include_HEADERS = sys/mman.h
libmman_la_SOURCES = mman.c

libmman_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@

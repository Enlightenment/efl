
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = dlfcn mman

lib_LTLIBRARIES = libevil.la

include_HEADERS = Evil.h
libevil_la_SOURCES = evil.c

libevil_la_CFLAGS = @win32_cflags@
libevil_la_LIBADD = @win32_libs@
libevil_la_LDFLAGS = -no-undefined -version-info @version_info@


MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = @EIO_CFLAGS@
AM_CFLAGS = @EIO_CFLAGS@

lib_LTLIBRARIES = libeio.la

includes_HEADERS = Eio.h
includesdir = $(includedir)/eio-@VMAJ@

libeio_la_SOURCES = \
eio_file.c

libeio_la_LIBADD = @EIO_LIBS@
libeio_la_LDFLAGS  = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@


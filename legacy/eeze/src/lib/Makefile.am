MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = @EEZE_CFLAGS@

lib_LTLIBRARIES = libeeze.la
includes_HEADERS = Eeze.h
includesdir = $(includedir)/eeze-@VMAJ@

libeeze_la_SOURCES = \
eeze_udev_syspath.c \
eeze_udev_watch.c \
eeze_main.c \
eeze_udev_find.c \
eeze_udev_walk.c \
eeze_udev_private.c

libeeze_la_LIBADD = @EEZE_LIBS@
libeeze_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@

EXTRA_DIST = eeze_udev_private.h

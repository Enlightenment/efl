MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = @ECORE_CFLAGS@

lib_LTLIBRARIES = libeeze.la
include_HEADERS = Eeze.h

noinst_HEADERS = eeze_udev_private.h eeze_private.h

libeeze_la_SOURCES = \
Eeze.h \
eeze_udev.c \
eeze_udev_watch.c \
eeze_main.c \
eeze_udev_find.c \
eeze_udev_walk.c \
eeze_udev_private.c

libeeze_la_LIBADD = @ECORE_LIBS@ @UDEV_LIBS@
libeeze_la_LDFLAGS = -no-undefined -version-info @version_info@ @eeze_release_info@


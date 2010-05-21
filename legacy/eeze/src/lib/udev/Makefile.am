MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = @ECORE_CFLAGS@

if BUILD_EEZE_UDEV

lib_LTLIBRARIES = libeeze_udev.la
include_HEADERS = Eeze_Udev.h

noinst_HEADERS = eeze_udev_private.h

libeeze_udev_la_SOURCES = \
Eeze_Udev.h \
eeze_udev.c \
eeze_udev_watch.c \
eeze_udev_main.c \
eeze_udev_find.c \
eeze_udev_walk.c \
eeze_udev_private.c

libeeze_udev_la_LIBADD = @ECORE_LIBS@ @UDEV_LIBS@
libeeze_udev_la_LDFLAGS = -no-undefined -version-info @version_info@ @eeze_udev_release_info@

endif

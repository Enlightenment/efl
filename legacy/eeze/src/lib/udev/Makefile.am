MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = @ECORE_CFLAGS@

if BUILD_EUDEV

lib_LTLIBRARIES = libeudev.la
include_HEADERS = E_Udev.h

noinst_HEADERS = e_udev_private.h

libeudev_la_SOURCES = \
E_Udev.h \
e_udev.c \
e_udev_watch.c \
e_udev_main.c

libeudev_la_LIBADD = @ECORE_LIBS@ @UDEV_LIBS@
libeudev_la_LDFLAGS = -no-undefined -version-info @version_info@ @eudev_release_info@

endif

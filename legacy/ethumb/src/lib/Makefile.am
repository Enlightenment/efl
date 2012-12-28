MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = .

if USE_MODULE_ETHUMBD
SUBDIRS += client
endif

AM_CPPFLAGS = \
-I$(top_srcdir) \
-DPLUGINSDIR=\"$(pkglibdir)/plugins\" \
-DEFL_ETHUMB_BUILD \
@EVIL_CFLAGS@ \
@EINA_CFLAGS@ \
@EVAS_CFLAGS@ \
@ECORE_CFLAGS@ \
@ECORE_EVAS_CFLAGS@ \
@ECORE_FILE_CFLAGS@ \
@EDJE_CFLAGS@

if HAVE_LIBEXIF
AM_CPPFLAGS += @LIBEXIF_CFLAGS@
endif

includes_HEADERS = Ethumb.h Ethumb_Plugin.h
includesdir = $(includedir)/ethumb-@VMAJ@

noinst_HEADERS = md5.h ethumb_private.h

lib_LTLIBRARIES = libethumb.la


libethumb_la_SOURCES = ethumb.c md5.c
libethumb_la_LIBADD = \
@EDJE_LIBS@ \
@ECORE_FILE_LIBS@ \
@ECORE_EVAS_LIBS@ \
@ECORE_LIBS@ \
@EVAS_LIBS@ \
@EINA_LIBS@ \
@EVIL_LIBS@

if HAVE_LIBEXIF
libethumb_la_LIBADD += @LIBEXIF_LIBS@
endif

libethumb_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@

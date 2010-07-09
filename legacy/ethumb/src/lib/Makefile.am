MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = .

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	-DPLUGINSDIR=\"$(libdir)/ethumb/plugins\" \
	@EINA_CFLAGS@ @EVAS_CFLAGS@ @ECORE_EVAS_CFLAGS@ @ECORE_FILE_CFLAGS@ @EDJE_CFLAGS@ \
	@EFL_ETHUMB_BUILD@

includes_HEADERS = Ethumb.h Ethumb_Plugin.h
includesdir = $(includedir)/ethumb-@VMAJ@

noinst_HEADERS = md5.h ethumb_private.h

lib_LTLIBRARIES = libethumb.la


libethumb_la_SOURCES = \
	Ethumb.c \
	md5.c
libethumb_la_LIBADD = \
	@EDJE_LIBS@ @ECORE_FILE_LIBS@ @ECORE_EVAS_LIBS@ @EVAS_LIBS@
libethumb_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@


if USE_MODULE_ETHUMBD
SUBDIRS += client
endif

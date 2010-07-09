MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	-I$(top_srcdir)/src/lib \
	@EDBUS_CFLAGS@ \
	@EINA_CFLAGS@ \
	@EFL_ETHUMB_CLIENT_BUILD@

includes_HEADERS = Ethumb_Client.h
includesdir = $(includedir)/ethumb-@VMAJ@

lib_LTLIBRARIES = libethumb_client.la

libethumb_client_la_SOURCES = \
	Ethumb_Client.c
libethumb_client_la_DEPENDENCIES = $(top_builddir)/config.h
libethumb_client_la_LIBADD = \
	$(top_builddir)/src/lib/libethumb.la \
	@EDBUS_LIBS@
libethumb_client_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@

# @EVAS_LIBS@ @ECORE_EVAS_LIBS@ @ECORE_FILE_LIBS@ @EDJE_LIBS@ 

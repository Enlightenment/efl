MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_builddir) \
-I$(top_srcdir)/src/lib \
@EFL_ETHUMB_CLIENT_BUILD@ \
@EDBUS_CFLAGS@ \
@EINA_CFLAGS@

includes_HEADERS = Ethumb_Client.h
includesdir = $(includedir)/ethumb-@VMAJ@

lib_LTLIBRARIES = libethumb_client.la

libethumb_client_la_SOURCES = Ethumb_Client.c
libethumb_client_la_LIBADD = \
$(top_builddir)/src/lib/libethumb.la \
@EDBUS_LIBS@ \
@EINA_LIBS@
libethumb_client_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@

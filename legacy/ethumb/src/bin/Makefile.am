MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/lib \
	@EINA_CFLAGS@ @EVAS_CFLAGS@ @ECORE_CFLAGS@ @ECORE_EVAS_CFLAGS@ \
	@EDJE_CFLAGS@ @ECORE_FILE_CFLAGS@

bin_PROGRAMS = ethumb
noinst_HEADERS =

ethumb_SOURCES = ethumb.c
ethumb_LDADD = \
	@EINA_LIBS@ @ECORE_LIBS@ \
	$(top_builddir)/src/lib/libethumb.la
ethumb_DEPENDENCIES = $(top_builddir)/config.h



if USE_MODULE_ETHUMBD

AM_CPPFLAGS += @EDBUS_CFLAGS@
bin_PROGRAMS += ethumbd ethumbd_client
noinst_HEADERS += ethumbd_private.h

ethumbd_SOURCES = ethumbd.c ethumbd_child.c
ethumbd_LDADD = \
	@EINA_LIBS@ @ECORE_LIBS@ @EDBUS_LIBS@ \
	$(top_builddir)/src/lib/libethumb.la
ethumbd_DEPENDENCIES = $(top_builddir)/config.h

ethumbd_client_SOURCES = ethumbd_client.c
ethumbd_client_LDADD = \
	@EINA_LIBS@ @ECORE_LIBS@ \
	$(top_builddir)/src/lib/client/libethumb_client.la
ethumbd_client_DEPENDENCIES = $(top_builddir)/config.h

endif

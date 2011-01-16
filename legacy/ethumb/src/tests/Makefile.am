MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/lib \
	-I$(top_srcdir)/src/lib/client \
	@EINA_CFLAGS@ @EVAS_CFLAGS@ @ECORE_CFLAGS@ @ECORE_EVAS_CFLAGS@ \
	@EDJE_CFLAGS@ @ECORE_FILE_CFLAGS@

check_PROGRAMS =

if USE_MODULE_ETHUMBD

AM_CPPFLAGS += @EDBUS_CFLAGS@
check_PROGRAMS += ethumb_dbus
ethumb_dbus_SOURCES = ethumb_dbus.c
ethumb_dbus_LDADD = \
	@EINA_LIBS@ @EVAS_LIBS@ @ECORE_LIBS@ @ECORE_EVAS_LIBS@ @EDJE_LIBS@ \
	@ECORE_FILE_LIBS@ @EDBUS_LIBS@ \
	$(top_builddir)/src/lib/libethumb.la \
	$(top_builddir)/src/lib/client/libethumb_client.la

endif

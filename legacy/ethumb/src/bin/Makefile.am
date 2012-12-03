MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/client \
@EINA_CFLAGS@ \
@ECORE_CFLAGS@ \
@EDBUS_CFLAGS@

if USE_MODULE_ETHUMBD

AM_CPPFLAGS += \
@EDBUS_CFLAGS@ \
-DETHUMB_LIBEXEC_DIR=\"$(libexecdir)\"

endif

bin_PROGRAMS = ethumb

ethumb_SOURCES = ethumb.c
ethumb_LDADD = \
$(top_builddir)/src/lib/libethumb.la \
@ECORE_LIBS@ \
@EINA_LIBS@


if USE_MODULE_ETHUMBD

bin_PROGRAMS += ethumbd ethumbd_client

libexec_PROGRAMS = ethumbd_slave

ethumbd_SOURCES = ethumbd.c ethumbd_private.h
ethumbd_LDADD = \
$(top_builddir)/src/lib/libethumb.la \
@EDBUS_LIBS@ \
@ECORE_LIBS@ \
@EINA_LIBS@

ethumbd_slave_SOURCES = ethumbd_child.c ethumbd_private.h
ethumbd_slave_LDADD = \
$(top_builddir)/src/lib/libethumb.la \
@EDBUS_LIBS@ \
@ECORE_LIBS@ \
@EINA_LIBS@

ethumbd_client_SOURCES = ethumbd_client.c
ethumbd_client_LDADD = \
$(top_builddir)/src/lib/client/libethumb_client.la \
$(top_builddir)/src/lib/libethumb.la \
@EDBUS_LIBS@ \
@ECORE_LIBS@ \
@EINA_LIBS@

endif

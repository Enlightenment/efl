MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_con \
-I$(top_builddir)/src/lib/ecore_dbus \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_con \
-I$(top_srcdir)/src/lib/ecore_dbus \
@SSL_CFLAGS@

libecore_dbus_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs \
-L$(top_builddir)/src/lib/ecore_con/.libs

if BUILD_ECORE_DBUS

lib_LTLIBRARIES = libecore_dbus.la
include_HEADERS = \
Ecore_DBus.h

libecore_dbus_la_SOURCES = \
ecore_dbus.c \
ecore_dbus_message.c \
ecore_dbus_marshal.c \
ecore_dbus_unmarshal.c \
ecore_dbus_methods.c \
ecore_dbus_utils.c \
ecore_dbus_address.c \
ecore_dbus_object.c \
ecore_dbus_private.h

libecore_dbus_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_con/libecore_con.la \
@SSL_LIBS@

libecore_dbus_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_con/libecore_con.la

endif

EXTRA_DIST = \
ecore_dbus.c \
ecore_dbus_private.h

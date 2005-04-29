## Process this file with automake to produce Makefile.in

INCLUDES = \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_con \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_con \
@SSL_CFLAGS@

libecore_con_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs

if BUILD_ECORE_DBUS

lib_LTLIBRARIES = libecore_con.la
include_HEADERS = \
Ecore_DBus.h

libecore_con_la_SOURCES = \
ecore_dbus.c \
ecore_dbus_private.h

libecore_con_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la

libecore_con_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la

endif

EXTRA_DIST = \
ecore_dbus.c \
ecore_dbus_private.h

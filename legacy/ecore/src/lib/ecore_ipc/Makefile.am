MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_con \
-I$(top_builddir)/src/lib/ecore_ipc \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_con \
-I$(top_srcdir)/src/lib/ecore_ipc \
@SSL_CFLAGS@ @EINA_CFLAGS@

if BUILD_ECORE_IPC

lib_LTLIBRARIES = libecore_ipc.la
include_HEADERS = \
Ecore_Ipc.h

libecore_ipc_la_SOURCES = \
ecore_ipc.c \
ecore_ipc_private.h

libecore_ipc_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_con/libecore_con.la \
@SSL_LIBS@ \
@EINA_LIBS@

libecore_ipc_la_LDFLAGS = -version-info @version_info@

libecore_ipc_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_con/libecore_con.la

endif

EXTRA_DIST = \
ecore_ipc.c \
ecore_ipc_private.h

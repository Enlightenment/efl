## Process this file with automake to produce Makefile.in

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_con \
@SSL_CFLAGS@

libecore_ipc_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs \
-L$(top_builddir)/src/lib/ecore_con/.libs

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
@SSL_LIBS@

libecore_ipc_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_con/libecore_con.la

endif

EXTRA_DIST = \
Ecore_Ipc.h.in \
ecore_ipc.c \
ecore_ipc_private.h

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_con \
-I$(top_builddir)/src/lib/ecore_ipc \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_con \
-I$(top_srcdir)/src/lib/ecore_ipc \
@EFL_ECORE_IPC_BUILD@ \
@SSL_CFLAGS@ \
@EINA_CFLAGS@

lib_LTLIBRARIES = libecore_ipc.la
includes_HEADERS = Ecore_Ipc.h
includesdir = $(includedir)/ecore-@VMAJ@

libecore_ipc_la_SOURCES = \
ecore_ipc.c

libecore_ipc_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_con/libecore_con.la \
@SSL_LIBS@ \
@EINA_LIBS@

libecore_ipc_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@

EXTRA_DIST = ecore_ipc_private.h

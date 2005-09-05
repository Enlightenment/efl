MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_con \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_con \
@SSL_CFLAGS@ @CURL_CFLAGS@

libecore_con_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs

if BUILD_ECORE_CON

lib_LTLIBRARIES = libecore_con.la
include_HEADERS = \
Ecore_Con.h

libecore_con_la_SOURCES = \
ecore_con.c \
ecore_con_dns.c \
ecore_con_url.c \
ecore_con_private.h

libecore_con_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@SSL_LIBS@ @winsock_libs@ @CURL_LIBS@

libecore_con_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la

endif

EXTRA_DIST = \
ecore_con.c \
ecore_con_dns.c \
ecore_con_url.c \
ecore_con_private.h

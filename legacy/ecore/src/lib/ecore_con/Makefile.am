MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_con \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_con \
@EFL_ECORE_CON_BUILD@ \
@SSL_CFLAGS@ \
@CURL_CFLAGS@ \
@EINA_CFLAGS@ \
@TLS_CFLAGS@ \
@CARES_CFLAGS@ \
@WIN32_CPPFLAGS@

lib_LTLIBRARIES = libecore_con.la
includes_HEADERS = Ecore_Con.h
includesdir = $(includedir)/ecore-@VMAJ@

libecore_con_la_SOURCES = \
ecore_con.c \
ecore_con_ssl.c \
ecore_con_url.c \
ecore_con_alloc.c

if ECORE_HAVE_WIN32
libecore_con_la_SOURCES += ecore_con_local_win32.c
else
libecore_con_la_SOURCES += ecore_con_local.c
endif

if HAVE_CARES
libecore_con_la_SOURCES += ecore_con_ares.c
else
libecore_con_la_SOURCES += ecore_con_info.c
endif

libecore_con_la_CFLAGS = @WIN32_CFLAGS@
libecore_con_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@SSL_LIBS@ @CURL_LIBS@ @EINA_LIBS@ @TLS_LIBS@ @CARES_LIBS@ @WIN32_LIBS@

libecore_con_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@

EXTRA_DIST = ecore_con_private.h

## Process this file with automake to produce Makefile.in

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore @SSL_CFLAGS@

libecore_con_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs

if BUILD_ECORE_CON

lib_LTLIBRARIES = libecore_con.la
include_HEADERS = \
Ecore_Con.h

libecore_con_la_SOURCES = \
ecore_con.c \
ecore_con_private.h

libecore_con_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la @SSL_LIBS@

libecore_con_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la

endif

EXTRA_DIST = \
Ecore_Con.h.in \
ecore_con.c \
ecore_con_private.h

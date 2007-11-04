MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
@DIRECTFB_CFLAGS@

if BUILD_ECORE_DIRECTFB

lib_LTLIBRARIES = libecore_directfb.la
include_HEADERS = \
Ecore_DirectFB.h

libecore_directfb_la_SOURCES = \
ecore_directfb.c \
ecore_directfb_keys.h \
ecore_directfb_private.h

libecore_directfb_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@DIRECTFB_LIBS@

libecore_directfb_la_LDFLAGS = -version-info @version_info@

libecore_directfb_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la

endif

EXTRA_DIST = \
Ecore_DirectFB.h \
ecore_directfb.c \
ecore_directfb_keys.h \
ecore_directfb_private.h

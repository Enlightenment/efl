MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
@DIRECTFB_CFLAGS@

libecore_directfb_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs 

if BUILD_ECORE_DIRECTFB

lib_LTLIBRARIES = libecore_directfb.la
include_HEADERS = \
Ecore_DirectFB.h

libecore_directfb_la_SOURCES = \
ecore_directfb.c \
ecore_directfb_private.h 

libecore_directfb_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@DIRECTFB_LIBS@

libecore_directfb_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la

endif

EXTRA_DIST = \
Ecore_DirectFB.h \
ecore_directfb.c \
ecore_directfb_private.h

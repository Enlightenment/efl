## Process this file with automake to produce Makefile.in

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore

libecore_fb_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs

if BUILD_ECORE_FB

lib_LTLIBRARIES = libecore_fb.la
include_HEADERS = \
Ecore_Fb.h

libecore_fb_la_SOURCES = \
ecore_fb.c \
ecore_fb_private.h

libecore_fb_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la

libecore_fb_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la

endif

EXTRA_DIST = \
Ecore_Fb.h \
ecore_fb.c \
ecore_fb_private.h

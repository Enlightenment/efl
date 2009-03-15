MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
@TSLIB_CFLAGS@ \
@EINA_CFLAGS@

if BUILD_ECORE_FB

lib_LTLIBRARIES = libecore_fb.la
include_HEADERS = \
Ecore_Fb.h

libecore_fb_la_SOURCES = \
ecore_fb.c \
ecore_fb_vt.c \
ecore_fb_li.c \
ecore_fb_ts.c
# deprecated sources (might not compile):
# ecore_fb_kbd.c
# ecore_fb_ps2.c

libecore_fb_la_LIBADD = \
@TSLIB_LIBS@ \
$(top_builddir)/src/lib/ecore/libecore.la \
@EINA_LIBS@

libecore_fb_la_LDFLAGS = -version-info @version_info@

endif

EXTRA_DIST = \
ecore_fb_private.h \
ecore_fb_keytable.h

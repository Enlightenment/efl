MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_input \
@EVAS_CFLAGS@ \
@EINA_CFLAGS@

if BUILD_ECORE_QUARTZ

lib_LTLIBRARIES = libecore_cocoa.la
include_HEADERS = \
Ecore_Cocoa.h \
Ecore_Cocoa_Keys.h

libecore_cocoa_la_SOURCES = \
ecore_cocoa.m \
ecore_cocoa_window.m

libecore_cocoa_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
@EVAS_LIBS@ \
@EINA_LIBS@

libecore_cocoa_la_LDFLAGS = @quartz_ldflags@ -version-info @version_info@ @ecore_quartz_release_info@

endif

EXTRA_DIST = ecore_cocoa_private.h

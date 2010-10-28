MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_input \
@EVAS_CFLAGS@ \
@EINA_CFLAGS@

lib_LTLIBRARIES = libecore_cocoa.la
includes_HEADERS = \
Ecore_Cocoa.h \
Ecore_Cocoa_Keys.h
includesdir = $(includedir)/ecore-@VMAJ@

libecore_cocoa_la_SOURCES = \
ecore_cocoa.m \
ecore_cocoa_window.m

libecore_cocoa_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
@EVAS_LIBS@ \
@EINA_LIBS@

libecore_cocoa_la_LDFLAGS = @cocoa_ldflags@ -version-info @version_info@ @release_info@

EXTRA_DIST = ecore_cocoa_private.h

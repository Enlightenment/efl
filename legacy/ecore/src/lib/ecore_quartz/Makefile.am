MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
@QUARTZ_CFLAGS@ @EINA_CFLAGS@

if BUILD_ECORE_QUARTZ

lib_LTLIBRARIES = libecore_quartz.la
include_HEADERS = \
Ecore_Quartz.h

libecore_quartz_la_SOURCES = \
Ecore_Quartz.h

### add to SOURCES one we know how to fix:
# /usr/share/automake-1.9/am/depend2.am: am__fastdepOBJC does not appear in
# AM_CONDITIONAL
# ecore_quartz.m

libecore_quartz_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@QUARTZ_LIBS@ \
@EINA_LIBS@

libecore_quartz_la_LDFLAGS = @lt_no_undefined@ @lt_enable_auto_import@ -version-info @version_info@

libecore_quartz_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la

endif

EXTRA_DIST = \
Ecore_Quartz.h \
ecore_quartz.m

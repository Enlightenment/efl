MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_input \
@EVAS_CFLAGS@ \
@EINA_CFLAGS@

if BUILD_ECORE_QUARTZ

lib_LTLIBRARIES = libecore_quartz.la
include_HEADERS = \
Ecore_Quartz.h

libecore_quartz_la_SOURCES = \
ecore_quartz.m

libecore_quartz_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
@EVAS_LIBS@ \
@EINA_LIBS@

libecore_quartz_la_LDFLAGS = @quartz_ldflags@ -version-info @version_info@

endif

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
@quartz_cflags@ \
@EINA_CFLAGS@

if BUILD_ECORE_QUARTZ

lib_LTLIBRARIES = libecore_quartz.la
include_HEADERS = \
Ecore_Quartz.h

libecore_quartz_la_SOURCES = \
ecore_quartz.m

libecore_quartz_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@quartz_libs@ \
@EINA_LIBS@

libecore_quartz_la_LDFLAGS = -version-info @version_info@

endif

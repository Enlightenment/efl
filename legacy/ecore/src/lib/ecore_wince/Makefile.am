MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
@EINA_CFLAGS@

AM_CFLAGS = @WIN32_CFLAGS@


if BUILD_ECORE_WINCE

lib_LTLIBRARIES = libecore_wince.la

include_HEADERS = \
Ecore_WinCE.h

libecore_wince_la_SOURCES = \
ecore_wince.c \
ecore_wince_event.c \
ecore_wince_window.c

libecore_wince_la_LIBADD = \
@WIN32_LIBS@ \
$(top_builddir)/src/lib/ecore/libecore.la \
@EINA_LIBS@

libecore_wince_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@

endif

EXTRA_DIST = ecore_wince_private.h

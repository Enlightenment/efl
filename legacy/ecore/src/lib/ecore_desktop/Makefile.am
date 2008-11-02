MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_file \
-I$(top_builddir)/src/lib/ecore_desktop \
-I$(top_builddir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_file \
-I$(top_srcdir)/src/lib/ecore_desktop \
@EINA_CFLAGS@

if BUILD_ECORE_DESKTOP

lib_LTLIBRARIES = libecore_desktop.la
include_HEADERS = \
Ecore_Desktop.h

libecore_desktop_la_SOURCES = \
ecore_desktop.c \
ecore_desktop_tree.c \
ecore_desktop_icon.c \
ecore_desktop_menu.c \
ecore_desktop_paths.c \
ecore_desktop_xmlame.c

libecore_desktop_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_file/libecore_file.la \
@EINA_LIBS@

libecore_desktop_la_LDFLAGS = -version-info @version_info@

endif

EXTRA_DIST = ecore_desktop_private.h

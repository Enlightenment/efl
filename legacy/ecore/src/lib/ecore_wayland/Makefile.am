MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_input \
@WAYLAND_CFLAGS@ \
@EVAS_CFLAGS@ \
@EINA_CFLAGS@

lib_LTLIBRARIES = libecore_wayland.la
includes_HEADERS = Ecore_Wayland.h
includesdir = $(includedir)/ecore-@VMAJ@

libecore_wayland_la_SOURCES = \
ecore_wl.c \
ecore_wl_output.c \
ecore_wl_input.c \
ecore_wl_window.c \
ecore_wl_dnd.c

libecore_wayland_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
@WAYLAND_LIBS@ \
@EVAS_LIBS@ \
@EINA_LIBS@

libecore_wayland_la_LDFLAGS = -version-info @version_info@ @release_info@
libecore_wayland_la_DEPENDENCIES = $(top_builddir)/src/lib/ecore/libecore.la

EXTRA_DIST = ecore_wl_private.h

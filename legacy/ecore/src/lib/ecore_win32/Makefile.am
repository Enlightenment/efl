MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
@EFL_ECORE_WIN32_BUILD@ \
@EINA_CFLAGS@


if BUILD_ECORE_WIN32

lib_LTLIBRARIES = libecore_win32.la

include_HEADERS = \
Ecore_Win32.h

libecore_win32_la_SOURCES = \
ecore_win32.c \
ecore_win32_cursor.c \
ecore_win32_dnd.c \
ecore_win32_dnd_enumformatetc.cpp \
ecore_win32_dnd_data_object.cpp \
ecore_win32_dnd_drop_source.cpp \
ecore_win32_dnd_drop_target.cpp \
ecore_win32_event.c \
ecore_win32_window.c

libecore_win32_la_LIBADD = \
@ecore_win32_libs@ \
@WIN32_LIBS@ \
$(top_builddir)/src/lib/ecore/libecore.la \
@EINA_LIBS@

libecore_win32_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@

endif

EXTRA_DIST = \
ecore_win32_private.h \
ecore_win32_dnd_enumformatetc.h \
ecore_win32_dnd_data_object.h \
ecore_win32_dnd_drop_source.h \
ecore_win32_dnd_drop_target.h

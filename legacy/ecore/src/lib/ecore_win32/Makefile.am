MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore

libecore_win32_la_LDFLAGS = \
-version-info 1:0:0 \
@create_shared_lib@  \
-L$(top_builddir)/src/lib/ecore/.libs


if BUILD_ECORE_WIN32

lib_LTLIBRARIES = libecore_win32.la
include_HEADERS = \
Ecore_Win32.h

libecore_win32_la_SOURCES = \
ecore_win32.c \
ecore_win32_cursor.c \
ecore_win32_direct3d.cpp \
ecore_win32_ddraw.c \
ecore_win32_event.c \
ecore_win32_window.c \
ecore_win32_private.h

libecore_win32_la_LIBADD = \
@ddraw_libs@ @direct3d_libs@ \
-lwsock32 \
$(top_builddir)/src/lib/ecore/libecore.la

endif

EXTRA_DIST = \
Ecore_Win32.h \
ecore_win32.c \
ecore_win32_cursor.c \
ecore_win32_direct3d.cpp \
ecore_win32_ddraw.c \
ecore_win32_event.c \
ecore_win32_window.c \
ecore_win32_private.h

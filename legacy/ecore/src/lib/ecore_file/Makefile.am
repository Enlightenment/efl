MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_con \
-I$(top_builddir)/src/lib/ecore \
-DEFL_ECORE_FILE_BUILD \
@CURL_CFLAGS@ \
@EVIL_CFLAGS@ \
@EINA_CFLAGS@

if BUILD_ECORE_CON
lib_ecore_con_la = $(top_builddir)/src/lib/ecore_con/libecore_con.la
endif

lib_LTLIBRARIES = libecore_file.la
includes_HEADERS = Ecore_File.h
includesdir = $(includedir)/ecore-@VMAJ@

libecore_file_la_SOURCES = \
ecore_file.c \
ecore_file_monitor.c \
ecore_file_monitor_inotify.c \
ecore_file_monitor_win32.c \
ecore_file_monitor_poll.c \
ecore_file_path.c \
ecore_file_download.c \
ecore_file_private.h

libecore_file_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(lib_ecore_con_la) \
@EVIL_LIBS@ \
@EINA_LIBS@

libecore_file_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@

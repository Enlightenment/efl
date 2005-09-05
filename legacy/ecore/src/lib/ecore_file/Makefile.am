MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
@CURL_CFLAGS@

libecore_file_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs

if BUILD_ECORE_FILE

lib_LTLIBRARIES = libecore_file.la
include_HEADERS = Ecore_File.h

libecore_file_la_SOURCES = \
ecore_file.c \
ecore_file_private.h \
ecore_file_monitor.c \
ecore_file_monitor_fam.c \
ecore_file_monitor_inotify.c \
ecore_file_monitor_poll.c \
ecore_file_path.c \
ecore_file_download.c

libecore_file_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@fam_libs@ @CURL_LIBS@

endif

EXTRA_DIST = \
ecore_file.c \
ecore_file_private.h \
ecore_file_monitor.c \
ecore_file_monitor_fam.c \
ecore_file_monitor_inotify.c \
ecore_file_monitor_poll.c \
ecore_file_path.c


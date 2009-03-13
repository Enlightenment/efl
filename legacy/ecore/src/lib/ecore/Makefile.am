MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = @EVIL_CFLAGS@ @WIN32_CPPFLAGS@ @EFL_ECORE_BUILD@
AM_CFLAGS = @WIN32_CFLAGS@ @EINA_CFLAGS@ @WIN32_CPPFLAGS@

lib_LTLIBRARIES = libecore.la
include_HEADERS = \
Ecore.h \
Ecore_Data.h \
Ecore_Str.h \
Ecore_Getopt.h

libecore_la_SOURCES = \
ecore.c \
ecore_anim.c \
ecore_app.c \
ecore_events.c \
ecore_exe.c \
ecore_getopt.c \
ecore_hash.c \
ecore_idle_enterer.c \
ecore_idle_exiter.c \
ecore_idler.c \
ecore_list.c \
ecore_main.c \
ecore_path.c \
ecore_pipe.c \
ecore_plugin.c \
ecore_poll.c \
ecore_sheap.c \
ecore_signal.c \
ecore_str.c \
ecore_strbuf.c \
ecore_time.c \
ecore_timer.c \
ecore_tree.c \
ecore_value.c

libecore_la_LIBADD = @dlopen_libs@ @EINA_LIBS@ @EVIL_LIBS@ @WIN32_LIBS@ -lm
libecore_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@

EXTRA_DIST = ecore_private.h

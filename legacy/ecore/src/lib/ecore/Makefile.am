MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS = @WIN32_CFLAGS@ @EINA_CFLAGS@

lib_LTLIBRARIES = libecore.la
include_HEADERS = \
Ecore.h \
Ecore_Data.h \
Ecore_Str.h

libecore_la_SOURCES = \
ecore.c \
ecore_app.c \
ecore_anim.c \
ecore_events.c \
ecore_exe.c \
ecore_hash.c \
ecore_idle_enterer.c \
ecore_idle_exiter.c \
ecore_idler.c \
ecore_list.c \
ecore_main.c \
ecore_path.c \
ecore_plugin.c \
ecore_sheap.c \
ecore_signal.c \
ecore_str.c \
ecore_strbuf.c \
ecore_strings.c \
ecore_time.c \
ecore_timer.c \
ecore_tree.c \
ecore_value.c \
ecore_poll.c \
ecore_private.h

libecore_la_LIBADD = @dlopen_libs@ @EVIL_LIBS@ @EINA_LIBS@ @WIN32_LIBS@ -lm
libecore_la_LDFLAGS = @lt_no_undefined@ @lt_enable_auto_import@ -version-info @version_info@

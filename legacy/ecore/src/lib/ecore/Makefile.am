MAINTAINERCLEANFILES = Makefile.in

INCLUDES =

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
ecore_strings.c \
ecore_time.c \
ecore_timer.c \
ecore_tree.c \
ecore_value.c \
ecore_private.h

libecore_la_LIBADD = -lm @dlopen_libs@ @winsock_libs@
libecore_la_LDFLAGS = -version-info 1:0:0

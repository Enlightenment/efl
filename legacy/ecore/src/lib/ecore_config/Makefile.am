## Process this file with automake to produce Makefile.in

SUBDIRS  = ipc

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore

libecore_config_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs

if BUILD_ECORE_CONFIG

lib_LTLIBRARIES = libecore_config.la
include_HEADERS = \
Ecore_Config.h

libecore_config_la_SOURCES = \
ecore_config.c \
ipc_main.c \
util.c \
edb.c

libecore_config_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la

libecore_config_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la

endif

EXTRA_DIST = \
Ecore_Config.h \
ecore_config.c \
ipc_main.c \
ipc.h \
util.c \
util.h \
edb.c

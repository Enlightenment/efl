## Process this file with automake to produce Makefile.in

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_ipc \
-I$(top_srcdir)/

DB = system.db
CLEANFILES = $(DB)

libecore_config_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs

ecore_config_ipc_ecore_la_LDFLAGS          = -no-undefined -module \
-avoid-version -rdynamic -shared

if BUILD_ECORE_CONFIG

lib_LTLIBRARIES = libecore_config.la ecore_config_ipc_ecore.la
include_HEADERS = \
Ecore_Config.h

config_DATA = $(DB)
configdir = $(pkgdatadir)

libecore_config_la_SOURCES = \
ecore_config.c \
ipc_main.c \
util.c \
edb.c

libecore_config_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la -ldl

libecore_config_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la


ecore_config_ipc_ecore_la_DEPENDENCIES     = \
$(top_builddir)/config.h \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_ipc/libecore_ipc.la \
libecore_config.la
ecore_config_ipc_ecore_la_SOURCES          = ipc_ecore.c
ecore_config_ipc_ecore_la_LIBADD           = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_ipc/libecore_ipc.la \
libecore_config.la \
-ledb

$(DB):
	sh build_$(DB).sh

endif

EXTRA_DIST = \
Ecore_Config.h \
ecore_config.c \
ipc_main.c \
ipc_ecore.c \
ipc.h \
util.c \
util.h \
edb.c


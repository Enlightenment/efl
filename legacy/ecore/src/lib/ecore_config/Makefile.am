## Process this file with automake to produce Makefile.in

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_ipc \
-I$(top_srcdir)/ @evas_cflags@ @edb_cflags@

CLEANFILES = $(DB)

libecore_config_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs

ecore_config_ipc_ecore_la_LDFLAGS          = -no-undefined -module \
-avoid-version -rdynamic -shared

if BUILD_ECORE_CONFIG

DB = system.db
$(DB): Makefile
	edb_ed $(top_builddir)/src/lib/ecore_config/$(DB) add /e/theme/name str "winter"
	edb_ed $(top_builddir)/src/lib/ecore_config/$(DB) add /apps/web/browser str `which firefox 2>/dev/null || which phoenix 2>/dev/null || which mozilla 2>/dev/null || which opera 2>/dev/null || which konqueror 2>/dev/null || which epiphany 2>/dev/null`
	edb_ed $(top_builddir)/src/lib/ecore_config/$(DB) add /apps/web/email str `which thunderbird 2>/dev/null || which mozilla 2>/dev/null || which kmail 2>/dev/null || which sylpheed 2>/dev/null || which evolution 2>/dev/null`

lib_LTLIBRARIES = libecore_config.la ecore_config_ipc_ecore.la
include_HEADERS = \
Ecore_Config.h

config_DATA = $(DB)
configdir = $(pkgdatadir)

libecore_config_la_SOURCES = \
ecore_config.c \
ipc_main.c \
util.c \
edb.c \
convenience.c

libecore_config_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
-ldl \
@edb_libs@ \
@evas_libs@

libecore_config_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la

ecore_config_ipc_ecore_la_DEPENDENCIES = \
$(top_builddir)/config.h \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_ipc/libecore_ipc.la \
libecore_config.la

ecore_config_ipc_ecore_la_SOURCES          = ipc_ecore.c
ecore_config_ipc_ecore_la_LIBADD           = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_ipc/libecore_ipc.la \
libecore_config.la \
@edb_libs@ \
@evas_libs@

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


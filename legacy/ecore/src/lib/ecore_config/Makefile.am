MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_ipc \
-I$(top_srcdir)/ \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_ipc \
-I$(top_builddir)/ \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVAS_CFLAGS@ @EET_CFLAGS@ @EINA_CFLAGS@

CLEANFILES = $(DB)

if BUILD_ECORE_CONFIG

#DB = system.db
#$(DB): Makefile
#	edb_ed $(top_builddir)/src/lib/ecore_config/$(DB) add /e/theme/name str "winter"
#	edb_ed $(top_builddir)/src/lib/ecore_config/$(DB) add /e/font/path str "$(pkgdatadir)/data/fonts"
#	edb_ed $(top_builddir)/src/lib/ecore_config/$(DB) add /apps/web/browser str `which firefox 2>/dev/null || which phoenix 2>/dev/null || which mozilla 2>/dev/null || which opera 2>/dev/null || which konqueror 2>/dev/null || which epiphany 2>/dev/null`
#	edb_ed $(top_builddir)/src/lib/ecore_config/$(DB) add /apps/web/email str `which thunderbird 2>/dev/null || which mozilla 2>/dev/null || which kmail 2>/dev/null || which sylpheed 2>/dev/null || which evolution 2>/dev/null`

lib_LTLIBRARIES = libecore_config.la
include_HEADERS = \
Ecore_Config.h

libecore_config_la_LDFLAGS = -version-info @version_info@

#config_DATA = $(DB)
#configdir = $(pkgdatadir)

if BUILD_ECORE_IPC

libecore_config_la_SOURCES = \
ecore_config.c \
ecore_config_ipc_main.c \
ecore_config_ipc_ecore.c \
ecore_config_util.c \
ecore_config_storage.c \
ecore_config_extra.c \
ecore_config_db.c \
ecore_config_private.h

libecore_config_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_ipc/libecore_ipc.la \
@EINA_LIBS@ \
@EET_LIBS@ \
@EVAS_LIBS@

else

libecore_config_la_SOURCES = \
ecore_config.c \
ecore_config_util.c \
ecore_config_storage.c \
ecore_config_extra.c \
ecore_config_db.c \
ecore_config_private.h

libecore_config_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@EET_LIBS@ \
@EINA_LIBS@ \
@EVAS_LIBS@

endif

endif

EXTRA_DIST = \
Ecore_Config.h \
ecore_config.c \
ecore_config_ipc_ecore.c \
ecore_config_ipc_main.c \
ecore_config_ipc.h \
ecore_config_util.c \
ecore_config_util.h \
ecore_config_storage.c


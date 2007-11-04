MAINTAINERCLEANFILES = Makefile.in

if BUILD_ECORE_CONFIG
ECORE_CONFIG_PROG = ecore_config
ECORE_CONFIG_LIB = $(top_builddir)/src/lib/ecore_config/libecore_config.la
else
ECORE_CONFIG_PROG =
ECORE_CONFIG_LIB =
endif

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_config \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_config \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EET_CFLAGS@

bin_PROGRAMS = $(ECORE_CONFIG_PROG)

ecore_config_SOURCES = \
ecore_config.c

ecore_config_LDADD = \
$(ECORE_CONFIG_LIB) \
$(top_builddir)/src/lib/ecore_ipc/libecore_ipc.la \
$(top_builddir)/src/lib/ecore_con/libecore_con.la \
$(top_builddir)/src/lib/ecore/libecore.la

ecore_config_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore_ipc/libecore_ipc.la \
$(top_builddir)/src/lib/ecore_con/libecore_con.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(ECORE_CONFIG_LIB)

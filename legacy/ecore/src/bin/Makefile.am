MAINTAINERCLEANFILES = Makefile.in

if BUILD_ECORE_CONFIG
ECORE_CONFIG_LIB = $(top_builddir)/src/lib/ecore_config/libecore_config.la
else
ECORE_CONFIG_LIB =
endif

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_config \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_config \
@EET_CFLAGS@ \
-DPACKAGE_BIN_DIR=\"@PACKAGE_BIN_DIR@\" \
-DPACKAGE_LIB_DIR=\"@PACKAGE_LIB_DIR@\" \
-DPACKAGE_DATA_DIR=\"@PACKAGE_DATA_DIR@\"

bin_PROGRAMS = \
ecore_config

ecore_config_SOURCES = \
ecore_config.c

ecore_config_LDADD = \
$(ECORE_CONFIG_LIB)

ecore_config_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(ECORE_CONFIG_LIB)

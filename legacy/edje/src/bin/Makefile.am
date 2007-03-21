## Process this file with automake to produce Makefile.in

INCLUDES = \
-I$(top_srcdir) \
-I$(top_srcdir)/bin \
-I$(top_srcdir)/src/lib \
@EDJE_CFLAGS@ \
@ECORE_CFLAGS@ \
-DPACKAGE_BIN_DIR=\"@PACKAGE_BIN_DIR@\" \
-DPACKAGE_LIB_DIR=\"@PACKAGE_LIB_DIR@\" \
-DPACKAGE_DATA_DIR=\"@PACKAGE_DATA_DIR@\" 
		      
bin_SCRIPTS = \
@EDJE_RECC_PRG@

EXTRA_DIST = @EDJE_RECC_PRG@
EXTRA_SCRIPTS = edje_recc

bin_PROGRAMS = \
@EDJE_CC_PRG@ \
@EDJE_DECC_PRG@

EXTRA_PROGRAMS = \
edje_cc \
edje_decc

edje_cc_SOURCES = \
edje_cc.c \
edje_cc.h \
edje_cc_out.c \
edje_cc_parse.c \
edje_cc_mem.c \
edje_cc_handlers.c \
edje_cc_sources.c \
edje_prefix.c \
edje_prefix.h \
edje_main.h

edje_cc_LDADD = \
$(top_builddir)/src/lib/libedje.la

edje_cc_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la


edje_decc_SOURCES = \
edje_decc.c \
edje_decc.h \
edje_cc_mem.c \
edje_cc_sources.c

edje_decc_LDADD = \
$(top_builddir)/src/lib/libedje.la

edje_decc_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la

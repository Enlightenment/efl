
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EDJE_CFLAGS@ \
@ECORE_EVAS_CFLAGS@ \
@ECORE_FILE_CFLAGS@ \
@EVIL_CFLAGS@

if BUILD_EDJE_CC
EDJE_CC = edje_cc
else
EDJE_CC =
endif

if BUILD_EDJE_DECC
EDJE_DECC = edje_decc
else
EDJE_DECC =
endif

bin_SCRIPTS = \
@EDJE_RECC_PRG@

EXTRA_DIST = @EDJE_RECC_PRG@
EXTRA_SCRIPTS = edje_recc

bin_PROGRAMS = $(EDJE_CC) $(EDJE_DECC)

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

edje_cc_CFLAGS = @WIN32_CFLAGS@
edje_cc_LDADD = $(top_builddir)/src/lib/libedje.la $(EDJE_LIBS) $(ECORE_EVAS_LIBS) $(EVIL_LIBS)
edje_cc_LDFLAGS = @lt_enable_auto_import@
edje_cc_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la


edje_decc_SOURCES = \
edje_decc.c \
edje_decc.h \
edje_cc_mem.c \
edje_cc_sources.c

edje_decc_CFLAGS = @WIN32_CFLAGS@
edje_decc_LDADD = $(top_builddir)/src/lib/libedje.la $(EDJE_LIBS) $(ECORE_EVAS_LIBS) $(ECORE_FILE_LIBS) $(EVIL_LIBS)
edje_decc_LDFLAGS = @lt_enable_auto_import@
edje_decc_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la

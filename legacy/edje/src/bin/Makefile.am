
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

bin_SCRIPTS = @EDJE_RECC_PRG@

bin_PROGRAMS = @EDJE_CC_PRG@ @EDJE_DECC_PRG@ @EDJE_PLAYER_PRG@

EXTRA_PROGRAMS = edje_cc edje_decc edje_player

edje_cc_SOURCES = \
edje_cc.c \
edje_cc_out.c \
edje_cc_parse.c \
edje_cc_mem.c \
edje_cc_handlers.c \
edje_cc_sources.c \
edje_prefix.c

edje_cc_LDADD = $(top_builddir)/src/lib/libedje.la $(ECORE_EVAS_LIBS) $(EVIL_LIBS)
edje_cc_LDFLAGS = @lt_enable_auto_import@


edje_decc_SOURCES = \
edje_decc.c \
edje_decc.h \
edje_cc_mem.c \
edje_cc_sources.c

edje_decc_LDADD = $(top_builddir)/src/lib/libedje.la $(ECORE_EVAS_LIBS) $(ECORE_FILE_LIBS)
edje_decc_LDFLAGS = @lt_enable_auto_import@

edje_player_SOURCES = edje_player.c
edje_player_LDADD = $(top_builddir)/src/lib/libedje.la $(ECORE_EVAS_LIBS) $(EVIL_LIBS)
edje_player_LDFLAGS = @lt_enable_auto_import@

EXTRA_DIST = @EDJE_RECC_PRG@ edje_prefix.h edje_cc.h
EXTRA_SCRIPTS = edje_recc

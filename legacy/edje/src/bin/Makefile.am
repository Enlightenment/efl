MAINTAINERCLEANFILES = Makefile.in

if BUILD_EPP
  SUBDIRS = epp
endif

bin_SCRIPTS = @EDJE_RECC_PRG@

bin_PROGRAMS = @EDJE_CC_PRG@ @EDJE_DECC_PRG@ @EDJE_PLAYER_PRG@ @EDJE_INSPECTOR_PRG@ @EDJE_EXTERNAL_INSPECTOR_PRG@

EXTRA_PROGRAMS = edje_cc edje_decc edje_player edje_inspector edje_external_inspector

edje_cc_SOURCES = \
edje_cc.c \
edje_cc_out.c \
edje_cc_parse.c \
edje_cc_mem.c \
edje_cc_handlers.c \
edje_cc_sources.c \
edje_multisense_convert.c

edje_cc_CPPFLAGS = \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DEPP_DIR=\"$(libdir)/$(PACKAGE)/utils\" \
@EDJE_CFLAGS@ @EDJE_CC_CFLAGS@ @EVIL_CFLAGS@ @SNDFILE_CFLAGS@
edje_cc_LDADD = $(top_builddir)/src/lib/libedje.la @EDJE_CC_LIBS@ @EVIL_LIBS@ @VORBISENC_LIBS@ @FLAC_LIBS@ @SNDFILE_LIBS@ -lm
edje_cc_LDFLAGS = @lt_enable_auto_import@


edje_decc_SOURCES = \
edje_decc.c \
edje_decc.h \
edje_cc_mem.c \
edje_cc_sources.c

edje_decc_CPPFLAGS = \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/lib \
@EDJE_CFLAGS@ @EDJE_DECC_CFLAGS@ @EVIL_CFLAGS@
edje_decc_LDADD = $(top_builddir)/src/lib/libedje.la @EDJE_DECC_LIBS@ @VORBISENC_LIBS@ @FLAC_LIBS@ @SNDFILE_LIBS@
edje_decc_LDFLAGS = @lt_enable_auto_import@

edje_player_SOURCES = edje_player.c
edje_player_CPPFLAGS = \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/lib \
@EDJE_PLAYER_CFLAGS@ @EVIL_CFLAGS@
edje_player_LDADD = $(top_builddir)/src/lib/libedje.la @EDJE_PLAYER_LIBS@ @EVIL_LIBS@ @VORBISENC_LIBS@ @FLAC_LIBS@ @SNDFILE_LIBS@
edje_player_LDFLAGS = @lt_enable_auto_import@

edje_inspector_SOURCES = edje_inspector.c
edje_inspector_CPPFLAGS = \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/lib \
@EDJE_INSPECTOR_CFLAGS@
edje_inspector_LDADD = $(top_builddir)/src/lib/libedje.la @EDJE_INSPECTOR_LIBS@ @VORBISENC_LIBS@ @FLAC_LIBS@
edje_inspector_LDFLAGS = @lt_enable_auto_import@

edje_external_inspector_SOURCES = edje_external_inspector.c
edje_external_inspector_CPPFLAGS = \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/lib \
@EDJE_EXTERNAL_INSPECTOR_CFLAGS@
edje_external_inspector_LDADD = $(top_builddir)/src/lib/libedje.la @EDJE_EXTERNAL_INSPECTOR_LIBS@ @VORBISENC_LIBS@ @FLAC_LIBS@ @SNDFILE_LIBS@
edje_external_inspector_LDFLAGS = @lt_enable_auto_import@

EXTRA_DIST = @EDJE_RECC_PRG@ edje_cc.h edje_convert.h edje_convert.c edje_multisense_convert.h edje_data_convert.c
EXTRA_SCRIPTS = edje_recc

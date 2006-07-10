## Process this file with automake to produce Makefile.in

INCLUDES = \
-I$(top_srcdir) \
-I$(top_srcdir)/bin \
-I$(top_srcdir)/src/lib \
@EET_CFLAGS@ \
@EVAS_CFLAGS@ \
@ECORE_CFLAGS@ \
@EMBRYO_CFLAGS@

bin_SCRIPTS = \
@EDJE_RECC_PRG@

EXTRA_DIST = @EDJE_RECC_PRG@
EXTRA_SCRIPTS = edje_recc

bin_PROGRAMS = \
edje \
edje_ls \
edje_test \
@EDJE_CC_PRG@ \
@EDJE_DECC_PRG@ \
@EDJE_THUMB_PRG@

EXTRA_PROGRAMS = \
edje_cc \
edje_decc \
edje_thumb

edje_SOURCES = \
edje_main.c \
edje_main.h \
edje_prefix.c \
edje_prefix.h

edje_LDADD = \
$(top_builddir)/src/lib/libedje.la

edje_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la


edje_cc_SOURCES = \
edje_cc.c \
edje_cc.h \
edje_cc_out.c \
edje_cc_parse.c \
edje_cc_mem.c \
edje_cc_handlers.c \
edje_cc_sources.c \
edje_prefix.c \
edje_prefix.h

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


edje_thumb_SOURCES = \
edje_thumb.c \
edje_thumb.h

edje_thumb_LDADD = \
$(top_builddir)/src/lib/libedje.la

edje_thumb_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la



edje_ls_SOURCES = \
edje_ls.c

edje_ls_LDADD = \
$(top_builddir)/src/lib/libedje.la

edje_ls_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la



edje_test_SOURCES = \
edje_test_main.c

edje_test_LDADD = \
$(top_builddir)/src/lib/libedje.la

edje_test_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la

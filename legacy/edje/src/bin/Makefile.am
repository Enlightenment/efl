## Process this file with automake to produce Makefile.in

INCLUDES = \
-I$(top_srcdir) \
-I$(top_srcdir)/bin \
-I$(top_srcdir)/src/lib \
@EET_CFLAGS@ \
@EVAS_CFLAGS@ \
@ECORE_CFLAGS@ \
@IMLIB2_CFLAGS@

bin_SCRIPTS = \
@EDJE_RECC_PRG@

bin_PROGRAMS = \
edje \
edje_ls \
@EDJE_CC_PRG@ \
@EDJE_DECC_PRG@

EXTRA_PROGRAMS = \
edje_cc \
edje_decc

edje_SOURCES = \
edje_main.c \
edje_main.h

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
edje_cc_sources.c

edje_cc_LDADD = \
$(top_builddir)/src/lib/libedje.la \
@IMLIB2_LIBS@

edje_cc_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la


edje_decc_SOURCES = \
edje_decc.c \
edje_decc.h \
edje_cc_mem.c \
edje_cc_sources.c

edje_decc_LDADD = \
$(top_builddir)/src/lib/libedje.la \
@IMLIB2_LIBS@

edje_decc_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la



edje_ls_SOURCES = \
edje_ls.c

edje_ls_LDADD = \
$(top_builddir)/src/lib/libedje.la

edje_ls_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la

EXTRA_SCRIPTS = edje_recc

EXTRA_DIST = edje_recc

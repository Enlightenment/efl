## Process this file with automake to produce Makefile.in

INCLUDES = \
-I$(top_srcdir) \
-I$(top_srcdir)/bin \
-I$(top_srcdir)/src/lib \
@eet_cflags@ \
@evas_cflags@ \
@ecore_cflags@ \
@imlib2_cflags@

bin_PROGRAMS = \
edje \
edje_ls \
@EDJE_CC_PRG@

EXTRA_PROGRAMS = \
edje_cc

edje_SOURCES = \
edje_main.c \
edje.h

edje_LDADD = \
$(top_builddir)/src/lib/libedje.la

#edje_LDFLAGS = -static
edje_LDFLAGS =

edje_CPPFLAGS = @edje_def@

edje_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la


edje_cc_SOURCES = \
edje_cc.c \
edje_cc.h \
edje_cc_out.c \
edje_cc_parse.c \
edje_cc_mem.c \
edje_cc_handlers.c

edje_cc_LDADD = \
$(top_builddir)/src/lib/libedje.la \
@imlib2_libs@

edje_cc_CPPFLAGS = @edje_def@

edje_cc_LDFLAGS =

edje_cc_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la


edje_ls_SOURCES = \
edje_ls.c

edje_ls_LDADD = \
$(top_builddir)/src/lib/libedje.la

edje_ls_CPPFLAGS = @edje_def@

edje_ls_LDFLAGS =

edje_ls_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la

## Process this file with automake to produce Makefile.in

INCLUDES = \
-I.. \
-I../lib \
-I../.. \
-I$(top_srcdir) \
-I$(top_builddir) \
-I$(top_srcdir)/lib \
-I$(top_builddir)/lib \
-I/usr/local/include \
@my_includes@ \
@imlib2_includes@

bin_PROGRAMS = edje edje_cc

edje_SOURCES = \
edje_main.c \
edje.h

edje_LDADD = \
$(top_builddir)/src/lib/libedje.la \
@imlib2_libs@

#edje_LDFLAGS = -static
edje_LDFLAGS =

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

edje_cc_LDFLAGS =


edje_cc_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la

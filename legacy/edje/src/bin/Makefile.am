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
@my_includes@

bin_PROGRAMS = edje

edje_SOURCES = \
edje_main.c

edje_LDADD = \
$(top_builddir)/src/lib/libedje.la

edje_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la

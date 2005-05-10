MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir)/src/lib

bin_PROGRAMS = eet eet_bench

eet_SOURCES = \
eet_main.c

eet_LDADD = $(top_builddir)/src/lib/libeet.la
eet_DEPENDENCIES = $(top_builddir)/src/lib/libeet.la

eet_bench_SOURCES = \
eet_bench_main.c

eet_bench_LDADD = $(top_builddir)/src/lib/libeet.la
eet_bench_DEPENDENCIES = $(top_builddir)/src/lib/libeet.la

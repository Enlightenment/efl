## Process this file with automake to produce Makefile.in

AM_CFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/bin \
-I$(top_srcdir)/src/lib

bin_PROGRAMS = eet

eet_SOURCES = eet_main.c
eet_LDADD = $(top_builddir)/src/lib/libeet.la
eet_DEPENDENCIES = $(top_builddir)/src/lib/libeet.la

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
@EFREET_CFLAGS@

bin_PROGRAMS = efreet_alloc efreet_menu_alloc

efreet_menu_alloc_SOURCES = efreet_menu_alloc.c comp.h
efreet_menu_alloc_LDADD = $(top_builddir)/src/lib/libefreet.la @EFREET_LIBS@

efreet_alloc_SOURCES = efreet_alloc.c comp.h
efreet_alloc_LDADD = $(top_builddir)/src/lib/libefreet.la @EFREET_LIBS@


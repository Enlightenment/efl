
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@EFL_EO_BUILD@ \
@EO_CFLAGS@

noinst_PROGRAMS = signals

signals_SOURCES = \
main.c \
simple.c \
simple.h

signals_LDADD = $(top_builddir)/src/lib/libeo.la @EO_LIBS@

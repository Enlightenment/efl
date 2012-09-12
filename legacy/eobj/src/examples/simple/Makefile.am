
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@EFL_EO_BUILD@ \
@EO_CFLAGS@

EXTRA_PROGRAMS = simple

examples: simple

simple_SOURCES = \
interface.c \
interface.h \
main.c \
mixin.c \
mixin.h \
simple.c \
simple.h

simple_LDADD = $(top_builddir)/src/lib/libeo.la @EO_LIBS@

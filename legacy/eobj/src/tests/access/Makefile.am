
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@EFL_EO_BUILD@ \
@EO_CFLAGS@

check_PROGRAMS = access

access_SOURCES = \
inherit.c \
inherit.h \
main.c \
simple.c \
simple.h \
simple_protected.h

access_LDADD = $(top_builddir)/src/lib/libeo.la @EO_LIBS@

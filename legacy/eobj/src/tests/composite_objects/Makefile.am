
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@EFL_EO_BUILD@ \
@EO_CFLAGS@

check_PROGRAMS = composite_objects

composite_objects_SOURCES = \
comp.c \
comp.h \
main.c \
simple.c \
simple.h

composite_objects_LDADD = $(top_builddir)/src/lib/libeo.la @EO_LIBS@

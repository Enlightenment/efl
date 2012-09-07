
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@EFL_EO_BUILD@ \
@EO_CFLAGS@

noinst_PROGRAMS = constructors

constructors_SOURCES = \
main.c \
mixin.c \
mixin.h \
simple.c \
simple.h \
simple2.c \
simple2.h \
simple3.c \
simple3.h \
simple4.c \
simple4.h \
simple5.c \
simple5.h \
simple6.c \
simple6.h \
simple7.c \
simple7.h

constructors_LDADD = $(top_builddir)/src/lib/libeo.la @EO_LIBS@

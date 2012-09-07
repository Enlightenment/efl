
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@EFL_EO_BUILD@ \
@EO_CFLAGS@

noinst_PROGRAMS = function_overrides

function_overrides_SOURCES = \
inherit.c \
inherit.h \
inherit2.c \
inherit2.h \
inherit3.c \
inherit3.h \
main.c \
simple.c \
simple.h

function_overrides_LDADD = $(top_builddir)/src/lib/libeo.la @EO_LIBS@

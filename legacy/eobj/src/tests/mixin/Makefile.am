
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@EFL_EO_BUILD@ \
@EO_CFLAGS@

noinst_PROGRAMS = mixin

mixin_SOURCES = \
inherit.c \
inherit.h \
main.c \
mixin.c \
mixin.h \
mixin2.c \
mixin2.h \
mixin3.c \
mixin3.h \
mixin4.c \
mixin4.h \
simple.c \
simple.h

mixin_LDADD = $(top_builddir)/src/lib/libeo.la @EO_LIBS@

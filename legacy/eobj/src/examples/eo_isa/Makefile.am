
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@EFL_EO_BUILD@ \
@EO_CFLAGS@

noinst_PROGRAMS =

if EFL_BUILD_EXAMPLES

noinst_PROGRAMS += eo_isa

eo_isa_SOURCES = \
complex.c \
complex.h \
interface.c \
interface.h \
main.c \
mixin.c \
mixin.h \
simple.c \
simple.h

eo_isa_LDADD = $(top_builddir)/src/lib/libeo.la @EO_LIBS@

endif


MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@EFL_EO_BUILD@ \
@EO_CFLAGS@

noinst_PROGRAMS = interface

interface_SOURCES = \
interface.c \
interface.h \
interface2.c \
interface2.h \
main.c \
simple.c \
simple.h

interface_LDADD = $(top_builddir)/src/lib/libeo.la @EO_LIBS@

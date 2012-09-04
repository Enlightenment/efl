
MAINTAINERCLEANFILES = Makefile.in

exampledir = $(pkgdatadir)/examples

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@EFL_EO_BUILD@ \
@EO_CFLAGS@

SRCS = \
interface.c \
interface.h \
main.c \
mixin.c \
mixin.h \
simple.c \
simple.h

example_PROGRAMS =

if EFL_INSTALL_EXAMPLES
filesdir = $(pkgdatadir)/examples
files_DATA = $(SRCS)
endif

if EFL_BUILD_EXAMPLES

example_PROGRAMS += simple

simple_SOURCES = $(SRCS)
simple_LDADD = $(top_builddir)/src/lib/libeo.la @EO_LIBS@

endif

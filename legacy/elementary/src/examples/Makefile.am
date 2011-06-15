MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/examples

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@ELEMENTARY_EDBUS_CFLAGS@ \
@ELEMENTARY_EFREET_CFLAGS@ \
@ELEMENTARY_ETHUMB_CFLAGS@ \
@ELEMENTARY_CFLAGS@

LDADD = \
	$(top_builddir)/src/lib/libelementary.la

SRCS = \
	actionslider_example_01.c

pkglib_PROGRAMS =

if EFL_INSTALL_EXAMPLES
filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA = $(SRCS)
endif

if EFL_BUILD_EXAMPLES
pkglib_PROGRAMS += \
	actionslider_example_01
endif


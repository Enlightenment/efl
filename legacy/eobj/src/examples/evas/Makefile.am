
MAINTAINERCLEANFILES = Makefile.in

exampledir = $(pkgdatadir)/examples

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@EO_CFLAGS@

SRCS = \
elw_box.c \
elw_box.h \
elw_boxedbutton.c \
elw_boxedbutton.h  \
elw_button.c \
elw_button.h \
elw_win.h \
elw_win.c \
evas_obj.c \
evas_obj.h \
test.c

example_PROGRAMS =

if EFL_INSTALL_EXAMPLES
filesdir = $(pkgdatadir)/examples
files_DATA = $(SRCS)
endif

if EFL_BUILD_EXAMPLES
if EO_BUILD_EXAMPLE_EVAS

example_PROGRAMS += evas

evas_SOURCES = $(SRCS)
evas_LDADD = $(top_builddir)/src/lib/libeo.la @EO_LIBS@

endif
endif

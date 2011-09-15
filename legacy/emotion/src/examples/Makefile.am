MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/examples

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
@EMOTION_CFLAGS@ \
@EMOTION_CPPFLAGS@ \
@EMOTION_BIN_CFLAGS@

LDADD = \
	$(top_builddir)/src/lib/libemotion.la \
	@EMOTION_BIN_LIBS@

SRCS = \
	emotion_basic_example.c \
	emotion_generic_example.c \
	emotion_border_example.c \
	emotion_signals_example.c

EXTRA_DIST = $(SRCS)

pkglib_PROGRAMS =

if EFL_INSTALL_EXAMPLES
filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA = $(SRCS)
endif

if EFL_BUILD_EXAMPLES
pkglib_PROGRAMS += \
	emotion_basic_example \
	emotion_generic_example \
	emotion_border_example \
	emotion_signals_example
endif


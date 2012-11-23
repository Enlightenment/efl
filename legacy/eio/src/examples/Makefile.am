MAINTAINERCLEANFILES = Makefile.in

examplesdir = $(datadir)/$(PACKAGE)/examples

filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA =

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib/ \
@EIO_CFLAGS@

ECOREBASELDADD = \
	$(top_builddir)/src/lib/libeio.la \
	@EIO_LIBS@


LDADD = \
	$(ECOREBASELDADD)

SRCS = \
	eio_file_ls.c

examples_PROGRAMS =

if EFL_BUILD_EXAMPLES
examples_PROGRAMS += \
	eio_file_ls

endif

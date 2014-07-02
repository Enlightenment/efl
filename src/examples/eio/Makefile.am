MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/eet \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/eio \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/eet \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/eio

EXTRA_PROGRAMS = \
eio_file_ls \
eio_file_copy

eio_file_ls_SOURCES = eio_file_ls.c
eio_file_ls_LDADD = \
$(top_builddir)/src/lib/eio/libeio.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/eet/libeet.la \
$(top_builddir)/src/lib/eina/libeina.la \
@EIO_LDFLAGS@

eio_file_copy_SOURCES = eio_file_copy.c
eio_file_copy_LDADD = \
$(top_builddir)/src/lib/eio/libeio.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/eet/libeet.la \
$(top_builddir)/src/lib/eina/libeina.la \
@EIO_LDFLAGS@

SRCS = \
eio_file_ls.c \
eio_file_copy.c

DATA_FILES = Makefile.examples

EXTRA_DIST = $(DATA_FILES)

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	mkdir -p $(datadir)/eio/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(SRCS) $(DATA_FILES) $(datadir)/eio/examples

uninstall-local:
	for f in $(SRCS) $(DATA_FILES); do \
	  rm -f $(datadir)/eio/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif

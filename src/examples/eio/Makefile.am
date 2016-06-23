MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/efl \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/emile \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/eet \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/eio \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/emile \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/efl \
-I$(top_builddir)/src/lib/eet \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/eio

if HAVE_WINDOWS
AM_CPPFLAGS += \
-I$(top_srcdir)/src/lib/evil \
-I$(top_builddir)/src/lib/evil
endif

EXTRA_PROGRAMS = \
eio_file_ls \
eio_file_copy \
efl_io_manager_open \
efl_io_manager_open_multi \
efl_io_manager_ls \
eio_sentry

EIO_EXAMPLES_LDADD = \
$(top_builddir)/src/lib/eio/libeio.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/efl/libefl.la \
$(top_builddir)/src/lib/eet/libeet.la \
$(top_builddir)/src/lib/emile/libemile.la \
$(top_builddir)/src/lib/eina/libeina.la \
@EIO_LDFLAGS@

eio_file_ls_SOURCES = eio_file_ls.c
eio_file_ls_LDADD = \
$(EIO_EXAMPLES_LDADD)

eio_file_copy_SOURCES = eio_file_copy.c
eio_file_copy_LDADD = \
$(EIO_EXAMPLES_LDADD)

efl_io_manager_open_SOURCES = efl_io_manager_open.c
efl_io_manager_open_LDADD = \
$(EIO_EXAMPLES_LDADD)

efl_io_manager_open_multi_SOURCES = efl_io_manager_open_multi.c
efl_io_manager_open_multi_LDADD = \
$(EIO_EXAMPLES_LDADD)

efl_io_manager_ls_SOURCES = efl_io_manager_ls.c
efl_io_manager_ls_LDADD = \
$(EIO_EXAMPLES_LDADD)

eio_sentry_SOURCES = eio_sentry.c
eio_sentry_LDADD = \
$(EIO_EXAMPLES_LDADD)

SRCS = \
eio_file_ls.c \
eio_file_copy.c \
efl_io_manager_open.c \
efl_io_manager_open_multi.c \
efl_io_manager_ls.c \
eio_sentry.c

DATA_FILES = Makefile.examples

EXTRA_DIST = $(DATA_FILES)

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	$(MKDIR_P) $(DESTDIR)$(datadir)/eio/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(SRCS) $(DATA_FILES) $(DESTDIR)$(datadir)/eio/examples

uninstall-local:
	for f in $(SRCS) $(DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/eio/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif

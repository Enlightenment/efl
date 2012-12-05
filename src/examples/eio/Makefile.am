MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
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

EXTRA_PROGRAMS = eio_file_ls

eio_file_ls_SOURCES = eio_file_ls.c
eio_file_ls_LDADD = \
$(top_builddir)/src/lib/eio/libeio.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/eet/libeet.la \
$(top_builddir)/src/lib/eina/libeina.la \
@EIO_LIBS@

SRCS = \
eio_file_ls

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	mkdir -p $(datadir)/eio/examples
	$(install_sh_DATA) -c $(SRCS) $(datadir)/eio/examples

uninstall-local:
	for f in $(SRCS) ; do \
	  rm -f $(datadir)/io/examples/$$f ; \
	done

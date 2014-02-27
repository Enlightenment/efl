MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_avahi\
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_avahi \
@AVAHI_CLIENT_CFLAGS@

EXTRA_PROGRAMS = \
ecore_avahi_example \
ecore_avahi_browse_example

ecore_avahi_example_SOURCES = ecore_avahi_example.c
ecore_avahi_example_LDADD = $(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_avahi/libecore_avahi.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/eina/libeina.la \
@AVAHI_CLIENT_LIBS@

ecore_avahi_browse_example_SOURCES = ecore_avahi_browse_example.c
ecore_avahi_browse_example_LDADD = $(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_avahi/libecore_avahi.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/eina/libeina.la \
@AVAHI_CLIENT_LIBS@

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	mkdir -p $(datadir)/ecore_avahi/examples
	$(install_sh_DATA) -c ecore_avahi_browse_example.c $(datadir)/ecore_avahi/examples
	$(install_sh_DATA) -c ecore_avahi_example.c $(datadir)/ecore_avahi/examples

uninstall-local:
	rm -f $(datadir)/ecore_avahi/examples/ecore_avahi_example.c
	rm -f $(datadir)/ecore_avahi/examples/ecore_avahi_browse_example.c

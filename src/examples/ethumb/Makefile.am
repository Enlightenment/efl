MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_builddir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_builddir)/src/lib/eo \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore_evas \
-I$(top_srcdir)/src/lib/ecore_file \
-I$(top_builddir)/src/lib/ecore_file \
-I$(top_srcdir)/src/lib/evas \
-I$(top_builddir)/src/lib/evas \
-I$(top_srcdir)/src/lib/edje \
-I$(top_builddir)/src/lib/edje \
-I$(top_srcdir)/src/lib/ethumb \
-I$(top_builddir)/src/lib/ethumb \
-I$(top_srcdir)/src/lib/ethumb/client \
-I$(top_builddir)/src/lib/ethumb/client \
@ETHUMB_CFLAGS@

LDADD = \
$(top_builddir)/src/lib/eina/libeina.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
$(top_builddir)/src/lib/ecore_file/libecore_file.la \
$(top_builddir)/src/lib/evas/libevas.la \
$(top_builddir)/src/lib/edje/libedje.la \
$(top_builddir)/src/lib/ethumb/libethumb.la \
$(top_builddir)/src/lib/ethumb/client/libethumb_client.la \
@ETHUMB_LDFLAGS@

EXTRA_PROGRAMS = \
ethumb_dbus

SRCS = \
ethumb_dbus.c

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	mkdir -p $(datadir)/ethumb/examples
	$(install_sh_DATA) -c $(SRCS) $(datadir)/ethumb/examples

uninstall-local:
	for f in $(SRCS); do \
	  rm -f $(datadir)/ethumb/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif

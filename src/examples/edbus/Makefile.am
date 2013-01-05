MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/edbus \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/edbus \
@EDBUS_CFLAGS@

EXAMPLES_LIBS = \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/eina/libeina.la \
$(top_builddir)/src/lib/edbus/libedbus2.la \
@EDBUS_LDFLAGS@

EXTRA_PROGRAMS = \
connman-list-services \
ofono-dial \
banshee \
complex-types \
complex-types-server \
server \
client \
simple-signal-emit \
complex-types-client-eina-value

connman_list_services_SOURCES = connman-list-services.c
connman_list_services_LDADD = $(EXAMPLES_LIBS)

ofono_dial_SOURCES = ofono-dial.c
ofono_dial_LDADD = $(EXAMPLES_LIBS)

banshee_SOURCES = banshee.c
banshee_LDADD = $(EXAMPLES_LIBS)

complex_types_SOURCES = complex-types.c
complex_types_LDADD = $(EXAMPLES_LIBS)

complex_types_server_SOURCES = complex-types-server.c
complex_types_server_LDADD = $(EXAMPLES_LIBS)

server_SOURCES = server.c
server_LDADD = $(EXAMPLES_LIBS)

client_SOURCES = client.c
client_LDADD = $(EXAMPLES_LIBS)

simple_signal_emit_SOURCES = simple-signal-emit.c
simple_signal_emit_LDADD = $(EXAMPLES_LIBS)

complex_types_client_eina_value_SOURCES = complex-types-client-eina-value.c
complex_types_client_eina_value_LDADD = $(EXAMPLES_LIBS)

SRCS = $(EXTRA_PROGRAMS)

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	mkdir -p $(datadir)/edbus/examples
	$(install_sh_DATA) -c $(SRCS) $(datadir)/edbus/examples

uninstall-local:
	for f in $(SRCS) ; do \
	  rm -f $(datadir)/edbus/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/eldbus \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/eldbus \
@ELDBUS_CFLAGS@

EXAMPLES_LIBS = \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/eina/libeina.la \
$(top_builddir)/src/lib/eldbus/libeldbus.la \
@ELDBUS_LDFLAGS@

SRCS = \
connman-list-services.c \
ofono-dial.c \
banshee.c \
complex-types.c \
complex-types-server.c \
server.c \
client.c \
simple-signal-emit.c \
complex-types-client-eina-value.c \
connect-address.c

EXTRA_PROGRAMS = \
connman-list-services \
ofono-dial \
banshee \
complex-types \
complex-types-server \
server \
client \
simple-signal-emit \
complex-types-client-eina-value \
connect-address

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

connect_address_SOURCES = connect-address.c
connect_address_LDADD = $(EXAMPLES_LIBS)

DATA_FILES = Makefile.examples

EXTRA_DIST = $(DATA_FILES)

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	mkdir -p $(datadir)/eldbus/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(SRCS) $(DATA_FILES) $(datadir)/eldbus/examples

uninstall-local:
	for f in $(SRCS) $(DATA_FILES); do \
	  rm -f $(datadir)/eldbus/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif

ELDBUS_SRCS = \
eldbus/connman-list-services.c \
eldbus/ofono-dial.c \
eldbus/banshee.c \
eldbus/complex-types.c \
eldbus/complex-types-server.c \
eldbus/server.c \
eldbus/client.c \
eldbus/simple-signal-emit.c \
eldbus/complex-types-client-eina-value.c \
eldbus/connect-address.c \
eldbus/dbusmodel.c

EXTRA_PROGRAMS += \
eldbus/connman-list-services \
eldbus/ofono-dial \
eldbus/banshee \
eldbus/complex-types \
eldbus/complex-types-server \
eldbus/server \
eldbus/client \
eldbus/simple-signal-emit \
eldbus/complex-types-client-eina-value \
eldbus/connect-address \
eldbus/dbusmodel


ELDBUS_DATA_FILES = eldbus/Makefile.examples
DATA_FILES += $(ELDBUS_DATA_FILES)

install-examples-eldbus:
	$(MKDIR_P) $(DESTDIR)$(datadir)/eldbus/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(ELDBUS_SRCS) $(ELDBUS_DATA_FILES) $(DESTDIR)$(datadir)/eldbus/examples

uninstall-local-eldbus:
	for f in $(ELDBUS_SRCS) $(ELDBUS_DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/eldbus/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-eldbus
UNINSTALL_EXAMPLES += uninstall-local-eldbus

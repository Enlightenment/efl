EXTRA_PROGRAMS += \
ethumb_client/ethumb_dbus

ETHUMB_CLIENT_SRCS = \
ethumb_client/ethumb_dbus.c

ETHUMB_CLIENT_DATA_FILES = ethumb_client/Makefile.examples

DATA_FILES += $(ETHUMB_CLIENT_DATA_FILES)

install-examples-ethumb-client:
	$(MKDIR_P) $(DESTDIR)$(datadir)/ethumb_client/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(ETHUMB_CLIENT_SRCS) $(ETHUMB_CLIENT_DATA_FILES) $(DESTDIR)$(datadir)/ethumb_client/examples

uninstall-local-ethumb-client:
	for f in $(ETHUMB_CLIENT_SRCS) $(ETHUMB_CLIENT_DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/ethumb_client/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-ethumb-client
UNINSTALL_EXAMPLES += uninstall-local-ethumb-client

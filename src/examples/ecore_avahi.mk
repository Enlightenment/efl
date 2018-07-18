ECORE_AVAHI_SRCS = \
ecore_avahi/ecore_avahi_browse_example.c \
ecore_avahi/ecore_avahi_example.c

EXTRA_PROGRAMS += \
ecore_avahi/ecore_avahi_example \
ecore_avahi/ecore_avahi_browse_example

install-examples-ecoreavahi: examples
	$(MKDIR_P) $(DESTDIR)$(datadir)/ecore_avahi/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(ECORE_AVAHI_SRCS) $(DESTDIR)$(datadir)/ecore_avahi/examples

uninstall-local-ecoreavahi:
	for f in $(ECORE_AVAHI_SRCS); do \
	  rm -f $(DESTDIR)$(datadir)/ecore_avahi/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-ecoreavahi
UNINSTALL_EXAMPLES += uninstall-local-ecoreavahi

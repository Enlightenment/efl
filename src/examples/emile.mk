EMILE_SRCS = \
emile/emile_base64.c

EXTRA_PROGRAMS += \
emile/emile_base64

install-examples-emile:
	$(MKDIR_P) $(DESTDIR)$(datadir)/emile/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(EMILE_SRCS) $(DESTDIR)$(datadir)/emile/examples

uninstall-local-emile:
	for f in $(EMILE_SRCS); do \
	  rm -f $(DESTDIR)$(datadir)/emile/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-emile
UNINSTALL_EXAMPLES += uninstall-local-emile

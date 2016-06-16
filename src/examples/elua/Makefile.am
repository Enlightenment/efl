if HAVE_ELUA

MAINTAINERCLEANFILES = Makefile.in

DATA_FILES =
EXTRA_DIST = $(DATA_FILES)

examples: ;

clean-local: ;

install-examples:
	$(MKDIR_P) $(DESTDIR)$(datadir)/elua/examples
#	cd $(srcdir) && $(install_sh_DATA) -c $(DATA_FILES) $(DESTDIR)$(datadir)/elua/examples

#uninstall-local:
#	for f in $(DATA_FILES); do \
#	  rm -f $(DESTDIR)$(datadir)/elua/examples/$$f ; \
#	done

endif

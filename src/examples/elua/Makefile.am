if HAVE_ELUA

MAINTAINERCLEANFILES = Makefile.in

DATA_FILES = Makefile.examples
EXTRA_DIST = $(DATA_FILES)

install-examples:
	mkdir -p $(datadir)/elua/examples
	$(install_sh_DATA) -c $(DATA_FILES) $(datadir)/elua/examples

uninstall-local:
	for f in $(DATA_FILES); do \
	  rm -f $(datadir)/elua/examples/$$f ; \
	done

endif

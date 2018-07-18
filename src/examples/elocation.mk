EXTRA_PROGRAMS += \
elocation/elocation

elocation_elocation_SOURCES = \
elocation/elocation.c

ELOCATION_SRCS = \
elocation/elocation.c

ELOCATION_DATA_FILES = elocation/Makefile.examples

DATA_FILES += $(ELOCATION_DATA_FILES)

install-examples-elocation:
	$(MKDIR_P) $(DESTDIR)$(datadir)/elocation/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(ELOCATION_SRCS) $(ELOCATION_DATA_FILES) $(DESTDIR)$(datadir)/elocation/examples

uninstall-local-elocation:
	for f in $(ELOCATION_SRCS) $(ELOCATION_DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/elocation/examples/$$f ; \
	done


INSTALL_EXAMPLES += install-examples-elocation
UNINSTALL_EXAMPLES += uninstall-local-elocation

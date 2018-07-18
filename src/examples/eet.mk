EXTRA_PROGRAMS += \
eet/eet-basic \
eet/eet-file \
eet/eet-data-simple \
eet/eet-data-nested \
eet/eet-data-file_descriptor_01 \
eet/eet-data-file_descriptor_02 \
eet/eet-data-cipher_decipher

EET_SRCS = \
eet/eet-basic.c \
eet/eet-file.c \
eet/eet-data-simple.c \
eet/eet-data-nested.c \
eet/eet-data-file_descriptor_01.c \
eet/eet-data-file_descriptor_02.c \
eet/eet-data-cipher_decipher.c

EET_DATA_FILES = eet/Makefile.examples
DATA_FILES += $(EET_DATA_FILES)

install-examples-eet:
	$(MKDIR_P) $(DESTDIR)$(datadir)/eet/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(EET_SRCS) $(EET_DATA_FILES) $(DESTDIR)$(datadir)/eet/examples

uninstall-local-eet:
	for f in $(EET_SRCS) $(EET_DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/eet/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-eet
UNINSTALL_EXAMPLES += uninstall-local-eet

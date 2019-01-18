EXTRA_PROGRAMS += \
eio/eio_file_ls \
eio/eio_file_copy \
eio/efl_io_manager_open \
eio/efl_io_manager_open_multi \
eio/efl_io_manager_ls \
eio/eio_sentry

eio_eio_file_ls_SOURCES = eio/eio_file_ls.c
eio_eio_file_copy_SOURCES = eio/eio_file_copy.c
eio_efl_io_manager_open_SOURCES = eio/efl_io_manager_open.c
eio_efl_io_manager_open_multi_SOURCES = eio/efl_io_manager_open_multi.c
eio_efl_io_manager_ls_SOURCES = eio/efl_io_manager_ls.c
eio_eio_sentry_SOURCES = eio/eio_sentry.c

EIO_SRCS = \
eio/eio_file_ls.c \
eio/eio_file_copy.c \
eio/efl_io_manager_open.c \
eio/efl_io_manager_open_multi.c \
eio/efl_io_manager_ls.c \
eio/eio_sentry.c

EIO_DATA_FILES = eio/Makefile.examples

DATA_FILES += $(EIO_DATA_FILES)

install-examples-eio:
	$(MKDIR_P) $(DESTDIR)$(datadir)/eio/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(EIO_SRCS) $(EIO_DATA_FILES) $(DESTDIR)$(datadir)/eio/examples

uninstall-local-eio:
	for f in $(EIO_SRCS) $(DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/eio/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-eio
UNINSTALL_EXAMPLES += uninstall-local-eio

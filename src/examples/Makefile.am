MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = eina eo eet evas ecore eio edbus ephysics edje emotion ethumb

.PHONY: examples install-examples

examples:
	@for d in $(SUBDIRS); do \
		$(MAKE) $(AM_MAKEFLAGS) -C "$$d" examples || exit 0; \
	done

install-examples:
	@for d in $(SUBDIRS); do \
		$(MAKE) $(AM_MAKEFLAGS) -C "$$d" install-examples || exit 0; \
	done

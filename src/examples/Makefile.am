MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = eina eet

examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eet examples

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina install-examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eet install-examples

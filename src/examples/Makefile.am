MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = eina eo eet

examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eo examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eet examples

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina install-examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eo install-examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eet install-examples

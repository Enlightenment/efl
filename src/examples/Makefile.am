MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = eina eo eet evas

examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eo examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eet examples
	@$(MAKE) $(AM_MAKEFLAGS) -C evas examples

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina install-examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eo install-examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eet install-examples
	@$(MAKE) $(AM_MAKEFLAGS) -C evas install-examples

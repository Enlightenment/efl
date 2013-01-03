MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = eina eo eet evas ecore eio edbus ephysics

examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eo examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eet examples
	@$(MAKE) $(AM_MAKEFLAGS) -C evas examples
	@$(MAKE) $(AM_MAKEFLAGS) -C ecore examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eio examples
	@$(MAKE) $(AM_MAKEFLAGS) -C edbus examples
	@$(MAKE) $(AM_MAKEFLAGS) -C ephysics examples

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina install-examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eo install-examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eet install-examples
	@$(MAKE) $(AM_MAKEFLAGS) -C evas install-examples
	@$(MAKE) $(AM_MAKEFLAGS) -C ecore install-examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eio install-examples
	@$(MAKE) $(AM_MAKEFLAGS) -C edbus install-examples
	@$(MAKE) $(AM_MAKEFLAGS) -C ephysics install-examples

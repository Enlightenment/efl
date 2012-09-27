MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = eina eet eo

examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eo examples
if EFL_BUILD_EET
	@$(MAKE) $(AM_MAKEFLAGS) -C eet examples
endif

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina install-examples
	@$(MAKE) $(AM_MAKEFLAGS) -C eo install-examples
if EFL_BUILD_EET
	@$(MAKE) $(AM_MAKEFLAGS) -C eet install-examples
endif

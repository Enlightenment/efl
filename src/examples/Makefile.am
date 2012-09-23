MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = eina eet

examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina examples
if EFL_BUILD_EET
	@$(MAKE) $(AM_MAKEFLAGS) -C eet examples
endif

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina install-examples
if EFL_BUILD_EET
	@$(MAKE) $(AM_MAKEFLAGS) -C eet install-examples
endif

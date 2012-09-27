MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = eina eet eo

examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina examples
if EFL_BUILD_EET
	@$(MAKE) $(AM_MAKEFLAGS) -C eet examples
endif
if EFL_BUILD_EO
	@$(MAKE) $(AM_MAKEFLAGS) -C eo examples
endif

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina install-examples
if EFL_BUILD_EET
	@$(MAKE) $(AM_MAKEFLAGS) -C eet install-examples
endif
if EFL_BUILD_EO
	@$(MAKE) $(AM_MAKEFLAGS) -C eo install-examples
endif

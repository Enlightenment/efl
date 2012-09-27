MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = eina eo

.PHONY: benchmark benchmark-e17

benchmark:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina benchmark
if EFL_BUILD_EO
	@$(MAKE) $(AM_MAKEFLAGS) -C eo benchmark
endif

benchmark-e17:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina benchmark-e17

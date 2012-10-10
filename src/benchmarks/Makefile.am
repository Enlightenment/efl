MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = eina eo

.PHONY: benchmark benchmark-e17

benchmark:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina benchmark
	@$(MAKE) $(AM_MAKEFLAGS) -C eo benchmark

benchmark-e17:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina benchmark-e17

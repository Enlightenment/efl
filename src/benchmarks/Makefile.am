MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = eina eo

.PHONY: benchmark

benchmark:
	@$(MAKE) $(AM_MAKEFLAGS) -C eina benchmark
	@$(MAKE) $(AM_MAKEFLAGS) -C eo benchmark

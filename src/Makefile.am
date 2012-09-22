MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = lib include modules bin scripts benchmarks tests examples

.PHONY: benchmark benchmark-e17 examples

benchmark:
	@$(MAKE) $(AM_MAKEFLAGS) -C lib
	@$(MAKE) $(AM_MAKEFLAGS) -C benchmarks benchmark

benchmark-e17:
	@$(MAKE) $(AM_MAKEFLAGS) -C lib
	@$(MAKE) $(AM_MAKEFLAGS) -C benchmarks benchmark-e17

examples:
	@make -C lib
	@$(MAKE) $(AM_MAKEFLAGS) -C examples examples

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C examples install-examples

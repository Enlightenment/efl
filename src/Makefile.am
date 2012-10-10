MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = lib modules bin scripts benchmarks tests examples

.PHONY: benchmark examples

benchmark:
	@$(MAKE) $(AM_MAKEFLAGS) -C lib
	@$(MAKE) $(AM_MAKEFLAGS) -C benchmarks benchmark

examples:
	@make -C lib
	@$(MAKE) $(AM_MAKEFLAGS) -C examples examples

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C examples install-examples

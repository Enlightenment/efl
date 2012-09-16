MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = lib include modules bin scripts benchmarks tests examples

.PHONY: benchmark benchmark-e17 examples

benchmark:
	@make -C lib
	@$(MAKE) -C benchmarks benchmark

benchmark-e17:
	@make -C lib
	@$(MAKE) -C benchmarks benchmark-e17

examples:
	@make -C lib
	@$(MAKE) -C examples examples

install-examples:
	@$(MAKE) -C examples install-examples

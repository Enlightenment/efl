MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = lib include modules bin scripts benchmarks tests

.PHONY: benchmark benchmark-e17

benchmark:
	@make -C lib
	@$(MAKE) -C benchmarks benchmark

benchmark-e17:
	@make -C lib
	@$(MAKE) -C benchmarks benchmark-e17

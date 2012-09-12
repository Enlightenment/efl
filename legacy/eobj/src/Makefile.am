
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = lib benchmarks examples tests

.PHONY: benchmark examples

benchmark:
	@make -C lib
	@$(MAKE) -C benchmarks benchmark

examples:
	@make -C lib
	@$(MAKE) -C examples examples

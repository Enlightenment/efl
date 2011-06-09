SUBDIRS = lib include modules tests examples

MAINTAINERCLEANFILES = Makefile.in

.PHONY: benchmark

if EFL_ENABLE_BENCHMARK

benchmark: all
	cd tests && make eina_bench

else

benchmark:
	@echo "reconfigure with --enable-benchmark"

endif

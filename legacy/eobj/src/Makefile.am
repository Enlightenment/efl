
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = lib benchmarks examples tests

.PHONY: benchmark

if EFL_ENABLE_BENCHMARK

benchmark: all
	cd benchamrks && make eo_bench

else

benchmark:
	@echo "reconfigure with --enable-benchmark"

endif

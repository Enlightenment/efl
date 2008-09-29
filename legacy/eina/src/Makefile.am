SUBDIRS = lib include modules tests

MAINTAINERCLEANFILES = Makefile.in

.PHONY: benchmark

if EFL_ENABLE_BENCHMARK

benchmark: all
	make -C tests eina_bench

else

benchmark:
	@echo "reconfigure with --enable-benchmark"

endif

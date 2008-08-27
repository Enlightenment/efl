SUBDIRS = lib include modules tests

MAINTAINERCLEANFILES = Makefile.in

.PHONY: benchmark

if EINA_ENABLE_BENCH

benchmark: all
	make -C tests eina_bench

else

benchmark:
	@echo "reconfigure with --enable-bench"

endif

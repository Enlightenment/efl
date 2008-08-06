SUBDIRS = lib include modules tests

MAINTAINERCLEANFILES = Makefile.in

if EINA_ENABLE_BENCH

bench: all
	make -C tests eina_bench

else

bench:
	@echo "reconfigure with --enable-bench"

endif

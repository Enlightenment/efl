AUTOMAKE_OPTIONS = subdir-objects

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = . benchmarks tests examples

lib_LTLIBRARIES =
bin_PROGRAMS =
EXTRA_DIST =

if HAVE_WINDOWS
include Makefile_Evil.am
endif

include Makefile_Eina.am
include Makefile_Eo.am
include Makefile_Eet.am
include Makefile_Evas.am
include Makefile_Embryo.am

.PHONY: benchmark examples

benchmark:
	@$(MAKE) $(AM_MAKEFLAGS)
	@$(MAKE) $(AM_MAKEFLAGS) -C benchmarks benchmark

examples:
	@make
	@$(MAKE) $(AM_MAKEFLAGS) -C examples examples

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C examples install-examples

clean-local:
	rm -rf lib/evil/*.gcno
	rm -rf lib/eina/*.gcno
	rm -rf lib/eo/*.gcno
	rm -rf lib/eet/*.gcno
	rm -rf lib/evas/*.gcno
	rm -rf lib/evas/cache/*.gcno
	rm -rf lib/evas/cache2/*.gcno
	rm -rf lib/evas/canvas/*.gcno
	rm -rf lib/evas/common/*.gcno
	rm -rf lib/evas/cserve2/*.gcno
	rm -rf lib/evas/file/*.gcno
	rm -rf lib/embryo/*.gcno
	rm -rf modules/eina/mp/pass_through/*.gcno
	rm -rf modules/eina/mp/one_big/*.gcno
	rm -rf modules/eina/mp/chained_pool/*.gcno

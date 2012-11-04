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
	@$(MAKE) $(AM_MAKEFLAGS) -C lib
	@$(MAKE) $(AM_MAKEFLAGS) -C benchmarks benchmark

examples:
	@make -C lib
	@$(MAKE) $(AM_MAKEFLAGS) -C examples examples

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C examples install-examples

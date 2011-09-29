## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
config.guess \
config.h.in \
config.h.in~ \
config.sub \
configure \
depcomp \
install-sh \
ltconfig \
ltmain.sh \
missing \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.gz \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc.tar.bz2 \
m4/libtool.m4 \
m4/lt~obsolete.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = eina.pc

EXTRA_DIST = \
AUTHORS \
COPYING \
README \
$(pkgconfig_DATA) \
autogen.sh \
eina.pc.in \
eina.spec \
eina.spec.in \
m4/efl_attribute.m4 \
m4/efl_benchmark.m4 \
m4/efl_compiler_flag.m4 \
m4/efl_coverage.m4 \
m4/efl_cpu.m4 \
m4/efl_doxygen.m4 \
m4/efl_fnmatch.m4 \
m4/efl_tests.m4 \
m4/efl_threads.m4 \
m4/eina_bench.m4 \
m4/eina_check.m4 \
m4/efl_path_max.m4

.PHONY: doc coverage benchmark

# Documentation

doc:
	@echo "entering doc/"
	@cd doc && make doc

# Unit tests

if EFL_ENABLE_TESTS

check-local:
	@./src/tests/eina_suite

else

check-local:
	@echo "reconfigure with --enable-tests"

endif

# Coverage report

if EFL_ENABLE_COVERAGE
lcov-reset:
	@rm -rf coverage
	@find . -name "*.gcda" -exec rm {} \;
	@lcov --directory . --zerocounters

lcov-report:
	@mkdir coverage
	@lcov --compat-libtool --directory $(top_srcdir)/src --capture --output-file coverage/coverage.info
	@lcov -l coverage/coverage.info |grep "\\.h"  |cut -d " " -f 2 > coverage/remove
	@lcov -r coverage/coverage.info `cat coverage/remove` > coverage/coverage.cleaned.info
	@rm coverage/remove
	@mv coverage/coverage.cleaned.info coverage/coverage.info
	@genhtml -t "$(PACKAGE_STRING)" -o coverage coverage/coverage.info

coverage:
	@make lcov-reset
	@make check
	@make lcov-report
else
lcov-reset:
	@echo "reconfigure with --enable-coverage"

lcov-report:
	@echo "reconfigure with --enable-coverage"

coverage:
	@echo "reconfigure with --enable-tests --enable-coverage"
endif

if EFL_ENABLE_BENCHMARK

benchmark:
	@cd src && make benchmark
	@mkdir result || true
	@cd result && ../src/tests/eina_bench `date +%F_%s`

else

benchmark:
	@echo "reconfigure with --enable-benchmark"
endif

clean-local:
	@rm -rf coverage benchmark

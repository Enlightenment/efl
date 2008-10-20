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
missing

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = eina-0.pc

EXTRA_DIST = \
AUTHORS \
COPYING \
OLD-COPYING.PLAIN \
README \
$(pkgconfig_DATA)

.PHONY: doc coverage benchmark

# Documentation

doc:
	@echo "entering doc/"
	make -C doc doc

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
	@lcov -l coverage/coverage.info | grep -v "`cd $(top_srcdir) && pwd`" | cut -d: -f1 > coverage/remove
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
	@make -C src benchmark
	@mkdir result || true
	@cd result && ../src/tests/eina_bench `date +%F_%s`

else

benchmark:
	@echo "reconfigure with --enable-bench"
endif

clean-local:
	@rm -rf coverage benchmark

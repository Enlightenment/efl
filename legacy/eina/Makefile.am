## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4 -I m4/common -I m4/eina

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
m4/common/efl_attribute.m4 \
m4/common/efl_benchmark.m4 \
m4/common/efl_compiler_flag.m4 \
m4/common/efl_coverage.m4 \
m4/common/efl_cpu.m4 \
m4/common/efl_doxygen.m4 \
m4/common/efl_fnmatch.m4 \
m4/common/efl_tests.m4 \
m4/common/efl_threads.m4 \
m4/common/efl_path_max.m4 \
m4/eina/eina_bench.m4 \
m4/eina/eina_check.m4

.PHONY: doc coverage benchmark

# Documentation

doc:
	@echo "entering doc/"
	@cd doc && $(MAKE) doc

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
	@rm -rf $(top_builddir)/coverage
	@find $(top_builddir) -name "*.gcda" -delete
	@lcov --zerocounters --directory $(top_builddir)

lcov-report:
	@mkdir $(top_builddir)/coverage
	lcov --capture --compat-libtool --output-file $(top_builddir)/coverage/coverage.info --directory $(top_builddir)
	lcov --remove $(top_builddir)/coverage/coverage.info '*.h' --output-file $(top_builddir)/coverage/coverage.cleaned.info
	genhtml -t "$(PACKAGE_STRING)" -o $(top_builddir)/coverage/html $(top_builddir)/coverage/coverage.cleaned.info
	@echo "Coverage Report at $(top_builddir)/coverage/html"

coverage:
	@$(MAKE) lcov-reset
	@$(MAKE) check
	@$(MAKE) lcov-report
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
	@cd src && $(MAKE) benchmark
	@mkdir result || true
	@cd result && ../src/tests/eina_bench `date +%F_%s`

else

benchmark:
	@echo "reconfigure with --enable-benchmark"
endif

clean-local:
	@rm -rf coverage benchmark

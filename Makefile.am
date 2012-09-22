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
ltmain.sh \
missing \
m4/libtool.m4 \
m4/lt~obsolete.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4

EXTRA_DIST =  \
m4/ac_path_generic.m4 \
m4/efl_attribute.m4 \
m4/efl_check_funcs.m4 \
m4/efl_compiler.m4 \
m4/efl_doxygen.m4 \
m4/efl_path_max.m4 \
m4/efl_threads.m4 \
m4/evil_windows.m4 \
m4/eina_check.m4

if HAVE_WINDOWS
EXTRA_DIST += \
ChangeLog.evil \
NEWS.evil \
README.evil \
evil.pc
endif

EXTRA_DIST += \
ChangeLog.eina \
NEWS.eina \
README.eina \
eina.spec \
eina.pc \
ChangeLog.eet \
NEWS.eet \
README.eet \
eet.spec \
eet.pc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =

if HAVE_WINDOWS
pkgconfig_DATA += evil.pc
endif

pkgconfig_DATA += eina.pc eet.pc


.PHONY: doc benchmark examples install-examples

# Documentation

doc:
	@echo "entering doc/"
	$(MAKE) $(AM_MAKEFLAGS) -C doc doc

# Unit tests

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

endif

if EFL_ENABLE_TESTS

check-local:
if EFL_ENABLE_COVERAGE
	@$(MAKE) $(AM_MAKEFLAGS) lcov-reset
endif
	@./src/tests/eina/eina_suite$(EXEEXT)
	@./src/tests/eet/eet_suite$(EXEEXT)
if EFL_ENABLE_COVERAGE
	@$(MAKE) $(AM_MAKEFLAGS) lcov-report
endif

endif

# benchmark

benchmark:
	@$(MAKE) $(AM_MAKEFLAGS) -C src benchmark
	@mkdir benchmark || true
	@cd benchmark && ../src/benchmarks/eina/eina_bench$(EXEEXT) `date +%F_%s`

benchmark-e17:
	@$(MAKE) $(AM_MAKEFLAGS) -C src benchmark-e17
	@mkdir benchmark || true
	@cd benchmark && ../src/benchmarks/eina/eina_bench$(EXEEXT) `date +%F_%s`

# examples

examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C src examples

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C src install-examples

# cleaning

clean-local:
	rm -rf benchmark coverage

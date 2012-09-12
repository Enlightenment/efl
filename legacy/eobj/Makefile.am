
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
config.cache-env \
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
pkgconfig_DATA = eo.pc

EXTRA_DIST = \
AUTHORS \
COPYING \
README \
autogen.sh \
eo.pc.in \
m4/efl_doxygen.m4

.PHONY: doc benchmark examples

# Documentation

doc:
	@echo "entering doc/"
	@cd doc && $(MAKE) doc

# Unit tests

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

check-local:
if EFL_ENABLE_COVERAGE
	@$(MAKE) lcov-reset
endif
	@dirs=`find src/tests -maxdepth 1 -type d \! -name '.*' -printf '%P\n' | sort` ; \
	for dir in $$dirs ; do \
	  ./src/tests/$$dir/$$dir ; \
	done
if EFL_ENABLE_COVERAGE
	@$(MAKE) lcov-report
endif

# benchmark

benchmark:
	@$(MAKE) -C src benchmark
	@mkdir benchmark || true
	@cd benchmark && ../src/benchmarks/eo_bench `date +%F_%s`

# examples

examples:
	@$(MAKE) -C src examples

clean-local:
	@rm -rf coverage benchmark

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
compile \
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
eeze*doc*tar* \
eeze.pc \
eeze.spec \
m4/l*

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = eeze.pc

EXTRA_DIST = \
AUTHORS \
COPYING \
README \
$(pkgconfig_DATA) \
autogen.sh \
eeze.pc.in \
eeze.spec.in \
eeze.spec \
m4/efl_doxygen.m4

.PHONY: doc

# Documentation

doc: all
	@echo "entering doc/"
	$(MAKE) -C doc doc

# Unit tests

if EFL_ENABLE_TESTS

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
	@$(MAKE) lcov-reset
	@./src/tests/eeze_suite
	@$(MAKE) lcov-report

else

lcov-reset:
	@echo "reconfigure with --enable-tests"

lcov-report:
	@echo "reconfigure with --enable-tests"

check-local:
	@echo "reconfigure with --enable-tests"

endif

clean-local:
	@rm -rf coverage

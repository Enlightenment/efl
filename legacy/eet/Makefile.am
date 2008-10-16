ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
compile \
config.guess \
config.h.in  \
config.sub  \
configure  \
depcomp \
install-sh \
ltmain.sh  \
missing  \
eet_docs.tar.gz \
m4/libtool.m4 \
m4/lt~obsolete.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4

EXTRA_DIST = \
AUTHORS \
COPYING \
COPYING-PLAIN \
autogen.sh \
eet.pc.in \
eet.spec.in \
eet.spec \
README.in \
README

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = eet.pc

.PHONY: doc coverage

# Documentation

doc:
	@echo "entering doc/"
	make -C doc doc

# Unit tests

if EFL_ENABLE_TESTS

check-local:
	@./src/tests/eet_suite

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
	@lcov --compat-libtool --directory . --capture --output-file coverage/coverage.info
	@lcov -l coverage/coverage.info | grep -v "`cd $(top_srcdir) && pwd`" | cut -d: -f1 > coverage/remove
	@lcov -r coverage/coverage.info `cat coverage/remove` > coverage/coverage.cleaned.info
	@rm coverage/remove
	@mv coverage/coverage.cleaned.info coverage/coverage.info
	@genhtml -t "$(PACKAGE_STRING)" -o coverage coverage/coverage.info

coverage:
	@make lcov-reset
	@make check
	@make lcov-report

clean-local:
	@rm -rf coverage
else
lcov-reset:
	@echo "reconfigure with --enable-gcov"

lcov-report:
	@echo "reconfigure with --enable-gcov"

coverage:
	@echo "reconfigure with --enable-tests --enable-gcov"
endif

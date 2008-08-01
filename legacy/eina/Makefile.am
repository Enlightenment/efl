## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
config.guess \
config.h.in \
config.sub \
configure \
install-sh \
ltconfig \
ltmain.sh \
missing mkinstalldirs \
stamp-h.in \
build-stamp \
configure-stamp \
depcomp \
eina.pc \
eina_docs.tar.gz

EXTRA_DIST = \
AUTHORS \
COPYING \
gendoc \
Doxyfile \
README \
eina.pc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = eina.pc

# Unit tests

if EINA_ENABLE_TESTS

check-local:
	@./src/tests/eina_suite

else

check-local:
	@echo "reconfigure with --enable-tests"

endif

# Coverage report

if EINA_ENABLE_COVERAGE
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
	@echo "reconfigure with --enable-coverage"

lcov-report:
	@echo "reconfigure with --enable-coverage"

coverage:
	@echo "reconfigure with --enable-tests --enable-coverage"
endif

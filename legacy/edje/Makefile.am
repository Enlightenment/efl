ACLOCAL_AMFLAGS = -I m4

SUBDIRS = data src utils doc

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
ltmain.sh \
missing \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.gz \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc.tar.bz2 \
m4/efl_binary.m4 \
m4/efl_coverage.m4 \
m4/efl_doxygen.m4 \
m4/efl_path_max.m4 \
m4/efl_tests.m4 \
m4/libtool.m4 \
m4/lt~obsolete.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4

### this is all fine and great - but edje should not be going and installing
### data in another software's data dir - it's just wrong. not to mention this
### also breaks make distcheck...
#	if test "x${vimdir}" != "x"; then \
#	  $(mkinstalldirs) $(DESTDIR)$(vimdir)/syntax; \
#	  cp -p $(srcdir)/data/edc.vim $(DESTDIR)$(vimdir)/syntax/; \
#	fi

bin_SCRIPTS = 

EXTRA_DIST = \
AUTHORS \
COPYING \
autogen.sh \
edje.pc.in \
edje.spec \
edje.spec.in \
README

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = edje.pc

.PHONY: doc coverage

# Documentation

doc:
	@echo "entering doc/"
	make -C doc doc

# Unit tests

if EFL_ENABLE_TESTS

check-local:
	@./src/tests/edje_suite

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

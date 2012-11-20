ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
config.guess \
config.h.in \
config.h.in~ \
config.rpath \
config.sub \
configure \
depcomp \
install-sh \
ltmain.sh \
missing \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.gz \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc.tar.bz2 \
m4/libtool.m4 \
m4/lt~obsolete.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4 \
m4/codeset.m4 \
m4/gettext.m4* \
m4/glibc2*.m4 \
m4/iconv.m4 \
m4/intdiv0.m4 \
m4/intldir.m4 \
m4/intl.m4 \
m4/intlmacosx.m4 \
m4/intmax.m4* \
m4/inttypes_h.m4 \
m4/inttypes.m4 \
m4/inttypes-pri.m4 \
m4/isc-posix.m4 \
m4/lcmessage.m4 \
m4/lib-ld.m4* \
m4/lib-link.m4 \
m4/lib-prefix.m4* \
m4/lock.m4 \
m4/longdouble.m4* \
m4/longlong.m4* \
m4/nls.m4 \
m4/po.m4* \
m4/printf-posix.m4* \
m4/progtest.m4 \
m4/signed.m4 \
m4/size_max.m4* \
m4/stdint_h.m4 \
m4/uintmax_t.m4 \
m4/ulonglong.m4* \
m4/visibility.m4 \
m4/wchar_t.m4 \
m4/wint_t.m4* \
m4/xsize.m4*

if HAVE_PO
SUBDIRS += po

MAINTAINERCLEANFILES += \
po/boldquot.sed \
po/en@boldquot.header \
po/en@quot.header \
po/insert-header.sin \
po/Makefile.in.in* \
po/Makevars.template \
po/quot.sed \
po/remove-potcdate.sin \
po/Rules-quot*

endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = efreet.pc efreet-mime.pc efreet-trash.pc

servicedir = @dbusservicedir@
service_DATA = org.enlightenment.Efreet.service

EXTRA_DIST = \
AUTHORS \
COPYING \
autogen.sh \
efreet.pc.in \
efreet-mime.pc.in \
efreet.spec.in \
efreet.spec \
m4/efl_coverage.m4\
m4/efl_tests.m4\
README

.PHONY: doc coverage

doc:
	@echo "entering doc/"
	make -C doc doc

# Unit tests

if EFL_ENABLE_TESTS

check-local:
	@./src/tests/efreet_suite

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

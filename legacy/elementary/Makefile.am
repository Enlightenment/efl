SUBDIRS = src data config doc

if HAVE_PO

SUBDIRS += po

endif

ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = \
ABOUT-NLS \
Makefile.in \
aclocal.m4 \
config.guess \
elementary_config.h.in* \
config.sub \
config.rpath \
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
m4/gettext.m4 \
m4/iconv.m4 \
m4/lcmessage.m4 \
m4/lib-ld.m4 \
m4/lib-link.m4 \
m4/lib-prefix.m4 \
m4/nls.m4 \
m4/po.m4 \
m4/progtest.m4

if HAVE_PO

MAINTAINERCLEANFILES += \
po/boldquot.sed \
po/en@boldquot.header \
po/en@quot.header \
po/insert-header.sin \
po/Makefile.in.in \
po/Makevars.template \
po/quot.sed \
po/remove-potcdate.sin \
po/Rules-quot

endif

EXTRA_DIST = \
README \
AUTHORS \
COPYING \
COPYING.images \
COMPLIANCE \
elementary.pc.in \
elementary-cxx.pc.in \
elementary.spec \
elementary.spec.in \
m4/efl_binary.m4 \
m4/efl_doxygen.m4 \
m4/elm_check_backend.m4 \
config.rpath \
pkgbuild/PKGBUILD

sed_process = \
$(SED) \
-e 's,@VERSION\@,$(VERSION),g' \
-e 's,@VMAJ\@,$(VMAJ),g' \
-e 's,@prefix\@,$(prefix),g' \
-e 's,@exec_prefix\@,$(exec_prefix),g' \
-e 's,@libdir\@,$(libdir),g' \
-e 's,@includedir\@,$(includedir),g' \
-e 's,@datarootdir\@,$(datarootdir),g' \
-e 's,@datadir\@,$(datadir),g' \
-e 's,@pkgdatadir\@,$(pkgdatadir),g' \
-e 's,@requirement_elm_pc\@,$(requirement_elm_pc),g' \
-e 's,@ELEMENTARY_LIBS\@,$(ELEMENTARY_LIBS),g' \
-e 's,@ELEMENTARY_PC_LIBS\@,$(ELEMENTARY_PC_LIBS),g' \
< $< > $@ || rm $@

pc_verbose = $(pc_verbose_@AM_V@)
pc_verbose_ = $(pc_verbose_@AM_DEFAULT_V@)
pc_verbose_0 = @echo "  PC      " $@;

elementary.pc: $(top_srcdir)/elementary.pc.in Makefile
	$(AM_V_at)rm -f $@
	$(pc_verbose)$(sed_process)

elementary-cxx.pc: $(top_srcdir)/elementary-cxx.pc.in Makefile
	$(AM_V_at)rm -f $@
	$(pc_verbose)$(sed_process)

if HAVE_FREEBSD
pkgconfigdir = $(libdir)data/pkgconfig
else
pkgconfigdir = $(libdir)/pkgconfig
endif

pkgconfig_DATA = elementary.pc elementary-cxx.pc

CLEANFILES = elementary.pc elementary-cxx.pc

cmakeconfigdir = $(libdir)/cmake/Elementary
cmakeconfig_DATA = \
cmakeconfig/ElementaryConfig.cmake \
cmakeconfig/ElementaryConfigVersion.cmake

.PHONY: doc screenshots

doc:
	@echo "entering doc/"
	$(MAKE) -C doc doc

screenshots: all
	@echo "entering src/examples/"
	$(MAKE) -C src/examples screenshots

if ELEMENTARY_ENABLE_COVERAGE

lcov-reset:
	@rm -rf $(top_builddir)/coverage
	@find $(top_builddir) -name "*.gcda" -delete
	@lcov --zerocounters --directory $(top_builddir)

lcov-report:
	@mkdir $(top_builddir)/coverage
	lcov --capture --compat-libtool --no-external --output-file $(top_builddir)/coverage/coverage.info --directory $(top_builddir) --base-directory $(top_srcdir)/src/lib/
	lcov --remove $(top_builddir)/coverage/coverage.info '*.h' --output-file $(top_builddir)/coverage/coverage.cleaned.info
	lcov --remove $(top_builddir)/coverage/coverage.cleaned.info '*/elm_test_*' --output-file $(top_builddir)/coverage/coverage.cleaned2.info
	lcov --remove $(top_builddir)/coverage/coverage.cleaned2.info '*/elm_suite.c' --output-file $(top_builddir)/coverage/coverage.cleaned3.info
	lcov --remove $(top_builddir)/coverage/coverage.cleaned3.info '*NONE*' --output-file $(top_builddir)/coverage/coverage.cleaned4.info
	genhtml --branch-coverage -t "$(PACKAGE_STRING)" -o $(top_builddir)/coverage/html $(top_builddir)/coverage/coverage.cleaned4.info
	@echo "Coverage Report at $(top_builddir)/coverage/html"

endif

if ELEMENTARY_ENABLE_TESTS

lcov-check:
if ELEMENTARY_ENABLE_COVERAGE
	@$(MAKE) $(AM_MAKEFLAGS) lcov-reset
endif
	@$(MAKE) $(AM_MAKEFLAGS) check
if ELEMENTARY_ENABLE_COVERAGE
	@$(MAKE) $(AM_MAKEFLAGS) lcov-report
endif
endif

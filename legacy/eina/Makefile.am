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
m4/ac_attribute.m4 \
m4/efl_benchmark.m4 \
m4/efl_compiler_flag.m4 \
m4/efl_cpu.m4 \
m4/efl_doxygen.m4 \
m4/efl_fnmatch.m4 \
m4/efl_threads.m4 \
m4/eina_bench.m4 \
m4/eina_check.m4 \
m4/efl_path_max.m4

.PHONY: doc

# Documentation

doc:
	@echo "entering doc/"
	@cd doc && make doc

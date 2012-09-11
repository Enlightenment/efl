ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

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
m4/efl_attribute.m4 \
m4/efl_compiler.m4 \
m4/efl_doxygen.m4 \
m4/evil_windows.m4

if HAVE_WINDOWS
EXTRA_DIST += evil.pc
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =

if HAVE_WINDOWS
pkgconfig_DATA += evil.pc
endif


.PHONY: doc

# Documentation

doc:
	@echo "entering doc/"
	make -C doc doc

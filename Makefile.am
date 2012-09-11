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


.PHONY: doc

# Documentation

doc:
	@echo "entering doc/"
	make -C doc doc

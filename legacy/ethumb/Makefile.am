MAINTAINERCLEANFILES = \
	Makefile.in \
	aclocal.m4 \
	compile \
	config.guess \
	config.h.in \
	config.sub \
	configure \
	depcomp \
	install-sh \
	ltmain.sh \
	missing \
	$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.gz \
	$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 \
	$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc.tar.bz2

EXTRA_DIST = \
autogen.sh  \
ethumb.spec

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ethumb.pc


if USE_MODULE_ETHUMBD

pkgconfig_DATA += ethumb_client.pc
servicedir = $(dbusservicedir)
service_DATA = org.enlightenment.Ethumb.service

endif

SUBDIRS = m4 src data doc

ACLOCAL_AMFLAGS = -I m4

.PHONY: doc

doc:
	@echo "entering doc/"
	$(MAKE) -C doc doc

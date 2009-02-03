ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
			config.h.in config.sub configure install-sh \
			ltconfig ltmain.sh missing mkinstalldirs \
			stamp-h.in depcomp \
			$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.gz \
			$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 \
			$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc.tar.bz2 \
			m4/libtool.m4 \
			m4/lt~obsolete.m4 \
			m4/ltoptions.m4 \
			m4/ltsugar.m4 \
			m4/ltversion.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = efreet.pc efreet-mime.pc efreet-trash.pc

EXTRA_DIST = \
AUTHORS \
COPYING \
autogen.sh \
efreet.pc.in \
efreet-mime.pc.in \
efreet.spec.in \
efreet.spec \
README

.PHONY: doc

doc:
	@echo "entering doc/"
	make -C doc doc

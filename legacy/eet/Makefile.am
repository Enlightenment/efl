SUBDIRS = src

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in build-stamp configure-stamp depcomp \
		       eet_docs.tar.gz \
		       README \
		       eet.pc \
		       eet.spec \
		       debian/changelog

bin_SCRIPTS = eet-config

EXTRA_DIST = AUTHORS COPYING COPYING-PLAIN eet.c.in \
	     README.in README \
	     eet.pc.in \
	     eet.spec.in eet.spec \
	     debian/changelog \
	     debian/changelog.in \
	     debian/control \
	     debian/copyright \
	     debian/eet-config.1 \
	     debian/eet.1 \
	     debian/libeet0-dev.install \
	     debian/libeet0.install \
	     debian/eet0-debug.install \
	     debian/rules

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = eet.pc


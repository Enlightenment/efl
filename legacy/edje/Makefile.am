## Process this file with automake to produce Makefile.in

SUBDIRS = data src

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in edje_docs.tar edje_docs.tar.gz edje.c \
		       debian/changelog

### this is all fine and great - ut edje should not be going and installing
### data in another software's data dir - it's just wrong. not to mention this
### also breaks make distcheck...
#	if test "x${vimdir}" != "x"; then \
#	  $(mkinstalldirs) $(DESTDIR)$(vimdir)/syntax; \
#	  cp -p $(srcdir)/data/edc.vim $(DESTDIR)$(vimdir)/syntax/; \
#	fi

bin_SCRIPTS = edje-config

EXTRA_DIST = README AUTHORS COPYING COPYING-PLAIN edje.spec \
	edje.pc.in edje.c.in gendoc Doxyfile edje_docs.tar.gz \
	debian/changelog debian/changelog.in debian/edje.files \
	debian/control debian/rules debian/copyright \
	debian/libedje0.install debian/edje-bin.install \
	libedje0-dev.install

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = edje.pc

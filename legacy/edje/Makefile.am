SUBDIRS = data src

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in edje_docs.tar edje_docs.tar.gz edje.c \
		       debian/changelog edje.spec

### this is all fine and great - but edje should not be going and installing
### data in another software's data dir - it's just wrong. not to mention this
### also breaks make distcheck...
#	if test "x${vimdir}" != "x"; then \
#	  $(mkinstalldirs) $(DESTDIR)$(vimdir)/syntax; \
#	  cp -p $(srcdir)/data/edc.vim $(DESTDIR)$(vimdir)/syntax/; \
#	fi

bin_SCRIPTS = edje-config

EXTRA_DIST = README AUTHORS COPYING COPYING-PLAIN edje.spec \
	edje.spec.in edje.pc.in edje.c.in gendoc Doxyfile \
	debian/changelog \
	debian/changelog.in \
	debian/control debian/rules debian/copyright \
	debian/libedje0.install debian/edje0-bin.install \
	debian/libedje0-dev.install edje0-debug.install

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = edje.pc

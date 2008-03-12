SUBDIRS = data src

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in edje_docs.tar edje_docs.tar.gz edje.c \
		       edje.spec depcomp compile edje.pc

### this is all fine and great - but edje should not be going and installing
### data in another software's data dir - it's just wrong. not to mention this
### also breaks make distcheck...
#	if test "x${vimdir}" != "x"; then \
#	  $(mkinstalldirs) $(DESTDIR)$(vimdir)/syntax; \
#	  cp -p $(srcdir)/data/edc.vim $(DESTDIR)$(vimdir)/syntax/; \
#	fi

bin_SCRIPTS = 

EXTRA_DIST = README AUTHORS COPYING COPYING-PLAIN autogen.sh edje.spec \
	edje.spec.in edje.pc.in edje.c.in Doxyfile \
	doc gendoc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = edje.pc

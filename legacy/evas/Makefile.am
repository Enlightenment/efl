# ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in build-stamp configure-stamp depcomp \
		       evas_docs.tar.gz evas.c \
		       README \
		       evas.pc \
		       evas.spec \
		       debian/changelog

bin_SCRIPTS = evas-config

EXTRA_DIST = AUTHORS COPYING COPYING-PLAIN evas.c.in gendoc \
             Doxyfile \
	     README.in README \
	     evas.pc.in \
	     evas.spec.in evas.spec \
	     debian/changelog \
	     debian/changelog.in \
	     debian/compat \
	     debian/control \
	     debian/copyright \
	     debian/libevas0-all.install \
	     debian/libevas0-dev.install \
	     debian/libevas0-engine-buffer.install \
	     debian/libevas0-engine-fb.install \
	     debian/libevas0-engine-software-generic.install \
	     debian/libevas0-engine-software-x11.install \
	     debian/libevas0-engine-xrender.install \
	     debian/libevas0-engines-all.install \
	     debian/libevas0-loader-eet.install \
	     debian/libevas0-loader-gif.install \
	     debian/libevas0-loader-jpeg.install \
	     debian/libevas0-loader-png.install \
	     debian/libevas0-loader-svg.install \
	     debian/libevas0-loader-tiff.install \
	     debian/libevas0-loader-xpm.install \
	     debian/libevas0-loaders-all.install \
	     debian/libevas0-saver-eet.install \
	     debian/libevas0-saver-jpeg.install \
	     debian/libevas0-saver-png.install \
	     debian/libevas0-saver-tiff.install \
	     debian/libevas0-savers-all.install \
	     debian/libevas0.install \
	     debian/rules

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = evas.pc

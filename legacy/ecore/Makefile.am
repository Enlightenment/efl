## Process this file with automake to produce Makefile.in

SUBDIRS = data src examples

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in  build-stamp configure-stamp depcomp \
		       ecore_docs.tar.gz ecore.c \
		       README \
		       ecore.pc \
		       ecore.spec \
		       debian/changelog

bin_SCRIPTS = ecore-config

EXTRA_DIST = AUTHORS COPYING COPYING-PLAIN ecore.c.in gendoc ecore.supp ecore.m4 \
             Doxyfile \
	     ecore.pc.in \
	     ecore.pc.in \
	     ecore.spec.in ecore.spec \
	     debian/changelog \
	     debian/changelog.in \
	     debian/control \
	     debian/copyright \
	     debian/ecore0-bin.install \
	     debian/ecore0-debug.install \
	     debian/libecore0-con.install \
	     debian/libecore0-config.install \
	     debian/libecore0-dbus.install \
	     debian/libecore0-desktop.install \
	     debian/libecore0-dev.install \
	     debian/libecore0-evas.install \
	     debian/libecore0-fb.install \
	     debian/libecore0-file.install \
	     debian/libecore0-ipc.install \
	     debian/libecore0-job.install \
	     debian/libecore0-txt.install \
	     debian/libecore0-x.install \
	     debian/libecore0.install \
	     debian/rules

m4datadir = $(datadir)/aclocal
m4data_DATA = ecore.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ecore.pc

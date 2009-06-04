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
	missing

EXTRA_DIST = config.rpath \
	m4/Makefile.in \
	README \
	AUTHORS \
	COPYING

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	ethumb.pc


if USE_MODULE_ETHUMBD

pkgconfig_DATA += ethumb_client.pc
servicedir = $(dbusservicedir)
service_DATA = org.enlightenment.Ethumb.service

endif

SUBDIRS = m4 src data

ACLOCAL_AMFLAGS = -I m4

## Process this file with automake to produce Makefile.in

SUBDIRS = src

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in  build-stamp configure-stamp depcomp \
		       ecore_docs.tar.gz ecore.c \
		       README \
		       ecore.spec \
		       debian/changelog \
		       ecore-con.pc \
		       ecore-config.pc \
		       ecore-dbus.pc \
		       ecore-desktop.pc \
		       ecore-directfb.pc\
		       ecore-evas.pc \
		       ecore-fb.pc \
		       ecore-file.pc \
		       ecore-ipc.pc \
		       ecore-job.pc \
		       ecore-txt.pc \
		       ecore-x.pc \
		       ecore.pc

bin_SCRIPTS = ecore-config

EXTRA_DIST = AUTHORS COPYING COPYING-PLAIN ecore.c.in gendoc ecore.supp \
             Doxyfile \
	     ecore.pc.in \
	     ecore-con.pc.in \
	     ecore-config.pc.in \
	     ecore-dbus.pc.in \
	     ecore-desktop.pc.in \
	     ecore-directfb.pc.in\
	     ecore-evas.pc.in \
	     ecore-fb.pc.in \
	     ecore-file.pc.in \
	     ecore-ipc.pc.in \
	     ecore-job.pc.in \
	     ecore-txt.pc.in \
	     ecore-x.pc.in \
	     ecore.spec.in ecore.spec \
	     debian/changelog \
	     debian/changelog.in \
	     debian/control \
	     debian/copyright \
	     debian/ecore0-bin.install \
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

if BUILD_ECORE_CON
pcon = ecore-con.pc
endif

if BUILD_ECORE_CONFIG
pconfig = ecore-config.pc
endif

if BUILD_ECORE_DBUS
pdbus = ecore-dbus.pc
endif

if BUILD_ECORE_DIRECTFB
pdfb = ecore-directfb.pc
endif

if BUILD_ECORE_EVAS
pevas = ecore-evas.pc
endif

if BUILD_ECORE_FB
pfb = ecore-fb.pc
endif

if BUILD_ECORE_FILE
pfile = ecore-file.pc
endif

if BUILD_ECORE_DESKTOP
pdesktop = ecore-desktop.pc
endif

if BUILD_ECORE_IPC
pipc = ecore-ipc.pc
endif

if BUILD_ECORE_JOB
pjob = ecore-job.pc
endif

if BUILD_ECORE_TXT
ptxt = ecore-txt.pc
endif

if BUILD_ECORE_X
px = ecore-x.pc
endif

if BUILD_ECORE_X_XCB
px = ecore-x.pc
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	ecore.pc $(pcon) $(pconfig) $(pdbus) $(pdfb) $(pevas) \
	$(pfb) $(pfile) $(pdesktop) $(pipc) $(pjob) $(ptxt) $(px)

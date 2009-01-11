## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = doc src

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in  build-stamp configure-stamp depcomp \
		       README \
		       ecore.spec \
		       ecore-con.pc \
		       ecore-config.pc \
		       ecore-directfb.pc\
		       ecore-evas.pc \
		       ecore-fb.pc \
		       ecore-file.pc \
		       ecore-imf.pc \
		       ecore-imf-evas.pc \
		       ecore-ipc.pc \
		       ecore-job.pc \
		       ecore-txt.pc \
		       ecore-x.pc \
		       ecore-win32.pc \
		       ecore-sdl.pc \
		       ecore-quartz.pc \
		       ecore-wince.pc \
		       ecore.pc \
		       $(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.gz \
		       $(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 \
		       $(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc.tar.bz2 \
		       m4/libtool.m4 \
		       m4/lt~obsolete.m4 \
		       m4/ltoptions.m4 \
		       m4/ltsugar.m4 \
		       m4/ltversion.m4

bin_SCRIPTS = 

EXTRA_DIST = AUTHORS COPYING COPYING-PLAIN \
	     autogen.sh ecore.supp \
	     ecore.pc.in \
	     ecore-con.pc.in \
	     ecore-config.pc.in \
	     ecore-directfb.pc.in\
	     ecore-evas.pc.in \
	     ecore-fb.pc.in \
	     ecore-file.pc.in \
	     ecore-imf.pc.in \
	     ecore-imf-evas.pc.in \
	     ecore-ipc.pc.in \
	     ecore-job.pc.in \
	     ecore-txt.pc.in \
	     ecore-x.pc.in \
	     ecore-win32.pc.in \
	     ecore-sdl.pc.in \
	     ecore-quartz.pc.in \
	     ecore-wince.pc.in \
	     ecore.spec.in ecore.spec

if BUILD_ECORE_CON
pcon = ecore-con.pc
endif

if BUILD_ECORE_CONFIG
pconfig = ecore-config.pc
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

if BUILD_ECORE_IMF
pimf = ecore-imf.pc
endif

if BUILD_ECORE_IMF_EVAS
pimfevas = ecore-imf-evas.pc
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

if BUILD_ECORE_WIN32
pwin32 = ecore-win32.pc
endif

if BUILD_ECORE_WINCE
pwince = ecore-wince.pc
endif

if BUILD_ECORE_SDL
psdl = ecore-sdl.pc
endif

if BUILD_ECORE_QUARTZ
pquartz = ecore-quartz.pc
endif

if BUILD_ECORE_X_XCB
px = ecore-x.pc
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	ecore.pc $(pcon) $(pconfig) $(pdfb) $(pevas) \
	$(pfb) $(pfile) $(pimf) $(pimfevas) $(pipc) $(pjob) $(ptxt) \
	$(px) $(pwin32) $(pwince) $(psdl) $(pquartz)

.PHONY: doc

doc:
	@echo "entering doc/"
	make -C doc doc


# ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in build-stamp configure-stamp depcomp \
		       evas_docs.tar.gz evas.c \
		       README \
		       evas-cairo-x11.pc \
		       evas-directfb.pc \
		       evas-framebuffer.pc \
		       evas-glitz-x11.pc \
		       evas-opengl-x11.pc \
		       evas-software-buffer.pc \
		       evas-software-qtopia.pc \
		       evas-software-x11.pc \
		       evas-software-xcb.pc \
		       evas-xrender-x11.pc \
		       evas-xrender-xcb.pc \
		       evas.pc \
		       evas.spec \
		       debian/changelog

bin_SCRIPTS = 

EXTRA_DIST = AUTHORS COPYING COPYING-PLAIN evas.c.in gendoc \
             Doxyfile \
	     README.in README \
	     evas.pc.in \
	     evas-cairo-x11.pc \
	     evas-directfb.pc \
	     evas-framebuffer.pc \
	     evas-glitz-x11.pc \
	     evas-opengl-x11.pc \
	     evas-software-buffer.pc \
	     evas-software-qtopia.pc \
	     evas-software-x11.pc \
	     evas-software-xcb.pc \
	     evas-xrender-x11.pc \
	     evas-xrender-xcb.pc \
	     evas.pc \
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

if BUILD_ENGINE_SOFTWARE_X11
psoftwarex11 = evas-software-x11.pc
endif

if BUILD_ENGINE_SOFTWARE_XCB
psoftwarexcb = evas-software-xcb.pc
endif

if BUILD_ENGINE_DIRECTFB
pdirectfb = evas-directfb.pc
endif

if BUILD_ENGINE_FB
pframebuffer = evas-framebuffer.pc
endif

if BUILD_ENGINE_GLITZ_X11
pglitzx11 = evas-glitz-x11.pc
endif

if BUILD_ENGINE_BUFFER
psoftwarebuffer = evas-software-buffer.pc
endif

if BUILD_ENGINE_SOFTWARE_QTOPIA
psoftwareqtopia = evas-software-qtopia.pc
endif

if BUILD_ENGINE_GL_X11
popenglx11 = evas-opengl-x11.pc
endif

if BUILD_ENGINE_CAIRO_X11
pcairox11 = evas-cairo-x11.pc
endif

if BUILD_ENGINE_XRENDER_X11
pxrenderx11 = evas-xrender-x11.pc
endif

if BUILD_ENGINE_XRENDER_XCB
pxrenderxcb = evas-xrender-xcb.pc
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	evas.pc $(psoftwarex11) $(psoftwarexcb) $(pdirectfb) $(pframebuffer) \
	$(psoftwarebuffer) $(psoftwareqtopia) $(popenglx11) $(pcairox11) \
	$(pxrenderx11) $(pxrenderxcb) $(pglitzx11)

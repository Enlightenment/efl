ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
config.guess \
config.h.in \
config.sub \
configure \
install-sh \
ltconfig \
ltmain.sh \
missing \
mkinstalldirs \
stamp-h.in \
build-stamp \
configure-stamp \
depcomp \
README \
evas-cairo-x11.pc \
evas-directfb.pc \
evas-fb.pc \
evas-opengl-x11.pc \
evas-opengl-glew.pc \
evas-quartz.pc \
evas-software-buffer.pc \
evas-software-qtopia.pc \
evas-software-x11.pc \
evas-software-16-x11.pc \
evas-xrender-x11.pc \
evas-xrender-xcb.pc \
evas-software-gdi.pc \
evas-software-ddraw.pc \
evas-software-16-ddraw.pc \
evas-direct3d.pc \
evas-software-16-wince.pc \
evas-software-sdl.pc \
evas.pc \
evas.spec \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.gz \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc.tar.bz2 \
m4/libtool.m4 \
m4/lt~obsolete.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4

bin_SCRIPTS =

EXTRA_DIST = \
AUTHORS \
COPYING \
COPYING-PLAIN \
autogen.sh \
README.in \
README \
evas.pc.in \
evas-cairo-x11.pc.in \
evas-directfb.pc.in \
evas-fb.pc.in \
evas-opengl-x11.pc.in \
evas-opengl-glew.pc.in \
evas-software-buffer.pc.in \
evas-software-qtopia.pc.in \
evas-software-x11.pc.in \
evas-software-16-x11.pc.in \
evas-xrender-x11.pc.in \
evas-xrender-xcb.pc.in \
evas-software-gdi.pc.in \
evas-software-ddraw.pc.in \
evas-software-16-ddraw.pc.in \
evas-direct3d.pc.in \
evas-software-16-wince.pc.in \
evas-software-sdl.pc.in \
evas.spec.in evas.spec

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = evas.pc

if BUILD_ENGINE_SOFTWARE_X11
pkgconfig_DATA += evas-software-x11.pc
endif

if BUILD_ENGINE_SOFTWARE_16_X11
pkgconfig_DATA += evas-software-16-x11.pc
endif

if BUILD_ENGINE_DIRECTFB
pkgconfig_DATA += evas-directfb.pc
endif

if BUILD_ENGINE_FB
pkgconfig_DATA += evas-fb.pc
endif

if BUILD_ENGINE_BUFFER
pkgconfig_DATA += evas-software-buffer.pc
endif

#if BUILD_ENGINE_SOFTWARE_QTOPIA
#pkgconfig_DATA += evas-software-qtopia.pc
#endif

if BUILD_ENGINE_GL_X11
pkgconfig_DATA += evas-opengl-x11.pc
endif

if BUILD_ENGINE_QUARTZ
pkgconfig_DATA += evas-quartz.pc
endif

if BUILD_ENGINE_GL_GLEW
pkgconfig_DATA += evas-opengl-glew.pc
endif

#if BUILD_ENGINE_CAIRO_X11
#pkgconfig_DATA += evas-cairo-x11.pc
#endif

if BUILD_ENGINE_XRENDER_X11
pkgconfig_DATA += evas-xrender-x11.pc
endif

if BUILD_ENGINE_XRENDER_XCB
pkgconfig_DATA += evas-xrender-xcb.pc
endif

if BUILD_ENGINE_SOFTWARE_GDI
pkgconfig_DATA += evas-software-gdi.pc
endif

if BUILD_ENGINE_SOFTWARE_DDRAW
pkgconfig_DATA += evas-software-ddraw.pc
endif

if BUILD_ENGINE_SOFTWARE_16_DDRAW
pkgconfig_DATA += evas-software-16-ddraw.pc
endif

if BUILD_ENGINE_DIRECT3D
pkgconfig_DATA += evas-direct3d.pc
endif

if BUILD_ENGINE_SOFTWARE_16_WINCE
pkgconfig_DATA += evas-software-16-wince.pc
endif

if BUILD_ENGINE_SOFTWARE_SDL
pkgconfig_DATA += evas-software-sdl.pc
endif

.PHONY: doc

doc:
	@echo "entering doc/"
	$(MAKE) -C doc doc

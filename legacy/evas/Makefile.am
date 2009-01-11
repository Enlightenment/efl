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
evas-glitz-x11.pc \
evas-opengl-x11.pc \
evas-opengl-glew.pc \
evas-quartz.pc \
evas-software-buffer.pc \
evas-software-qtopia.pc \
evas-software-x11.pc \
evas-software-16-x11.pc \
evas-software-xcb.pc \
evas-xrender-x11.pc \
evas-xrender-xcb.pc \
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
evas-glitz-x11.pc.in \
evas-opengl-x11.pc.in \
evas-opengl-glew.pc.in \
evas-software-buffer.pc.in \
evas-software-qtopia.pc.in \
evas-software-x11.pc.in \
evas-software-16-x11.pc.in \
evas-software-xcb.pc.in \
evas-xrender-x11.pc.in \
evas-xrender-xcb.pc.in \
evas-software-ddraw.pc.in \
evas-software-16-ddraw.pc.in \
evas-direct3d.pc.in \
evas-software-16-wince.pc.in \
evas-software-sdl.pc.in \
evas.spec.in evas.spec

if BUILD_ENGINE_SOFTWARE_X11
psoftwarex11 = evas-software-x11.pc
endif

if BUILD_ENGINE_SOFTWARE_16_X11
psoftware16x11 = evas-software-16-x11.pc
endif

if BUILD_ENGINE_SOFTWARE_XCB
psoftwarexcb = evas-software-xcb.pc
endif

if BUILD_ENGINE_DIRECTFB
pdirectfb = evas-directfb.pc
endif

if BUILD_ENGINE_FB
pfb = evas-fb.pc
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

if BUILD_ENGINE_QUARTZ
pquartz = evas-quartz.pc
endif

if BUILD_ENGINE_GL_GLEW
popenglglew = evas-opengl-glew.pc
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

if BUILD_ENGINE_SOFTWARE_DDRAW
psoftwareddraw = evas-software-ddraw.pc
endif

if BUILD_ENGINE_SOFTWARE_16_DDRAW
psoftware16ddraw = evas-software-16-ddraw.pc
endif

if BUILD_ENGINE_DIRECT3D
pdirect3d = evas-direct3d.pc
endif

if BUILD_ENGINE_SOFTWARE_16_WINCE
psoftware16wince = evas-software-16-wince.pc
endif

if BUILD_ENGINE_SOFTWARE_SDL
psoftwaresdl = evas-software-sdl.pc
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	evas.pc $(psoftwarex11) $(psoftwarexcb) $(pdirectfb) $(pfb) \
	$(psoftwarebuffer) $(psoftwareqtopia) $(popenglx11) $(pquartz) $(pcairox11) \
	$(pxrenderx11) $(pxrenderxcb) $(pglitzx11) $(psoftwareddraw) $(psoftwaresdl) \
	$(psoftware16x11) $(pdirect3d) $(psoftware16ddraw) $(psoftware16wince) $(popenglglew)

.PHONY: doc

doc:
	@echo "entering doc/"
	$(MAKE) -C doc doc

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
config.guess \
config.h.in \
config.h.in~ \
config.sub \
configure \
depcomp \
install-sh \
ltmain.sh \
missing \
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
autogen.sh \
evas.pc.in \
evas-directfb.pc.in \
evas-fb.pc.in \
evas-opengl-x11.pc.in \
evas-opengl-sdl.pc.in \
evas-software-buffer.pc.in \
evas-software-x11.pc.in \
evas-software-16-x11.pc.in \
evas-software-8-x11.pc.in \
evas-software-gdi.pc.in \
evas-software-ddraw.pc.in \
evas-software-16-ddraw.pc.in \
evas-direct3d.pc.in \
evas-software-16-wince.pc.in \
evas-psl1ght.pc.in \
evas-wayland-shm.pc.in \
evas-wayland-egl.pc.in \
evas.spec.in \
evas.spec \
m4/efl_attribute.m4 \
m4/efl_coverage.m4 \
m4/efl_doxygen.m4 \
m4/efl_fnmatch.m4 \
m4/efl_path_max.m4 \
m4/efl_pthread.m4 \
m4/efl_shm_open.m4 \
m4/efl_tests.m4 \
m4/evas_check_engine.m4 \
m4/evas_check_loader.m4 \
m4/evas_converter.m4 \
m4/evas_dither.m4 \
m4/evas_scaler.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = evas.pc

if BUILD_ENGINE_SOFTWARE_X11
pkgconfig_DATA += evas-software-x11.pc
endif

if BUILD_ENGINE_SOFTWARE_16_X11
pkgconfig_DATA += evas-software-16-x11.pc
endif

if BUILD_ENGINE_SOFTWARE_8_X11
pkgconfig_DATA += evas-software-8-x11.pc
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

if BUILD_ENGINE_GL_X11
pkgconfig_DATA += evas-opengl-x11.pc
endif

if BUILD_ENGINE_GL_SDL
pkgconfig_DATA += evas-opengl-sdl.pc
endif

if BUILD_ENGINE_GL_COCOA
pkgconfig_DATA += evas-opengl-cocoa.pc
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

if BUILD_ENGINE_SOFTWARE_16_SDL
pkgconfig_DATA += evas-software-sdl.pc
endif

if BUILD_ENGINE_PSL1GHT
pkgconfig_DATA += evas-psl1ght.pc
endif

if BUILD_ENGINE_WAYLAND_SHM
pkgconfig_DATA += evas-wayland-shm.pc
endif

if BUILD_ENGINE_WAYLAND_EGL
pkgconfig_DATA += evas-wayland-egl.pc
endif

.PHONY: doc coverage

doc:
	@echo "entering doc/"
	$(MAKE) -C doc doc

# Unit tests

if EFL_ENABLE_TESTS

check-local:
	@./src/tests/evas_suite

else

check-local:
	@echo "reconfigure with --enable-tests"

endif

# Coverage report

if EFL_ENABLE_COVERAGE
lcov-reset:
	@rm -rf $(top_builddir)/coverage
	@find $(top_builddir) -name "*.gcda" -delete
	@lcov --zerocounters --directory $(top_builddir)

lcov-report:
	@mkdir $(top_builddir)/coverage
	lcov --capture --compat-libtool --output-file $(top_builddir)/coverage/coverage.info --directory $(top_builddir)
	lcov --remove $(top_builddir)/coverage/coverage.info '*.h' --output-file $(top_builddir)/coverage/coverage.cleaned.info
	genhtml -t "$(PACKAGE_STRING)" -o $(top_builddir)/coverage/html $(top_builddir)/coverage/coverage.cleaned.info
	@echo "Coverage Report at $(top_builddir)/coverage/html"


coverage:
	@$(MAKE) lcov-reset
	@$(MAKE) check
	@$(MAKE) lcov-report
else
lcov-reset:
	@echo "reconfigure with --enable-coverage"

lcov-report:
	@echo "reconfigure with --enable-coverage"

coverage:
	@echo "reconfigure with --enable-tests --enable-coverage"
endif

maintainer-clean-local:
	rm -rf coverage

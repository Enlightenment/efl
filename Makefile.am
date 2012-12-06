ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src data doc

if HAVE_PO

SUBDIRS += po

endif

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
compile \
config.cache-env \
config.guess \
config.h.in \
config.h.in~ \
config.sub \
configure \
depcomp \
install-sh \
ltmain.sh \
missing \
m4/libtool.m4 \
m4/lt~obsolete.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4

EXTRA_DIST =  \
m4/ac_path_generic.m4 \
m4/efl_attribute.m4 \
m4/efl_check_funcs.m4 \
m4/efl_compiler.m4 \
m4/efl_doxygen.m4 \
m4/efl_path_max.m4 \
m4/efl_threads.m4 \
m4/evil_windows.m4 \
m4/eina_check.m4 \
m4/eina_config.m4 \
m4/evas_check_engine.m4 \
m4/evas_check_loader.m4 \
m4/evas_dither.m4 \
m4/ecore_check_c_extension.m4 \
m4/ecore_check_module.m4 \
m4/eio_check_options.m4

EXTRA_DIST += \
autogen.sh \
licenses/COPYING.LGPL \
licenses/COPYING.GPL \
licenses/COPYING.BSD \
old/ChangeLog.evil \
old/ChangeLog.eina \
old/ChangeLog.eet \
old/NEWS.evil \
old/README.evil \
old/NEWS.eina \
old/README.eina \
old/NEWS.eet \
old/README.eet \
spec/efl.spec \
pc/evil.pc \
pc/escape.pc \
pc/eina.pc \
pc/eet.pc \
pc/eo.pc \
pc/evas.pc \
pc/evas-fb.pc \
pc/evas-opengl-x11.pc \
pc/evas-opengl-sdl.pc \
pc/evas-software-buffer.pc \
pc/evas-software-x11.pc \
pc/evas-software-gdi.pc \
pc/evas-software-ddraw.pc \
pc/evas-psl1ght.pc \
pc/evas-wayland-shm.pc \
pc/evas-wayland-egl.pc \
pc/ecore.pc \
pc/ecore-con.pc \
pc/ecore-ipc.pc \
pc/ecore-file.pc \
pc/ecore-input.pc \
pc/ecore-input-evas.pc \
pc/ecore-cocoa.pc \
pc/ecore-fb.pc \
pc/ecore-psl1ght.pc \
pc/ecore-sdl.pc \
pc/ecore-wayland.pc \
pc/ecore-win32.pc \
pc/ecore-wince.pc \
pc/ecore-x.pc \
pc/ecore-imf.pc \
pc/ecore-imf-evas.pc \
pc/ecore-evas.pc \
pc/embryo.pc \
pc/eio.pc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =

if HAVE_WINDOWS
pkgconfig_DATA += pc/evil.pc
endif

if HAVE_PS3
pkgconfig_DATA += pc/escape.pc
endif

pkgconfig_DATA += \
pc/eina.pc \
pc/eo.pc \
pc/eet.pc \
pc/evas.pc \
pc/ecore.pc \
pc/ecore-con.pc \
pc/ecore-ipc.pc \
pc/ecore-file.pc \
pc/ecore-input.pc \
pc/ecore-input-evas.pc \
pc/ecore-imf.pc \
pc/ecore-imf-evas.pc \
pc/ecore-evas.pc \
pc/embryo.pc \
pc/eio.pc

if BUILD_ENGINE_SOFTWARE_X11
pkgconfig_DATA += pc/evas-software-x11.pc
endif

if BUILD_ENGINE_FB
pkgconfig_DATA += pc/evas-fb.pc
endif

if BUILD_ENGINE_BUFFER
pkgconfig_DATA += pc/evas-software-buffer.pc
endif

if BUILD_ENGINE_GL_X11
pkgconfig_DATA += pc/evas-opengl-x11.pc
endif

if BUILD_ENGINE_GL_SDL
pkgconfig_DATA += pc/evas-opengl-sdl.pc
endif

if BUILD_ENGINE_GL_COCOA
pkgconfig_DATA += pc/evas-opengl-cocoa.pc
endif

if BUILD_ENGINE_SOFTWARE_GDI
pkgconfig_DATA += pc/evas-software-gdi.pc
endif

if BUILD_ENGINE_SOFTWARE_DDRAW
pkgconfig_DATA += pc/evas-software-ddraw.pc
endif

if BUILD_ENGINE_PSL1GHT
pkgconfig_DATA += pc/evas-psl1ght.pc
endif

if BUILD_ENGINE_WAYLAND_SHM
pkgconfig_DATA += pc/evas-wayland-shm.pc
endif

if BUILD_ENGINE_WAYLAND_EGL
pkgconfig_DATA += pc/evas-wayland-egl.pc
endif

if HAVE_ECORE_COCOA
pkgconfig_DATA += pc/ecore-cocoa.pc
endif

if HAVE_ECORE_FB
pkgconfig_DATA += pc/ecore-fb.pc
endif

if HAVE_PS3
pkgconfig_DATA += pc/ecore-psl1ght.pc
endif

if HAVE_ECORE_SDL
pkgconfig_DATA += pc/ecore-sdl.pc
endif

if HAVE_ECORE_WAYLAND
pkgconfig_DATA += pc/ecore-wayland.pc
endif

if HAVE_WIN32
pkgconfig_DATA += pc/ecore-win32.pc
endif

if HAVE_WINCE
pkgconfig_DATA += pc/ecore-wince.pc
endif

if HAVE_ECORE_X
pkgconfig_DATA += pc/ecore-x.pc
endif


.PHONY: doc benchmark examples install-examples

# Documentation

doc:
	@echo "entering doc/"
	$(MAKE) $(AM_MAKEFLAGS) -C doc doc

# Unit tests

if EFL_ENABLE_COVERAGE

lcov-reset:
	@rm -rf $(top_builddir)/coverage
	@find $(top_builddir) -name "*.gcda" -delete
	@lcov --zerocounters --directory $(top_builddir)

lcov-report:
	@mkdir $(top_builddir)/coverage
	lcov --capture --compat-libtool --output-file $(top_builddir)/coverage/coverage.info --directory $(top_builddir) --base-directory $(top_srcdir)/src/
	lcov --remove $(top_builddir)/coverage/coverage.info '*.h' --output-file $(top_builddir)/coverage/coverage.cleaned.info
	genhtml -t "$(PACKAGE_STRING)" -o $(top_builddir)/coverage/html $(top_builddir)/coverage/coverage.cleaned.info
	@echo "Coverage Report at $(top_builddir)/coverage/html"

endif

if EFL_ENABLE_TESTS

check-local:
if EFL_ENABLE_COVERAGE
	@$(MAKE) $(AM_MAKEFLAGS) lcov-reset
endif
	@./src/tests/eet/eet_suite$(EXEEXT)
	@./src/tests/eo/eo_suite$(EXEEXT)
	@./src/tests/evas/evas_suite$(EXEEXT)
	@./src/tests/ecore/ecore_suite$(EXEEXT)
if EFL_ENABLE_COVERAGE
	@$(MAKE) $(AM_MAKEFLAGS) lcov-report
endif

endif

# benchmark

benchmark:
	@$(MAKE) $(AM_MAKEFLAGS) -C src benchmark
	@mkdir benchmark || true
	@cd benchmark && ../src/benchmarks/eo/eo_bench$(EXEEXT) `date +%F_%s`

# examples

examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C src examples

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C src install-examples

# cleaning

clean-local:
	rm -rf benchmark coverage

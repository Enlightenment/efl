ACLOCAL_AMFLAGS = -I m4
AM_MAKEFLAGS = --no-print-directory
AM_DISTCHECK_CONFIGURE_FLAGS = \
--with-tests=regular \
--with-systemdunitdir=. \
--enable-always-build-examples

SUBDIRS = src data config doc

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
m4/pkg_var.m4 \
m4/ac_path_generic.m4 \
m4/ecore_check_c_extension.m4 \
m4/ecore_check_module.m4 \
m4/efl_attribute.m4 \
m4/efl_check_funcs.m4 \
m4/efl_compiler.m4 \
m4/efl_doxygen.m4 \
m4/efl_path_max.m4 \
m4/efl_threads.m4 \
m4/efl_libunwind.m4 \
m4/eina_check.m4 \
m4/eina_config.m4 \
m4/eio_check_options.m4 \
m4/evas_check_engine.m4 \
m4/evas_check_loader.m4 \
m4/evil_windows.m4 \
m4/libgcrypt.m4 \
m4/gcc_check_builtin.m4

EXTRA_DIST += \
README \
AUTHORS \
COPYING \
COMPLIANCE \
licenses/COPYING.LGPL \
licenses/COPYING.GPL \
licenses/COPYING.BSD \
licenses/COPYING.SMALL \
licenses/COPYING.FTL \
licenses/COPYING.NGINX-MIT \
old/ChangeLog.ecore \
old/ChangeLog.edje \
old/ChangeLog.eet \
old/ChangeLog.eeze \
old/ChangeLog.efreet \
old/ChangeLog.eina \
old/ChangeLog.eio \
old/ChangeLog.embryo \
old/ChangeLog.emotion \
old/ChangeLog.escape \
old/ChangeLog.ethumb \
old/ChangeLog.evas \
old/ChangeLog.evil \
old/ChangeLog.evil.0 \
old/NEWS.ecore \
old/NEWS.edje \
old/NEWS.eet \
old/NEWS.eeze \
old/NEWS.efreet \
old/NEWS.eina \
old/NEWS.eio \
old/NEWS.embryo \
old/NEWS.emotion \
old/NEWS.escape \
old/NEWS.ethumb \
old/NEWS.evas \
old/NEWS.evil \
old/README.ecore \
old/README.eldbus \
old/README.edje \
old/README.eet \
old/README.eeze \
old/README.efreet \
old/README.eina \
old/README.eio \
old/README.embryo \
old/README.emotion \
old/README.ephysics \
old/README.escape \
old/README.ethumb \
old/README.evas \
old/README.evil \
spec/efl.spec \
pkgbuild/PKGBUILD \
pkgbuild/efl.install

if HAVE_FREEBSD
pkgconfigdir = $(libdir)data/pkgconfig
else
pkgconfigdir = $(libdir)/pkgconfig
endif

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
pc/eolian.pc \
pc/efl.pc \
pc/emile.pc \
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
pc/ecore-avahi.pc \
pc/ector.pc \
pc/embryo.pc \
pc/eio.pc \
pc/eldbus.pc \
pc/efreet.pc \
pc/efreet-mime.pc \
pc/efreet-trash.pc \
pc/edje.pc \
pc/emotion.pc \
pc/ethumb.pc \
pc/ethumb_client.pc \
pc/elocation.pc \
pc/elementary.pc

if HAVE_CXX11
pkgconfig_DATA += \
pc/eina-cxx.pc \
pc/evas-cxx.pc \
pc/ecore-cxx.pc \
pc/efl-cxx.pc \
pc/eolian-cxx.pc \
pc/edje-cxx.pc \
pc/eet-cxx.pc \
pc/eo-cxx.pc \
pc/eio-cxx.pc \
pc/elementary-cxx.pc
endif

if HAVE_ELUA
pkgconfig_DATA += pc/elua.pc
endif

if HAVE_JS
pkgconfig_DATA += \
pc/eolian-js.pc \
pc/efl-js.pc \
pc/eo-js.pc \
pc/eina-js.pc
endif

if BUILD_ENGINE_SOFTWARE_X11
pkgconfig_DATA += pc/evas-software-x11.pc
endif

if BUILD_ENGINE_FB
pkgconfig_DATA += pc/evas-fb.pc
endif

if BUILD_ENGINE_EGLFS
pkgconfig_DATA += pc/evas-eglfs.pc
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

if BUILD_ENGINE_WAYLAND_SHM
pkgconfig_DATA += pc/evas-wayland-shm.pc
endif

if BUILD_ENGINE_WAYLAND_EGL
pkgconfig_DATA += pc/evas-wayland-egl.pc
endif

if BUILD_ENGINE_DRM
pkgconfig_DATA += pc/evas-drm.pc
endif

if HAVE_ECORE_COCOA
pkgconfig_DATA += pc/ecore-cocoa.pc
endif

if HAVE_ECORE_FB
pkgconfig_DATA += pc/ecore-fb.pc
endif

if HAVE_ECORE_DRM
pkgconfig_DATA += pc/ecore-drm.pc
endif

if HAVE_ECORE_SDL
pkgconfig_DATA += pc/ecore-sdl.pc
endif

if HAVE_ECORE_WAYLAND
pkgconfig_DATA += pc/ecore-wayland.pc
endif

if HAVE_ECORE_WL2
pkgconfig_DATA += pc/ecore-wl2.pc
endif

if HAVE_EFL_WL
pkgconfig_DATA += pc/efl-wl.pc
endif

if HAVE_WIN32
pkgconfig_DATA += pc/ecore-win32.pc
endif

if HAVE_ECORE_X
pkgconfig_DATA += pc/ecore-x.pc
endif

if HAVE_EEZE
pkgconfig_DATA += pc/eeze.pc
endif

if HAVE_EPHYSICS
pkgconfig_DATA += pc/ephysics.pc
endif

if HAVE_ECORE_AUDIO
pkgconfig_DATA += pc/ecore-audio.pc pc/ecore-audio-cxx.pc
endif

if HAVE_ECORE_BUFFER
pkgconfig_DATA += pc/ecore-buffer.pc
endif

if HAVE_ELPUT
pkgconfig_DATA += pc/elput.pc
endif

if HAVE_ECORE_DRM2
pkgconfig_DATA += pc/ecore-drm2.pc
endif

# Cmake configs:
efl_cmakeconfigdir = $(libdir)/cmake/Efl/
efl_cmakeconfig_DATA = \
cmakeconfig/EflConfig.cmake \
cmakeconfig/EflConfigVersion.cmake

eina_cmakeconfigdir = $(libdir)/cmake/Eina/
eina_cmakeconfig_DATA = \
cmakeconfig/EinaConfig.cmake \
cmakeconfig/EinaConfigVersion.cmake

eio_cmakeconfigdir = $(libdir)/cmake/Eio/
eio_cmakeconfig_DATA = \
cmakeconfig/EioConfig.cmake \
cmakeconfig/EioConfigVersion.cmake

eeze_cmakeconfigdir = $(libdir)/cmake/Eeze/
eeze_cmakeconfig_DATA = \
cmakeconfig/EezeConfig.cmake \
cmakeconfig/EezeConfigVersion.cmake

eo_cmakeconfigdir = $(libdir)/cmake/Eo/
eo_cmakeconfig_DATA = \
cmakeconfig/EoConfig.cmake \
cmakeconfig/EoConfigVersion.cmake

eolian_cmakeconfigdir = $(libdir)/cmake/Eolian/
eolian_cmakeconfig_DATA = \
cmakeconfig/EolianConfig.cmake \
cmakeconfig/EolianConfigVersion.cmake \
cmakeconfig/EolianHelper.cmake

eolian_cxx_cmakeconfigdir = $(libdir)/cmake/EolianCxx/
eolian_cxx_cmakeconfig_DATA = \
cmakeconfig/EolianCxxConfig.cmake \
cmakeconfig/EolianCxxConfigVersion.cmake

eina_cxx_cmakeconfigdir = $(libdir)/cmake/EinaCxx/
eina_cxx_cmakeconfig_DATA = \
cmakeconfig/EinaCxxConfig.cmake \
cmakeconfig/EinaCxxConfigVersion.cmake

eo_cxx_cmakeconfigdir = $(libdir)/cmake/EoCxx/
eo_cxx_cmakeconfig_DATA = \
cmakeconfig/EoCxxConfig.cmake \
cmakeconfig/EoCxxConfigVersion.cmake

ecore_cxx_cmakeconfigdir = $(libdir)/cmake/EcoreCxx/
ecore_cxx_cmakeconfig_DATA = \
cmakeconfig/EcoreCxxConfig.cmake \
cmakeconfig/EcoreCxxConfigVersion.cmake

evas_cxx_cmakeconfigdir = $(libdir)/cmake/EvasCxx/
evas_cxx_cmakeconfig_DATA = \
cmakeconfig/EvasCxxConfig.cmake \
cmakeconfig/EvasCxxConfigVersion.cmake

eet_cxx_cmakeconfigdir = $(libdir)/cmake/EetCxx/
eet_cxx_cmakeconfig_DATA = \
cmakeconfig/EetCxxConfig.cmake \
cmakeconfig/EetCxxConfigVersion.cmake

eet_cmakeconfigdir = $(libdir)/cmake/Eet/
eet_cmakeconfig_DATA = \
cmakeconfig/EetConfig.cmake \
cmakeconfig/EetConfigVersion.cmake

evas_cmakeconfigdir =  $(libdir)/cmake/Evas/
evas_cmakeconfig_DATA = \
cmakeconfig/EvasConfig.cmake \
cmakeconfig/EvasConfigVersion.cmake

ecore_cmakeconfigdir =  $(libdir)/cmake/Ecore/
ecore_cmakeconfig_DATA = \
cmakeconfig/EcoreConfig.cmake \
cmakeconfig/EcoreConfigVersion.cmake

edje_cmakeconfigdir =  $(libdir)/cmake/Edje/
edje_cmakeconfig_DATA = \
cmakeconfig/EdjeConfig.cmake \
cmakeconfig/EdjeConfigVersion.cmake

eldbus_cmakeconfigdir = $(libdir)/cmake/Eldbus/
eldbus_cmakeconfig_DATA = \
cmakeconfig/EldbusConfig.cmake \
cmakeconfig/EldbusConfigVersion.cmake

efreet_cmakeconfigdir =  $(libdir)/cmake/Efreet/
efreet_cmakeconfig_DATA = \
cmakeconfig/EfreetConfig.cmake \
cmakeconfig/EfreetConfigVersion.cmake

ethumb_cmakeconfigdir =  $(libdir)/cmake/Ethumb/
ethumb_cmakeconfig_DATA = \
cmakeconfig/EthumbConfig.cmake \
cmakeconfig/EthumbConfigVersion.cmake

ethumbclient_cmakeconfigdir =  $(libdir)/cmake/EthumbClient/
ethumbclient_cmakeconfig_DATA = \
cmakeconfig/EthumbClientConfig.cmake \
cmakeconfig/EthumbClientConfigVersion.cmake

cmakeconfigdir = $(libdir)/cmake/Elementary
cmakeconfig_DATA = \
cmakeconfig/ElementaryConfig.cmake \
cmakeconfig/ElementaryConfigVersion.cmake

emotion_cmakeconfigdir =  $(libdir)/cmake/Emotion/
emotion_cmakeconfig_DATA = \
cmakeconfig/EmotionConfig.cmake \
cmakeconfig/EmotionConfigVersion.cmake

elua_cmakeconfigdir = $(libdir)/cmake/Elua
elua_cmakeconfig_DATA = \
cmakeconfig/EluaConfig.cmake \
cmakeconfig/EluaConfigVersion.cmake

emile_cmakeconfigdir = $(libdir)/cmake/Emile
emile_cmakeconfig_DATA = \
cmakeconfig/EmileConfig.cmake \
cmakeconfig/EmileConfigVersion.cmake

# D-Bus services:

servicedir = @dbusservicedir@
service_DATA = \
dbus-services/org.enlightenment.Ethumb.service

if HAVE_SYSTEMD_USER_SESSION
systemdunitsdir = @USER_SESSION_DIR@
systemdunits_DATA = \
systemd-services/ethumb.service
endif
EXTRA_DIST += $(systemdunits_DATA)


.PHONY: doc benchmark examples install-examples

# Documentation

doc:
	@echo "entering doc/"
	$(MAKE) $(AM_MAKEFLAGS) -C doc doc

# Unit tests

if EFL_ENABLE_COVERAGE

lcov-reset:
	@rm -rf $(abs_top_builddir)/coverage
	@find $(abs_top_builddir) -name "*.gcda" -delete
	@lcov --zerocounters --directory $(abs_top_builddir)

lcov-baseline:
	$(MKDIR_P) $(abs_top_builddir)/coverage
	lcov --capture --initial --compat-libtool --no-external --output-file $(abs_top_builddir)/coverage/coverage-baseline.info --directory $(abs_top_builddir) --config-file .lcov-config

lcov-report: lcov-baseline
	lcov --capture --compat-libtool --no-external --output-file $(abs_top_builddir)/coverage/coverage-check.info --directory $(abs_top_builddir) --config-file .lcov-config
	lcov --no-external --compat-libtool --add-tracefile $(abs_top_builddir)/coverage/coverage-baseline.info --add-tracefile $(abs_top_builddir)/coverage/coverage-check.info --output-file $(abs_top_builddir)/coverage/coverage.info --config-file .lcov-config
	lcov --no-external --compat-libtool --remove $(abs_top_builddir)/coverage/coverage.info '*.h' --output-file $(abs_top_builddir)/coverage/coverage.cleaned.info --config-file .lcov-config
	lcov --no-external --compat-libtool --remove $(abs_top_builddir)/coverage/coverage.cleaned.info '*/tests/*' --output-file $(abs_top_builddir)/coverage/coverage.cleaned2.info --config-file .lcov-config
	lcov --no-external --compat-libtool --remove $(abs_top_builddir)/coverage/coverage.cleaned2.info '*NONE*' --output-file $(abs_top_builddir)/coverage/coverage.cleaned3.info --config-file .lcov-config
	lcov --no-external --compat-libtool --remove $(abs_top_builddir)/coverage/coverage.cleaned3.info '*/doc/*' --output-file $(abs_top_builddir)/coverage/coverage.cleaned4.info --config-file .lcov-config
	lcov --no-external --compat-libtool --remove $(abs_top_builddir)/coverage/coverage.cleaned4.info '*usr/include*' --output-file $(abs_top_builddir)/coverage/coverage.cleaned5.info --config-file .lcov-config
	genhtml --config-file .lcov-config --legend -t "$(PACKAGE_STRING)" -o $(abs_top_builddir)/coverage/html $(abs_top_builddir)/coverage/coverage.cleaned5.info
	@echo "Coverage Report at $(abs_top_builddir)/coverage/html"

endif

if EFL_ENABLE_TESTS

lcov-check:
if EFL_ENABLE_COVERAGE
	@$(MAKE) $(AM_MAKEFLAGS) lcov-reset
	@$(MAKE) $(AM_MAKEFLAGS) lcov-baseline
endif
	@$(MAKE) $(AM_MAKEFLAGS) check
if EFL_ENABLE_COVERAGE
	@$(MAKE) $(AM_MAKEFLAGS) lcov-report
endif

check-build:
	@$(MAKE) $(AM_MAKEFLAGS) -C src check-build
endif

# benchmark

benchmark:
	@$(MAKE) $(AM_MAKEFLAGS) -C src benchmark
	$(MKDIR_P) benchmark
	@cd benchmark && ../src/benchmarks/eo/eo_bench$(EXEEXT) `date +%F_%s`

# examples

examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C src examples

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C src install-examples

pkgbuild::
	@echo "making arch packages"
	makepkg -f -p $(top_builddir)/pkgbuild/PKGBUILD

# cleaning

clean-local:
	rm -rf benchmark coverage

DISTCLEANFILES= \
./src/lib/emile/Makefile \
./src/lib/eo/Makefile \
./src/lib/eet/Makefile \
./src/lib/ecore/Makefile \
./src/lib/efl/Makefile \
./src/lib/eina/Makefile \
./src/lib/eio/Makefile \
./src/lib/elementary/Makefile \
./src/lib/ector/Makefile \
./src/lib/evil/Makefile \
./src/lib/edje/Makefile \
./src/lib/efl/Efl_Config.h \
./src/lib/eina/eina_config.h

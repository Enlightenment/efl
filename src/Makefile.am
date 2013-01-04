AUTOMAKE_OPTIONS = subdir-objects

MAINTAINERCLEANFILES = Makefile.in
BUILT_SOURCES =

SUBDIRS = . benchmarks examples

lib_LTLIBRARIES =
bin_PROGRAMS =
bin_SCRIPTS =
noinst_PROGRAMS =
check_PROGRAMS =
EXTRA_DIST =

include Makefile_Evil.am
include Makefile_Escape.am

include Makefile_Eina.am
include Makefile_Eo.am
include Makefile_Eet.am
include Makefile_Evas.am
include Makefile_Ecore.am
include Makefile_Ecore_Con.am
include Makefile_Ecore_Ipc.am
include Makefile_Ecore_File.am
include Makefile_Ecore_Input.am
include Makefile_Ecore_Cocoa.am
include Makefile_Ecore_FB.am
include Makefile_Ecore_Psl1ght.am
include Makefile_Ecore_SDL.am
include Makefile_Ecore_Wayland.am
include Makefile_Ecore_Win32.am
include Makefile_Ecore_WinCE.am
include Makefile_Ecore_X.am
include Makefile_Ecore_Imf.am
include Makefile_Ecore_Evas.am
include Makefile_Ecore_Audio.am
include Makefile_Embryo.am
include Makefile_Eio.am
include Makefile_EDBus.am
include Makefile_Efreet.am
include Makefile_Eeze.am
include Makefile_EPhysics.am
include Makefile_Edje.am

.PHONY: benchmark examples

benchmark:
	@$(MAKE) $(AM_MAKEFLAGS)
	@$(MAKE) $(AM_MAKEFLAGS) -C benchmarks benchmark

examples: $(lib_LTLIBRARIES) $(bin_PROGRAMS)
	@make
	@$(MAKE) $(AM_MAKEFLAGS) -C examples examples

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C examples install-examples

clean-local:
	rm -rf bin/eet/*.gcno
	rm -rf bin/efreet/*.gcno
	rm -rf bin/eeze/*.gcno
	rm -rf bin/edje/*.gcno
	rm -rf lib/evil/*.gcno
	rm -rf lib/eina/*.gcno
	rm -rf lib/eo/*.gcno
	rm -rf lib/eet/*.gcno
	rm -rf lib/evas/*.gcno
	rm -rf lib/evas/cache/*.gcno
	rm -rf lib/evas/cache2/*.gcno
	rm -rf lib/evas/canvas/*.gcno
	rm -rf lib/evas/common/*.gcno
	rm -rf lib/evas/common/evas_op_blend/*.gcno
	rm -rf lib/evas/common/language/*.gcno
	rm -rf lib/evas/cserve2/*.gcno
	rm -rf lib/evas/file/*.gcno
	rm -rf lib/embryo/*.gcno
	rm -rf lib/edbus/*.gcno
	rm -rf lib/efreet/*.gcno
	rm -rf lib/eeze/*.gcno
	rm -rf lib/ephysics/*.gcno
	rm -rf lib/edje/*.gcno
	rm -rf modules/eina/mp/pass_through/*.gcno
	rm -rf modules/eina/mp/one_big/*.gcno
	rm -rf modules/eina/mp/chained_pool/*.gcno
	rm -rf modules/evas/engines/buffer/*.gcno
	rm -rf modules/evas/engines/direct3d/*.gcno
	rm -rf modules/evas/engines/fb/*.gcno
	rm -rf modules/evas/engines/gl_cocoa/*.gcno
	rm -rf modules/evas/engines/gl_common/*.gcno
	rm -rf modules/evas/engines/gl_sdl/*.gcno
	rm -rf modules/evas/engines/gl_x11/*.gcno
	rm -rf modules/evas/engines/psl1ght/*.gcno
	rm -rf modules/evas/engines/software_ddraw/*.gcno
	rm -rf modules/evas/engines/software_gdi/*.gcno
	rm -rf modules/evas/engines/software_generic/*.gcno
	rm -rf modules/evas/engines/software_x11/*.gcno
	rm -rf modules/evas/engines/wayland_egl/*.gcno
	rm -rf modules/evas/engines/wayland_shm/*.gcno
	rm -rf modules/evas/loaders/bmp/*.gcno
	rm -rf modules/evas/loaders/eet/*.gcno
	rm -rf modules/evas/loaders/generic/*.gcno
	rm -rf modules/evas/loaders/gif/*.gcno
	rm -rf modules/evas/loaders/ico/*.gcno
	rm -rf modules/evas/loaders/jpeg/*.gcno
	rm -rf modules/evas/loaders/pmaps/*.gcno
	rm -rf modules/evas/loaders/png/*.gcno
	rm -rf modules/evas/loaders/psd/*.gcno
	rm -rf modules/evas/loaders/svg/*.gcno
	rm -rf modules/evas/loaders/tga/*.gcno
	rm -rf modules/evas/loaders/tiff/*.gcno
	rm -rf modules/evas/loaders/wbmp/*.gcno
	rm -rf modules/evas/loaders/webp/*.gcno
	rm -rf modules/evas/loaders/xpm/*.gcno
	rm -rf modules/evas/savers/eet/*.gcno
	rm -rf modules/evas/savers/jpeg/*.gcno
	rm -rf modules/evas/savers/png/*.gcno
	rm -rf modules/evas/savers/tiff/*.gcno
	rm -rf modules/edje/alsa_snd_player/*.gcno
	rm -rf modules/edje/eet_snd_reader/*.gcno
	rm -rf modules/edje/multisense_factory/*.gcno
	rm -rf static_libs/liblinebreak/*.gcno
	rm -rf static_libs/lz4/*.gcno

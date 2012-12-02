AUTOMAKE_OPTIONS = subdir-objects

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = . benchmarks examples

lib_LTLIBRARIES =
bin_PROGRAMS =
check_PROGRAMS =
EXTRA_DIST =

if HAVE_WINDOWS
include Makefile_Evil.am
endif

if HAVE_PS3
include Makefile_Escape.am
endif

include Makefile_Eina.am
include Makefile_Eo.am
include Makefile_Eet.am
include Makefile_Evas.am
include Makefile_Ecore.am
include Makefile_Ecore_Con.am
include Makefile_Ecore_Ipc.am
include Makefile_Ecore_File.am
include Makefile_Ecore_Input.am
if HAVE_ECORE_COCOA
include Makefile_Ecore_Cocoa.am
endif
if HAVE_ECORE_DIRECTFB
include Makefile_Ecore_DirectFB.am
endif
if HAVE_ECORE_FB
include Makefile_Ecore_FB.am
endif
if HAVE_PS3
include Makefile_Ecore_Psl1ght.am
endif
if HAVE_ECORE_SDL
include Makefile_Ecore_SDL.am
endif
if HAVE_ECORE_WAYLAND
include Makefile_Ecore_Wayland.am
endif
if HAVE_WIN32
include Makefile_Ecore_Win32.am
endif
if HAVE_WINCE
include Makefile_Ecore_WinCE.am
endif
if HAVE_ECORE_X
include Makefile_Ecore_X.am
endif
include Makefile_Ecore_Imf.am
include Makefile_Embryo.am

.PHONY: benchmark examples

benchmark:
	@$(MAKE) $(AM_MAKEFLAGS)
	@$(MAKE) $(AM_MAKEFLAGS) -C benchmarks benchmark

examples:
	@make
	@$(MAKE) $(AM_MAKEFLAGS) -C examples examples

install-examples:
	@$(MAKE) $(AM_MAKEFLAGS) -C examples install-examples

clean-local:
	rm -rf bin/eet/*.gcno
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
	rm -rf modules/eina/mp/pass_through/*.gcno
	rm -rf modules/eina/mp/one_big/*.gcno
	rm -rf modules/eina/mp/chained_pool/*.gcno
	rm -rf modules/evas/engines/buffer/*.gcno
	rm -rf modules/evas/engines/direct3d/*.gcno
	rm -rf modules/evas/engines/directfb/*.gcno
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
	rm -rf static_libs/liblinebreak/*.gcno
	rm -rf static_libs/lz4/*.gcno

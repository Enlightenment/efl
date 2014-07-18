MAINTAINERCLEANFILES = Makefile.in
CLEANFILES =
BUILT_SOURCES =

EOLIAN_FLAGS = \
	       -I$(srcdir)/lib/eo \
	       -I$(srcdir)/lib/evas/canvas \
	       -I$(srcdir)/lib/edje \
	       -I$(srcdir)/lib/ecore_audio

ELUA_GEN_FLAGS =

DIST_SUBDIRS =
SUBDIRS =

lib_LTLIBRARIES =
bin_PROGRAMS =
bin_SCRIPTS =
noinst_PROGRAMS =
check_PROGRAMS =
TESTS =
EXTRA_DIST =


EFL_INSTALL_EXEC_HOOK=

include Makefile_Evil.am
include Makefile_Escape.am

include Makefile_Eina.am
include Makefile_Eo.am
include Makefile_Efl.am
include Makefile_Eet.am
include Makefile_Eolian.am
include Makefile_Ecore.am
include Makefile_Ecore_Con.am
include Makefile_Ecore_Ipc.am
include Makefile_Ecore_File.am
include Makefile_Ecore_Avahi.am
include Makefile_Embryo.am
include Makefile_Eio.am
include Makefile_Eldbus.am
include Makefile_Efreet.am
include Makefile_Eeze.am

if BUILD_GUI
include Makefile_Evas.am
include Makefile_Ecore_Input.am
include Makefile_Ecore_Input_Evas.am
include Makefile_Ecore_Cocoa.am
include Makefile_Ecore_Drm.am
include Makefile_Ecore_FB.am
include Makefile_Ecore_Psl1ght.am
include Makefile_Ecore_SDL.am
include Makefile_Ecore_Wayland.am
include Makefile_Ecore_Win32.am
include Makefile_Ecore_X.am
include Makefile_Ecore_Audio.am
include Makefile_Ecore_IMF.am
include Makefile_Ecore_IMF_Evas.am
include Makefile_Ecore_Evas.am
include Makefile_EPhysics.am
include Makefile_Edje.am
include Makefile_Emotion.am
include Makefile_Ethumb.am
include Makefile_Ethumb_Client.am

include Makefile_Ecore_Audio_Cxx.am
include Makefile_Edje_Cxx.am
include Makefile_Evas_Cxx.am
endif

include Makefile_Eina_Cxx.am
include Makefile_Ecore_Cxx.am
include Makefile_Eldbus_Cxx.am
include Makefile_Eolian_Cxx.am
include Makefile_Eet_Cxx.am
include Makefile_Eo_Cxx.am

include Makefile_Elua.am

.PHONY: benchmark examples

BENCHMARK_SUBDIRS = \
benchmarks/eina \
benchmarks/eo \
benchmarks/evas
DIST_SUBDIRS += $(BENCHMARK_SUBDIRS)

benchmark: all-am
	@for d in $(BENCHMARK_SUBDIRS); do \
	   echo "Making benchmark in $$d"; \
	   $(MAKE) $(AM_MAKEFLAGS) -C $$d benchmark; \
	done

EXAMPLES_SUBDIRS = \
examples/eina \
examples/eo \
examples/eet \
examples/evas \
examples/ecore \
examples/ecore_avahi \
examples/eio \
examples/eldbus \
examples/ephysics \
examples/edje \
examples/emotion \
examples/ethumb_client

if HAVE_ELUA
EXAMPLES_SUBDIRS += examples/elua
endif

if HAVE_CXX11
EXAMPLES_SUBDIRS += \
examples/eina_cxx \
examples/eolian_cxx
endif

if ALWAYS_BUILD_EXAMPLES
SUBDIRS += . $(EXAMPLES_SUBDIRS)
endif

DIST_SUBDIRS += $(EXAMPLES_SUBDIRS)

examples: all-am
	@for d in $(EXAMPLES_SUBDIRS); do \
	   echo "Making examples in $$d"; \
	   $(MAKE) $(AM_MAKEFLAGS) -C $$d examples || exit 1; \
	done

install-examples:
	@for d in $(EXAMPLES_SUBDIRS); do \
	   echo "Making install-examples in $$d"; \
	   $(MAKE) $(AM_MAKEFLAGS) -C $$d install-examples || exit 1; \
	done

clean-local:
	find . -type f -name '*.gcno' -delete
	find . -type f -name '*.gcda' -delete
	find . -type f -name check-results.xml -delete

install-exec-hook:
	for i in $(lib_LTLIBRARIES); do \
	   FNAME="$(DESTDIR)$(libdir)/`echo $$i | sed "s#lib/.*/##"`"; \
	   FNAMEEXT="$$FNAME.$$$$"; \
	   sed "s/-luuid//" $$FNAME > $$FNAMEEXT; \
	   mv $$FNAMEEXT $$FNAME; \
	done
	$(EFL_INSTALL_EXEC_HOOK)

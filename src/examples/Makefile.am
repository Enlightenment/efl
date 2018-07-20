AUTOMAKE_OPTIONS= -Wno-portability
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in
EXTRA_PROGRAMS =
DATA_FILES =
EXTRA_DIST =
EDCS =
PHONIES =
EOLIAN_FLAGS := \
  -I$(srcdir) \
  -I$(top_srcdir)/src/lib/eo \
  -I$(top_srcdir)/src/lib/evas/canvas \
  -I$(top_srcdir)/src/lib/evas/gesture \
  -I$(top_srcdir)/src/lib/edje \
  -I$(top_srcdir)/src/lib/efl \
  -I$(top_srcdir)/src/lib/ecore \
  -I$(top_srcdir)/src/lib/ecore_audio \
  -I$(top_srcdir)/src/lib/elementary

#defined in Makefile_Edje_Helper.am
#EDJE_CC_FLAGS =

CLEAN_EXAMPLES =
INSTALL_EXAMPLES =
UNINSTALL_EXAMPLES =

include ../Makefile_Edje_Helper.am
include $(top_srcdir)/src/Makefile_Eolian_Helper.am
if HAVE_CXX
include $(top_srcdir)/src/Makefile_Eolian_Cxx_Helper.am
if ! HAVE_EOLIAN_CXX
_EOLIAN_CXX_DEP = ../bin/eolian_cxx/eolian_cxx$(EXEEXT)
bin/eolian_cxx/eolian_cxx$(EXEEXT):
	false
endif
endif
# Eolian_Files_Helper
# Thus we need to override it here in order to make its pattern
# rules work properly.
if ! HAVE_EOLIAN_GEN
_EOLIAN_GEN_DEP = ../bin/eolian/eolian_gen$(EXEEXT)
endif

if HAVE_CSHARP
include $(top_srcdir)/src/Makefile_Efl_Mono_MSBuild_Gen_Helper.am
endif

AM_CPPFLAGS = \
-I. \
-I$(top_builddir)/src/bindings \
-I$(top_srcdir)/src/bindings \
-I$(top_builddir)/src/lib/efl/interfaces \
-I$(top_builddir)/src/lib/evas/canvas \
-I$(top_builddir)/src/lib/evas/gesture \
-I$(top_srcdir)/src/bindings/cxx/eina_cxx \
-I$(top_builddir)/src/bindings/cxx/eina_cxx \
-I$(top_srcdir)/src/bindings/cxx/eo_cxx \
-I$(top_builddir)/src/bindings/cxx/eo_cxx \
-I$(top_srcdir)/src/bindings/cxx/eldbus_cxx \
-I$(top_builddir)/src/bindings/cxx/eldbus_cxx \
-I$(top_srcdir)/src/bindings/cxx/ecore_cxx \
-I$(top_builddir)/src/bindings/cxx/ecore_cxx \
-I$(top_builddir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_audio \
-I$(top_srcdir)/src/lib/ecore_audio \
-I$(top_builddir)/src/lib/ecore_avahi \
-I$(top_srcdir)/src/lib/ecore_avahi \
-I$(top_builddir)/src/lib/ecore_buffer \
-I$(top_srcdir)/src/lib/ecore_buffer \
-I$(top_builddir)/src/lib/ecore_cocoa \
-I$(top_srcdir)/src/lib/ecore_cocoa \
-I$(top_builddir)/src/lib/ecore_con \
-I$(top_srcdir)/src/lib/ecore_con \
-I$(top_builddir)/src/lib/ecore_drm \
-I$(top_srcdir)/src/lib/ecore_drm \
-I$(top_builddir)/src/lib/ecore_drm2 \
-I$(top_srcdir)/src/lib/ecore_drm2 \
-I$(top_builddir)/src/lib/ecore_evas \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore_fb \
-I$(top_srcdir)/src/lib/ecore_fb \
-I$(top_builddir)/src/lib/ecore_file \
-I$(top_srcdir)/src/lib/ecore_file \
-I$(top_builddir)/src/lib/ecore_imf \
-I$(top_srcdir)/src/lib/ecore_imf \
-I$(top_builddir)/src/lib/ecore_imf_evas \
-I$(top_srcdir)/src/lib/ecore_imf_evas \
-I$(top_builddir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore_input_evas \
-I$(top_srcdir)/src/lib/ecore_input_evas \
-I$(top_builddir)/src/lib/ecore_ipc \
-I$(top_srcdir)/src/lib/ecore_ipc \
-I$(top_builddir)/src/lib/ecore_sdl \
-I$(top_srcdir)/src/lib/ecore_sdl \
-I$(top_builddir)/src/lib/ecore_wayland \
-I$(top_srcdir)/src/lib/ecore_wayland \
-I$(top_builddir)/src/lib/ecore_win32 \
-I$(top_srcdir)/src/lib/ecore_win32 \
-I$(top_builddir)/src/lib/ecore_wl2 \
-I$(top_srcdir)/src/lib/ecore_wl2 \
-I$(top_builddir)/src/lib/ecore_x \
-I$(top_srcdir)/src/lib/ecore_x \
-I$(top_builddir)/src/lib/ector \
-I$(top_srcdir)/src/lib/ector \
-I$(top_builddir)/src/lib/edje \
-I$(top_srcdir)/src/lib/edje \
-I$(top_builddir)/src/lib/eet \
-I$(top_srcdir)/src/lib/eet \
-I$(top_builddir)/src/lib/eeze \
-I$(top_srcdir)/src/lib/eeze \
-I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/lib/efl \
-I$(top_builddir)/src/lib/efl_mono \
-I$(top_srcdir)/src/lib/efl_mono \
-I$(top_builddir)/src/lib/efl_wl \
-I$(top_srcdir)/src/lib/efl_wl \
-I$(top_builddir)/src/lib/efreet \
-I$(top_srcdir)/src/lib/efreet \
-I$(top_builddir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eina \
-I$(top_builddir)/src/lib/eio \
-I$(top_srcdir)/src/lib/eio \
-I$(top_builddir)/src/lib/eldbus \
-I$(top_srcdir)/src/lib/eldbus \
-I$(top_builddir)/src/lib/elementary \
-I$(top_srcdir)/src/lib/elementary \
-I$(top_builddir)/src/lib/elocation \
-I$(top_srcdir)/src/lib/elocation \
-I$(top_builddir)/src/lib/elput \
-I$(top_srcdir)/src/lib/elput \
-I$(top_builddir)/src/lib/elua \
-I$(top_srcdir)/src/lib/elua \
-I$(top_builddir)/src/lib/embryo \
-I$(top_srcdir)/src/lib/embryo \
-I$(top_builddir)/src/lib/emile \
-I$(top_srcdir)/src/lib/emile \
-I$(top_builddir)/src/lib/emotion \
-I$(top_srcdir)/src/lib/emotion \
-I$(top_builddir)/src/lib/eo \
-I$(top_srcdir)/src/lib/eo \
-I$(top_builddir)/src/lib/eolian \
-I$(top_srcdir)/src/lib/eolian \
-I$(top_builddir)/src/lib/eolian_cxx \
-I$(top_srcdir)/src/lib/eolian_cxx \
-I$(top_builddir)/src/lib/ephysics \
-I$(top_srcdir)/src/lib/ephysics \
-I$(top_builddir)/src/lib/escape \
-I$(top_srcdir)/src/lib/escape \
-I$(top_builddir)/src/lib/ethumb \
-I$(top_srcdir)/src/lib/ethumb \
-I$(top_builddir)/src/lib/ethumb_client \
-I$(top_srcdir)/src/lib/ethumb_client \
-I$(top_builddir)/src/lib/evas \
-I$(top_srcdir)/src/lib/evas \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
@ELEMENTARY_CFLAGS@

if HAVE_WINDOWS
AM_CPPFLAGS += \
-I$(top_srcdir)/src/lib/evil \
-I$(top_builddir)/src/lib/evil \
@EFL_WINDOWS_VERSION_CFLAGS@
endif

AM_CXXFLAGS = \
$(AM_CPPFLAGS) \
@ELEMENTARY_CXX_CFLAGS@

LDADD = \
-lm \
@EFL_PTHREAD_LIBS@ \
$(top_builddir)/src/lib/ecore_avahi/libecore_avahi.la \
$(top_builddir)/src/lib/ecore_con/libecore_con.la \
$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
$(top_builddir)/src/lib/ecore_file/libecore_file.la \
$(top_builddir)/src/lib/ecore_imf_evas/libecore_imf_evas.la \
$(top_builddir)/src/lib/ecore_imf/libecore_imf.la \
$(top_builddir)/src/lib/ecore_input_evas/libecore_input_evas.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
$(top_builddir)/src/lib/ecore_ipc/libecore_ipc.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ector/libector.la \
$(top_builddir)/src/lib/edje/libedje.la \
$(top_builddir)/src/lib/eet/libeet.la \
$(top_builddir)/src/lib/efl/libefl.la \
$(top_builddir)/src/lib/efreet/libefreet.la \
$(top_builddir)/src/lib/efreet/libefreet_mime.la \
$(top_builddir)/src/lib/efreet/libefreet_trash.la \
$(top_builddir)/src/lib/eina/libeina.la \
$(top_builddir)/src/lib/eio/libeio.la \
$(top_builddir)/src/lib/eldbus/libeldbus.la \
$(top_builddir)/src/lib/elementary/libelementary.la \
$(top_builddir)/src/lib/elocation/libelocation.la \
$(top_builddir)/src/lib/embryo/libembryo.la \
$(top_builddir)/src/lib/emile/libemile.la \
$(top_builddir)/src/lib/emotion/libemotion.la \
$(top_builddir)/src/lib/eolian/libeolian.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/ethumb_client/libethumb_client.la \
$(top_builddir)/src/lib/ethumb/libethumb.la \
$(top_builddir)/src/lib/evas/libevas.la


AM_LDFLAGS = @ELEMENTARY_LDFLAGS@

if HAVE_ECORE_AUDIO
LDADD += $(top_builddir)/src/lib/ecore_audio/libecore_audio.la
endif

if HAVE_EPHYSICS
LDADD += $(top_builddir)/src/lib/ephysics/libephysics.la
endif

if HAVE_ECORE_WL2
LDADD += \
$(top_builddir)/src/lib/ecore_wl2/libecore_wl2.la \
$(top_builddir)/src/lib/efl_wl/libefl_wl.la
endif

if HAVE_EEZE
LDADD += $(top_builddir)/src/lib/eeze/libeeze.la
endif

if HAVE_ELPUT
LDADD += $(top_builddir)/src/lib/elput/libelput.la
endif

if HAVE_ELUA
LDADD += $(top_builddir)/src/lib/elua/libelua.la
endif

if HAVE_ECORE_X
LDADD += $(top_builddir)/src/lib/ecore_x/libecore_x.la
endif

include ecore_avahi.mk
include ecore.mk
include edje.mk
include eet.mk
include efl_mono.mk
include eina_cxx.mk
include eina.mk
include eio.mk
include eldbus.mk
include elementary.mk
include elocation.mk
include emile.mk
include emotion.mk
include eolian_cxx.mk
include ephysics.mk
include ethumb_client.mk
include evas.mk

$(top_builddir)/src/bin/edje_cc$(EXEEXT):
	@test -f $(top_builddir)/src/bin/edje_cc$(EXEEXT) || $(MAKE) $(AM_MAKEFLAGS) -C ../ bin/edje_cc$(EXEEXT)

# this is a bit hacky
# currently all options from all directories are passed to all edje_cc cmds
# so if there are any files with the same name in different dirs it will fail
.edc.edj: $(top_builddir)/src/bin/edje_cc$(EXEEXT)
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) $< $(builddir)/$@

EXTRA_DIST += $(EDCS) $(DATA_FILES)
EDJS = $(EDCS:%.edc=%.edj)

examples: $(EXTRA_PROGRAMS) $(EDJS)

clean-local: $(CLEAN_EXAMPLES)
	rm -f $(EXTRA_PROGRAMS) $(EDJS)

install-examples: $(INSTALL_EXAMPLES)
uninstall-local: $(UNINSTALL_EXAMPLES)
.PHONY: $(PHONIES)

EXTRA_DIST += $(EDCS) $(DATA_FILES)

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
nodist_pkgdata_DATA = $(EDJS)
else
endif

check: ;

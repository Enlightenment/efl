MAINTAINERCLEANFILES = Makefile.in

include ../../Makefile_Edje_Helper.am

EDJE_CODEGEN = EFL_RUN_IN_TREE=1 $(top_builddir)/src/bin/edje/edje_codegen
EDJE_CODEGEN_FLAGS = --prefix=codegen_example

if ENABLE_MULTISENSE
SND_DIR = -sd $(top_srcdir)/src/examples/edje/
endif

MO_DIR = -md $(top_builddir)/src/examples/edje/

#put here all EDCs one needs to the examples
EDCS = \
animations.edc \
animations2.edc \
basic.edc \
basic2.edc \
bezier-transition-example.edc \
box.edc \
box_example.edc \
codegen.edc \
color-class.edc \
drag.edc \
dynamic_multiseat.edc \
edje-group.edc \
edje-nested.edc \
edje-threshold.edc \
edje_example.edc \
embryo_custom_state.edc \
embryo_pong.edc \
embryo_run_program.edc \
embryo_set_state.edc \
embryo_set_state_anim.edc \
embryo_set_text.edc \
embryo_timer.edc \
embryo_tween_anim.edc \
entry.edc \
external_elm_anchorblock.edc \
external_elm_button.edc \
external_elm_check.edc \
external_elm_panes.edc \
external_emotion_elm.edc \
lua_script.edc \
messages_echo.edc \
multiseat.edc \
perspective.edc \
signals-messages.edc \
signalsBubble.edc \
sigtest.edc \
svg.edc \
swallow.edc \
table.edc \
text.edc \
textblock-hyphen.edc \
toggle_using_filter.edc

DIST_EDCS = $(EDCS)

MULTISENSE_EDCS = multisense.edc
if ENABLE_MULTISENSE
EDCS += $(MULTISENSE_EDCS)
endif
DIST_EDCS += $(MULTISENSE_EDCS)

PHYSICS_EDCS = \
physics_3d.edc \
physics_actions.edc \
physics_backcull.edc \
physics_basic.edc \
physics_complex.edc \
physics_soft_bodies.edc
if HAVE_EPHYSICS
EDCS += $(PHYSICS_EDCS)
endif
DIST_EDCS += $(PHYSICS_EDCS)

# Support for SVG
EDJE_CC_FLAGS += -beta

.edc.edj:
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) $(SND_DIR) $(MO_DIR) $< $(builddir)/$(@F)

text.edj: text.edc
	@cp -ua $(top_srcdir)/src/examples/edje/en_IN/ $(top_builddir)/src/examples/edje/ || true
	@cp -ua $(top_srcdir)/src/examples/edje/hi_IN/ $(top_builddir)/src/examples/edje/ || true
	@cp -ua $(top_srcdir)/src/examples/edje/ta_IN/ $(top_builddir)/src/examples/edje/ || true
	@chmod -R +w $(top_builddir)/src/examples/edje/en_IN/ || true
	@chmod -R +w $(top_builddir)/src/examples/edje/hi_IN/ || true
	@chmod -R +w $(top_builddir)/src/examples/edje/ta_IN/ || true
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) $(SND_DIR) $(MO_DIR) $(top_srcdir)/src/examples/edje/text.edc $(top_builddir)/src/examples/edje/text.edj

EDJS = $(EDCS:%.edc=%.edj)

DATA_FILES = \
bubble.png \
bubble-blue.png \
red.png \
test.png \
Vera.ttf \
duck.wav \
dst.svg \
atom.svg \
bojo.svg \
bzrfeed.svg \
yinyang.svg \
ubuntu.svg \
duke.svg \
cartman.svg \
ibm.svg \
hg0.svg \
fsm.svg \
osa.svg \
wikimedia.svg \
scion.svg \
eee.svg \
google.svg \
tiger.svg \
venus.svg \
lineargrad1.svg \
radialgrad1.svg \
yadis.svg \
favorite_on.svg \
favorite_off.svg \
batman1.svg \
batman2.svg \
en_IN/domain_name.po \
hi_IN/domain_name.po \
ta_IN/domain_name.mo \
Makefile.examples

SRCS = \
animations2.c \
edje-animations.c \
edje-basic.c \
edje-basic2.c \
edje-box.c \
edje-box2.c \
edje-codegen-example.c \
edje-color-class.c \
edje-drag.c \
edje-dynamic-multiseat.c \
edje-edit-part-box.c \
edje-entry.c \
edje-multiseat.c \
edje-multisense.c \
edje-perspective.c \
edje-signals-messages.c \
edje-swallow.c \
edje-swallow2.c \
edje-table.c \
edje-text.c \
edje-textblock-hyphenation.c \
edje_example.c \
signals2.c \
sigtest.c \
svg-test.c

EXTRA_DIST = $(DIST_EDCS) $(DATA_FILES)

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I. \
-I$(builddir) \
-I$(top_srcdir)/src/lib/eina \
-I$(top_builddir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_builddir)/src/lib/eo \
-I$(top_srcdir)/src/lib/eet \
-I$(top_builddir)/src/lib/eet \
-I$(top_srcdir)/src/lib/evas \
-I$(top_builddir)/src/lib/evas \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore_evas \
-I$(top_srcdir)/src/lib/ecore_file \
-I$(top_builddir)/src/lib/ecore_file \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_imf \
-I$(top_builddir)/src/lib/ecore_imf \
-I$(top_srcdir)/src/lib/embryo \
-I$(top_builddir)/src/lib/embryo \
-I$(top_srcdir)/src/lib/edje \
-I$(top_srcdir)/src/lib/edje/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/edje/examples\" \
@EDJE_CFLAGS@

CODEGEN_GENERATED = \
codegen_example_generated.c \
codegen_example_generated.h

codegen_example_generated.c codegen_example_generated.h: codegen.edj
	$(AM_V_GEN)$(EDJE_CODEGEN) $(EDJE_CODEGEN_FLAGS) codegen.edj example_group $(builddir)/codegen_example_generated.c $(builddir)/codegen_example_generated.h

edje-codegen-example.$(OBJEXT): $(CODEGEN_GENERATED)

edje_codegen_example_SOURCES = \
edje-codegen-example.c
nodist_edje_codegen_example_SOURCES = \
codegen_example_generated.c \
codegen_example_generated.h

EXTRA_PROGRAMS = \
animations2 \
edje-animations \
edje-basic \
edje-basic2 \
edje-box \
edje-box2 \
edje-codegen-example \
edje-color-class \
edje-drag \
edje-dynamic-multiseat \
edje-edit-part-box \
edje-entry \
edje-multiseat \
edje-perspective \
edje-signals-messages \
edje-swallow \
edje-swallow2 \
edje-table \
edje-text \
edje-textblock-hyphenation \
edje_example \
signals2 \
sigtest \
svg-test

if ENABLE_MULTISENSE
EXTRA_PROGRAMS += edje-multisense
endif

LDADD = \
$(top_builddir)/src/lib/eina/libeina.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/eet/libeet.la \
$(top_builddir)/src/lib/evas/libevas.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
$(top_builddir)/src/lib/ecore_file/libecore_file.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
$(top_builddir)/src/lib/ecore_imf/libecore_imf.la \
$(top_builddir)/src/lib/ecore_imf_evas/libecore_imf_evas.la \
$(top_builddir)/src/lib/embryo/libembryo.la \
$(top_builddir)/src/lib/edje/libedje.la \
@EDJE_LDFLAGS@

if HAVE_EPHYSICS
LDADD += $(top_builddir)/src/lib/ephysics/libephysics.la
endif

examples: $(EXTRA_PROGRAMS) $(EDJS)

clean-local:
	rm -f $(EXTRA_PROGRAMS) $(EDJS) $(CODEGEN_GENERATED)
	if [ $(top_builddir)/ != $(top_srcdir)/  ]; then \
		rm -f $(top_builddir)/src/examples/edje/hi_IN/domain_name.po; \
		rm -f $(top_builddir)/src/examples/edje/ta_IN/domain_name.mo; \
		rm -f $(top_builddir)/src/examples/edje/en_IN/domain_name.po; \
	fi

install-examples: $(EDJS)
	$(MKDIR_P) $(DESTDIR)$(datadir)/edje/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(SRCS) $(DIST_EDCS) $(DATA_FILES) $(DESTDIR)$(datadir)/edje/examples
	cd $(builddir) && $(install_sh_DATA) -c $(EDJS) $(DESTDIR)$(datadir)/edje/examples

uninstall-local:
	for f in $(SRCS) $(DIST_EDCS) $(DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/edje/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
if BUILD_LOADER_PNG
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
nodist_pkgdata_DATA = $(EDJS)
endif
endif

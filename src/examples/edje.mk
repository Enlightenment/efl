EDJE_CODEGEN = EFL_RUN_IN_TREE=1 $(top_builddir)/src/bin/edje/edje_codegen
EDJE_CODEGEN_FLAGS = --prefix=codegen_example

#put here all EDCs one needs to the examples
EDJE_EDCS = \
edje/animations.edc \
edje/animations2.edc \
edje/basic.edc \
edje/basic2.edc \
edje/bezier-transition-example.edc \
edje/box.edc \
edje/box_example.edc \
edje/codegen.edc \
edje/color-class.edc \
edje/drag.edc \
edje/dynamic_multiseat.edc \
edje/edje-group.edc \
edje/edje-nested.edc \
edje/edje-threshold.edc \
edje/edje_example.edc \
edje/embryo_custom_state.edc \
edje/embryo_pong.edc \
edje/embryo_run_program.edc \
edje/embryo_set_state.edc \
edje/embryo_set_state_anim.edc \
edje/embryo_set_text.edc \
edje/embryo_timer.edc \
edje/embryo_tween_anim.edc \
edje/entry.edc \
edje/external_elm_anchorblock.edc \
edje/external_elm_button.edc \
edje/external_elm_check.edc \
edje/external_elm_panes.edc \
edje/external_emotion_elm.edc \
edje/focus.edc \
edje/lua_script.edc \
edje/messages_echo.edc \
edje/multiseat.edc \
edje/multiseat_custom_names.edc \
edje/multiseat_filter.edc \
edje/perspective.edc \
edje/signals-messages.edc \
edje/signalsBubble.edc \
edje/sigtest.edc \
edje/svg.edc \
edje/swallow.edc \
edje/table.edc \
edje/text.edc \
edje/textblock-hyphen.edc \
edje/toggle_using_filter.edc

MULTISENSE_EDCS = edje/multisense.edc
if ENABLE_MULTISENSE
EDJE_EDCS += $(MULTISENSE_EDCS)
endif

PHYSICS_EDCS = \
edje/physics_3d.edc \
edje/physics_actions.edc \
edje/physics_backcull.edc \
edje/physics_basic.edc \
edje/physics_complex.edc \
edje/physics_soft_bodies.edc
if HAVE_EPHYSICS
EDJE_EDCS += $(PHYSICS_EDCS)
endif

EDJE_DATA_FILES = \
edje/bubble.png \
edje/bubble-blue.png \
edje/red.png \
edje/test.png \
edje/Vera.ttf \
edje/duck.wav \
edje/dst.svg \
edje/atom.svg \
edje/bojo.svg \
edje/bzrfeed.svg \
edje/yinyang.svg \
edje/ubuntu.svg \
edje/duke.svg \
edje/cartman.svg \
edje/ibm.svg \
edje/hg0.svg \
edje/fsm.svg \
edje/osa.svg \
edje/wikimedia.svg \
edje/scion.svg \
edje/eee.svg \
edje/google.svg \
edje/tiger.svg \
edje/venus.svg \
edje/lineargrad1.svg \
edje/radialgrad1.svg \
edje/yadis.svg \
edje/favorite_on.svg \
edje/favorite_off.svg \
edje/batman1.svg \
edje/batman2.svg \
edje/en_IN/domain_name.po \
edje/hi_IN/domain_name.po \
edje/ta_IN/domain_name.mo \
edje/Makefile.examples

EDJE_SRCS = \
edje/animations2.c \
edje/edje-animations.c \
edje/edje-basic.c \
edje/edje-basic2.c \
edje/edje-box.c \
edje/edje-box2.c \
edje/edje-codegen-example.c \
edje/edje-color-class.c \
edje/edje-drag.c \
edje/edje-dynamic-multiseat.c \
edje/edje-edit-part-box.c \
edje/edje-entry.c \
edje/edje-focus.c \
edje/edje-multiseat.c \
edje/edje-multiseat-custom-names.c \
edje/edje-multiseat-filter.c \
edje/edje-multisense.c \
edje/edje-perspective.c \
edje/edje-signals-messages.c \
edje/edje-swallow.c \
edje/edje-swallow2.c \
edje/edje-table.c \
edje/edje-text.c \
edje/edje-textblock-hyphenation.c \
edje/edje_example.c \
edje/signals2.c \
edje/sigtest.c \
edje/svg-test.c

EDJE_CPPFLAGS = \
$(AM_CPPFLAGS) \
-I$(top_builddir)/src/examples/edje \
-DPACKAGE_DATA_DIR=\"$(datadir)/edje/examples\"

CODEGEN_GENERATED = \
edje/codegen_example_generated.c \
edje/codegen_example_generated.h

DISTCLEANFILES += $(CODEGEN_GENERATED)

$(CODEGEN_GENERATED): edje/codegen.edj
	$(AM_V_GEN)$(EDJE_CODEGEN) $(EDJE_CODEGEN_FLAGS) edje/codegen.edj example_group $(builddir)/edje/codegen_example_generated.c $(builddir)/edje/codegen_example_generated.h

edje_edje_codegen_example_SOURCES = \
edje/edje-codegen-example.c
edje_edje_codegen_example_CPPFLAGS = $(EDJE_CPPFLAGS)

nodist_edje_edje_codegen_example_CPPFLAGS = $(EDJE_CPPFLAGS)
nodist_edje_edje_codegen_example_SOURCES = \
$(CODEGEN_GENERATED)

if HAVE_AM_16
edje/edje_codegen_example-edje-codegen-example.$(OBJEXT): $(CODEGEN_GENERATED)
else
edje/edje_edje_codegen_example-edje-codegen-example.$(OBJEXT): $(CODEGEN_GENERATED)
endif

if BUILD_LOADER_PNG
EXTRA_PROGRAMS += \
edje/animations2 \
edje/edje-animations \
edje/edje-basic \
edje/edje-basic2 \
edje/edje-box \
edje/edje-box2 \
edje/edje-codegen-example \
edje/edje-color-class \
edje/edje-drag \
edje/edje-dynamic-multiseat \
edje/edje-edit-part-box \
edje/edje-entry \
edje/edje-focus \
edje/edje-multiseat \
edje/edje-multiseat-custom-names \
edje/edje-multiseat-filter \
edje/edje-perspective \
edje/edje-signals-messages \
edje/edje-swallow \
edje/edje-swallow2 \
edje/edje-table \
edje/edje-text \
edje/edje-textblock-hyphenation \
edje/edje_example \
edje/signals2 \
edje/sigtest \
edje/svg-test

if ENABLE_MULTISENSE
EXTRA_PROGRAMS += edje/edje-multisense
endif
endif

DATA_FILES += $(EDJE_DATA_FILES)
EDJE_EDJS = $(EDJE_EDCS:%.edc=%.edj)
EDCS += $(EDJE_EDCS)
if ! HAVE_EPHYSICS
EXTRA_DIST += $(PHYSICS_EDCS)
endif

if ENABLE_MULTISENSE
EDJE_CC_FLAGS += -sd $(top_srcdir)/src/examples/edje
endif

EDJE_CC_FLAGS += -id $(top_srcdir)/src/examples/edje -md $(top_srcdir)/src/examples/edje -fd $(top_srcdir)/src/examples/edje -beta

CLEANFILES += \
$(top_builddir)/src/examples/edje/en_IN/domain_name.mo \
$(top_builddir)/src/examples/edje/hi_IN/domain_name.mo

install-examples-edje: examples
	$(MKDIR_P) $(DESTDIR)$(datadir)/edje/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(EDJE_SRCS) $(EDJE_EDCS) $(EDJE_DATA_FILES) $(DESTDIR)$(datadir)/edje/examples
	cd $(builddir) && $(install_sh_DATA) -c $(EDJE_EDJS) $(DESTDIR)$(datadir)/edje/examples

uninstall-local-edje:
	for f in $(EDJE_SRCS) $(EDJE_EDCS) $(EDJE_DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/edje/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-edje
UNINSTALL_EXAMPLES += uninstall-local-edje

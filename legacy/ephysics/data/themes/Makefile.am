MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_FLAGS_VERBOSE_ =
EDJE_FLAGS_VERBOSE_0 =
EDJE_FLAGS_VERBOSE_1 = -v
EDJE_FLAGS = $(EDJE_FLAGS_$(V)) -id $(top_srcdir)/data/themes/images -fd $(top_srcdir)/data/themes/fonts

filesdir = $(datadir)/ephysics/

files_DATA = ephysics_test.edj ephysics_sandbox.edj

TEST_FILES = \
	ephysics_test.edc \
	balls.edc \
	buttons.edc \
	cubes.edc \
	effects.edc \
	ephysics_test.edc \
	frame.edc \
	list.edc \
	loading_bar.edc \
	others.edc \
	shadows.edc \
	shapes.edc \
	spinner.edc \
	images/arrows.png \
	images/background.jpg \
	images/ball-blue.png \
	images/ball-blue70.png \
	images/ball-green.png \
	images/ball-green70.png \
	images/ball-red.png \
	images/ball-red70.png \
	images/bg_floor.jpg \
	images/brown-pole.png \
	images/bt_jump.png \
	images/bt_jump_pressed.png \
	images/bt_restart.png \
	images/bt_restart_pressed.png \
	images/bt_shoot.png \
	images/bt_shoot_pressed.png \
	images/bt_stop.png \
	images/bt_stop_pressed.png \
	images/collisionL.png \
	images/cube-blue.png \
	images/cube-purple.png \
	images/green-flag.png \
	images/hexagon.png \
	images/list-item.png \
	images/list-item-corner.png \
	images/list-item-corner-pressed.png \
	images/list-item-pressed.png \
	images/loading_bar.png \
	images/loading_knob.png \
	images/pentagon.png \
	images/shadow-ball.png \
	images/shadow-cube.png \
	images/spinner_base.png \
	images/spinner_bt_left.png \
	images/spinner_bt_left_pressed.png \
	images/spinner_bt_right.png \
	images/spinner_bt_right_pressed.png \
	images/star.png

SANDBOX_FILES = \
	ephysics_sandbox.edc \
	sandbox_misc.edc \
	images/background.jpg \
	images/ball-custom.png \
	images/ball-concrete.png \
	images/ball-iron.png \
	images/ball-plastic.png \
	images/ball-polystyrene.png \
	images/ball-rubber.png \
	images/ball-wood.png

EXTRA_DIST = \
    $(TEST_FILES) \
    $(SANDBOX_FILES)

ephysics_test.edj: Makefile $(TEST_FILES)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/themes/ephysics_test.edc \
	$(top_builddir)/data/themes/ephysics_test.edj

ephysics_sandbox.edj: Makefile $(SANDBOX_FILES)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/themes/ephysics_sandbox.edc \
	$(top_builddir)/data/themes/ephysics_sandbox.edj

clean-local:
	rm -f *.edj

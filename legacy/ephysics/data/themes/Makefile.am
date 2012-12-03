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
	pages.edc \
	shadows.edc \
	shapes.edc \
	spinner.edc \
	toggle.edc \
	images/arrows.png \
	images/background.jpg \
	images/ball-blue.png \
	images/ball-blue70.png \
	images/ball-green.png \
	images/ball-green70.png \
	images/ball-red.png \
	images/ball-red70.png \
	images/door-back.png \
	images/door-front.png \
	images/door-post.png \
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
	images/coin0.png \
	images/coin1.png \
	images/collisionL.png \
	images/cube-blue.png \
	images/cube-purple.png \
	images/cube-purple420.png \
	images/cube_back.png \
	images/cube_bottom.png \
	images/cube_front.png \
	images/cube_left.png \
	images/cube_right.png \
	images/cube_top.png \
	images/dice1.png \
	images/dice2.png \
	images/dice3.png \
	images/dice4.png \
	images/dice5.png \
	images/dice6.png \
	images/green-flag.png \
	images/hexagon.png \
	images/list-item.png \
	images/list-item-corner.png \
	images/list-item-corner-pressed.png \
	images/list-item-pressed.png \
	images/loading_bar.png \
	images/loading_knob.png \
	images/page_00.png \
	images/page_01.png \
	images/page_02.png \
	images/page_03.png \
	images/pentagon.png \
	images/shadow-ball.png \
	images/shadow-cube.png \
	images/spinner_base.png \
	images/spinner_bt_left.png \
	images/spinner_bt_left_pressed.png \
	images/spinner_bt_right.png \
	images/spinner_bt_right_pressed.png \
	images/star.png \
	images/tog_base_off.png \
	images/tog_base_on.png \
	images/tog_bt_off.png \
	images/tog_bt_on.png

SANDBOX_FILES = \
	ephysics_sandbox.edc \
	sandbox_misc.edc \
	images/background.jpg \
	images/ball-concrete.png \
	images/ball-iron.png \
	images/ball-plastic.png \
	images/ball-polystyrene.png \
	images/ball-red70.png \
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

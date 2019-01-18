EPHYSICS_SRCS = \
ephysics/ephysics_logo.c \
ephysics/ephysics_test.h \
ephysics/test_bouncing_ball.c \
ephysics/test_bouncing_text.c \
ephysics/test.c \
ephysics/test_camera.c \
ephysics/test_camera_track.c \
ephysics/test_colliding_balls.c \
ephysics/test_collision_detection.c \
ephysics/test_collision_filter.c \
ephysics/test_constraint.c \
ephysics/test_delete.c \
ephysics/test_forces.c \
ephysics/test_growing_balls.c \
ephysics/test_no_gravity.c \
ephysics/test_rotating_forever.c \
ephysics/test_shapes.c \
ephysics/test_sleeping_threshold.c \
ephysics/test_slider.c \
ephysics/test_velocity.c

# NOTE: neither ephysics_test or ephysics_logo won't be built as they needs Elementary
# TODO: convert these to bare ecore_evas, at least a reduced set to have compile-able examples

# TODO: when EXTRA_PROGRAMS is populated, remove EPHYSICS_SRCS from here
DATA_FILES += $(EPHYSICS_SRCS)

# ephysics_test_SOURCES = \
# ephysics_test.h \
# test_bouncing_ball.c \
# test_bouncing_text.c \
# test.c \
# test_camera.c \
# test_camera_track.c \
# test_colliding_balls.c \
# test_collision_detection.c \
# test_collision_filter.c \
# test_constraint.c \
# test_delete.c \
# test_forces.c \
# test_growing_balls.c \
# test_no_gravity.c \
# test_rotating_forever.c \
# test_shapes.c \
# test_sleeping_threshold.c \
# test_slider.c \
# test_velocity.c

install-examples-ephysics:
	$(MKDIR_P) $(DESTDIR)$(datadir)/ephysics/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(EPHYSICS_SRCS) $(DESTDIR)$(datadir)/ephysics/examples

uninstall-local-ephysics:
	for f in $(EPHYSICS_SRCS) ; do \
	  rm -f $(DESTDIR)$(datadir)/ephysics/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-ephysics
UNINSTALL_EXAMPLES += uninstall-local-ephysics

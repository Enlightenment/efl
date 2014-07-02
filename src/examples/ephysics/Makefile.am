MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I. \
-I$(top_srcdir)/src/lib/eina \
-I$(top_builddir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_builddir)/src/lib/eo \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/evas \
-I$(top_builddir)/src/lib/evas \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore_evas \
-I$(top_srcdir)/src/lib/ephysics \
-I$(top_builddir)/src/lib/ephysics

LDADD = \
$(top_builddir)/src/lib/eina/libeina.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/evas/libevas.la \
$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
$(top_builddir)/src/lib/ephysics/libephysics.la \
@EPHYSICS_LDFLAGS@

SRCS = \
ephysics_logo.c \
ephysics_test.h \
test_bouncing_ball.c \
test_bouncing_text.c \
test.c \
test_camera.c \
test_camera_track.c \
test_colliding_balls.c \
test_collision_detection.c \
test_collision_filter.c \
test_constraint.c \
test_delete.c \
test_forces.c \
test_growing_balls.c \
test_no_gravity.c \
test_rotating_forever.c \
test_shapes.c \
test_sleeping_threshold.c \
test_slider.c \
test_velocity.c

EXTRA_PROGRAMS =
# NOTE: neither ephysics_test or ephysics_logo won't be built as they needs Elementary
# TODO: convert these to bare ecore_evas, at least a reduced set to have compile-able examples

# TODO: when EXTRA_PROGRAMS is populated, remove SRCS from here
EXTRA_DIST = $(SRCS)

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

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	mkdir -p $(datadir)/ephysics/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(SRCS) $(datadir)/ephysics/examples

uninstall-local:
	for f in $(SRCS) ; do \
	  rm -f $(datadir)/ephysics/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif

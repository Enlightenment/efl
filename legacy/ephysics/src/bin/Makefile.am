MAINTAINERCLEANFILES = Makefile.in

if BUILD_TESTS
AM_CPPFLAGS = \
    -I$(top_srcdir) \
    -I$(top_srcdir)/src/lib \
    -I$(top_builddir)/src/lib \
    -I$(top_srcdir)/src/bin \
    -DPACKAGE_BIN_DIR=\"$(bindir)\" \
    -DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
    @EPHYSICS_TEST_CFLAGS@

bin_PROGRAMS = ephysics_test ephysics_logo

EXTRA_PROGRAMS = ephysics_test ephysics_logo

ephysics_test_SOURCES = \
test.c \
test_bouncing_ball.c \
test_bouncing_text.c \
test_colliding_balls.c \
test_collision_detection.c \
test_collision_speed.c \
test_constraint.c \
test_falling_letters.c \
test_jumping_balls.c \
test_rotate.c \
test_velocity.c

ephysics_logo_SOURCES = \
ephysics_logo.c

ephysics_test_LDADD = \
    $(top_builddir)/src/lib/libephysics.la \
    @EPHYSICS_TEST_LIBS@

ephysics_logo_LDADD = \
    $(top_builddir)/src/lib/libephysics.la \
    @EPHYSICS_TEST_LIBS@

ephysics_logo_LDFLAGS =

ephysics_test_LDFLAGS =

EXTRA_DIST = ephysics_test.h
endif

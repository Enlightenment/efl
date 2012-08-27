MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
@EVIL_CFLAGS@ \
@EPHYSICS_CFLAGS@

lib_LTLIBRARIES = libephysics.la

includes_HEADERS = EPhysics.h
includesdir = $(includedir)/ephysics-@VMAJ@

base_sources = \
    ephysics_body.cpp \
    ephysics_camera.cpp \
    ephysics_constraints.cpp \
    ephysics_main.cpp \
    ephysics_shape.cpp \
    ephysics_world.cpp

libephysics_la_SOURCES = $(base_sources)

libephysics_la_LIBADD = @EPHYSICS_LIBS@ @EVIL_LIBS@ -lm
libephysics_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@

EXTRA_DIST = ephysics_private.h ephysics_trimesh.h

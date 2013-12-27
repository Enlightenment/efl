MAINTAINERCLEANFILES = Makefile.in

include ../../Makefile_Edje_Helper.am

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/evas \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/evas \
@EVAS_CFLAGS@

EDCS = aspect.edc

EXTRA_PROGRAMS =

AM_CPPFLAGS += -I$(top_srcdir)/src/modules/evas/engines/buffer

EXTRA_PROGRAMS += evas_buffer_simple
evas_buffer_simple_SOURCES = evas-buffer-simple.c
evas_buffer_simple_LDADD = $(top_builddir)/src/lib/evas/libevas.la @EVAS_LDFLAGS@

EXTRA_PROGRAMS += evas_init_shutdown
evas_init_shutdown_SOURCES = evas-init-shutdown.c
evas_init_shutdown_LDADD = $(top_builddir)/src/lib/evas/libevas.la @EVAS_LDFLAGS@

ECORE_EVAS_COMMON_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/evas \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/efl \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/evas \
-I$(top_builddir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore_evas \
-DPACKAGE_EXAMPLES_DIR=\".\" \
@EVAS_CFLAGS@

ECORE_EVAS_COMMON_LDADD = \
$(top_builddir)/src/lib/eina/libeina.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
$(top_builddir)/src/lib/evas/libevas.la \
@EVAS_LDFLAGS@

# EDJE_COMMON_* is similar to src/examples/edje/Makefile.am
EDJE_COMMON_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
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
-DPACKAGE_EXAMPLES_DIR=\".\" \
@EVAS_CFLAGS@

EDJE_COMMON_LDADD = \
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
@EVAS_LDFLAGS@

EXTRA_PROGRAMS += evas_images
evas_images_SOURCES = evas-images.c
evas_images_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_images_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_events
evas_events_SOURCES = evas-events.c
evas_events_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_events_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_object_manipulation
evas_object_manipulation_SOURCES = evas-object-manipulation.c
evas_object_manipulation_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_object_manipulation_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_object_manipulation-eo
evas_object_manipulation_eo_SOURCES = evas-object-manipulation-eo.c
evas_object_manipulation_eo_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_object_manipulation_eo_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_hints
evas_hints_SOURCES = evas-hints.c
evas_hints_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_hints_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_stacking
evas_stacking_SOURCES = evas-stacking.c
evas_stacking_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_stacking_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_images2
evas_images2_SOURCES = evas-images2.c
evas_images2_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_images2_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_images3
evas_images3_SOURCES = evas-images3.c
evas_images3_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_images3_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_text
evas_text_SOURCES = evas-text.c
evas_text_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_text_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_smart_object
evas_smart_object_SOURCES = evas-smart-object.c
evas_smart_object_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_smart_object_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_smart_interface
evas_smart_interface_SOURCES = evas-smart-interface.c
evas_smart_interface_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_smart_interface_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_box
evas_box_SOURCES = evas-box.c
evas_box_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_box_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_map_utils
evas_map_utils_SOURCES = evas-map-utils.c
evas_map_utils_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_map_utils_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_table
evas_table_SOURCES = evas-table.c
evas_table_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_table_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_multi_touch
evas_multi_touch_SOURCES = evas-multi-touch.c
evas_multi_touch_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_multi_touch_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_3d_cube
evas_3d_cube_SOURCES = evas-3d-cube.c
evas_3d_cube_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_3d_cube_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_3d_cube2
evas_3d_cube2_SOURCES = evas-3d-cube2.c
evas_3d_cube2_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_3d_cube2_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

.edc.edj:
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) $< $(builddir)/$(@F)

EXTRA_PROGRAMS += evas_aspect_hints
evas_aspect_hints_DEPS = $(srcdir)/aspect.edc
evas_aspect_hints_SOURCES = evas-aspect-hints.c
evas_aspect_hints_CPPFLAGS = $(EDJE_COMMON_CPPFLAGS)
evas_aspect_hints_LDADD = $(EDJE_COMMON_LDADD)

EDJS = $(EDCS:%.edc=%.edj)

SRCS = \
evas-aspect-hints.c \
evas-box.c \
evas-buffer-simple.c \
evas-events.c \
evas-hints.c \
evas-images.c \
evas-images2.c \
evas-images3.c \
evas-init-shutdown.c \
evas-map-utils.c \
evas-object-manipulation-eo.c \
evas-object-manipulation.c \
evas-smart-interface.c \
evas-smart-object.c \
evas-stacking.c \
evas-table.c \
evas-multi-touch.c \
evas-text.c

DATA_FILES = \
enlightenment.png \
red.png \
im1.png \
cube1.png \
Makefile.examples

examples: $(EXTRA_PROGRAMS) $(EDJS)

clean-local:
	rm -f $(EXTRA_PROGRAMS) $(EDJS)

install-examples:
	mkdir -p $(datadir)/evas/examples
	$(install_sh_DATA) -c $(SRCS) $(EDCS) $(DATA_FILES) $(datadir)/evas/examples

uninstall-local:
	for f in $(SRCS) $(EDCS) $(DATA_FILES); do \
	  rm -f $(datadir)/evas/examples/$$f ; \
	done

EXTRA_DIST = $(EDCS) $(DATA_FILES)

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
nodist_pkgdata_DATA = $(EDJS)
endif

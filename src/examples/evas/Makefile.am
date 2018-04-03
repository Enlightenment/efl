MAINTAINERCLEANFILES = Makefile.in

include ../../Makefile_Edje_Helper.am

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I$(top_builddir)/src/lib/efl/interfaces \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/ector \
-I$(top_srcdir)/src/lib/evas \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/ector \
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
-I$(top_srcdir)/src/lib/ector \
-I$(top_srcdir)/src/lib/evas \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_file \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/efl \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/ector \
-I$(top_builddir)/src/lib/evas \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_file \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore_evas \
-DPACKAGE_EXAMPLES_DIR=\"$(top_srcdir)/src/examples/evas\" \
@EVAS_CFLAGS@

ECORE_EVAS_COMMON_LDADD = \
$(top_builddir)/src/lib/efl/libefl.la \
$(top_builddir)/src/lib/eina/libeina.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_file/libecore_file.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
$(top_builddir)/src/lib/ector/libector.la \
$(top_builddir)/src/lib/evas/libevas.la \
@EVAS_LDFLAGS@ -lm

# EDJE_COMMON_* is similar to src/examples/edje/Makefile.am
EDJE_COMMON_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/lib/eina \
-I$(top_builddir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_builddir)/src/lib/eo \
-I$(top_srcdir)/src/lib/eet \
-I$(top_builddir)/src/lib/eet \
-I$(top_srcdir)/src/lib/ector \
-I$(top_srcdir)/src/lib/evas \
-I$(top_builddir)/src/lib/ector \
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
-I$(top_builddir)/src/lib/edje \
-I$(top_srcdir)/src/lib/edje/include \
-I$(top_builddir)/src/lib/edje/include \
-DPACKAGE_EXAMPLES_DIR=\"$(top_srcdir)/src/examples/evas\" \
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

EXTRA_PROGRAMS += evas_event_filter
evas_event_filter_SOURCES = evas-event-filter.c
evas_event_filter_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_event_filter_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_multiseat_events
evas_multiseat_events_SOURCES = evas-multiseat-events.c
evas_multiseat_events_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_multiseat_events_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

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

EXTRA_PROGRAMS += evas_images4
evas_images4_SOURCES = evas-images4.c
evas_images4_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_images4_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_images5
evas_images5_SOURCES = evas-images5.c
evas_images5_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_images5_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_text
evas_text_SOURCES = evas-text.c
evas_text_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_text_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_textblock_obstacles
evas_textblock_obstacles_SOURCES = evas-textblock-obstacles.c
evas_textblock_obstacles_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_textblock_obstacles_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

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

EXTRA_PROGRAMS += evas_map_aa
evas_map_aa_SOURCES = evas-map-aa.c
evas_map_aa_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_map_aa_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_map_utils_eo
evas_map_utils_eo_SOURCES = evas-map-utils-eo.c
evas_map_utils_eo_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_map_utils_eo_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_map_aa_eo
evas_map_aa_eo_SOURCES = evas-map-aa-eo.c
evas_map_aa_eo_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_map_aa_eo_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_table
evas_table_SOURCES = evas-table.c
evas_table_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_table_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_transparent
evas_transparent_SOURCES = evas-transparent.c
evas_transparent_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_transparent_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_multi_touch
evas_multi_touch_SOURCES = evas-multi-touch.c
evas_multi_touch_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_multi_touch_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_cube
evas_canvas3d_cube_SOURCES = evas-3d-cube.c
evas_canvas3d_cube_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_cube_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_cube2
evas_canvas3d_cube2_SOURCES = evas-3d-cube2.c
evas_canvas3d_cube2_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_cube2_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_proxy
evas_canvas3d_proxy_SOURCES = evas-3d-proxy.c
evas_canvas3d_proxy_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_proxy_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_pick
evas_canvas3d_pick_SOURCES = evas-3d-pick.c
evas_canvas3d_pick_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_pick_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_md2
evas_canvas3d_md2_SOURCES = evas-3d-md2.c
evas_canvas3d_md2_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_md2_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_eet
evas_canvas3d_eet_SOURCES = evas-3d-eet.c
evas_canvas3d_eet_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_eet_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_ply
evas_canvas3d_ply_SOURCES = evas-3d-ply.c
evas_canvas3d_ply_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_ply_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_hull
evas_canvas3d_hull_SOURCES = evas-3d-hull.c
evas_canvas3d_hull_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_hull_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_fog
evas_canvas3d_fog_SOURCES = evas-3d-fog.c
evas_canvas3d_fog_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_fog_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_frustum
evas_canvas3d_frustum_SOURCES = evas-3d-frustum.c
evas_canvas3d_frustum_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_frustum_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_aabb
evas_canvas3d_aabb_SOURCES = evas-3d-aabb.c
evas_canvas3d_aabb_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_aabb_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_cube_rotate
evas_canvas3d_cube_rotate_SOURCES = evas-3d-cube-rotate.c
evas_canvas3d_cube_rotate_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_cube_rotate_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_shadows
evas_canvas3d_shadows_SOURCES = evas-3d-shadows.c
evas_canvas3d_shadows_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_shadows_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_shooter
evas_canvas3d_shooter_SOURCES = shooter/evas-3d-shooter.c \
				shooter/evas-3d-shooter-header.c \
				shooter/evas-3d-shooter-header.h \
				shooter/evas-3d-shooter-macros.h
evas_canvas3d_shooter_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_shooter_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_obj
evas_canvas3d_obj_SOURCES = evas-3d-obj.c
evas_canvas3d_obj_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_obj_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_blending
evas_canvas3d_blending_SOURCES = evas-3d-blending.c
evas_canvas3d_blending_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_blending_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_mmap
evas_canvas3d_mmap_SOURCES = evas-3d-mmap-set.c
evas_canvas3d_mmap_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_mmap_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_colorpick
evas_canvas3d_colorpick_SOURCES = evas-3d-colorpick.c
evas_canvas3d_colorpick_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_colorpick_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_parallax_occlusion
evas_canvas3d_parallax_occlusion_SOURCES = evas-3d-parallax-occlusion.c
evas_canvas3d_parallax_occlusion_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_parallax_occlusion_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_canvas3d_static_lod
evas_canvas3d_static_lod_SOURCES = evas-3d-static-lod.c
evas_canvas3d_static_lod_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_canvas3d_static_lod_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_gl
evas_gl_SOURCES = evas-gl.c
evas_gl_LDADD = $(ECORE_EVAS_COMMON_LDADD) @EFL_PTHREAD_LIBS@
evas_gl_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_vg_simple
evas_vg_simple_SOURCES = evas-vg-simple.c
evas_vg_simple_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_vg_simple_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_vg_batman
evas_vg_batman_SOURCES = evas-vg-batman.c
evas_vg_batman_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_vg_batman_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

.edc.edj:
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) $< $(builddir)/$(@F)

EXTRA_PROGRAMS += evas_aspect_hints
evas_aspect_hints_DEPS = $(srcdir)/aspect.edc
evas_aspect_hints_SOURCES = evas-aspect-hints.c
evas_aspect_hints_CPPFLAGS = $(EDJE_COMMON_CPPFLAGS)
evas_aspect_hints_LDADD = $(EDJE_COMMON_LDADD)

if HAVE_CXX

EXTRA_PROGRAMS += evas_cxx_rectangle
evas_cxx_rectangle_SOURCES = evas_cxx_rectangle.cc
evas_cxx_rectangle_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_cxx_rectangle_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS) @EINA_CXX_CFLAGS@ @EO_CXX_CFLAGS@ @EVAS_CXX_CFLAGS@ \
-I$(top_builddir)/src/lib/evas/canvas/ \
$(AM_CPPFLAGS) @EVAS_CFLAGS@

endif

if HAVE_CSHARP

EXTRA_PROGRAMS += \
    evas_mono_box_csharp \
    evas_mono_image_csharp \
    evas_mono_image2_csharp \
    evas_mono_rectangle_csharp \
    evas_mono_table_csharp \
    evas_mono_text_csharp


evas_mono_box_csharp_SOURCES = \
	evas_mono_box.cs

evas_mono_box_csharp$(EXEEXT): $(evas_mono_box_csharp_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

evas_mono_image_csharp_SOURCES = \
	evas_mono_image.cs

evas_mono_image_csharp$(EXEEXT): $(evas_mono_image_csharp_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

evas_mono_image2_csharp_SOURCES = \
	evas_mono_image2.cs

evas_mono_image2_csharp$(EXEEXT): $(evas_mono_image2_csharp_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

evas_mono_rectangle_csharp_SOURCES = \
	evas_mono_rectangle.cs

evas_mono_rectangle_csharp$(EXEEXT): $(evas_mono_rectangle_csharp_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

evas_mono_table_csharp_SOURCES = \
	evas_mono_table.cs

evas_mono_table_csharp$(EXEEXT): $(evas_mono_table_csharp_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

evas_mono_text_csharp_SOURCES = \
	evas_mono_text.cs

evas_mono_text_csharp$(EXEEXT): $(evas_mono_text_csharp_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

endif

EDJS = $(EDCS:%.edc=%.edj)

SRCS = \
evas-aspect-hints.c \
evas-box.c \
evas-buffer-simple.c \
evas-events.c \
evas-event-filter.c \
evas-hints.c \
evas-images.c \
evas-images2.c \
evas-images3.c \
evas-images4.c \
evas-images5.c \
evas-init-shutdown.c \
evas-map-utils.c \
evas-map-aa.c \
evas-object-manipulation-eo.c \
evas-object-manipulation.c \
evas-smart-interface.c \
evas-smart-object.c \
evas-stacking.c \
evas-table.c \
evas-multi-touch.c \
evas-text.c \
evas-vg-simple.c \
evas-vg-batman.c

DATA_FILES = \
resources/images/enlightenment.png \
resources/images/red.png \
resources/images/im1.png \
resources/images/cube1.png \
resources/images/normal_lego.png \
resources/images/sonic.png \
resources/images/sweet_home_reversed.png \
resources/images/wood.jpg \
resources/images/rocks.jpg \
resources/images/four_NM_height.tga \
resources/images/rocks_NM_height.tga \
resources/images/grid.png \
resources/images/grid_n.png \
resources/images/billboard.png \
resources/models/mesh_for_mmap.eet \
resources/models/mesh_for_mmap.md2 \
resources/models/mesh_for_mmap.obj \
resources/models/mesh_for_mmap.ply \
resources/models/Normal_UVs_Colors.ply \
resources/models/Normal_UVs_NoColors.ply \
resources/models/Normal_NoUVs_Colors.ply \
resources/models/Normal_NoUVs_NoColors.ply \
resources/models/NoNormal_UVs_Colors.ply \
resources/models/NoNormal_UVs_NoColors.ply \
resources/models/NoNormal_NoUVs_Colors.ply \
resources/models/NoNormal_NoUVs_NoColors.ply \
resources/models/sonic.md2 \
resources/models/sweet_home.obj \
resources/models/sweet_home_only_vertex_coords.obj \
resources/models/sweet_home_without_normals.obj \
resources/models/sweet_home_without_tex_coords.obj \
resources/models/lod/iso_sphere0.obj \
resources/models/lod/iso_sphere1.obj \
resources/models/lod/iso_sphere2.obj \
resources/models/convex_hull/column_blender_ch.ply \
resources/models/convex_hull/column.ply \
resources/models/convex_hull/cube_blender_ch.ply \
resources/models/convex_hull/eagle_blender_ch.ply \
resources/models/convex_hull/home_blender_ch.obj \
resources/models/convex_hull/plain.ply \
resources/models/convex_hull/plain_blender_ch.ply \
resources/models/convex_hull/sphere_blender_ch.ply \
resources/models/convex_hull/torus_blender_ch.ply \
shooter/assets/images/brick-stone_n.jpg \
shooter/assets/images/brick-stone.jpg \
shooter/assets/images/bricks_n.jpg \
shooter/assets/images/eagle.png \
shooter/assets/images/snake.png \
shooter/assets/images/snake_n.png \
shooter/assets/images/bricks.jpg \
shooter/assets/images/soldier.png \
shooter/assets/images/gazebo.png \
shooter/assets/images/warrior.png \
shooter/assets/images/gazebo_t.png \
shooter/assets/images/gazebo_t_t.png \
shooter/assets/images/gazebo_t_n.png \
shooter/assets/images/gazebo_b_n.png \
shooter/assets/models/warrior.md2 \
shooter/assets/models/gazebo_b.md2 \
shooter/assets/models/gazebo_t.md2 \
shooter/assets/models/eagle.md2 \
shooter/assets/models/snake.md2 \
shooter/assets/models/soldier_jump.md2 \
shooter/assets/models/soldier.md2 \
shooter/assets/models/tommy.ply \
filters/filter_blend.lua \
filters/filter_blur.lua \
filters/filter_bump.lua \
filters/filter_curve.lua \
filters/filter_example_1.lua \
filters/filter_grow.lua \
filters/filter_mask.lua \
filters/filter_padding.lua \
filters/filter_transform.lua \
Makefile.examples \
evas-common.h

examples: $(EXTRA_PROGRAMS) $(EDJS)

clean-local:
	rm -f $(EXTRA_PROGRAMS) $(EDJS)

install-examples:
	$(MKDIR_P) $(DESTDIR)$(datadir)/evas/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(SRCS) $(EDCS) $(DATA_FILES) $(DESTDIR)$(datadir)/evas/examples

uninstall-local:
	for f in $(SRCS) $(EDCS) $(DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/evas/examples/$$f ; \
	done

EXTRA_DIST = $(EDCS) $(DATA_FILES)

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
nodist_pkgdata_DATA = $(EDJS)
endif

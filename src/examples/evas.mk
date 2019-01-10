EVAS_EDCS = evas/aspect.edc

EVAS_BUFFER_COMMON_CPPFLAGS = \
$(AM_CPPFLAGS) \
-I$(top_srcdir)/src/modules/evas/engines/buffer

EXTRA_PROGRAMS += evas/evas_buffer_simple
evas_evas_buffer_simple_SOURCES = evas/evas-buffer-simple.c
evas_evas_buffer_simple_CPPFLAGS = $(EVAS_BUFFER_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_init_shutdown
evas_evas_init_shutdown_SOURCES = evas/evas-init-shutdown.c

ECORE_EVAS_COMMON_CPPFLAGS = \
$(AM_CPPFLAGS) \
-DPACKAGE_EXAMPLES_DIR=\"$(top_srcdir)/src/examples/evas\"

EXTRA_PROGRAMS += evas/evas_images
evas_evas_images_SOURCES = evas/evas-images.c
evas_evas_images_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_events
evas_evas_events_SOURCES = evas/evas-events.c
evas_evas_events_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_event_filter
evas_evas_event_filter_SOURCES = evas/evas-event-filter.c
evas_evas_event_filter_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_multiseat_events
evas_evas_multiseat_events_SOURCES = evas/evas-multiseat-events.c
evas_evas_multiseat_events_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_object_manipulation
evas_evas_object_manipulation_SOURCES = evas/evas-object-manipulation.c
evas_evas_object_manipulation_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_object_manipulation-eo
evas_evas_object_manipulation_eo_SOURCES = evas/evas-object-manipulation-eo.c
evas_evas_object_manipulation_eo_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_hints
evas_evas_hints_SOURCES = evas/evas-hints.c
evas_evas_hints_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_stacking
evas_evas_stacking_SOURCES = evas/evas-stacking.c
evas_evas_stacking_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_images2
evas_evas_images2_SOURCES = evas/evas-images2.c
evas_evas_images2_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_images3
evas_evas_images3_SOURCES = evas/evas-images3.c
evas_evas_images3_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_images4
evas_evas_images4_SOURCES = evas/evas-images4.c
evas_evas_images4_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_images5
evas_evas_images5_SOURCES = evas/evas-images5.c
evas_evas_images5_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_text
evas_evas_text_SOURCES = evas/evas-text.c
evas_evas_text_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_textblock_obstacles
evas_evas_textblock_obstacles_SOURCES = evas/evas-textblock-obstacles.c
evas_evas_textblock_obstacles_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_smart_object
evas_evas_smart_object_SOURCES = evas/evas-smart-object.c
evas_evas_smart_object_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_smart_interface
evas_evas_smart_interface_SOURCES = evas/evas-smart-interface.c
evas_evas_smart_interface_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_box
evas_evas_box_SOURCES = evas/evas-box.c
evas_evas_box_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_map_utils
evas_evas_map_utils_SOURCES = evas/evas-map-utils.c
evas_evas_map_utils_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_map_aa
evas_evas_map_aa_SOURCES = evas/evas-map-aa.c
evas_evas_map_aa_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_map_utils_eo
evas_evas_map_utils_eo_SOURCES = evas/evas-map-utils-eo.c
evas_evas_map_utils_eo_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_map_aa_eo
evas_evas_map_aa_eo_SOURCES = evas/evas-map-aa-eo.c
evas_evas_map_aa_eo_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_table
evas_evas_table_SOURCES = evas/evas-table.c
evas_evas_table_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_transparent
evas_evas_transparent_SOURCES = evas/evas-transparent.c
evas_evas_transparent_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_multi_touch
evas_evas_multi_touch_SOURCES = evas/evas-multi-touch.c
evas_evas_multi_touch_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_cube
evas_evas_canvas3d_cube_SOURCES = evas/evas-3d-cube.c
evas_evas_canvas3d_cube_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_cube2
evas_evas_canvas3d_cube2_SOURCES = evas/evas-3d-cube2.c
evas_evas_canvas3d_cube2_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_proxy
evas_evas_canvas3d_proxy_SOURCES = evas/evas-3d-proxy.c
evas_evas_canvas3d_proxy_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_pick
evas_evas_canvas3d_pick_SOURCES = evas/evas-3d-pick.c
evas_evas_canvas3d_pick_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_md2
evas_evas_canvas3d_md2_SOURCES = evas/evas-3d-md2.c
evas_evas_canvas3d_md2_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_eet
evas_evas_canvas3d_eet_SOURCES = evas/evas-3d-eet.c
evas_evas_canvas3d_eet_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_ply
evas_evas_canvas3d_ply_SOURCES = evas/evas-3d-ply.c
evas_evas_canvas3d_ply_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_hull
evas_evas_canvas3d_hull_SOURCES = evas/evas-3d-hull.c
evas_evas_canvas3d_hull_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_fog
evas_evas_canvas3d_fog_SOURCES = evas/evas-3d-fog.c
evas_evas_canvas3d_fog_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_frustum
evas_evas_canvas3d_frustum_SOURCES = evas/evas-3d-frustum.c
evas_evas_canvas3d_frustum_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_aabb
evas_evas_canvas3d_aabb_SOURCES = evas/evas-3d-aabb.c
evas_evas_canvas3d_aabb_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_cube_rotate
evas_evas_canvas3d_cube_rotate_SOURCES = evas/evas-3d-cube-rotate.c
evas_evas_canvas3d_cube_rotate_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_shadows
evas_evas_canvas3d_shadows_SOURCES = evas/evas-3d-shadows.c
evas_evas_canvas3d_shadows_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_shooter
evas_evas_canvas3d_shooter_SOURCES = evas/shooter/evas-3d-shooter.c \
evas/shooter/evas-3d-shooter-header.c \
evas/shooter/evas-3d-shooter-header.h \
evas/shooter/evas-3d-shooter-macros.h
evas_evas_canvas3d_shooter_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_obj
evas_evas_canvas3d_obj_SOURCES = evas/evas-3d-obj.c
evas_evas_canvas3d_obj_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_blending
evas_evas_canvas3d_blending_SOURCES = evas/evas-3d-blending.c
evas_evas_canvas3d_blending_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_mmap
evas_evas_canvas3d_mmap_SOURCES = evas/evas-3d-mmap-set.c
evas_evas_canvas3d_mmap_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_colorpick
evas_evas_canvas3d_colorpick_SOURCES = evas/evas-3d-colorpick.c
evas_evas_canvas3d_colorpick_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_parallax_occlusion
evas_evas_canvas3d_parallax_occlusion_SOURCES = evas/evas-3d-parallax-occlusion.c
evas_evas_canvas3d_parallax_occlusion_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_canvas3d_static_lod
evas_evas_canvas3d_static_lod_SOURCES = evas/evas-3d-static-lod.c
evas_evas_canvas3d_static_lod_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_gl
evas_evas_gl_SOURCES = evas/evas-gl.c
evas_evas_gl_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_vg_simple
evas_evas_vg_simple_SOURCES = evas/evas-vg-simple.c
evas_evas_vg_simple_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_vg_batman
evas_evas_vg_batman_SOURCES = evas/evas-vg-batman.c
evas_evas_vg_batman_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_vg_json
evas_evas_vg_json_SOURCES = evas/evas-vg-json.c
evas_evas_vg_json_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas/evas_aspect_hints
evas_evas_aspect_hints_DEPS = $(srcdir)/aspect.edc
evas_evas_aspect_hints_SOURCES = evas/evas-aspect-hints.c
evas_evas_aspect_hints_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

if HAVE_CXX

EXTRA_PROGRAMS += evas/evas_cxx_rectangle
evas_evas_cxx_rectangle_SOURCES = evas/evas_cxx_rectangle.cc
evas_evas_cxx_rectangle_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS) @EINA_CXX_CFLAGS@ @EO_CXX_CFLAGS@ @EVAS_CXX_CFLAGS@ \
-I$(top_builddir)/src/lib/evas/canvas/ \
$(AM_CPPFLAGS) @EVAS_CFLAGS@

endif

if HAVE_CSHARP

EXTRA_PROGRAMS += \
    evas/evas_mono_box_csharp \
    evas/evas_mono_image_csharp \
    evas/evas_mono_image2_csharp \
    evas/evas_mono_rectangle_csharp \
    evas/evas_mono_table_csharp \
    evas/evas_mono_text_csharp


evas_evas_mono_box_csharp_SOURCES = \
	evas/evas_mono_box.cs

evas/evas_mono_box_csharp$(EXEEXT): $(evas_evas_mono_box_csharp_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

evas_evas_mono_image_csharp_SOURCES = \
	evas/evas_mono_image.cs

evas/evas_mono_image_csharp$(EXEEXT): $(evas_evas_mono_image_csharp_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

evas_evas_mono_image2_csharp_SOURCES = \
	evas/evas_mono_image2.cs

evas/evas_mono_image2_csharp$(EXEEXT): $(evas_evas_mono_image2_csharp_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

evas_evas_mono_rectangle_csharp_SOURCES = \
	evas/evas_mono_rectangle.cs

evas/evas_mono_rectangle_csharp$(EXEEXT): $(evas_evas_mono_rectangle_csharp_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

evas_evas_mono_table_csharp_SOURCES = \
	evas/evas_mono_table.cs

evas/evas_mono_table_csharp$(EXEEXT): $(evas_evas_mono_table_csharp_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

evas_evas_mono_text_csharp_SOURCES = \
	evas/evas_mono_text.cs

evas/evas_mono_text_csharp$(EXEEXT): $(evas_evas_mono_text_csharp_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

endif



EVAS_SRCS = \
evas/evas-aspect-hints.c \
evas/evas-box.c \
evas/evas-buffer-simple.c \
evas/evas-events.c \
evas/evas-event-filter.c \
evas/evas-hints.c \
evas/evas-images.c \
evas/evas-images2.c \
evas/evas-images3.c \
evas/evas-images4.c \
evas/evas-images5.c \
evas/evas-init-shutdown.c \
evas/evas-map-utils.c \
evas/evas-map-aa.c \
evas/evas-object-manipulation-eo.c \
evas/evas-object-manipulation.c \
evas/evas-smart-interface.c \
evas/evas-smart-object.c \
evas/evas-stacking.c \
evas/evas-table.c \
evas/evas-multi-touch.c \
evas/evas-text.c \
evas/evas-vg-simple.c \
evas/evas-vg-batman.c

EVAS_DATA_FILES = \
evas/resources/images/enlightenment.png \
evas/resources/images/red.png \
evas/resources/images/im1.png \
evas/resources/images/cube1.png \
evas/resources/images/normal_lego.png \
evas/resources/images/sonic.png \
evas/resources/images/sweet_home_reversed.png \
evas/resources/images/wood.jpg \
evas/resources/images/rocks.jpg \
evas/resources/images/four_NM_height.tga \
evas/resources/images/rocks_NM_height.tga \
evas/resources/images/star.jpg \
evas/resources/images/grid.png \
evas/resources/images/grid_n.png \
evas/resources/images/billboard.png \
evas/resources/models/mesh_for_mmap.eet \
evas/resources/models/mesh_for_mmap.md2 \
evas/resources/models/mesh_for_mmap.obj \
evas/resources/models/mesh_for_mmap.ply \
evas/resources/models/Normal_UVs_Colors.ply \
evas/resources/models/Normal_UVs_NoColors.ply \
evas/resources/models/Normal_NoUVs_Colors.ply \
evas/resources/models/Normal_NoUVs_NoColors.ply \
evas/resources/models/NoNormal_UVs_Colors.ply \
evas/resources/models/NoNormal_UVs_NoColors.ply \
evas/resources/models/NoNormal_NoUVs_Colors.ply \
evas/resources/models/NoNormal_NoUVs_NoColors.ply \
evas/resources/models/sonic.md2 \
evas/resources/models/sweet_home.obj \
evas/resources/models/sweet_home_only_vertex_coords.obj \
evas/resources/models/sweet_home_without_normals.obj \
evas/resources/models/sweet_home_without_tex_coords.obj \
evas/resources/models/lod/iso_sphere0.obj \
evas/resources/models/lod/iso_sphere1.obj \
evas/resources/models/lod/iso_sphere2.obj \
evas/resources/models/convex_hull/column_blender_ch.ply \
evas/resources/models/convex_hull/column_blender_ch_test.ply \
evas/resources/models/convex_hull/column.ply \
evas/resources/models/convex_hull/cube_blender_ch.ply \
evas/resources/models/convex_hull/cube_blender_ch_test.ply \
evas/resources/models/convex_hull/eagle_blender_ch.ply \
evas/resources/models/convex_hull/eagle_blender_ch_test.ply \
evas/resources/models/convex_hull/home_blender_ch.obj \
evas/resources/models/convex_hull/home_blender_ch_test.ply \
evas/resources/models/convex_hull/plain.ply \
evas/resources/models/convex_hull/plain_blender_ch.ply \
evas/resources/models/convex_hull/plain_blender_ch_test.ply \
evas/resources/models/convex_hull/sonic_blender_ch.ply \
evas/resources/models/convex_hull/sonic_blender_ch_test.ply \
evas/resources/models/convex_hull/sphere_blender_ch.ply \
evas/resources/models/convex_hull/sphere_blender_ch_test.ply \
evas/resources/models/convex_hull/torus_blender_ch.ply \
evas/resources/models/convex_hull/torus_blender_ch_test.ply \
evas/shooter/assets/images/brick-stone_n.jpg \
evas/shooter/assets/images/brick-stone.jpg \
evas/shooter/assets/images/bricks_n.jpg \
evas/shooter/assets/images/eagle.png \
evas/shooter/assets/images/snake.png \
evas/shooter/assets/images/snake_n.png \
evas/shooter/assets/images/bricks.jpg \
evas/shooter/assets/images/soldier.png \
evas/shooter/assets/images/gazebo.png \
evas/shooter/assets/images/warrior.png \
evas/shooter/assets/images/gazebo_t.png \
evas/shooter/assets/images/gazebo_t_t.png \
evas/shooter/assets/images/gazebo_t_n.png \
evas/shooter/assets/images/gazebo_b_n.png \
evas/shooter/assets/models/warrior.md2 \
evas/shooter/assets/models/gazebo_b.md2 \
evas/shooter/assets/models/gazebo_t.md2 \
evas/shooter/assets/models/eagle.md2 \
evas/shooter/assets/models/snake.md2 \
evas/shooter/assets/models/soldier_jump.md2 \
evas/shooter/assets/models/soldier.md2 \
evas/shooter/assets/models/tommy.ply \
evas/filters/filter_blend.lua \
evas/filters/filter_blur.lua \
evas/filters/filter_bump.lua \
evas/filters/filter_curve.lua \
evas/filters/filter_example_1.lua \
evas/filters/filter_grow.lua \
evas/filters/filter_mask.lua \
evas/filters/filter_padding.lua \
evas/filters/filter_transform.lua \
evas/Makefile.examples \
evas/README \
evas/evas-common.h

DATA_FILES += $(EVAS_DATA_FILES)
EVAS_EDJS = $(EVAS_EDCS:%.edc=%.edj)
EDCS += $(EVAS_EDCS)

install-examples-evas: examples
	$(MKDIR_P) $(DESTDIR)$(datadir)/evas/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(EVAS_SRCS) $(EVAS_EDCS) $(EVAS_DATA_FILES) $(DESTDIR)$(datadir)/evas/examples
	cd $(builddir) && $(install_sh_DATA) -c $(EVAS_EDJS) $(DESTDIR)$(datadir)/evas/examples

uninstall-local-evas:
	for f in $(EVAS_SRCS) $(EVAS_EDCS) $(EVAS_DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/evas/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-evas
UNINSTALL_EXAMPLES += uninstall-local-evas

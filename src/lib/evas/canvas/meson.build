
pub_eo_files = [
  'efl_canvas_polygon.eo',
  'efl_canvas_image_internal.eo',
  'efl_canvas_image.eo',
  'efl_canvas_snapshot.eo',
  'efl_canvas_proxy.eo',
  'efl_canvas_filter_internal.eo',
  'efl_input_state.eo',
  'efl_input_interface.eo',
  'efl_input_event.eo',
  'efl_input_key.eo',
  'efl_input_pointer.eo',
  'efl_input_hold.eo',
  'efl_input_focus.eo',
  'efl_input_clickable.eo',
  'efl_input_device.eo',
  'efl_canvas_pointer.eo',
  'efl_canvas_scene.eo',
  'efl_canvas_animation.eo',
  'efl_canvas_alpha_animation.eo',
  'efl_canvas_rotate_animation.eo',
  'efl_canvas_scale_animation.eo',
  'efl_canvas_translate_animation.eo',
  'efl_canvas_group_animation.eo',
  'efl_canvas_parallel_group_animation.eo',
  'efl_canvas_sequential_group_animation.eo',
  'efl_canvas_textblock_factory.eo',
  'efl_canvas_rectangle.eo',
  'efl_canvas_object.eo',
  'efl_gfx_vg_value_provider.eo',
  'efl_canvas_vg_object.eo',
  'efl_canvas_vg_node.eo',
  'efl_canvas_vg_container.eo',
  'efl_canvas_vg_shape.eo',
  'efl_canvas_vg_image.eo',
  'efl_canvas_vg_gradient.eo',
  'efl_canvas_vg_gradient_radial.eo',
  'efl_canvas_vg_gradient_linear.eo',
  'efl_canvas_group.eo',
  'efl_gfx_mapping.eo',
  'efl_canvas_event_grabber.eo',
  'efl_text_cursor_object.eo',
  'efl_canvas_textblock.eo',
  'efl_text_formatter.eo',
  'efl_canvas_object_animation.eo',
]

priv_eo_files = [
  'efl_canvas_surface.eo',
  'efl_canvas_surface_tbm.eo',
  'efl_canvas_surface_x11.eo',
  'efl_canvas_surface_wayland.eo',
]

foreach eo_file : priv_eo_files
  priv_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-e', 'EVAS_API',
                           '-gchd', '@INPUT@'])
endforeach

evas_canvas_eo_files = pub_eo_files

pub_evas_eo_files += files(pub_eo_files)

foreach eo_file : pub_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : join_paths(dir_package_include, 'canvas'),
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-e', 'EVAS_API',
                           '-gchd', '@INPUT@'])
endforeach


pub_eo_types_files = [
    'efl_canvas_animation_types.eot',
    'efl_input_types.eot',
]

evas_canvas_eot_files = pub_eo_types_files

foreach eo_file : pub_eo_types_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : join_paths(dir_package_include, 'canvas'),
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-e', 'EVAS_API',
                           '-ghd', '@INPUT@'])
endforeach

pub_evas_eot_files += files(pub_eo_types_files)

evas_legacy_h = files([
  'efl_canvas_event_grabber_eo.legacy.h',
  'efl_canvas_group_eo.legacy.h',
  'efl_canvas_object_eo.legacy.h',
  'efl_canvas_textblock_eo.legacy.h',
  'efl_canvas_vg_container_eo.legacy.h',
  'efl_canvas_vg_node_eo.legacy.h',
  'efl_canvas_vg_object_eo.legacy.h',
  'efl_canvas_vg_shape_eo.legacy.h',
  'efl_canvas_vg_gradient_eo.legacy.h',
  'evas_box_eo.legacy.h',
  'evas_canvas_eo.legacy.h',
  'evas_grid_eo.legacy.h',
  'evas_image_eo.legacy.h',
  'evas_line_eo.legacy.h',
  'evas_table_eo.legacy.h',
  'evas_text_eo.legacy.h',
  'evas_textgrid_eo.legacy.h',
])

evas_src += files([
  'evas_callbacks.c',
  'evas_clip.c',
  'evas_data.c',
  'evas_device.c',
  'evas_events.c',
  'evas_events_legacy.c',
  'evas_focus.c',
  'evas_key.c',
  'evas_key_grab.c',
  'evas_layer.c',
  'evas_main.c',
  'evas_name.c',
  'evas_image_legacy.c',
  'evas_object_image.c',
  'evas_object_main.c',
  'evas_object_inform.c',
  'evas_object_intercept.c',
  'evas_object_line.c',
  'evas_object_polygon.c',
  'evas_object_rectangle.c',
  'evas_object_smart.c',
  'evas_object_smart_clipped.c',
  'evas_object_box.c',
  'evas_object_table.c',
  'evas_object_text.c',
  'evas_object_textblock.c',
  'evas_object_textgrid.c',
  'evas_object_grid.c',
  'evas_font_dir.c',
  'evas_rectangle.c',
  'evas_render.c',
  'evas_smart.c',
  'evas_async_events.c',
  'evas_stats.c',
  'evas_touch_point.c',
  'evas_map.c',
  'evas_map.h',
  'evas_gl.c',
  'evas_out.c',
  'efl_canvas_image.c',
  'efl_canvas_event_grabber.c',
  'efl_canvas_proxy.c',
  'efl_canvas_snapshot.c',
  'efl_canvas_surface.c',
  'efl_canvas_surface_tbm.c',
  'efl_canvas_surface_x11.c',
  'efl_canvas_surface_wayland.c',
  'efl_gfx_mapping.c',
  'efl_input_event.c',
  'efl_input_key.c',
  'efl_input_pointer.c',
  'efl_input_hold.c',
  'efl_input_focus.c',
  'efl_input_clickable.c',
  'efl_input_device.c',
  'efl_canvas_animation.c',
  'efl_canvas_alpha_animation.c',
  'efl_canvas_rotate_animation.c',
  'efl_canvas_scale_animation.c',
  'efl_canvas_translate_animation.c',
  'efl_canvas_group_animation.c',
  'efl_canvas_parallel_group_animation.c',
  'efl_canvas_sequential_group_animation.c',
  'efl_gfx_vg_value_provider.c',
  'efl_canvas_vg_object.c',
  'efl_canvas_vg_node.c',
  'efl_canvas_vg_container.c',
  'efl_canvas_vg_gradient.c',
  'efl_canvas_vg_gradient_linear.c',
  'efl_canvas_vg_gradient_radial.c',
  'efl_canvas_vg_utils.c',
  'efl_canvas_vg_shape.c',
  'efl_canvas_vg_image.c',
  'evas_filter_mixin.c',
  'efl_canvas_object_animation.c',
  'efl_text_cursor_object.c',
  'efl_text_formatter.c'
  ])

evas_include_directories += include_directories('.')

install_headers(['evas_textblock_legacy.h', evas_legacy_h],
  install_dir : join_paths(dir_package_include, 'canvas'),
)

pub_eo_types_files = []

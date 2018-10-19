pub_legacy_eo_files = [
  'efl_gfx_fill.eo',
  'efl_gfx_entity.eo',
  'efl_gfx_color.eo',
  'efl_gfx_image.eo',
  'efl_gfx_image_animation_controller.eo',
  'efl_input_device.eo',
  'efl_ui_draggable.eo',
  'efl_ui_clickable.eo',
  'efl_ui_scrollable.eo',
  'efl_ui_scrollable_interactive.eo',
  'efl_ui_scrollbar.eo',
  'efl_ui_selectable.eo',
  'efl_ui_zoom.eo',
]

pub_eo_file_target = []
foreach eo_file : pub_legacy_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : join_paths(dir_package_include, 'interfaces'),
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
  pub_eo_file_target += custom_target('eolian_gen_legacy_' + eo_file,
    input : eo_file,
    output : [eo_file + '.legacy.h'],
    depfile : eo_file + '.legacy.d',
    install : true,
    install_dir : join_paths(dir_package_include, 'interfaces'),
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'l:' + join_paths(meson.current_build_dir(), eo_file + '.legacy.h'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.legacy.d'),
                           '-gld', '@INPUT@'])
endforeach

pub_eo_files = [
  'efl_playable.eo',
  'efl_canvas_scene.eo',
  'efl_canvas_pointer.eo',
  'efl_config.eo',
  'efl_control.eo',
  'efl_duplicate.eo',
  'efl_file.eo',
  'efl_gfx_image_load_controller.eo',
  'efl_part.eo',
  'efl_player.eo',
  'efl_text.eo',
  'efl_text_font.eo',
  'efl_text_style.eo',
  'efl_text_format.eo',
  'efl_text_cursor.eo',
  'efl_text_annotate.eo',
  'efl_text_markup.eo',
  'efl_text_markup_util.eo',
  'efl_gfx_stack.eo',
  'efl_gfx_view.eo',
  'efl_gfx_buffer.eo',
  'efl_gfx_path.eo',
  'efl_gfx_shape.eo',
  'efl_gfx_gradient.eo',
  'efl_gfx_gradient_linear.eo',
  'efl_gfx_gradient_radial.eo',
  'efl_gfx_filter.eo',
  'efl_gfx_blur.eo',
  'efl_gfx_size_hint.eo',
  'efl_model.eo',
  'efl_animator.eo',
  'efl_orientation.eo',
  'efl_container.eo',
  'efl_content.eo',
  'efl_pack.eo',
  'efl_pack_layout.eo',
  'efl_pack_linear.eo',
  'efl_pack_table.eo',
  'efl_ui_base.eo',
  'efl_ui_direction.eo',
  'efl_ui_drag.eo',
  'efl_ui_range.eo',
  'efl_ui_view.eo',
  'efl_ui_model_connect.eo',
  'efl_ui_factory.eo',
  'efl_ui_multi_selectable.eo',
  'efl_screen.eo',
  'efl_io_closer.eo',
  'efl_io_positioner.eo',
  'efl_io_reader.eo',
  'efl_io_sizer.eo',
  'efl_io_writer.eo',
  'efl_io_buffer.eo',
  'efl_io_queue.eo',
  'efl_observer.eo',
  'efl_observable.eo',
  'efl_ui_autorepeat.eo',
  'efl_ui_format.eo',
  'efl_gfx_color_class.eo',
  'efl_gfx_text_class.eo',
  'efl_gfx_size_class.eo',
]

eolian_include_directories += ['-I', join_paths(meson.source_root(), 'src', 'lib', 'evas', 'canvas')]

foreach eo_file : pub_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : join_paths(dir_package_include, 'interfaces'),
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
endforeach

pub_eo_files += pub_legacy_eo_files

pub_eo_types_files = [
  'efl_gfx_types.eot',
  'efl_ui_types.eot',
  'efl_input_types.eot',
  'efl_types.eot',
  'efl_text_types.eot',
]

foreach eo_file : pub_eo_types_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : join_paths(dir_package_include, 'interfaces'),
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-ghd', '@INPUT@'])
endforeach

install_data(pub_eo_files + pub_legacy_eo_files + pub_eo_types_files,
  install_dir: join_paths(eolian_include_dir, package_version_name)
)

efl_interfaces_eo_files = pub_eo_files
efl_interfaces_eot_files = pub_eo_types_files

pub_eo_files = []
pub_eo_types_files = []

efl_src += files([
  'efl_interfaces_main.c',
  'efl_model_common.c',
  'efl_gfx_path.c',
  'efl_gfx_shape.c',
  'efl_gfx_color.c',
  'efl_input_device.c',
  'efl_io_closer.c',
  'efl_io_positioner.c',
  'efl_io_reader.c',
  'efl_io_sizer.c',
  'efl_io_writer.c',
  'efl_io_buffer.c',
  'efl_io_queue.c',
  'efl_observer.c',
  'efl_file.c',
  'efl_ui_format.c',
  'efl_text_markup_util.c',
])

#efl_header_src += files([
#  'efl_common_internal.h'
#])

eolian_include_directories += ['-I', meson.current_source_dir()]

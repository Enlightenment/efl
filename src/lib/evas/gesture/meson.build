pub_eo_files = [
  'efl_canvas_gesture_touch.eo',
  'efl_canvas_gesture.eo',
  'efl_canvas_gesture_tap.eo',
  'efl_canvas_gesture_long_tap.eo',
  'efl_canvas_gesture_recognizer.eo',
  'efl_canvas_gesture_manager.eo'
]

foreach eo_file : pub_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    install : true,
    install_dir : dir_package_include,
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'l:' + join_paths(meson.current_build_dir(), eo_file + '.legacy.h'),
                           '-gchl', '@INPUT@'])
endforeach

pub_eo_files = [
  'efl_canvas_gesture_recognizer_tap.eo',
  'efl_canvas_gesture_recognizer_long_tap.eo'
]

foreach eo_file : pub_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    install : false,
    install_dir : dir_package_include,
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'l:' + join_paths(meson.current_build_dir(), eo_file + '.legacy.h'),
                           '-gchl', '@INPUT@'])
endforeach

pub_eo_types_files = [
  'efl_canvas_gesture_types.eot',
]

foreach eo_file : pub_eo_types_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    install : true,
    install_dir : dir_package_include,
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-gh', '@INPUT@'])
endforeach

evas_src += files([
  'efl_canvas_gesture_touch.c',
  'efl_canvas_gesture.c',
  'efl_canvas_gesture_tap.c',
  'efl_canvas_gesture_long_tap.c',
  'efl_canvas_gesture_recognizer.c',
  'efl_canvas_gesture_recognizer_tap.c',
  'efl_canvas_gesture_recognizer_long_tap.c',
  'efl_canvas_gesture_manager.c',
])
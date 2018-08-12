pub_legacy_eo_files = [
   'efl_canvas_video.eo'
]

pub_eo_file_target = []
priv_eo_file_target = []

foreach eo_file : pub_legacy_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h', eo_file + '.legacy.h',],
    install : true,
    install_dir : dir_package_include,
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'l:' + join_paths(meson.current_build_dir(), eo_file + '.legacy.h'),
                           '-gchl', '@INPUT@'])

endforeach

eolian_include_directories += ['-I', meson.current_source_dir()]

emotion_header_src = [
  'Emotion.h',
  'Emotion_Legacy.h',
  'Emotion_Eo.h'
]

emotion_src = [
  'emotion_modules.h',
  'emotion_private.h',
  'emotion_smart.c',
  'emotion_webcam.c',
  'emotion_modules.c',
  'emotion_main.c'
]

emotion_deps = [eina, eo, ecore, eet, evas, efl, eio, emile]

if get_option('eeze') == true
  emotion_deps += [eeze]
endif

emotion_lib = library('emotion',
    emotion_src, pub_eo_file_target, priv_eo_file_target,
    dependencies: [emotion_deps],
    include_directories : config_dir + [include_directories('.')],
    install: true,
    c_args : package_c_args,
)

emotion = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: emotion_lib,
  sources : pub_eo_file_target,
  dependencies: [eina, eo, efl],
)

install_data(pub_legacy_eo_files,
  install_dir: join_paths(eolian_include_dir, package_version_name)
)

install_headers(emotion_header_src,
  install_dir : dir_package_include,
)

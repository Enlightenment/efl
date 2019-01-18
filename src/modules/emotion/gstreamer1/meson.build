generic_src = files([
  'emotion_gstreamer.h',
  'emotion_gstreamer.c',
  'emotion_alloc.c',
  'emotion_convert.c',
  'emotion_sink.c'
])

generic_deps = [
  dependency('gstreamer-1.0'),
  dependency('gstreamer-plugins-base-1.0'),
  dependency('gstreamer-video-1.0'),
  dependency('gstreamer-audio-1.0'),
  dependency('gstreamer-tag-1.0'),
  dependency('gstreamer-pbutils-1.0'),
  ]

shared_module(emotion_loader,
    generic_src,
    include_directories : config_dir,
    dependencies: [eina, evas, emotion, generic_deps],
    install: true,
    install_dir : mod_install_dir,
    c_args : package_c_args,
)

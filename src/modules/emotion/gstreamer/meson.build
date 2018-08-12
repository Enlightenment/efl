generic_src = files([
  'emotion_gstreamer.h',
  'emotion_gstreamer.c',
  'emotion_alloc.c',
  'emotion_convert.c',
  'emotion_sink.c'
])

generic_deps = [dependency('gstreamer-0.1')]

shared_module(emotion_loader,
    generic_src,
    include_directories : config_dir,
    dependencies: [eina, evas, emotion, generic_deps],
    install: true,
    install_dir : mod_install_dir,
    c_args : package_c_args,
)

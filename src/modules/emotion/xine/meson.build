generic_src = files([
  'emotion_xine.h',
  'emotion_xine.c',
  'emotion_xine_vo_out.c',
])

generic_deps = dependency('libxine')

shared_module(emotion_loader,
    generic_src,
    include_directories : config_dir,
    dependencies: [eina, evas, emotion, generic_deps],
    install: true,
    install_dir : mod_install_dir,
    c_args : package_c_args,
)

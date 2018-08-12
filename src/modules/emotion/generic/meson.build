generic_src = files([
  'emotion_generic.c',
  'emotion_generic.h',
])

emotion_generic = declare_dependency(
  include_directories: include_directories('.'),
  dependencies: emotion,
)

shared_module(emotion_loader,
    generic_src,
    include_directories : config_dir,
    dependencies: [eina, evas, emotion, generic_deps],
    install: true,
    install_dir : mod_install_dir,
    c_args : package_c_args,
)

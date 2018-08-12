generic_src = files([
  'emotion_libvlc.c',
])

generic_deps = [dependency('libvlc', version: '>= 3.0')]

shared_module(emotion_loader,
    generic_src,
    include_directories : config_dir,
    dependencies: [eina, evas, emotion, generic_deps],
    install: true,
    install_dir : mod_install_dir,
    c_args : package_c_args,
)

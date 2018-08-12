
buildsystem_src = [
  'buildsystem.h',
  'buildsystem_meson.c',
]


buildsystem_lib = static_library('buildsystem',
    buildsystem_src,
    dependencies : [eina, efl],
    include_directories : config_dir + [include_directories(join_paths('..', '..', 'lib'))],
    install: false,
    c_args : ['-DPACKAGE_SRC_DIR="'+ meson.source_root() +'"'],
)

buildsystem = declare_dependency(
  include_directories: [include_directories('.'), include_directories(join_paths('..', '..', 'lib'))],
  link_with: buildsystem_lib,
  dependencies: [eina],
)

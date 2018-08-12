
triangulator_src = [
  'triangulator_simple.c',
  'triangulator_simple.h',
  'triangulator_stroker.c',
  'triangulator_stroker.h',
]


triangulator_lib = static_library('triangulator',
    triangulator_src,
    dependencies : [eina, efl],
    include_directories : config_dir + [include_directories(join_paths('..', '..', 'lib'))],
    install: false,
)

triangulator = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: triangulator_lib,
  dependencies: [eina],
)

elua_suite_deps = [check, eina, elua]

elua_suite_src = [
   'elua_suite.c',
   'elua_suite.h',
   'elua_lib.c'
]

elua_bindings_dir = join_paths(meson.source_root(), 'src', 'bindings', 'luajit')
elua_core_dir = join_paths(meson.source_root(), 'src', 'scripts', 'elua', 'core')
elua_modules_dir = join_paths(meson.source_root(), 'src', 'scripts', 'elua', 'modules')
elua_apps_dir = join_paths(meson.source_root(), 'src', 'tests', 'elua', 'data', 'apps')

elua_suite = executable('elua_suite',
  elua_suite_src,
  include_directories : config_dir,
  dependencies: [elua_suite_deps, check],
  c_args : [
    '-DELUA_BINDINGS_DIR="'+elua_bindings_dir+'"',
    '-DELUA_CORE_DIR="'+elua_core_dir+'"',
    '-DELUA_MODULES_DIR="'+elua_modules_dir+'"',
    '-DELUA_APPS_DIR="'+elua_apps_dir+'"',
    '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
    '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"'
  ]
)

test('elua-suite', elua_suite,
  env : test_env
)

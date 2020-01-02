# Generation of .eo.cs files and test library common to dotnet and mono
eo_files = [
    'dummy_child.eo',
    'dummy_numberwrapper.eo',
    'dummy_test_object.eo',
    'dummy_test_iface.eo',
    'dummy_inherit_helper.eo',
    'dummy_inherit_iface.eo',
    'dummy_part_holder.eo',
    'dummy_event_manager.eo',
    'dummy_constructible_object.eo',
]

private_eo_files = [
    'dummy_hidden_object.eo'
]

eo_file_targets = []

foreach eo_file : eo_files + private_eo_files
  eo_file_targets += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-gch', '@INPUT@'])
endforeach



efl_mono_native_test = library('efl_mono_native_test',
    eo_file_targets + [
        'libefl_mono_native_test.h',
        'dummy_child.c',
        'dummy_inherit_helper.c',
        'dummy_interfaces.c',
        'dummy_numberwrapper.c',
        'dummy_part_holder.c',
        'dummy_test_object.c',
        'dummy_event_manager.c',
        'dummy_constructible_object.c',
        'dummy_hidden_object.c',
    ],
    dependencies : [ecore, eo, efl],
)

eo_file_targets = []

foreach mono_gen_file : eo_files
  eo_file_targets += custom_target('eolian_mono_gen_'+mono_gen_file.underscorify()+'',
    input : mono_gen_file,
    output : [mono_gen_file + '.cs'],
    command : [eolian_mono_gen, beta_option, '-I', meson.current_source_dir(), eolian_include_directories,
                                '--dllimport', 'efl_mono_native_test',
                                '-o', join_paths(meson.current_build_dir(), mono_gen_file + '.cs'),
                                '@INPUT@'])
endforeach

efl_mono_src = [
      'Main.cs',
      'TestUtils.cs',
      'EinaTestData.cs',
      'StructHelpers.cs',
      'BasicDirection.cs',
      'Eina.cs',
      'Eldbus.cs',
      'Eo.cs',
      'EoPromises.cs',
      'EoConstruction.cs',
      'Errors.cs',
      'Events.cs',
      'FunctionPointers.cs',
      'FunctionPointerMarshalling.cs',
      'Model.cs',
      'Parts.cs',
      'Promises.cs',
      'Strbuf.cs',
      'Strings.cs',
      'Structs.cs',
      'Value.cs',
      'ValueEolian.cs',
      'Inheritance.cs',
      'Hash.cs',
      'List.cs',
      'Array.cs',
]

if get_option('dotnet')

  dotnet_test_conf_data = configuration_data()

  dotnet_test_conf_data.set('EFL_VERSION', meson.project_version())
  dotnet_test_conf_data.set('BINDING_BUILD', join_paths(meson.current_build_dir(),
                                                        '..', '..', 'bindings', 'mono'))
  dotnet_test_conf_data.set('BINDING_TEST_SRC', meson.current_source_dir())
  dotnet_test_conf_data.set('CA_RULESET', join_paths(meson.build_root(), '@0@'.format(ca_ruleset)))

  dotnet_test_conf_data.set('NETCOREAPP_VERSION', dotnet_core_app_version)

  if get_option('mono-beta')
    dotnet_test_conf_data.set('EFL_BETA', 'EFL_BETA')
  else
    dotnet_test_conf_data.set('EFL_BETA', '')
  endif

  dotnet_test_csproj = configure_file(input: 'efl_sharp_test_suite.csproj.in',
                                      output: 'efl_sharp_test_suite.csproj',
                                      configuration: dotnet_test_conf_data,
  )

  efl_mono_suite = custom_target('efl_mono_test',
    input: eo_file_targets + [dotnet_test_csproj] + efl_mono_src,
    output: 'efl_sharp_test_suite.dll',
    depends: [efl_mono],
    build_by_default: true,
    command: [dotnet,
              'build',
              '-o',
              meson.current_build_dir(),
              '--framework',
              'netcoreapp' + dotnet_core_app_version,
              dotnet_test_csproj],
  )

else
  efl_mono_test = library('efl_mono_test',
      eo_file_targets,
      link_with : [efl_mono],
      cs_args : extra_cs_args
  )

  efl_mono_suite = executable('efl-mono-suite',
    efl_mono_src,
    link_with : [efl_mono, efl_mono_test],
    cs_args : extra_cs_args
  )
endif

# Common environment shared by both dotnet and mono
env_mono = environment()
env_mono.set('MONO_PATH', efl_mono_test_suite_path )
env_mono.set('EFL_RUN_IN_TREE', '1')


config_libs = ['eina', 'ecore', 'eo', 'efl', 'evas', 'eldbus', 'elementary', 'efl_mono']
load_lib = efl_mono_test_suite_path + ':'

foreach config : config_libs
  lib = get_variable(config+'_lib')
  path = lib.full_path().split('/')
  repaired_path = ''
  prev = ''

  foreach p : path
    repaired_path += prev
    prev = p+'/'
  endforeach

  load_lib += repaired_path+':'
endforeach

load_lib += ':' + meson.current_build_dir()

env_mono.set('LD_LIBRARY_PATH', load_lib)


if get_option('dotnet')
  test('efl-mono-suite',
    dotnet,
    args: [join_paths(meson.current_build_dir(), 'efl_sharp_test_suite.dll')],
    env: env_mono,
  )
else
  test('efl-mono-suite',
    efl_mono_suite,
    env : env_mono
  )
endif

eolian_mono_helpers_suite = executable('eolian-mono-suite',
  ['eolian_mono_suite.cc'],
  include_directories : config_dir,
  dependencies: [check, eolian_cxx, eina_cxx],
  cpp_args : package_c_args + [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"']
)

test('eolian-mono-suite', eolian_mono_helpers_suite)

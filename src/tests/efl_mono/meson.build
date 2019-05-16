eo_files = [
    'dummy_child.eo',
    'dummy_numberwrapper.eo',
    'dummy_test_object.eo',
    'dummy_test_iface.eo',
    'dummy_inherit_helper.eo',
    'dummy_inherit_iface.eo',
    'dummy_part_holder.eo',
    'dummy_event_manager.eo',
]

eo_file_targets = []

foreach eo_file : eo_files
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

efl_mono_test = library('efl_mono_test',
    eo_file_targets,
    link_with : [efl_mono],
    cs_args : extra_cs_args
)

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
     'Errors.cs',
     'Events.cs',
     'FunctionPointers.cs',
     'FunctionPointerMarshalling.cs',
     'Parts.cs',
     'Promises.cs',
     'Strbuf.cs',
     'Strings.cs',
     'Structs.cs',
     'Value.cs',
     'ValueEolian.cs',
     'Inheritance.cs',
]

efl_mono_suite = executable('efl-mono-suite',
  efl_mono_src,
  link_with : [efl_mono, efl_mono_test],
  cs_args : extra_cs_args
)

env_mono = environment()
env_mono.set('MONO_PATH', efl_mono_test_suite_path )

if (cs_is_dotnet)
env_mono.set('LD_LIBRARY_PATH', efl_mono_test_suite_path )
copy_prog = find_program(['cp', 'copy'])

configure_file(input : 'efl-mono-suite.runtimeconfig.json',
               output : 'efl-mono-suite.runtimeconfig.json',
               copy : true)

custom_target('copy_efl_mono_dll',
              build_by_default : true,
              input : efl_mono,
              output : efl_mono.full_path().split('/')[-1],
              command : [copy_prog, '@INPUT@', '@OUTPUT@'])

custom_target('copy_efl_mono_lib_dll',
              build_by_default : true,
              input : efl_mono_lib,
              output : efl_mono_lib.full_path().split('/')[-1],
              command : [copy_prog, '@INPUT@', '@OUTPUT@'])
endif

config_libs = ['eina', 'ecore', 'eo', 'efl', 'evas', 'eldbus', 'elementary']
load_lib = ''

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

env_mono.set('LD_LIBRARY_PATH', load_lib)

test('efl-mono-suite',
  efl_mono_suite,
  env : env_mono
)

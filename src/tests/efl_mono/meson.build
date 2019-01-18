eo_files = ['dummy_child.eo', 'dummy_numberwrapper.eo', 'dummy_test_object.eo', 'dummy_test_iface.eo', 'dummy_another_iface.eo', 'dummy_inherit_helper.eo', 'dummy_inherit_iface.eo']

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
    eo_file_targets + ['libefl_mono_native_test.c'],
    dependencies : [ecore, eo, efl],
)

eo_file_targets = []

foreach mono_gen_file : eo_files
  eo_file_targets += custom_target('eolian_mono_gen_'+mono_gen_file.underscorify()+'',
    input : mono_gen_file,
    output : [mono_gen_file + '.cs'],
    command : [eolian_mono_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                                '--dllimport', 'efl_mono_native_test',
                                '-o', join_paths(meson.current_build_dir(), mono_gen_file + '.cs'),
                                '@INPUT@'])
endforeach

efl_mono_test = library('efl_mono_test',
    eo_file_targets,
    link_with : [efl_mono],
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
     'Evas.cs',
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
)

env = environment()
env.set('MONO_PATH', efl_mono_test_suite_path )

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

env.set('LD_LIBRARY_PATH', load_lib)

test('efl-mono-suite',
  efl_mono_suite,
  env : env
)

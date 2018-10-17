eo_files = ['test_child.eo', 'test_numberwrapper.eo', 'test_testing.eo']

eo_file_targets = []

foreach eo_file : eo_files
  eo_file_targets += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
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
     'ValueEolian.cs'
]

efl_mono_suite = executable('efl-mono-suite',
  efl_mono_src,
  link_with : [efl_mono, efl_mono_test],
)

test('efl-mono-suite',
  efl_mono_suite,
  env : ['MONO_PATH='+efl_mono_test_suite_path]
)

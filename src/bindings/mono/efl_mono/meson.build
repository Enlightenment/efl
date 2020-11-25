mono_files += files(
  'efl_all.cs',
  'efl_csharp_application.cs',
  'UserModel.cs',
  'GenericModel.cs',
  'ItemFactory.cs',
  'Bind.cs'
  )

bash = find_program('bash')

map = run_command('map_generate.sh').stdout()


efl_libs = configuration_data()
efl_libs.set('EFL_MONO_LIBRARY_MAP', map)
efl_libs.set('CUSTOM_EXPORTS_MONO_DL_MONO', 'eflcustomexportsmono')
efl_libs.set('EVIL_DL_MONO', 'dl')
if sys_osx
  efl_libs.set('LIBDL_DL_MONO', 'dl.dylib')
else
  efl_libs.set('LIBDL_DL_MONO', 'libdl.so')
endif

foreach mono_libs : mono_sublibs
  key = mono_libs[0].to_upper()+'_DL_MONO'
  efl_libs.set(key, mono_libs[0].to_lower())
endforeach

configure_file(
  input: 'efl_libs.csv.in',
  output: 'efl_libs.csv',
  configuration: efl_libs
)

efl_src = configure_file(
  input: 'efl_libs.cs.in',
  output: 'efl_libs.cs',
  configuration: efl_libs
)

friend_assemblies = ''
mono_friend_assemblies = get_option('mono-friend-assemblies')
if get_option('build-tests')
  if get_option('dotnet')
     mono_friend_assemblies += 'efl_sharp_test_suite'
  else
     mono_friend_assemblies += 'efl_mono_test'
     mono_friend_assemblies += 'efl-mono-suite'
  endif
endif

foreach f : mono_friend_assemblies
   friend_assemblies += '[assembly: InternalsVisibleTo("@0@")]\n'.format(f)
endforeach
friend_assembly_data = configuration_data()
friend_assembly_data.set('MONO_FRIEND_ASSEMBLIES', friend_assemblies)

mono_files += configure_file(
  input: 'AssemblyInfo.cs.in',
  output: 'AssemblyInfo.cs',
  configuration: friend_assembly_data
)

mono_eo_files = [
  'efl_mono_model_internal.eo',
  'efl_mono_model_internal_child.eo'
]

# mono_eo_c_files = []

pub_eo_file_target = []

foreach eo_file : mono_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : join_paths('..', '..', '..', 'lib', 'efl_mono', eo_file),
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : false,
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
  # mono_eo_c_files += join_paths(meson.current_build_dir(), eo_file + '.c')
endforeach

efl_mono_lib = library('eflcustomexportsmono',
    [
      join_paths('..', '..', '..', 'lib', 'efl_mono', 'efl_custom_exports_mono.c'),
      join_paths('..', '..', '..', 'lib', 'efl_mono', 'efl_mono_accessors.c'),
      join_paths('..', '..', '..', 'lib', 'efl_mono', 'efl_mono_model_internal.c'),
    ],
    pub_eo_file_target,
    install : true,
    include_directories : config_dir + [include_directories(join_paths('.'))],
    dependencies : [eo, eina, ecore],
    version : meson.project_version()
)

foreach eo_file : mono_eo_files
  if not blacklisted_files.contains(eo_file)
    mono_generator_target += custom_target('eolian_mono_gen_'+eo_file.underscorify()+'',
      input : join_paths('..', '..', '..', 'lib', 'efl_mono', eo_file),
      output : [eo_file + '.cs'],
      command : [eolian_mono_gen, beta_option, '-I', meson.current_source_dir(), eolian_include_directories,
                                 '--dllimport', 'eflcustomexportsmono',
                                 '-o', join_paths(meson.current_build_dir(), eo_file + '.cs'),
                                 '-e', get_option('mono-examples-dir'),
                                 '@INPUT@'])

  endif
endforeach

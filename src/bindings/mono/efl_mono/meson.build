mono_files += files(
  'efl_all.cs',
  'efl_csharp_application.cs'
  )

bash = find_program('bash')

map = run_command('map_generate.sh').stdout()


efl_libs = configuration_data()
efl_libs.set('EFL_MONO_LIBRARY_MAP', map)
efl_libs.set('CUSTOM_EXPORTS_MONO_DL_MONO', 'eflcustomexportsmono')
efl_libs.set('EVIL_DL_MONO', 'dl')

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

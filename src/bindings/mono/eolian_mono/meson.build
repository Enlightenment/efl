inc_dir = include_directories(join_paths('..', '..', '..', 'bin', 'eolian_mono'))

eolian_mono_gen = executable('eolian_mono',
                   join_paths('..', '..', '..', 'bin', 'eolian_mono', 'eolian_mono.cc'),
                   dependencies: [eina_cxx, eolian_cxx],
                   install: true,
                   include_directories : inc_dir,
                   cpp_args : package_c_args,
                 )

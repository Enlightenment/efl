inc_dir = include_directories(join_paths('..', '..', '..', 'bin', 'eolian_mono'))

eolian_mono_gen_bin = executable('eolian_mono',
                   join_paths('..', '..', '..', 'bin', 'eolian_mono', 'eolian_mono.cc'),
                   dependencies: [eina_cxx, eolian_cxx],
                   install: true,
                   include_directories : inc_dir,
                   cpp_args : package_c_args,
                 )
if meson.is_cross_build()
  eolian_mono_gen = find_program('eolian_mono', native: true)
else
  eolian_mono_gen = eolian_mono_gen_bin
endif

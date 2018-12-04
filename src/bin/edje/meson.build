subdir('epp')

edje_bin_deps = [
  eina, eo, efl, m,
  luajit, eet, evas,
  ecore_file, ecore_input,
  ecore_imf, ecore_imf_evas,
  embryo, efreet, eio,
  edje, ecore_evas
]

edje_cc_src = [
  'edje_multisense_convert.h',
  'edje_cc.h',
  'edje_cc.c',
  'edje_cc_out.c',
  'edje_cc_parse.c',
  'edje_cc_mem.c',
  'edje_cc_handlers.c',
  'edje_cc_sources.c',
  'edje_cc_script.c',
  'edje_multisense_convert.c'
]

edje_cc = executable('edje_cc',
        edje_cc_src,
        dependencies: [edje_bin_deps, eet, ecore_evas, ecore_file],
        install: true,
        c_args : package_c_args,
        link_args : bin_linker_args
)

if meson.is_cross_build()
  _edje_cc = find_program('edje_cc', native: true)
  edje_cc_path = _edje_cc.path()
  edje_cc_exe = [_edje_cc]
else
  env = find_program('env', native: true)
  edje_cc_exe = [env, 'EFL_RUN_IN_TREE=1', edje_cc.full_path()]
endif

edje_decc_src = [
  'edje_decc.c',
  'edje_decc.h',
  'edje_cc_mem.c',
  'edje_cc_sources.c',
]

edje_decc = executable('edje_decc',
        edje_decc_src,
        dependencies: [edje_bin_deps, eet, ecore_file, ecore_evas],
        install: true,
        c_args : package_c_args,
)

edje_player = executable('edje_player',
        'edje_player.c',
        dependencies: [edje_bin_deps, eet, ecore, eio, ecore_evas],
        install: true,
        c_args : package_c_args,
)

edje_inspector = executable('edje_inspector',
        'edje_inspector.c',
        dependencies: [edje_bin_deps, eet, ecore, eio, ecore_evas],
        install: true,
        c_args : package_c_args,
)

edje_external_inspector = executable('edje_external_inspector',
        'edje_external_inspector.c',
        dependencies: [edje_bin_deps, eet, ecore, eio, ecore_evas],
        install: true,
        c_args : package_c_args,
)

edje_codegen = executable('edje_codegen',
        'edje_codegen.c',
        dependencies: [edje_bin_deps, eet, ecore, eio, ecore_evas],
        install: true,
        c_args : package_c_args,
)

edje_pick = executable('edje_pick',
        'edje_pick.c',
        dependencies: [edje_bin_deps, eet, ecore, eio, ecore_evas],
        install: true,
        c_args : package_c_args,
)

edje_watch = executable('edje_watch',
        'edje_watch.c',
        dependencies: [edje_bin_deps, eet, ecore, eio, ecore_evas],
        install: true,
        c_args : package_c_args,
)

src = files([
  'mod.c',
])

elm_mod_test_entry_c_args = [ '-DEFL_BUILD' ]

shared_module(mod,
    src,
    c_args : elm_mod_test_entry_c_args,
    dependencies: [elementary],
    install: true,
    install_dir : mod_install_dir,
    name_suffix : sys_mod_extension
)

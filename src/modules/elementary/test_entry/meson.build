src = files([
  'mod.c',
])

shared_module(mod,
    src,
    c_args : package_c_args,
    dependencies: [elementary],
    install: true,
    install_dir : mod_install_dir,
    name_suffix : sys_mod_extension
)

mod_src = files([
  'ibus_module.c',
  'ibus_imcontext.c',
  'ibus_imcontext.h'
])

mod_deps = [ecore, ecore_imf, ecore_x, ecore_x_deps, dependency('ibus-1.0'), dependency('glib-2.0'), ecore_evas]

shared_module(mod_name,
    mod_src,
    dependencies: mod_deps,
    install: true,
    install_dir : mod_install_dir
)

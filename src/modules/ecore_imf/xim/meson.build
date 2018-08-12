mod_src = files([
  'ecore_imf_xim.c',
])

mod_deps = [ecore, ecore_imf, ecore_input, ecore_x, dependency('ibus-1.0'), dependency('glib-2.0'), ecore_evas]

shared_module(mod_name,
    mod_src,
    dependencies: mod_deps,
    install: true,
    install_dir : mod_install_dir
)

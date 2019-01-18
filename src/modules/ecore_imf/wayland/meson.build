mod_src = files([
  'wayland_module.c',
  'wayland_imcontext.c',
  'wayland_imcontext.h'
])
mod_deps = [ecore, ecore_imf, ecore_wl2, wayland_protocol, ecore_evas, ecore_input]

shared_module(mod_name,
    mod_src,
    dependencies: mod_deps,
    install: true,
    install_dir : mod_install_dir
)

mod_src = files([
  'scim_module.cpp',
  'scim_imcontext.cpp',
  'scim_imcontext.h'
])
mod_deps = [ecore, ecore_imf, ecore_x, dependency('scim-1.0'), dependency('glib-2.0')]


shared_module(mod_name,
    mod_src,
    dependencies: mod_deps,
    install: true,
    install_dir : mod_install_dir
)

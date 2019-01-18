ecore_systemd_mod = shared_module('tizen',
    'ecore_system_tizen.c',
    dependencies: [ecore, eldbus],
    install: true,
    install_dir: mod_install_dir,
)
module_files += join_paths(mod_install_dir, 'libtizen.' + sys_mod_extension)

ecore_systemd_mod = shared_module('upower',
    'ecore_system_upower.c',
    dependencies: [ecore, eldbus],
    install: true,
    install_dir: mod_install_dir,
)
module_files += join_paths(mod_install_dir, 'libupower' + sys_mod_extension)

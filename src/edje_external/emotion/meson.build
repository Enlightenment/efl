ecore_systemd_mod = shared_module('emotion',
    'emotion.c',
    dependencies: [edje, emotion],
    install: true,
    install_dir: mod_install_dir,
    c_args : emotion_package_c_args
)
module_files += join_paths(mod_install_dir, 'libemotion.' + sys_mod_extension)

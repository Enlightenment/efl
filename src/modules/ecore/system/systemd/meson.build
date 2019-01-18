if get_option('systemd')
  ecore_systemd_mod = shared_module('systemd',
      'ecore_system_systemd.c',
      dependencies: [ecore, eldbus],
      install: true,
      install_dir: mod_install_dir,
  )
  module_files += join_paths(mod_install_dir, 'libsystemd.' + sys_mod_extension)
endif

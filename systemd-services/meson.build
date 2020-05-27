if get_option('systemd') == true
  sys_dep = dependency('systemd', required: false)
  if sys_dep.found()
    sys_dir = sys_dep.get_pkgconfig_variable('systemduserunitdir')
  else
    sys_dir = join_paths(dir_lib, 'systemd', 'user')
  endif

  ethumb_service_config = configuration_data()
  ethumb_service_config.set('prefix', dir_prefix)

  configure_file(input         : 'ethumb.service.in',
                 output        : 'ethumb.service',
                 configuration : ethumb_service_config,
                 install_dir   : sys_dir)
endif

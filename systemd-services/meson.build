if get_option('systemd') == true
  dep = dependency('systemd')

  ethumb_service_config = configuration_data()

  ethumb_service_config.set('prefix', dir_prefix)

  configure_file(
    input: 'ethumb.service.in',
    output: 'ethumb.service',
    configuration: ethumb_service_config,
    install_dir: dep.get_pkgconfig_variable('systemduserunitdir'))
endif

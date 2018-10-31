dep = dependency('dbus-1')

org_enlightenment_ethumb_service_config = configuration_data()

org_enlightenment_ethumb_service_config.set('prefix', dir_prefix)

systemd_dbus_prefix = '# '
if get_option('systemd') == true
  systemd_dbus_prefix = ''
endif
org_enlightenment_ethumb_service_config.set('systemd_dbus_prefix',
                                            systemd_dbus_prefix)

configure_file(
  input: 'org.enlightenment.Ethumb.service.in',
  output: 'org.enlightenment.Ethumb.service',
  configuration: org_enlightenment_ethumb_service_config,
  install_dir: dep.get_pkgconfig_variable('session_bus_services_dir'))

#FIXME
#weird to have here ... but oooooh well and its not correct aaarg
efl_config_h = configuration_data()
efl_config_h.set('EFL_VERSION_MAJOR', version_major)
efl_config_h.set('EFL_VERSION_MINOR', version_minor)
efl_config_h.set('EFL_VERSION_MICRO', version_micro)
efl_config_h.set('EFL_BUILD_ID', get_option('build-id'))

#FIXME placeholder
efl_config_h.set('EFL_API_EO_DEF', '#define EFL_API_EO_DEF "FIXME NOT IMPLEMENTED"')
efl_config_h.set('EFL_API_LEGACY_DEF', '#define EFL_API_LEGACY_DEF "FIXME NOT IMPLEMENTED"')

efl_config_file = configure_file(
  input: '../efl/Efl_Config.h.in',
  output: 'Efl_Config.h',
  configuration: efl_config_h)

install_headers(efl_config_file)

config_dir += include_directories('.')

#end FIXME

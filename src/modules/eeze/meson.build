sensors = [
['fake', [ecore]], 
#['tizen', []], 
['udev', [ecore]],
]

foreach sensor_conf : sensors
  sensor = sensor_conf[0]
  src = join_paths('sensor', sensor, sensor+'.c')
  shared_module('eeze_'+sensor, src,
    include_directories : config_dir,
    dependencies : [eina, eeze] + sensor_conf[1],
    install : true,
    install_dir : join_paths(dir_package_modules, 'sensor', sensor, module_arch)
  )
  module_files += join_paths(dir_package_modules, 'sensor', sensor, module_arch, 'libeeze_' + sensor + '.so')
endforeach
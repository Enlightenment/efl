sensors = [
['fake', [ecore]],
#['tizen', []],
['udev', [ecore]],
]

foreach sensor_conf : sensors
  module_name = sensor_conf[0]
  subdir(join_paths('sensor', module_name))
endforeach

generic_src = [files(['emotion.c'])]

shared_module(emotion_loader+'_el',
  generic_src,
  include_directories : config_dir,
  dependencies: [ethumb, edje, eina, eo, eet, ecore_file, ecore, evas, ecore_evas, emotion],
  install: true,
  install_dir : mod_install_dir,
  c_args : package_c_args,
)

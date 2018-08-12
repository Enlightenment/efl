emotion_loaders = [
'emotion'
]

foreach emotion_loader : emotion_loaders
  generic_src = []
  generic_deps = []
  mod_install_dir = join_paths(dir_lib, 'ethumb', 'modules', emotion_loader, module_arch)

  subdir(emotion_loader)

  module_files += join_paths(mod_install_dir, 'lib'+emotion_loader+'_el.so')
endforeach

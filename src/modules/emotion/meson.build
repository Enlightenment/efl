emotion_loaders = [
'gstreamer',
'gstreamer1',
'libvlc',
'xine'
]

if sys_windows == false
  emotion_loaders += 'generic'
endif

foreach emotion_loader : emotion_loaders
  generic_src = []
  generic_deps = []
  mod_install_dir = join_paths(dir_lib, 'emotion', 'modules', emotion_loader, version_name)

  if get_option('emotion-loaders-disabler').contains(emotion_loader) == false
    subdir(emotion_loader)
    module_files += join_paths(mod_install_dir, 'lib'+emotion_loader+'.'+sys_mod_extension)
    config_h.set('EMOTION_BUILD_'+emotion_loader.to_upper(), 1)
  endif
endforeach

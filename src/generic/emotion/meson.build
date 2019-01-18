generic_loaders = ['vlc']

foreach loader : generic_loaders
  if get_option('emotion-generic-loaders-disabler').contains(loader) == false
    subdir(loader)
  endif
endforeach


vlc = dependency('libvlc')

executable('vlc',
  'emotion_generic_vlc.c',
  dependencies: [emotion_generic, eina, ecore, rt, vlc],
  install: true,
  install_dir: join_paths(dir_lib, 'emotion', 'generic_players', version_name)
)

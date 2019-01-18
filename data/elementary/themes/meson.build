edc_files = [
  'default.edc',
]

elm_themes = []

elm_themes_image_include = join_paths(meson.current_source_dir(), 'img')

foreach edc_file : edc_files
   elm_themes += custom_target('edje_cc_' + edc_file,
    input : edc_file,
    depfile: '@BASENAME@.edj.d',
    output : '@BASENAME@.edj',
    command : edje_cc_exe + [ '-beta',
              '-deps', '@DEPFILE@',
              '-sd', join_paths(meson.current_source_dir(), 'snd'),
              '-id', elm_themes_image_include,
              '-id', join_paths(meson.current_source_dir(), 'fdo'),
              '-fd', join_paths(meson.current_source_dir(), 'fnt'),
              '@INPUT@', '@OUTPUT@'],
    depends : edje_cc,
    install : true,
    install_dir : join_paths(dir_data, 'elementary', 'themes'),
   )
endforeach

install_subdir('fdo',
  install_dir : join_paths(dir_data, 'icons', 'Enlightenment-X'),
  strip_directory : true,
  exclude_directories : [
    join_paths('status', '128-svg'),
    join_paths('places', '16-svg'),
    join_paths('places', '128-svg'),
    join_paths('emblems', '96-svg'),
    join_paths('devices', '128-svg'),
    join_paths('categories', '128-svg'),
    join_paths('actions', '128-svg'),
  ]
)

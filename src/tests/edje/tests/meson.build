edc_files = [
  'edje_inherit_nearest.edc',
]

foreach edc_file : edc_files
   themes += custom_target('edje_cc_' + edc_file,
    input : edc_file,
    output : '@BASENAME@.edj',
    command : edje_cc_exe + [ '-beta', '-fastcomp',
              '-id', join_paths(meson.source_root(), 'src', 'tests', 'emotion', 'data'),
              '-fd', meson.current_source_dir(),
              '-sd', meson.current_source_dir(),
              '-vd', meson.current_source_dir(),
              '-dd', meson.current_source_dir(),
              '-md', meson.current_source_dir(),
              '-td', meson.current_source_dir(),
              '@INPUT@', '@OUTPUT@'],
    depends : edje_cc)
endforeach

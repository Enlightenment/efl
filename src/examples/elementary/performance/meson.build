examples = [
   'performance.c',
   'camera_light.c',
   'graphical.c',
]

edc_files = [
  'layout.edc'
]

themes = []

foreach edc_file : edc_files
   themes += custom_target('edje_cc_' + edc_file,
    depends : edje_cc,
    input : edc_file,
    output : '@BASENAME@.edj',
    command : edje_cc_exe + ['-beta',
              '-id', meson.current_source_dir(),
              '-fd', meson.current_source_dir(),
              '-sd', meson.current_source_dir(),
              '-vd', meson.current_source_dir(),
              '-dd', meson.current_source_dir(),
              '-md', meson.current_source_dir(),
              '-td', meson.current_source_dir(),
              '@INPUT@', '@OUTPUT@'],
   )
endforeach

executable(example, [examples] + themes, dependencies: [elementary, ecore])


examples = [
   'evas_3d_sphere_hunter'
]

edc_files = [
  'sphere_hunter.edc'
]

themes = []

foreach edc_file : edc_files
   themes += custom_target('edje_cc_' + edc_file,
    depends : edje_cc,
    input : edc_file,
    output : '@BASENAME@.edj',
    command : edje_cc_exe + ['-beta',
              '-id', elementary_img_data,
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

foreach example : examples
  executable(example, [example + '.c'] + themes, dependencies: [elementary, ecore])
endforeach

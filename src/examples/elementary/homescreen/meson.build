
foreach edc_file : ['button_theme.edc']
   themes += custom_target('edje_cc_' + edc_file,
    depends : edje_depends,
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
executable('homescreen',
  ['homescreen.c', themes],
  dependencies: elementary,
)

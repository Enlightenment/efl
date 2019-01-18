themes = []

edc_files = [
  'template.edc'
]

foreach edc_file : edc_files
  themes += custom_target('edje_cc_' + edc_file,
    input : edc_file,
    output : '@BASENAME@.edj',
    command : edje_cc_exe + [ '-beta',
              '-id', meson.current_source_dir(),
              '-fd', meson.current_source_dir(),
              '-sd', meson.current_source_dir(),
              '-vd', meson.current_source_dir(),
              '-dd', meson.current_source_dir(),
              '-md', meson.current_source_dir(),
              '-td', meson.current_source_dir(),
              '@INPUT@', '@OUTPUT@'],
    depends : edje_cc)
endforeach

generic_src = [files(['emotion.c']) + themes]

shared_module(emotion_loader+'_el',
  generic_src,
  include_directories : config_dir,
  dependencies: [ethumb, edje, eina, eo, eet, ecore_file, ecore, evas, ecore_evas, emotion],
  install: true,
  install_dir : mod_install_dir,
  c_args : package_c_args,
)

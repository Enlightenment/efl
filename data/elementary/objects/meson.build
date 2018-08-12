edc_files = [
  'test.edc',
]

elm_themes = []

foreach edc_file : edc_files
   elm_themes += custom_target('edje_cc_' + edc_file,
    input : edc_file,
    output : '@BASENAME@.edj',
    command : ['/usr/bin/env', 'EFL_RUN_IN_TREE=1', edje_cc.full_path(), '-beta',
              '-sd', join_paths(meson.current_source_dir()),
              '-id', join_paths(meson.current_source_dir()),
              '-id', join_paths(meson.current_source_dir()),
              '-fd', join_paths(meson.current_source_dir()),
              '@INPUT@', '@OUTPUT@'],
    depends : edje_cc,
    install : true,
    install_dir : join_paths(dir_data, 'elementary', 'themes'),
   )
endforeach
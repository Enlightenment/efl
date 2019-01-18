edc_files = [
  'complex_layout.edc',
  'test_box.edc',
  'test_color_class.edc',
  'test_combine_keywords.edc',
  'test_filters.edc',
  'test_layout.edc',
  'test_masking.edc',
  'test_messages.edc',
  'test_parens.edc',
  'test_signal_callback_del_full.edc',
  'test_signals.edc',
  'test_size_class.edc',
  'test_snapshot.edc',
  'test_swallows.edc',
  'test_table.edc',
  'test_text_cursor.edc',
  'test_textblock.edc',
  'test_text.edc',
]

themes = []

foreach edc_file : edc_files
   themes += custom_target('edje_cc_' + edc_file,
    input : edc_file,
    output : '@BASENAME@.edj',
    command : edje_cc_exe + [ '-beta',
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

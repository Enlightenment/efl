priv_eo_files = [
   'focus_test.eo',
   'focus_test_sub_main.eo'
]

priv_eo_file_target = []
foreach eo_file : priv_eo_files
  priv_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
endforeach


elementary_suite_src = [
  'elm_suite.c',
  'elm_test_atspi.c',
  'elm_test_check.c',
  'elm_test_colorselector.c',
  'elm_test_entry.c',
  'elm_test_init.c',
  'elm_test_list.c',
  'elm_test_button.c',
  'elm_test_image.c',
  'elm_test_photo.c',
  'elm_test_actionslider.c',
  'elm_test_box.c',
  'elm_test_table.c',
  'elm_test_thumb.c',
  'elm_test_menu.c',
  'elm_test_photocam.c',
  'elm_test_win.c',
  'elm_test_icon.c',
  'elm_test_prefs.c',
  'elm_test_map.c',
  'elm_test_glview.c',
  'elm_test_web.c',
  'elm_test_toolbar.c',
  'elm_test_grid.c',
  'elm_test_diskselector.c',
  'elm_test_notify.c',
  'elm_test_mapbuf.c',
  'elm_test_flip.c',
  'elm_test_layout.c',
  'elm_test_slider.c',
  'elm_test_ctxpopup.c',
  'elm_test_separator.c',
  'elm_test_calendar.c',
  'elm_test_inwin.c',
  'elm_test_gengrid.c',
  'elm_test_radio.c',
  'elm_test_scroller.c',
  'elm_test_frame.c',
  'elm_test_datetime.c',
  'elm_test_player.c',
  'elm_test_bg.c',
  'elm_test_video.c',
  'elm_test_segmentcontrol.c',
  'elm_test_progressbar.c',
  'elm_test_fileselector.c',
  'elm_test_fileselector_button.c',
  'elm_test_fileselector_entry.c',
  'elm_test_hoversel.c',
  'elm_test_multibuttonentry.c',
  'elm_test_naviframe.c',
  'elm_test_popup.c',
  'elm_test_bubble.c',
  'elm_test_clock.c',
  'elm_test_conformant.c',
  'elm_test_dayselector.c',
  'elm_test_flipselector.c',
  'elm_test_genlist.c',
  'elm_test_hover.c',
  'elm_test_index.c',
  'elm_test_label.c',
  'elm_test_panel.c',
  'elm_test_panes.c',
  'elm_test_slideshow.c',
  'elm_test_spinner.c',
  'elm_test_plug.c',
  'elm_test_config.c',
  'elm_code_file_test_load.c',
  'elm_code_file_test_memory.c',
  'elm_code_test_basic.c',
  'elm_code_test_line.c',
  'elm_code_test_parse.c',
  'elm_code_test_syntax.c',
  'elm_code_test_text.c',
  'elm_code_test_indent.c',
  'elm_code_test_widget.c',
  'elm_code_test_widget_text.c',
  'elm_code_test_widget_selection.c',
  'elm_code_test_widget_undo.c',
  'elm_test_focus_common.c',
  'elm_test_focus_common.h',
  'elm_test_focus.c',
  'elm_test_focus_sub.c',
  'efl_ui_test_grid.c',
  'elm_test_widget_focus.c'
]

elementary_suite = executable('elementary_suite',
  elementary_suite_src, priv_eo_file_target,
  dependencies: [check, eina, elementary, elementary_deps],
  include_directories : [config_dir] + [elementary_config_dir],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"',
  '-DELM_IMAGE_DATA_DIR="'+join_paths(meson.source_root(), 'data', 'elementary')+'"',
  '-DELM_TEST_DATA_DIR="'+join_paths(meson.build_root(), 'data', 'elementary')+'"',
  ]
)

efl_ui_suite_src = [
  'efl_ui_suite.c',
]

efl_ui_suite = executable('efl_ui_suite',
  efl_ui_suite_src, priv_eo_file_target,
  dependencies: [check, eina, elementary, elementary_deps],
  include_directories : [config_dir] + [elementary_config_dir],
  c_args : [
  '-DTESTS_BUILD_DIR="'+meson.current_build_dir()+'"',
  '-DTESTS_SRC_DIR="'+meson.current_source_dir()+'"',
  '-DELM_IMAGE_DATA_DIR="'+join_paths(meson.source_root(), 'data', 'elementary')+'"',
  '-DELM_TEST_DATA_DIR="'+join_paths(meson.build_root(), 'data', 'elementary')+'"',
  ]
)

test('elementary-suite', elementary_suite,
  env : test_env
)

test('efl-ui-suite', efl_ui_suite,
  env : test_env
)

install_data(files(['testdiff.diff', 'testfile-windows.txt', 'testfile-withblanks.txt', 'testfile.txt']),
  install_dir : join_paths(dir_data, 'elementary')
)

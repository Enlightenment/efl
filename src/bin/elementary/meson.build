elementary_test_src = [
  'test.c',
  'test_explode.c',
  'test_explode.h',
  'test_3d.c',
  'test_access.c',
  'test_actionslider.c',
  'test_anim.c',
  'test_bg.c',
  'test_box.c',
  'test_bubble.c',
  'test_button.c',
  'test_ui_button.c',
  'test_calendar.c',
  'test_check.c',
  'test_clock.c',
  'test_cnp.c',
  'test_code.c',
  'test_colorselector.c',
  'test_colorclass.c',
  'test_combobox.c',
  'test_config.c',
  'test_conform.c',
  'test_conform_indicator.c',
  'test_ctxpopup.c',
  'test_cursor.c',
  'test_datetime.c',
  'test_dayselector.c',
  'test_disable.c',
  'test_diskselector.c',
  'test_dnd.c',
  'test_efl_anim_alpha.c',
  'test_efl_anim_rotate.c',
  'test_efl_anim_scale.c',
  'test_efl_anim_translate.c',
  'test_efl_anim_group_parallel.c',
  'test_efl_anim_group_sequential.c',
  'test_efl_anim_event_anim.c',
  'test_efl_anim_pause.c',
  'test_efl_anim_repeat.c',
  'test_efl_anim_start_delay.c',
  'test_efl_anim_interpolator.c',
  'test_eio.c',
  'test_entry.c',
  'test_entry_anchor.c',
  'test_entry_anchor2.c',
  'test_events.c',
  'test_evas_mask.c',
  'test_evas_map.c',
  'test_efl_gfx_map.c',
  'test_evas_snapshot.c',
  'test_external.c',
  'test_fileselector_button.c',
  'test_fileselector.c',
  'test_fileselector_entry.c',
  'test_flip.c',
  'test_flip_page.c',
  'test_flip_page_eo.c',
  'test_flipselector.c',
  'test_floating.c',
  'test_focus.c',
  'test_focus_custom_chain.c',
  'test_focus_policy.c',
  'test_focus_style.c',
  'test_gengrid.c',
  'test_genlist.c',
  'test_gesture_layer.c',
  'test_gesture_layer2.c',
  'test_gesture_layer3.c',
  'test_gfx_filters.c',
  'test_glview_simple.c',
  'test_glview.c',
  'test_glview_manygears.c',
  'test_grid.c',
  'test_ui_table_static.c',
  'test_ui_stack.c',
  'test_hover.c',
  'test_hoversel.c',
  'test_icon.c',
  'test_icon_desktops.c',
  'test_icon_animated.c',
  'test_image.c',
  'test_index.c',
  'test_inwin.c',
  'test_label.c',
  'test_launcher.c',
  'test_layout.c',
  'test_list.c',
  'test_map.c',
  'test_main_menu.c',
  'test_menu.c',
  'test_multi.c',
  'test_multibuttonentry.c',
  'test_naviframe.c',
  'test_naviframe_complex.c',
  'test_notify.c',
  'test_nstate.c',
  'test_panel.c',
  'test_panes.c',
  'test_ui_panes.c',
  'test_ui_panel.c',
  'test_ui_pager.c',
  'test_ui_pager_scroll.c',
  'test_part_bg.c',
  'test_part_shadow.c',
  'test_photo.c',
  'test_photocam.c',
  'test_popup.c',
  'test_prefs.c',
  'test_progressbar.c',
  'test_ui_progressbar.c',
  'test_radio.c',
  'test_scaling.c',
  'test_scroller.c',
  'test_ui_scroller.c',
  'test_segment_control.c',
  'test_separator.c',
  'test_slider.c',
  'test_ui_slider.c',
  'test_ui_slider_interval.c',
  'test_ui_spin.c',
  'test_ui_spin_button.c',
  'test_ui_datepicker.c',
  'test_ui_timepicker.c',
  'test_ui_tags.c',
  'test_slideshow.c',
  'test_spinner.c',
  'test_store.c',
  'test_sys_notify.c',
  'test_systray.c',
  'test_table.c',
  'test_thumb.c',
  'test_toolbar.c',
  'test_tooltip.c',
  'test_transit.c',
  'test_transit_bezier.c',
  'test_ui_box.c',
  'test_ui_clock.c',
  'test_ui_table.c',
  'test_ui_popup.c',
  'test_ui_textpath.c',
  'test_video.c',
  'test_weather.c',
  'test_web.c',
  'test_win_inline.c',
  'test_win_keygrab.c',
  'test_win_socket.c',
  'test_win_plug.c',
  'test_win_state.c',
  'test_win_wm_rotation.c',
  'test_win_dialog.c',
  'test_win_modal.c',
  'test_efl_ui_text.c',
  'test_win_stack.c',
  'test_win_indicator.c',
  'test_gesture_framework.c',
  'test_ui_tab_pager.c',
  'test.h'
]

if sys_windows == false
  link_args = ['-rdynamic', '-fPIC', '-pie']
  package_c_args =  package_c_args + ['-fPIC']
else
  link_args = []
endif
elementary_test = executable('elementary_test',
        elementary_test_src,
        dependencies: [elementary] + elementary_deps + elementary_pub_deps,
        install: true,
        c_args : package_c_args + [
          '-Delementary_test_BIN_DIR="'+dir_bin+'"',
          '-Delementary_test_LIB_DIR="'+dir_lib+'"',
          '-Delementary_test_DATA_DIR="'+join_paths(dir_data,'elementary')+'"'
        ],
        link_args: link_args
)

elementary_config_src = [
   'config.c'
]

elementary_config = executable('elementary_config',
        elementary_config_src,
        dependencies: [elementary] + elementary_deps + elementary_pub_deps,
        install: true,
        c_args : package_c_args,
        link_args: link_args
)

if sys_windows == false
  elementary_quicklaunch_src = [
     'quicklaunch.c'
  ]

  elementary_quicklaunch = executable('elementary_quicklaunch',
          elementary_quicklaunch_src,
          dependencies: [elementary] + elementary_deps + elementary_pub_deps,
          install: true,
          c_args : package_c_args,
          link_args: link_args
  )
endif

elementary_codegen_src = [
   'elementary_codegen.c'
]

elementary_codegen = executable('elementary_codegen',
        elementary_codegen_src,
        dependencies: [elementary] + elementary_deps + elementary_pub_deps,
        install: true,
        c_args : package_c_args,
        link_args: link_args
)

elm_prefs_cc_src = [
   'elm_prefs_cc.h',
   'elm_prefs_cc.c',
   'elm_prefs_cc_mem.c',
   'elm_prefs_cc_out.c',
   'elm_prefs_cc_parse.c',
   'elm_prefs_cc_handlers.c'
]

elm_prefs_cc = executable('elm_prefs_cc',
        elm_prefs_cc_src,
        dependencies: [elementary] + elementary_deps + elementary_pub_deps,
        install: true,
        c_args : package_c_args,
        link_args: link_args
)

if meson.is_cross_build()
  _elm_prefs_cc = find_program('elm_prefs_cc', native: true)
  elm_prefs_cc_path = _elm_prefs_cc.path()
  elm_prefs_cc_exe = [_elm_prefs_cc]
else
  env = find_program('env', native: true)
  elm_prefs_cc_exe = [env, 'EFL_RUN_IN_TREE=1', elm_prefs_cc.full_path()]
endif

if sys_windows == false
  elementary_run_src = [
     'run.c'
  ]

  elementary_run = executable('elementary_run',
          elementary_run_src,
          dependencies: [elementary] + elementary_deps + elementary_pub_deps,
          install: true,
          c_args : package_c_args,
          link_args: link_args
  )
endif

elementary_perf_src = [
   'perf.c',
   'perf_rnd.c',
   'perf_test_01.c',
   'perf_test_02.c',
   'perf_test_03.c',
   'perf_test_04.c',
   'perf_test_05.c',
   'perf_test_06.c'
]

elementary_perf = executable('elementary_perf',
        elementary_perf_src,
        dependencies: [elementary] + elementary_deps + elementary_pub_deps,
        install: true,
        c_args : package_c_args + [
          '-Delementary_test_BIN_DIR="'+dir_bin+'"',
          '-Delementary_test_LIB_DIR="'+dir_lib+'"',
          '-Delementary_test_DATA_DIR="'+join_paths(dir_data,'elementary')+'"'
        ],
        link_args: link_args
)


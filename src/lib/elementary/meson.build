pub_legacy_eo_files = [
  'efl_ui_clock_legacy.eo',
  'elm_ctxpopup_part.eo',
  'elm_dayselector_part.eo',
  'elm_entry_part.eo',
  'elm_fileselector_entry_part.eo',
  'elm_flip_part.eo',
  'elm_hover_part.eo',
  'elm_mapbuf_part.eo',
  'elm_naviframe_part.eo',
  'elm_notify_part.eo',
  'elm_panel_part.eo',
  'elm_player_part.eo',
  'elm_popup_part.eo',
  'elm_scroller_part.eo',
  'elm_label_part.eo',
  'elm_actionslider_part.eo',
  'elm_bubble_part.eo',
  'elm_fileselector_part.eo',
  'elm_multibuttonentry_part.eo',
  'elm_interface_scrollable.eo',
  'elm_code_widget.eo',
]

pub_eo_file_target = []

foreach eo_file : pub_legacy_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : dir_package_include,
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])

endforeach

pub_eo_files = [
  'efl_ui_widget.eo',
  'efl_ui_widget_scrollable_content.eo',
  'efl_ui_vg_animation.eo',
  'efl_ui_bg.eo',
  'efl_ui_button.eo',
  'efl_ui_calendar.eo',
  'efl_ui_calendar_item.eo',
  'efl_ui_check.eo',
  'efl_ui_flip.eo',
  'efl_ui_frame.eo',
  'efl_ui_image.eo',
  'efl_ui_image_zoomable.eo',
  'efl_ui_layout.eo',
  'efl_ui_layout_base.eo',
  'efl_ui_navigation_bar.eo',
  'efl_ui_navigation_bar_part.eo',
  'efl_ui_navigation_bar_part_back_button.eo',
  'efl_ui_navigation_layout.eo',
  'efl_ui_panes.eo',
  'efl_ui_progressbar.eo',
  'efl_ui_radio.eo',
  'efl_ui_radio_group.eo',
  'efl_ui_radio_box.eo',
  'efl_ui_radio_group_impl.eo',
  'efl_ui_slider.eo',
  'efl_ui_slider_interval.eo',
  'efl_ui_spin.eo',
  'efl_ui_spin_button.eo',
  'efl_ui_panel.eo',
  'efl_ui_datepicker.eo',
  'efl_ui_timepicker.eo',
  'efl_ui_tags.eo',
  'efl_ui_video.eo',
  'efl_ui_win.eo',
  'efl_ui_win_inlined.eo',
  'efl_ui_win_socket.eo',
  'efl_ui_box.eo',
  'efl_ui_box_flow.eo',
  'efl_ui_box_stack.eo',
  'efl_ui_table.eo',
  'efl_ui_table_static.eo',
  'efl_text_interactive.eo',
  'efl_ui_textbox.eo',
  'efl_ui_popup.eo',
  'efl_ui_popup_part_backwall.eo',
  'efl_ui_alert_popup.eo',
  'efl_ui_alert_popup_part_title.eo',
  'efl_ui_textbox_async.eo',
  'efl_ui_text_factory_images.eo',
  'efl_ui_text_factory_emoticons.eo',
  'efl_ui_text_factory_fallback.eo',
  'efl_ui_textpath.eo',
  'efl_ui_l10n.eo',
  'efl_ui_clock.eo',
  'efl_ui_image_factory.eo',
  'efl_ui_focus_manager.eo',
  'efl_ui_focus_manager_calc.eo',
  'efl_ui_focus_manager_sub.eo',
  'efl_ui_focus_manager_root_focus.eo',
  'efl_ui_focus_object.eo',
  'efl_ui_focus_layer.eo',
  'efl_ui_focus_composition.eo',
  'efl_ui_focus_util.eo',
  'efl_ui_flip_part.eo',
  'efl_ui_layout_factory.eo',
  'efl_ui_layout_part.eo',
  'efl_ui_layout_part_box.eo',
  'efl_ui_layout_part_content.eo',
  'efl_ui_layout_part_text.eo',
  'efl_ui_layout_part_table.eo',
  'efl_ui_layout_part_bg.eo',
  'efl_ui_layout_part_legacy.eo',
  'efl_ui_list_view.eo',
  'efl_ui_item.eo',
  'efl_ui_default_item.eo',
  'efl_ui_group_item.eo',
  'efl_ui_list_default_item.eo',
  'efl_ui_list_placeholder_item.eo',
  'efl_ui_list.eo',
  'efl_ui_grid_default_item.eo',
  'efl_ui_grid.eo',
  'efl_ui_panes_part.eo',
  'efl_ui_progressbar_part.eo',
  'efl_ui_textpath_part.eo',
  'efl_ui_widget_part.eo',
  'efl_ui_widget_part_bg.eo',
  'efl_ui_widget_part_shadow.eo',
  'efl_ui_win_part.eo',
  'efl_ui_scroller.eo',
  'efl_ui_scroll_manager.eo',
  'efl_ui_pan.eo',
  'efl_access_object.eo',
  'efl_access_action.eo',
  'efl_access_component.eo',
  'efl_access_editable_text.eo',
  'efl_access_selection.eo',
  'efl_access_text.eo',
  'efl_access_value.eo',
  'efl_access_widget_action.eo',
  'efl_access_window.eo',
  'efl_ui_theme.eo',
  'efl_config_global.eo',
  'efl_ui_selection.eo',
  'efl_ui_dnd.eo',
  'efl_ui_focus_manager_window_root.eo',
  'efl_ui_spotlight_container.eo',
  'efl_ui_spotlight_manager.eo',
  'efl_ui_spotlight_scroll_manager.eo',
  'efl_ui_spotlight_fade_manager.eo',
  'efl_ui_spotlight_indicator.eo',
  'efl_ui_spotlight_icon_indicator.eo',
  'efl_ui_spotlight_util.eo',
  'efl_ui_tab_pager.eo',
  'efl_ui_tab_bar.eo',
  'efl_ui_tab_page.eo',
  'efl_ui_widget_focus_manager.eo',
  'efl_ui_legacy.eo',
  'efl_ui_button_legacy_part.eo',
  'efl_ui_radio_legacy_part.eo',
  'efl_ui_check_legacy_part.eo',
  'efl_ui_progressbar_legacy_part.eo',
  'efl_ui_image_zoomable_pan.eo',
  'efl_ui_textbox_part.eo',
  'efl_ui_caching_factory.eo',
  'efl_ui_widget_factory.eo',
  'efl_ui_property_bind_part.eo',
  'efl_ui_relative_container.eo',
  'efl_ui_action_connector.eo',
  'efl_ui_format.eo',
  'efl_ui_item_clickable.eo',
  'efl_ui_collection.eo',
  'efl_ui_position_manager_entity.eo',
  'efl_ui_position_manager_list.eo',
  'efl_ui_position_manager_grid.eo',
  'efl_ui_selectable.eo',
  'efl_ui_multi_selectable.eo',
  'efl_ui_multi_selectable_object_range.eo',
  'efl_ui_multi_selectable_index_range.eo',
  'efl_ui_single_selectable.eo',
  'efl_ui_position_manager_data_access_v1.eo',
  'efl_ui_tab_bar_default_item.eo',
  'efl_ui_select_model.eo',
  'efl_ui_view_model.eo',
  'efl_ui_collection_view.eo',
  'efl_ui_collection_view_focus_manager.eo',
  'efl_ui_grid_view.eo',
  'efl_ui_pager.eo',
  'efl_ui_stack.eo',
  'efl_ui_separator.eo',
  'efl_ui_spotlight_animation_manager.eo',
]

foreach eo_file : pub_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : dir_package_include,
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
endforeach


pub_eo_types_files = [
  'elm_general.eot',
  'efl_ui.eot',
]

foreach eo_file : pub_eo_types_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : dir_package_include,
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
endforeach

priv_eo_files = [
  'efl_ui_internal_text_interactive.eo',
  'efl_ui_internal_text_scroller.eo',
  'efl_ui_focus_composition_adapter.eo',
  'efl_ui_focus_parent_provider.eo',
  'efl_ui_focus_parent_provider_standard.eo',
  'efl_ui_state_model.eo',
  'efl_datetime_manager.eo',
  'efl_ui_size_model.eo',
  'efl_ui_homogeneous_model.eo',
  'efl_ui_exact_model.eo',
  'efl_ui_average_model.eo',
  'efl_ui_spotlight_plain_manager.eo',
  'efl_ui_collection_focus_manager.eo',
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

eolian_include_directories += ['-I', meson.current_source_dir()]

embed_script = find_program('config_embed')

embed_config = custom_target('create_embedded_default_config',
  input: join_paths(meson.project_source_root(), 'data', 'elementary', 'config', 'standard', 'base.src.in'),
  output: 'elm_default_config.x',
  command: [embed_script, '@INPUT@', '@OUTPUT@']
)

elementary_headers_unstable = [
  'elm_gen_common.h',
  'efl_access_object.h',
  'efl_access_text.h',
  'efl_access_widget_action.h',
  'efl_access_window.h',
  'elm_interface_fileselector.h',
  'elm_interface_scrollable.h',
  'elm_interfaces.h',
  'elm_widget.h',
  'elm_widget_actionslider.h',
  'elm_widget_box.h',
  'elm_widget_bubble.h',
  'efl_ui_button_private.h',
  'efl_ui_bg_private.h',
  'elm_widget_calendar.h',
  'efl_ui_calendar_private.h',
  'efl_ui_check_private.h',
  'elm_widget_clock.h',
  'elm_widget_colorselector.h',
  'elm_widget_conform.h',
  'elm_widget_container.h',
  'elm_widget_combobox.h',
  'elm_widget_ctxpopup.h',
  'elm_widget_dayselector.h',
  'elm_widget_diskselector.h',
  'elm_widget_entry.h',
  'elm_widget_fileselector.h',
  'elm_widget_fileselector_button.h',
  'elm_widget_fileselector_entry.h',
  'efl_ui_widget_flip.h',
  'elm_widget_flipselector.h',
  'efl_ui_widget_frame.h',
  'efl_ui_vg_animation_private.h',
  'elm_widget_gengrid.h',
  'elm_widget_genlist.h',
  'elm_widget_glview.h',
  'elm_widget_grid.h',
  'elm_widget_hover.h',
  'elm_widget_hoversel.h',
  'elm_widget_icon.h',
  'efl_ui_widget_image.h',
  'efl_ui_popup_private.h',
  'efl_ui_alert_popup_private.h',
  'elm_widget_index.h',
  'elm_widget_inwin.h',
  'elm_widget_label.h',
  'elm_widget_layout.h',
  'elm_widget_list.h',
  'elm_widget_map.h',
  'elm_widget_mapbuf.h',
  'elm_widget_menu.h',
  'elm_widget_multibuttonentry.h',
  'elm_widget_naviframe.h',
  'efl_ui_navigation_bar_private.h',
  'efl_ui_navigation_layout_private.h',
  'elm_widget_notify.h',
  'elm_widget_panel.h',
  'efl_ui_panes_private.h',
  'elm_widget_photo.h',
  'efl_ui_image_zoomable_private.h',
  'elm_widget_player.h',
  'elm_widget_plug.h',
  'elm_widget_popup.h',
  'elm_widget_prefs.h',
  'efl_ui_progressbar_private.h',
  'efl_ui_radio_private.h',
  'elm_widget_route.h',
  'elm_widget_scroller.h',
  'elm_widget_segment_control.h',
  'elm_widget_separator.h',
  'elm_widget_slider.h',
  'efl_ui_slider_private.h',
  'efl_ui_slider_interval_private.h',
  'elm_widget_slideshow.h',
  'elm_widget_spinner.h',
  'efl_ui_spin_private.h',
  'efl_ui_spin_button_private.h',
  'efl_ui_panel_private.h',
  'efl_ui_datepicker_private.h',
  'efl_ui_timepicker_private.h',
  'efl_ui_tags_private.h',
  'elm_widget_table.h',
  'elm_widget_thumb.h',
  'elm_widget_toolbar.h',
  'efl_ui_video_private.h',
  'efl_ui_item_private.h',
  'elm_widget_web.h',
  'efl_ui_clock.h',
  'elm_code.h',
  'elm_code_widget_legacy.h',
  'elm_code_widget_selection.h',
  'elm_code_diff_widget.h',
  'elm_code_common.h',
  'elm_code_line.h',
  'elm_code_text.h',
  'elm_code_indent.h',
  'elm_code_file.h',
  'elm_code_parse.h',
  'elm_code_syntax.h',
  'efl_ui_widget_scroller.h',
  'efl_ui_widget_scroll_manager.h',
  'efl_ui_widget_pan.h',
  'Efl_Ui.h',
  'efl_ui_tab_pager_private.h',
  'efl_ui_tab_bar_private.h',
  'efl_ui_tab_page_private.h',
  'efl_ui_relative_container_private.h'
]

elementary_pub_headers = [
  'elc_ctxpopup.h',
  'elc_ctxpopup_legacy.h',
  'elc_combobox.h',
  'elc_combobox_legacy.h',
  'elc_fileselector.h',
  'elc_fileselector_legacy.h',
  'elc_fileselector_common.h',
  'elc_fileselector_button.h',
  'elc_fileselector_button_legacy.h',
  'elc_fileselector_entry.h',
  'elc_fileselector_entry_legacy.h',
  'elc_hoversel.h',
  'elc_hoversel_legacy.h',
  'elc_multibuttonentry.h',
  'elc_multibuttonentry_eo.h',
  'elc_multibuttonentry_legacy.h',
  'elc_multibuttonentry_common.h',
  'elc_naviframe.h',
  'elc_naviframe_legacy.h',
  'elc_naviframe_common.h',
  'elc_popup.h',
  'elc_popup_legacy.h',
  'efl_ui_vg_animation.h',
  'efl_ui_vg_animation_legacy.h',
  'elm_access.h',
  'elm_actionslider.h',
  'elm_actionslider_legacy.h',
  'elm_app.h',
  'elm_atspi_app_object.h',
  'elm_atspi_app_object_eo.legacy.h',
  'elm_atspi_bridge.h',
  'elm_atspi_bridge_eo.legacy.h',
  'elm_authors.h',
  'elm_bg.h',
  'elm_bg_legacy.h',
  'elm_box.h',
  'elm_box_legacy.h',
  'elm_box_common.h',
  'elm_bubble.h',
  'elm_bubble_legacy.h',
  'elm_button.h',
  'efl_ui_widget_common.h',
  'efl_ui_button_eo.h',
  'elm_button_legacy.h',
  'elm_cache.h',
  'elm_calendar.h',
  'elm_calendar_legacy.h',
  'elm_calendar_common.h',
  'elm_check.h',
  'efl_ui_check_eo.h',
  'elm_check_legacy.h',
  'elm_clock.h',
  'elm_clock_eo.h',
  'elm_clock_legacy.h',
  'elm_cnp.h',
  'elm_colorselector.h',
  'elm_colorselector_legacy.h',
  'elm_colorselector_common.h',
  'elm_color_class.h',
  'elm_config.h',
  'elm_conform.h',
  'elm_conform_legacy.h',
  'elm_cursor.h',
  'elm_datetime.h',
  'elm_dayselector.h',
  'elm_dayselector_legacy.h',
  'elm_debug.h',
  'elm_deprecated.h',
  'elm_diskselector.h',
  'elm_diskselector_common.h',
  'elm_diskselector_legacy.h',
  'elm_entry.h',
  'elm_entry_common.h',
  'elm_entry_eo.h',
  'elm_entry_legacy.h',
  'elm_finger.h',
  'elm_flip.h',
  'efl_ui_flip_eo.h',
  'efl_ui_flip_legacy.h',
  'elm_flipselector.h',
  'elm_flipselector_legacy.h',
  'elm_focus.h',
  'elm_focus_item.h',
  'elm_font.h',
  'elm_frame.h',
  'efl_ui_frame_eo.h',
  'efl_ui_frame_legacy.h',
  'elm_gen.h',
  'elm_general.h',
  'elm_gengrid.h',
  'elm_gengrid_common.h',
  'elm_gengrid_legacy.h',
  'elm_genlist.h',
  'elm_genlist_common.h',
  'elm_genlist_legacy.h',
  'elm_gesture_layer.h',
  'elm_gesture_layer_common.h',
  'elm_gesture_layer_legacy.h',
  'elm_getting_started.h',
  'elm_glview.h',
  'elm_glview_common.h',
  'elm_glview_legacy.h',
  'elm_grid.h',
  'elm_grid_legacy.h',
  'elm_hover.h',
  'elm_hover_legacy.h',
  'elm_icon.h',
  'elm_icon_legacy.h',
  'elm_image.h',
  'elm_image_eo.h',
  'elm_image_legacy.h',
  'elm_index.h',
  'elm_index_legacy.h',
  'elm_inwin.h',
  'elm_inwin_legacy.h',
  'elm_label.h',
  'elm_label_legacy.h',
  'elm_layout.h',
  'elm_layout_common.h',
  'elm_layout_legacy.h',
  'elm_list.h',
  'elm_list_legacy.h',
  'elm_macros.h',
  'elm_map.h',
  'elm_map_common.h',
  'elm_map_legacy.h',
  'elm_mapbuf.h',
  'elm_mapbuf_legacy.h',
  'elm_menu.h',
  'elm_menu_legacy.h',
  'elm_mirroring.h',
  'elm_need.h',
  'elm_notify.h',
  'elm_notify_common.h',
  'elm_notify_legacy.h',
  'elm_object.h',
  'elm_object_item.h',
  'elm_panel.h',
  'elm_panel_common.h',
  'elm_panel_legacy.h',
  'elm_panes.h',
  'efl_ui_panes_eo.h',
  'elm_panes_legacy.h',
  'elm_photo.h',
  'elm_photo_legacy.h',
  'elm_photocam.h',
  'efl_ui_image_zoomable_eo.h',
  'elm_photocam_legacy.h',
  'elm_plug.h',
  'elm_plug_legacy.h',
  'elm_prefs.h',
  'elm_prefs_common.h',
  'elm_prefs_legacy.h',
  'elm_prefs_data.h',
  'elm_progressbar.h',
  'elm_progressbar_common.h',
  'efl_ui_progressbar_eo.h',
  'elm_progressbar_legacy.h',
  'elm_radio.h',
  'efl_ui_radio_eo.h',
  'elm_radio_legacy.h',
  'elm_route.h',
  'elm_route_legacy.h',
  'elm_scale.h',
  'elm_scroll.h',
  'elm_scroller.h',
  'elm_scroller_legacy.h',
  'elm_segment_control.h',
  'elm_segment_control_legacy.h',
  'elm_separator.h',
  'elm_separator_legacy.h',
  'elm_slider.h',
  'elm_slider_common.h',
  'elm_slider_legacy.h',
  'elm_slideshow.h',
  'elm_slideshow_common.h',
  'elm_slideshow_legacy.h',
  'elm_spinner.h',
  'elm_spinner_legacy.h',
  'elm_store.h',
  'elm_systray.h',
  'elm_systray_common.h',
  'elm_systray_watcher.h',
  'elm_sys_notify.h',
  'elm_table.h',
  'elm_table_legacy.h',
  'elm_theme.h',
  'elm_thumb.h',
  'elm_thumb_common.h',
  'elm_thumb_legacy.h',
  'elm_textpath.h',
  'elm_textpath_legacy.h',
  'elm_toolbar.h',
  'elm_toolbar_common.h',
  'elm_toolbar_eo.h',
  'elm_toolbar_legacy.h',
  'elm_tooltip.h',
  'elm_transit.h',
  'efl_ui_video.h',
  'efl_ui_video_eo.h',
  'efl_ui_video_legacy.h',
  'elm_view_list.h',
  'elm_view_form.h',
  'elm_web.h',
  'elm_web_common.h',
  'elm_web_eo.h',
  'elm_web_legacy.h',
  'elm_win.h',
  'elm_win_common.h',
  'elm_win_eo.h',
  'elm_win_legacy.h',
  'elm_helper.h',
  'elm_part_helper.h',
  'efl_ui_vg_animation_eo.legacy.h',
  'efl_ui_bg_eo.legacy.h',
  'efl_ui_bg_legacy_eo.legacy.h',
  'efl_ui_button_eo.legacy.h',
  'efl_ui_button_legacy_eo.legacy.h',
  'efl_ui_check_eo.legacy.h',
  'efl_ui_check_legacy_eo.legacy.h',
  'efl_ui_flip_eo.legacy.h',
  'efl_ui_flip_legacy_eo.legacy.h',
  'efl_ui_frame_eo.legacy.h',
  'efl_ui_frame_legacy_eo.legacy.h',
  'efl_ui_image_legacy_eo.legacy.h',
  'efl_ui_image_zoomable_eo.legacy.h',
  'efl_ui_image_zoomable_legacy_eo.legacy.h',
  'efl_ui_layout_legacy_eo.legacy.h',
  'efl_ui_panes_eo.legacy.h',
  'efl_ui_panes_legacy_eo.legacy.h',
  'efl_ui_progressbar_legacy_eo.legacy.h',
  'efl_ui_radio_eo.legacy.h',
  'efl_ui_radio_legacy_eo.legacy.h',
  'efl_ui_textpath_eo.legacy.h',
  'efl_ui_textpath_legacy_eo.legacy.h',
  'efl_ui_video_eo.legacy.h',
  'efl_ui_video_legacy_eo.legacy.h',
  'efl_ui_widget_eo.legacy.h',
  'efl_ui_win_inlined_legacy_eo.legacy.h',
  'efl_ui_win_legacy_eo.legacy.h',
  'efl_ui_win_socket_legacy_eo.legacy.h',
  'elm_access_eo.legacy.h',
  'elm_actionslider_eo.legacy.h',
  'elm_box_eo.legacy.h',
  'elm_bubble_eo.legacy.h',
  'elm_calendar_eo.legacy.h',
  'elm_calendar_item_eo.legacy.h',
  'elm_clock_eo.legacy.h',
  'elm_code_widget_eo.legacy.h',
  'elm_code_widget_legacy_eo.legacy.h',
  'elm_color_item_eo.legacy.h',
  'elm_colorselector_eo.legacy.h',
  'elm_conformant_eo.legacy.h',
  'elm_ctxpopup_eo.legacy.h',
  'elm_ctxpopup_item_eo.legacy.h',
  'elm_dayselector_eo.legacy.h',
  'elm_dayselector_item_eo.legacy.h',
  'elm_diskselector_eo.legacy.h',
  'elm_diskselector_item_eo.legacy.h',
  'elm_entry_eo.legacy.h',
  'elm_fileselector_button_eo.legacy.h',
  'elm_fileselector_entry_eo.legacy.h',
  'elm_fileselector_eo.legacy.h',
  'elm_flipselector_eo.legacy.h',
  'elm_flipselector_item_eo.legacy.h',
  'elm_gengrid_eo.legacy.h',
  'elm_gengrid_item_eo.legacy.h',
  'elm_gengrid_pan_eo.legacy.h',
  'elm_genlist_eo.legacy.h',
  'elm_genlist_item_eo.legacy.h',
  'elm_genlist_pan_eo.legacy.h',
  'elm_gesture_layer_eo.legacy.h',
  'elm_glview_eo.legacy.h',
  'elm_grid_eo.legacy.h',
  'elm_hover_eo.legacy.h',
  'elm_hoversel_eo.legacy.h',
  'elm_hoversel_item_eo.legacy.h',
  'elm_icon_eo.legacy.h',
  'elm_index_eo.legacy.h',
  'elm_index_item_eo.legacy.h',
  'elm_interface_fileselector_eo.legacy.h',
  'elm_inwin_eo.legacy.h',
  'elm_label_eo.legacy.h',
  'elm_list_eo.legacy.h',
  'elm_list_item_eo.legacy.h',
  'elm_mapbuf_eo.legacy.h',
  'elm_map_eo.legacy.h',
  'elm_map_pan_eo.legacy.h',
  'elm_menu_eo.legacy.h',
  'elm_menu_item_eo.legacy.h',
  'elm_multibuttonentry_eo.legacy.h',
  'elm_multibuttonentry_item_eo.legacy.h',
  'elm_naviframe_eo.legacy.h',
  'elm_naviframe_item_eo.legacy.h',
  'elm_notify_eo.legacy.h',
  'elm_panel_eo.legacy.h',
  'elm_pan_eo.legacy.h',
  'elm_photo_eo.legacy.h',
  'elm_player_eo.legacy.h',
  'elm_plug_eo.legacy.h',
  'elm_popup_eo.legacy.h',
  'elm_popup_item_eo.legacy.h',
  'elm_prefs_eo.legacy.h',
  'elm_route_eo.legacy.h',
  'elm_scroller_eo.legacy.h',
  'elm_segment_control_eo.legacy.h',
  'elm_segment_control_item_eo.legacy.h',
  'elm_separator_eo.legacy.h',
  'elm_slider_part_indicator_eo.legacy.h',
  'elm_slideshow_eo.legacy.h',
  'elm_slideshow_item_eo.legacy.h',
  'elm_spinner_eo.legacy.h',
  'elm_sys_notify_dbus_eo.legacy.h',
  'elm_sys_notify_eo.legacy.h',
  'elm_sys_notify_interface_eo.legacy.h',
  'elm_systray_eo.legacy.h',
  'elm_table_eo.legacy.h',
  'elm_thumb_eo.legacy.h',
  'elm_toolbar_eo.legacy.h',
  'elm_toolbar_item_eo.legacy.h',
  'elm_view_form_eo.legacy.h',
  'elm_view_list_eo.legacy.h',
  'elm_view_form_eo.h',
  'elm_view_list_eo.h',
  'elm_web_eo.legacy.h',
  'elm_widget_item_container_eo.h',
  'elm_widget_item_container_eo.legacy.h',
  'elm_widget_item_eo.legacy.h',
  'elm_widget_item_static_focus_eo.legacy.h',
]

elementary_header_src = [
  'Elementary.h',
  'Elementary_Cursor.h'
] + elementary_pub_headers + elementary_headers_unstable

elementary_src = files([
  'elm_priv.h',
  'elementary_config.h',
  'els_box.h',
  'elc_ctxpopup.c',
  'elc_fileselector.c',
  'elc_fileselector_button.c',
  'elc_fileselector_entry.c',
  'elc_hoversel.c',
  'elc_multibuttonentry.c',
  'elc_naviframe.c',
  'efl_ui_navigation_bar.c',
  'efl_ui_navigation_layout.c',
  'elc_player.c',
  'elc_popup.c',
  'elc_scrolled_entry.c',
  'elm_access.c',
  'efl_ui_vg_animation.c',
  'elm_actionslider.c',
  'elm_atspi_app_object.c',
  'elm_atspi_bridge.c',
  'efl_ui_legacy.c',
  'efl_ui_bg.c',
  'elm_box.c',
  'elm_bubble.c',
  'efl_ui_button.c',
  'elm_calendar.c',
  'efl_ui_calendar.c',
  'efl_ui_check.c',
  'elm_clock.c',
  'elm_code_line.c',
  'elm_code_text.c',
  'elm_code_indent.c',
  'elm_code_file.c',
  'elm_code_parse.c',
  'elm_code_syntax.c',
  'elm_code_widget_selection.c',
  'elm_code_widget.c',
  'elm_code_diff_widget.c',
  'elm_code.c',
  'elm_code_private.h',
  'elm_code_widget_private.h',
  'elm_colorselector.c',
  'elm_color_class.c',
  'elc_combobox.c',
  'elm_config.c',
  'elm_conform.c',
  'elm_datetime.c',
  'elm_dayselector.c',
  'elm_dbus_menu.c',
  'elm_diskselector.c',
  'elm_entry.c',
  'efl_ui_flip.c',
  'efl_ui_internal_text_interactive.c',
  'efl_ui_internal_text_interactive.h',
  'efl_ui_internal_text_scroller.c',
  'efl_ui_internal_text_scroller.h',
  'elm_flipselector.c',
  'elm_font.c',
  'efl_ui_frame.c',
  'elm_gengrid.c',
  'elm_genlist.c',
  'elm_gesture_layer.c',
  'elm_gesture_layer_extra_gestures.c',
  'elm_glview.c',
  'elm_grid.c',
  'elm_hover.c',
  'elm_icon.c',
  'efl_ui_image.c',
  'elm_index.c',
  'elm_cnp.c',
  'elm_dnd.c',
  'efl_access_object.c',
  'efl_access_action.c',
  'efl_access_component.c',
  'efl_access_editable_text.c',
  'efl_access_selection.c',
  'efl_access_text.c',
  'efl_access_value.c',
  'efl_access_widget_action.c',
  'efl_access_window.c',
  'elm_interface_fileselector.c',
  'elm_interface_scrollable.c',
  'elm_inwin.c',
  'elm_label.c',
  'efl_ui_layout.c',
  'efl_ui_layout_pack.c',
  'elm_list.c',
  'elm_main.c',
  'elm_map.c',
  'elm_mapbuf.c',
  'elm_menu.c',
  'elm_module.c',
  'elm_notify.c',
  'elm_panel.c',
  'efl_ui_panes.c',
  'elm_photo.c',
  'efl_ui_image_zoomable.c',
  'elm_plug.c',
  'elm_prefs.c',
  'elm_prefs_data.c',
  'efl_ui_progressbar.c',
  'efl_ui_radio.c',
  'efl_ui_radio_group_impl.c',
  'efl_ui_radio_box.c',
  'elm_route.c',
  'elm_scroller.c',
  'elm_segment_control.c',
  'elm_separator.c',
  'elm_slider.c',
  'efl_ui_slider.c',
  'efl_ui_slider_interval.c',
  'efl_ui_spin.c',
  'efl_ui_spin_button.c',
  'efl_ui_panel.c',
  'efl_ui_datepicker.c',
  'efl_ui_timepicker.c',
  'efl_ui_tags.c',
  'elm_slideshow.c',
  'elm_spinner.c',
  'elm_store.c',
  'elm_systray.c',
  'elm_systray_watcher.c',
  'elm_sys_notify_interface.c',
  'elm_sys_notify.c',
  'elm_sys_notify_dbus.c',
  'elm_table.c',
  'elm_theme.c',
  'elm_thumb.c',
  'elm_toolbar.c',
  'elm_transit.c',
  'elm_util.c',
  'efl_ui_video.c',
  'elm_view_list.c',
  'elm_view_form.c',
  'elm_web2.c',
  'efl_ui_widget.c',
  'efl_ui_widget_scrollable_content.c',
  'efl_ui_widget_common.c',
  'efl_ui_win.c',
  'efl_ui_win_inlined.c',
  'efl_ui_win_socket.c',
  'elm_helper.c',
  'els_box.c',
  'els_cursor.c',
  'els_tooltip.c',
  'efl_ui_container_layout.c',
  'efl_ui_container_layout.h',
  'efl_ui_box.c',
  'efl_ui_box_flow.c',
  'efl_ui_box_stack.c',
  'efl_ui_box_layout.c',
  'efl_ui_box_private.h',
  'efl_ui_popup.c',
  'efl_ui_alert_popup.c',
  'efl_ui_table.c',
  'efl_ui_table_static.c',
  'efl_ui_table_layout.c',
  'efl_ui_table_private.h',
  'efl_ui_textbox.c',
  'efl_ui_text_factory_images.c',
  'efl_ui_text_factory_emoticons.c',
  'efl_ui_text_factory_fallback.c',
  'efl_ui_clock.c',
  'efl_ui_clock_private.h',
  'efl_ui_image_factory.c',
  'efl_ui_focus_manager.c',
  'efl_ui_focus_manager_calc.c',
  'efl_ui_focus_manager_sub.c',
  'efl_ui_focus_object.c',
  'efl_ui_focus_manager_root_focus.c',
  'efl_ui_textpath.c',
  'efl_ui_focus_layer.c',
  'efl_ui_focus_composition.c',
  'efl_ui_focus_parent_provider.c',
  'efl_ui_focus_parent_provider_standard.c',
  'efl_ui_focus_parent_provider_gen.c',
  'efl_ui_focus_util.c',
  'elm_widget_item_static_focus.c',
  'efl_ui_item.c',
  'efl_ui_default_item.c',
  'efl_ui_group_item.c',
  'efl_ui_list_default_item.c',
  'efl_ui_list_placeholder_item.c',
  'efl_ui_list.c',
  'efl_ui_grid_default_item.c',
  'efl_ui_grid.c',
  'efl_ui_list_view.c',
  'efl_ui_grid_view.c',
  'efl_ui_layout_factory.c',
  'efl_ui_scroller.c',
  'efl_ui_scroll_manager.c',
  'efl_ui_pan.c',
  'efl_ui_selection_manager_private.h',
  'efl_ui_selection.c',
  'efl_datetime_manager.c',
  'efl_ui_dnd.c',
  'elm_focus_legacy.c',
  'efl_ui_spotlight_container.c',
  'efl_ui_spotlight_manager.c',
  'efl_ui_spotlight_plain_manager.c',
  'efl_ui_spotlight_scroll_manager.c',
  'efl_ui_spotlight_fade_manager.c',
  'efl_ui_spotlight_indicator.c',
  'efl_ui_spotlight_icon_indicator.c',
  'efl_ui_spotlight_util.c',
  'efl_ui_focus_graph.h',
  'efl_ui_focus_graph.c',
  'efl_ui_tab_pager.c',
  'efl_ui_tab_bar.c',
  'efl_ui_tab_page.c',
  'efl_ui_widget_focus_manager.c',
  'efl_ui_caching_factory.c',
  'efl_ui_widget_factory.c',
  'efl_ui_size_model.c',
  'efl_ui_homogeneous_model.c',
  'efl_ui_exact_model.c',
  'efl_ui_average_model.c',
  'efl_ui_relative_container.c',
  'efl_ui_action_connector.c',
  'efl_ui_format.c',
  'efl_ui_scroll_util.c',
  'efl_ui_collection.c',
  'efl_ui_position_manager_entity.c',
  'efl_ui_position_manager_list.c',
  'efl_ui_position_manager_grid.c',
  'efl_ui_tab_bar_default_item.c',
  'efl_ui_select_model.c',
  'efl_ui_view_model.c',
  'efl_ui_collection_view.c',
  'efl_ui_pager.c',
  'efl_ui_stack.c',
  'efl_ui_separator.c',
  'efl_ui_spotlight_animation_manager.c',
])

elementary_deps = [emile, eo, efl, edje, ethumb, ethumb_client, emotion, ecore_imf, ecore_con, eldbus, efreet, eio, buildsystem]
elementary_pub_deps = [eina, eet, evas, ecore, ecore_evas, ecore_file, ecore_input, ecore_imf, ecore_con,
                       edje, eldbus, efreet, ethumb_client, efl]
elementary_ext_deps = [atspi, dl, intl, buildsystem_simple]
elm_options = configuration_data()

config_h.set_quoted('ELM_TOP_BUILD_DIR', meson.project_build_root())
config_h.set_quoted('MODULES_PATH', join_paths(dir_lib, 'modules'))
config_h.set_quoted('ELEMENTARY_BASE_DIR', get_option('elementary-base-dir'))
config_h.set_quoted('ICON_DIR', join_paths(dir_lib, 'icons'))

if sys_windows == false
  elm_options.set('ELM_UNIX', '1')
endif

elm_options.set('ELM_EFREET', '1')

if config_h.has('HAVE_ALLOCA_H')
  elm_options.set('ELM_ALLOCA_H', '1')
endif

if config_h.has('HAVE_LIBINTL_H')
  elm_options.set('ELM_LIBINTL_H', '1')
endif

if config_h.has('HAVE_DIRENT_H')
  elm_options.set('ELM_DIRENT_H', '1')
endif

if get_option('x11')
  config_h.set('HAVE_ELEMENTARY_X', '1')
  elementary_deps += ecore_x
endif

if get_option('wl')
  config_h.set('HAVE_ELEMENTARY_WL2', '1')
  elementary_deps += ecore_wl2
  elementary_ext_deps += declare_dependency(
    include_directories: include_directories([join_paths('..', '..', 'modules', 'ecore_evas', 'engines', 'wayland'), join_paths('..', '..', 'lib', 'ecore_wl2')]),
    dependencies : ecore_wl2_internal_headers
  )
endif

if get_option('drm')
  config_h.set('HAVE_ELEMENTARY_DRM', '1')
  elementary_deps += ecore_drm2
endif

if get_option('cocoa')
  config_h.set('HAVE_ELEMENTARY_COCOA', '1')
  elementary_deps += ecore_cocoa
endif

if sys_windows == true
  config_h.set('HAVE_ELEMENTARY_WIN32', '1')
  elementary_deps += ecore_win32
endif

c = configure_file(
  output: 'Elementary_Options.h',
  install_dir : dir_package_include,
  configuration: elm_options)

# Save in extra variable here to pass to edje_external
elm_package_c_args =  package_c_args

elementary_lib = library('elementary',
    elementary_src, pub_eo_file_target, priv_eo_file_target, c, embed_config,
    dependencies: elementary_pub_deps + elementary_deps + elementary_ext_deps,
    include_directories : config_dir + [include_directories('.')] + [include_directories(join_paths('..', '..', '..'))],
    install: true,
    c_args : elm_package_c_args,
    version : meson.project_version()
)

elementary = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: elementary_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: elementary_pub_deps,
)

if get_option('install-eo-files')
  install_data(pub_eo_files + pub_eo_types_files,
    install_dir: join_paths(eolian_include_dir, package_version_name)
  )
endif

install_headers(elementary_header_src,
  install_dir : dir_package_include,
)

elementary_config_dir = include_directories('.')

package_pc_variables = ['themes='+join_paths(dir_data, package_name, 'themes')]

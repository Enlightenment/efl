
void _elm_web_webkit2_evas_object_smart_add(Eo *obj, Elm_Web_Webkit2_Data *pd);


void _elm_web_webkit2_evas_object_smart_del(Eo *obj, Elm_Web_Webkit2_Data *pd);


Eina_Bool _elm_web_webkit2_elm_widget_on_focus(Eo *obj, Elm_Web_Webkit2_Data *pd, Elm_Widget_Item *item);


Eina_Bool _elm_web_webkit2_elm_web_tab_propagate_get(Eo *obj, Elm_Web_Webkit2_Data *pd);


void _elm_web_webkit2_elm_web_tab_propagate_set(Eo *obj, Elm_Web_Webkit2_Data *pd, Eina_Bool propagate);


Evas_Object * _elm_web_webkit2_elm_web_webkit_view_get(Eo *obj, Elm_Web_Webkit2_Data *pd);


void _elm_web_webkit2_elm_web_window_create_hook_set(Eo *obj, Elm_Web_Webkit2_Data *pd, Elm_Web_Window_Open func, void *data);


void _elm_web_webkit2_elm_web_dialog_alert_hook_set(Eo *obj, Elm_Web_Webkit2_Data *pd, Elm_Web_Dialog_Alert func, void *data);


void _elm_web_webkit2_elm_web_dialog_confirm_hook_set(Eo *obj, Elm_Web_Webkit2_Data *pd, Elm_Web_Dialog_Confirm func, void *data);


void _elm_web_webkit2_elm_web_dialog_prompt_hook_set(Eo *obj, Elm_Web_Webkit2_Data *pd, Elm_Web_Dialog_Prompt func, void *data);


void _elm_web_webkit2_elm_web_dialog_file_selector_hook_set(Eo *obj, Elm_Web_Webkit2_Data *pd, Elm_Web_Dialog_File_Selector func, void *data);


void _elm_web_webkit2_elm_web_console_message_hook_set(Eo *obj, Elm_Web_Webkit2_Data *pd, Elm_Web_Console_Message func, void *data);


void _elm_web_webkit2_elm_web_useragent_set(Eo *obj, Elm_Web_Webkit2_Data *pd, const char *user_agent);


const char * _elm_web_webkit2_elm_web_useragent_get(Eo *obj, Elm_Web_Webkit2_Data *pd);


Eina_Bool _elm_web_webkit2_elm_web_url_set(Eo *obj, Elm_Web_Webkit2_Data *pd, const char *url);


const char * _elm_web_webkit2_elm_web_url_get(Eo *obj, Elm_Web_Webkit2_Data *pd);


Eina_Bool _elm_web_webkit2_elm_web_html_string_load(Eo *obj, Elm_Web_Webkit2_Data *pd, const char *html, const char *base_url, const char *unreachable_url);


const char * _elm_web_webkit2_elm_web_title_get(Eo *obj, Elm_Web_Webkit2_Data *pd);


void _elm_web_webkit2_elm_web_bg_color_set(Eo *obj, Elm_Web_Webkit2_Data *pd, int r, int g, int b, int a);


void _elm_web_webkit2_elm_web_bg_color_get(Eo *obj, Elm_Web_Webkit2_Data *pd, int *r, int *g, int *b, int *a);


const char * _elm_web_webkit2_elm_web_selection_get(Eo *obj, Elm_Web_Webkit2_Data *pd);


void _elm_web_webkit2_elm_web_popup_selected_set(Eo *obj, Elm_Web_Webkit2_Data *pd, int idx);


Eina_Bool _elm_web_webkit2_elm_web_popup_destroy(Eo *obj, Elm_Web_Webkit2_Data *pd);


Eina_Bool _elm_web_webkit2_elm_web_text_search(const Eo *obj, Elm_Web_Webkit2_Data *pd, const char *string, Eina_Bool case_sensitive, Eina_Bool forward, Eina_Bool wrap);


unsigned int _elm_web_webkit2_elm_web_text_matches_mark(Eo *obj, Elm_Web_Webkit2_Data *pd, const char *string, Eina_Bool case_sensitive, Eina_Bool highlight, unsigned int limit);


Eina_Bool _elm_web_webkit2_elm_web_text_matches_unmark_all(Eo *obj, Elm_Web_Webkit2_Data *pd);


Eina_Bool _elm_web_webkit2_elm_web_text_matches_highlight_set(Eo *obj, Elm_Web_Webkit2_Data *pd, Eina_Bool highlight);


Eina_Bool _elm_web_webkit2_elm_web_text_matches_highlight_get(Eo *obj, Elm_Web_Webkit2_Data *pd);


double _elm_web_webkit2_elm_web_load_progress_get(Eo *obj, Elm_Web_Webkit2_Data *pd);


Eina_Bool _elm_web_webkit2_elm_web_stop(Eo *obj, Elm_Web_Webkit2_Data *pd);


Eina_Bool _elm_web_webkit2_elm_web_reload(Eo *obj, Elm_Web_Webkit2_Data *pd);


Eina_Bool _elm_web_webkit2_elm_web_reload_full(Eo *obj, Elm_Web_Webkit2_Data *pd);


Eina_Bool _elm_web_webkit2_elm_web_back(Eo *obj, Elm_Web_Webkit2_Data *pd);


Eina_Bool _elm_web_webkit2_elm_web_forward(Eo *obj, Elm_Web_Webkit2_Data *pd);


Eina_Bool _elm_web_webkit2_elm_web_navigate(Eo *obj, Elm_Web_Webkit2_Data *pd, int steps);


Eina_Bool _elm_web_webkit2_elm_web_back_possible_get(Eo *obj, Elm_Web_Webkit2_Data *pd);


Eina_Bool _elm_web_webkit2_elm_web_forward_possible_get(Eo *obj, Elm_Web_Webkit2_Data *pd);


Eina_Bool _elm_web_webkit2_elm_web_navigate_possible_get(Eo *obj, Elm_Web_Webkit2_Data *pd, int steps);


Eina_Bool _elm_web_webkit2_elm_web_history_enabled_get(Eo *obj, Elm_Web_Webkit2_Data *pd);


void _elm_web_webkit2_elm_web_history_enabled_set(Eo *obj, Elm_Web_Webkit2_Data *pd, Eina_Bool enable);


void _elm_web_webkit2_elm_web_zoom_set(Eo *obj, Elm_Web_Webkit2_Data *pd, double zoom);


double _elm_web_webkit2_elm_web_zoom_get(Eo *obj, Elm_Web_Webkit2_Data *pd);


void _elm_web_webkit2_elm_web_zoom_mode_set(Eo *obj, Elm_Web_Webkit2_Data *pd, Elm_Web_Zoom_Mode mode);


Elm_Web_Zoom_Mode _elm_web_webkit2_elm_web_zoom_mode_get(Eo *obj, Elm_Web_Webkit2_Data *pd);


void _elm_web_webkit2_elm_web_region_show(Eo *obj, Elm_Web_Webkit2_Data *pd, int x, int y, int w, int h);


void _elm_web_webkit2_elm_web_region_bring_in(Eo *obj, Elm_Web_Webkit2_Data *pd, int x, int y, int w, int h);


void _elm_web_webkit2_elm_web_inwin_mode_set(Eo *obj, Elm_Web_Webkit2_Data *pd, Eina_Bool value);


Eina_Bool _elm_web_webkit2_elm_web_inwin_mode_get(Eo *obj, Elm_Web_Webkit2_Data *pd);


static const Eo_Op_Description _elm_web_webkit2_op_desc[] = {
     EO_OP_FUNC_OVERRIDE(evas_obj_smart_add, _elm_web_webkit2_evas_object_smart_add),
     EO_OP_FUNC_OVERRIDE(evas_obj_smart_del, _elm_web_webkit2_evas_object_smart_del),
     EO_OP_FUNC_OVERRIDE(elm_obj_widget_on_focus, _elm_web_webkit2_elm_widget_on_focus),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_tab_propagate_get, _elm_web_webkit2_elm_web_tab_propagate_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_tab_propagate_set, _elm_web_webkit2_elm_web_tab_propagate_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_webkit_view_get, _elm_web_webkit2_elm_web_webkit_view_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_window_create_hook_set, _elm_web_webkit2_elm_web_window_create_hook_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_dialog_alert_hook_set, _elm_web_webkit2_elm_web_dialog_alert_hook_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_dialog_confirm_hook_set, _elm_web_webkit2_elm_web_dialog_confirm_hook_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_dialog_prompt_hook_set, _elm_web_webkit2_elm_web_dialog_prompt_hook_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_dialog_file_selector_hook_set, _elm_web_webkit2_elm_web_dialog_file_selector_hook_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_console_message_hook_set, _elm_web_webkit2_elm_web_console_message_hook_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_useragent_set, _elm_web_webkit2_elm_web_useragent_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_useragent_get, _elm_web_webkit2_elm_web_useragent_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_url_set, _elm_web_webkit2_elm_web_url_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_url_get, _elm_web_webkit2_elm_web_url_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_html_string_load, _elm_web_webkit2_elm_web_html_string_load),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_title_get, _elm_web_webkit2_elm_web_title_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_bg_color_set, _elm_web_webkit2_elm_web_bg_color_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_bg_color_get, _elm_web_webkit2_elm_web_bg_color_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_selection_get, _elm_web_webkit2_elm_web_selection_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_popup_selected_set, _elm_web_webkit2_elm_web_popup_selected_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_popup_destroy, _elm_web_webkit2_elm_web_popup_destroy),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_text_search, _elm_web_webkit2_elm_web_text_search),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_text_matches_mark, _elm_web_webkit2_elm_web_text_matches_mark),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_text_matches_unmark_all, _elm_web_webkit2_elm_web_text_matches_unmark_all),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_text_matches_highlight_set, _elm_web_webkit2_elm_web_text_matches_highlight_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_text_matches_highlight_get, _elm_web_webkit2_elm_web_text_matches_highlight_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_load_progress_get, _elm_web_webkit2_elm_web_load_progress_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_stop, _elm_web_webkit2_elm_web_stop),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_reload, _elm_web_webkit2_elm_web_reload),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_reload_full, _elm_web_webkit2_elm_web_reload_full),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_back, _elm_web_webkit2_elm_web_back),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_forward, _elm_web_webkit2_elm_web_forward),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_navigate, _elm_web_webkit2_elm_web_navigate),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_back_possible_get, _elm_web_webkit2_elm_web_back_possible_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_forward_possible_get, _elm_web_webkit2_elm_web_forward_possible_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_navigate_possible_get, _elm_web_webkit2_elm_web_navigate_possible_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_history_enabled_get, _elm_web_webkit2_elm_web_history_enabled_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_history_enabled_set, _elm_web_webkit2_elm_web_history_enabled_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_zoom_set, _elm_web_webkit2_elm_web_zoom_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_zoom_get, _elm_web_webkit2_elm_web_zoom_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_zoom_mode_set, _elm_web_webkit2_elm_web_zoom_mode_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_zoom_mode_get, _elm_web_webkit2_elm_web_zoom_mode_get),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_region_show, _elm_web_webkit2_elm_web_region_show),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_region_bring_in, _elm_web_webkit2_elm_web_region_bring_in),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_inwin_mode_set, _elm_web_webkit2_elm_web_inwin_mode_set),
     EO_OP_FUNC_OVERRIDE(elm_obj_web_inwin_mode_get, _elm_web_webkit2_elm_web_inwin_mode_get),
};

static const Eo_Class_Description _elm_web_webkit2_class_desc = {
     EO_VERSION,
     "Elm_Web_Webkit2",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(_elm_web_webkit2_op_desc),
     NULL,
     sizeof(Elm_Web_Webkit2_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(elm_web_webkit2_class_get, &_elm_web_webkit2_class_desc, ELM_WEB_CLASS, NULL);
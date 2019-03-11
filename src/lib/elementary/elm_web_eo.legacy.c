
EAPI Eina_Bool
elm_web_text_matches_highlight_set(Elm_Web *obj, Eina_Bool highlight)
{
   return elm_obj_web_text_matches_highlight_set(obj, highlight);
}

EAPI Eina_Bool
elm_web_text_matches_highlight_get(const Elm_Web *obj)
{
   return elm_obj_web_text_matches_highlight_get(obj);
}

EAPI void
elm_web_useragent_set(Elm_Web *obj, const char *user_agent)
{
   elm_obj_web_useragent_set(obj, user_agent);
}

EAPI const char *
elm_web_useragent_get(const Elm_Web *obj)
{
   return elm_obj_web_useragent_get(obj);
}

EAPI Eina_Bool
elm_web_url_set(Elm_Web *obj, const char *url)
{
   return elm_obj_web_url_set(obj, url);
}

EAPI const char *
elm_web_url_get(const Elm_Web *obj)
{
   return elm_obj_web_url_get(obj);
}

EAPI void
elm_web_bg_color_set(Elm_Web *obj, int r, int g, int b, int a)
{
   elm_obj_web_bg_color_set(obj, r, g, b, a);
}

EAPI void
elm_web_bg_color_get(const Elm_Web *obj, int *r, int *g, int *b, int *a)
{
   elm_obj_web_bg_color_get(obj, r, g, b, a);
}

EAPI void
elm_web_inwin_mode_set(Elm_Web *obj, Eina_Bool value)
{
   elm_obj_web_inwin_mode_set(obj, value);
}

EAPI Eina_Bool
elm_web_inwin_mode_get(const Elm_Web *obj)
{
   return elm_obj_web_inwin_mode_get(obj);
}

EAPI void
elm_web_tab_propagate_set(Elm_Web *obj, Eina_Bool propagate)
{
   elm_obj_web_tab_propagate_set(obj, propagate);
}

EAPI Eina_Bool
elm_web_tab_propagate_get(const Elm_Web *obj)
{
   return elm_obj_web_tab_propagate_get(obj);
}

EAPI void
elm_web_history_enabled_set(Elm_Web *obj, Eina_Bool enable)
{
   elm_obj_web_history_enabled_set(obj, enable);
}

EAPI Eina_Bool
elm_web_history_enabled_get(const Elm_Web *obj)
{
   return elm_obj_web_history_enabled_get(obj);
}

EAPI void
elm_web_console_message_hook_set(Elm_Web *obj, Elm_Web_Console_Message func, void *data)
{
   elm_obj_web_console_message_hook_set(obj, func, data);
}

EAPI void
elm_web_window_create_hook_set(Elm_Web *obj, Elm_Web_Window_Open func, void *data)
{
   elm_obj_web_window_create_hook_set(obj, func, data);
}

EAPI void
elm_web_dialog_file_selector_hook_set(Elm_Web *obj, Elm_Web_Dialog_File_Selector func, void *data)
{
   elm_obj_web_dialog_file_selector_hook_set(obj, func, data);
}

EAPI void
elm_web_dialog_confirm_hook_set(Elm_Web *obj, Elm_Web_Dialog_Confirm func, void *data)
{
   elm_obj_web_dialog_confirm_hook_set(obj, func, data);
}

EAPI void
elm_web_popup_selected_set(Elm_Web *obj, int idx)
{
   elm_obj_web_popup_selected_set(obj, idx);
}

EAPI void
elm_web_dialog_prompt_hook_set(Elm_Web *obj, Elm_Web_Dialog_Prompt func, void *data)
{
   elm_obj_web_dialog_prompt_hook_set(obj, func, data);
}

EAPI void
elm_web_dialog_alert_hook_set(Elm_Web *obj, Elm_Web_Dialog_Alert func, void *data)
{
   elm_obj_web_dialog_alert_hook_set(obj, func, data);
}

EAPI Eina_Bool
elm_web_forward_possible_get(const Elm_Web *obj)
{
   return elm_obj_web_forward_possible_get(obj);
}

EAPI Efl_Canvas_Object *
elm_web_webkit_view_get(const Elm_Web *obj)
{
   return elm_obj_web_webkit_view_get(obj);
}

EAPI Eina_Bool
elm_web_back_possible_get(const Elm_Web *obj)
{
   return elm_obj_web_back_possible_get(obj);
}

EAPI double
elm_web_load_progress_get(const Elm_Web *obj)
{
   return elm_obj_web_load_progress_get(obj);
}

EAPI char *
elm_web_selection_get(const Elm_Web *obj)
{
   return elm_obj_web_selection_get(obj);
}

EAPI Eina_Stringshare *
elm_web_title_get(const Elm_Web *obj)
{
   return elm_obj_web_title_get(obj);
}

EAPI Eina_Bool
elm_web_navigate(Elm_Web *obj, int steps)
{
   return elm_obj_web_navigate(obj, steps);
}

EAPI Eina_Bool
elm_web_back(Elm_Web *obj)
{
   return elm_obj_web_back(obj);
}

EAPI Eina_Bool
elm_web_html_string_load(Elm_Web *obj, const char *html, const char *base_url, const char *unreachable_url)
{
   return elm_obj_web_html_string_load(obj, html, base_url, unreachable_url);
}

EAPI Eina_Bool
elm_web_text_search(const Elm_Web *obj, const char *string, Eina_Bool case_sensitive, Eina_Bool forward, Eina_Bool wrap)
{
   return elm_obj_web_text_search(obj, string, case_sensitive, forward, wrap);
}

EAPI Eina_Bool
elm_web_popup_destroy(Elm_Web *obj)
{
   return elm_obj_web_popup_destroy(obj);
}

EAPI void
elm_web_region_show(Elm_Web *obj, int x, int y, int w, int h)
{
   elm_obj_web_region_show(obj, x, y, w, h);
}

EAPI Eina_Bool
elm_web_forward(Elm_Web *obj)
{
   return elm_obj_web_forward(obj);
}

EAPI unsigned int
elm_web_text_matches_mark(Elm_Web *obj, const char *string, Eina_Bool case_sensitive, Eina_Bool highlight, unsigned int limit)
{
   return elm_obj_web_text_matches_mark(obj, string, case_sensitive, highlight, limit);
}

EAPI void
elm_web_region_bring_in(Elm_Web *obj, int x, int y, int w, int h)
{
   elm_obj_web_region_bring_in(obj, x, y, w, h);
}

EAPI Eina_Bool
elm_web_stop(Elm_Web *obj)
{
   return elm_obj_web_stop(obj);
}

EAPI Eina_Bool
elm_web_navigate_possible_get(Elm_Web *obj, int steps)
{
   return elm_obj_web_navigate_possible_get(obj, steps);
}

EAPI Eina_Bool
elm_web_reload_full(Elm_Web *obj)
{
   return elm_obj_web_reload_full(obj);
}

EAPI Eina_Bool
elm_web_text_matches_unmark_all(Elm_Web *obj)
{
   return elm_obj_web_text_matches_unmark_all(obj);
}

EAPI Eina_Bool
elm_web_reload(Elm_Web *obj)
{
   return elm_obj_web_reload(obj);
}

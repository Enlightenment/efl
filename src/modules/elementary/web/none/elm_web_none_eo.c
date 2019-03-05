
void _elm_web_none_elm_web_tab_propagate_set(Eo *obj, Elm_Web_None_Data *pd, Eina_Bool propagate);


Eina_Bool _elm_web_none_elm_web_tab_propagate_get(const Eo *obj, Elm_Web_None_Data *pd);


Efl_Canvas_Object *_elm_web_none_elm_web_webkit_view_get(const Eo *obj, Elm_Web_None_Data *pd);


void _elm_web_none_elm_web_window_create_hook_set(Eo *obj, Elm_Web_None_Data *pd, Elm_Web_Window_Open func, void *data);


void _elm_web_none_elm_web_dialog_alert_hook_set(Eo *obj, Elm_Web_None_Data *pd, Elm_Web_Dialog_Alert func, void *data);


void _elm_web_none_elm_web_dialog_confirm_hook_set(Eo *obj, Elm_Web_None_Data *pd, Elm_Web_Dialog_Confirm func, void *data);


void _elm_web_none_elm_web_dialog_prompt_hook_set(Eo *obj, Elm_Web_None_Data *pd, Elm_Web_Dialog_Prompt func, void *data);


void _elm_web_none_elm_web_dialog_file_selector_hook_set(Eo *obj, Elm_Web_None_Data *pd, Elm_Web_Dialog_File_Selector func, void *data);


void _elm_web_none_elm_web_console_message_hook_set(Eo *obj, Elm_Web_None_Data *pd, Elm_Web_Console_Message func, void *data);


void _elm_web_none_elm_web_useragent_set(Eo *obj, Elm_Web_None_Data *pd, const char *user_agent);


const char *_elm_web_none_elm_web_useragent_get(const Eo *obj, Elm_Web_None_Data *pd);


Eina_Bool _elm_web_none_elm_web_url_set(Eo *obj, Elm_Web_None_Data *pd, const char *url);


const char *_elm_web_none_elm_web_url_get(const Eo *obj, Elm_Web_None_Data *pd);


Eina_Bool _elm_web_none_elm_web_html_string_load(Eo *obj, Elm_Web_None_Data *pd, const char *html, const char *base_url, const char *unreachable_url);


Eina_Stringshare *_elm_web_none_elm_web_title_get(const Eo *obj, Elm_Web_None_Data *pd);


void _elm_web_none_elm_web_bg_color_set(Eo *obj, Elm_Web_None_Data *pd, int r, int g, int b, int a);


void _elm_web_none_elm_web_bg_color_get(const Eo *obj, Elm_Web_None_Data *pd, int *r, int *g, int *b, int *a);


char *_elm_web_none_elm_web_selection_get(const Eo *obj, Elm_Web_None_Data *pd);


void _elm_web_none_elm_web_popup_selected_set(Eo *obj, Elm_Web_None_Data *pd, int idx);


Eina_Bool _elm_web_none_elm_web_popup_destroy(Eo *obj, Elm_Web_None_Data *pd);


Eina_Bool _elm_web_none_elm_web_text_search(const Eo *obj, Elm_Web_None_Data *pd, const char *string, Eina_Bool case_sensitive, Eina_Bool forward, Eina_Bool wrap);


unsigned int _elm_web_none_elm_web_text_matches_mark(Eo *obj, Elm_Web_None_Data *pd, const char *string, Eina_Bool case_sensitive, Eina_Bool highlight, unsigned int limit);


Eina_Bool _elm_web_none_elm_web_text_matches_unmark_all(Eo *obj, Elm_Web_None_Data *pd);


Eina_Bool _elm_web_none_elm_web_text_matches_highlight_set(Eo *obj, Elm_Web_None_Data *pd, Eina_Bool highlight);


Eina_Bool _elm_web_none_elm_web_text_matches_highlight_get(const Eo *obj, Elm_Web_None_Data *pd);


double _elm_web_none_elm_web_load_progress_get(const Eo *obj, Elm_Web_None_Data *pd);


Eina_Bool _elm_web_none_elm_web_stop(Eo *obj, Elm_Web_None_Data *pd);


Eina_Bool _elm_web_none_elm_web_reload(Eo *obj, Elm_Web_None_Data *pd);


Eina_Bool _elm_web_none_elm_web_reload_full(Eo *obj, Elm_Web_None_Data *pd);


Eina_Bool _elm_web_none_elm_web_back(Eo *obj, Elm_Web_None_Data *pd);


Eina_Bool _elm_web_none_elm_web_forward(Eo *obj, Elm_Web_None_Data *pd);


Eina_Bool _elm_web_none_elm_web_navigate(Eo *obj, Elm_Web_None_Data *pd, int steps);


Eina_Bool _elm_web_none_elm_web_back_possible_get(const Eo *obj, Elm_Web_None_Data *pd);


Eina_Bool _elm_web_none_elm_web_forward_possible_get(const Eo *obj, Elm_Web_None_Data *pd);


Eina_Bool _elm_web_none_elm_web_navigate_possible_get(Eo *obj, Elm_Web_None_Data *pd, int steps);


void _elm_web_none_elm_web_history_enabled_set(Eo *obj, Elm_Web_None_Data *pd, Eina_Bool enable);


Eina_Bool _elm_web_none_elm_web_history_enabled_get(const Eo *obj, Elm_Web_None_Data *pd);


void _elm_web_none_efl_ui_zoom_zoom_level_set(Eo *obj, Elm_Web_None_Data *pd, double zoom);


double _elm_web_none_efl_ui_zoom_zoom_level_get(const Eo *obj, Elm_Web_None_Data *pd);


void _elm_web_none_efl_ui_zoom_zoom_mode_set(Eo *obj, Elm_Web_None_Data *pd, Efl_Ui_Zoom_Mode mode);


Efl_Ui_Zoom_Mode _elm_web_none_efl_ui_zoom_zoom_mode_get(const Eo *obj, Elm_Web_None_Data *pd);


void _elm_web_none_elm_web_region_show(Eo *obj, Elm_Web_None_Data *pd, int x, int y, int w, int h);


void _elm_web_none_elm_web_region_bring_in(Eo *obj, Elm_Web_None_Data *pd, int x, int y, int w, int h);


void _elm_web_none_elm_web_inwin_mode_set(Eo *obj, Elm_Web_None_Data *pd, Eina_Bool value);


Eina_Bool _elm_web_none_elm_web_inwin_mode_get(const Eo *obj, Elm_Web_None_Data *pd);


static Eina_Bool
_elm_web_none_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_WEB_NONE_EXTRA_OPS
#define ELM_WEB_NONE_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_web_tab_propagate_set, _elm_web_none_elm_web_tab_propagate_set),
      EFL_OBJECT_OP_FUNC(elm_obj_web_tab_propagate_get, _elm_web_none_elm_web_tab_propagate_get),
      EFL_OBJECT_OP_FUNC(elm_obj_web_webkit_view_get, _elm_web_none_elm_web_webkit_view_get),
      EFL_OBJECT_OP_FUNC(elm_obj_web_window_create_hook_set, _elm_web_none_elm_web_window_create_hook_set),
      EFL_OBJECT_OP_FUNC(elm_obj_web_dialog_alert_hook_set, _elm_web_none_elm_web_dialog_alert_hook_set),
      EFL_OBJECT_OP_FUNC(elm_obj_web_dialog_confirm_hook_set, _elm_web_none_elm_web_dialog_confirm_hook_set),
      EFL_OBJECT_OP_FUNC(elm_obj_web_dialog_prompt_hook_set, _elm_web_none_elm_web_dialog_prompt_hook_set),
      EFL_OBJECT_OP_FUNC(elm_obj_web_dialog_file_selector_hook_set, _elm_web_none_elm_web_dialog_file_selector_hook_set),
      EFL_OBJECT_OP_FUNC(elm_obj_web_console_message_hook_set, _elm_web_none_elm_web_console_message_hook_set),
      EFL_OBJECT_OP_FUNC(elm_obj_web_useragent_set, _elm_web_none_elm_web_useragent_set),
      EFL_OBJECT_OP_FUNC(elm_obj_web_useragent_get, _elm_web_none_elm_web_useragent_get),
      EFL_OBJECT_OP_FUNC(elm_obj_web_url_set, _elm_web_none_elm_web_url_set),
      EFL_OBJECT_OP_FUNC(elm_obj_web_url_get, _elm_web_none_elm_web_url_get),
      EFL_OBJECT_OP_FUNC(elm_obj_web_html_string_load, _elm_web_none_elm_web_html_string_load),
      EFL_OBJECT_OP_FUNC(elm_obj_web_title_get, _elm_web_none_elm_web_title_get),
      EFL_OBJECT_OP_FUNC(elm_obj_web_bg_color_set, _elm_web_none_elm_web_bg_color_set),
      EFL_OBJECT_OP_FUNC(elm_obj_web_bg_color_get, _elm_web_none_elm_web_bg_color_get),
      EFL_OBJECT_OP_FUNC(elm_obj_web_selection_get, _elm_web_none_elm_web_selection_get),
      EFL_OBJECT_OP_FUNC(elm_obj_web_popup_selected_set, _elm_web_none_elm_web_popup_selected_set),
      EFL_OBJECT_OP_FUNC(elm_obj_web_popup_destroy, _elm_web_none_elm_web_popup_destroy),
      EFL_OBJECT_OP_FUNC(elm_obj_web_text_search, _elm_web_none_elm_web_text_search),
      EFL_OBJECT_OP_FUNC(elm_obj_web_text_matches_mark, _elm_web_none_elm_web_text_matches_mark),
      EFL_OBJECT_OP_FUNC(elm_obj_web_text_matches_unmark_all, _elm_web_none_elm_web_text_matches_unmark_all),
      EFL_OBJECT_OP_FUNC(elm_obj_web_text_matches_highlight_set, _elm_web_none_elm_web_text_matches_highlight_set),
      EFL_OBJECT_OP_FUNC(elm_obj_web_text_matches_highlight_get, _elm_web_none_elm_web_text_matches_highlight_get),
      EFL_OBJECT_OP_FUNC(elm_obj_web_load_progress_get, _elm_web_none_elm_web_load_progress_get),
      EFL_OBJECT_OP_FUNC(elm_obj_web_stop, _elm_web_none_elm_web_stop),
      EFL_OBJECT_OP_FUNC(elm_obj_web_reload, _elm_web_none_elm_web_reload),
      EFL_OBJECT_OP_FUNC(elm_obj_web_reload_full, _elm_web_none_elm_web_reload_full),
      EFL_OBJECT_OP_FUNC(elm_obj_web_back, _elm_web_none_elm_web_back),
      EFL_OBJECT_OP_FUNC(elm_obj_web_forward, _elm_web_none_elm_web_forward),
      EFL_OBJECT_OP_FUNC(elm_obj_web_navigate, _elm_web_none_elm_web_navigate),
      EFL_OBJECT_OP_FUNC(elm_obj_web_back_possible_get, _elm_web_none_elm_web_back_possible_get),
      EFL_OBJECT_OP_FUNC(elm_obj_web_forward_possible_get, _elm_web_none_elm_web_forward_possible_get),
      EFL_OBJECT_OP_FUNC(elm_obj_web_navigate_possible_get, _elm_web_none_elm_web_navigate_possible_get),
      EFL_OBJECT_OP_FUNC(elm_obj_web_history_enabled_set, _elm_web_none_elm_web_history_enabled_set),
      EFL_OBJECT_OP_FUNC(elm_obj_web_history_enabled_get, _elm_web_none_elm_web_history_enabled_get),
      EFL_OBJECT_OP_FUNC(efl_ui_zoom_level_set, _elm_web_none_efl_ui_zoom_zoom_level_set),
      EFL_OBJECT_OP_FUNC(efl_ui_zoom_level_get, _elm_web_none_efl_ui_zoom_zoom_level_get),
      EFL_OBJECT_OP_FUNC(efl_ui_zoom_mode_set, _elm_web_none_efl_ui_zoom_zoom_mode_set),
      EFL_OBJECT_OP_FUNC(efl_ui_zoom_mode_get, _elm_web_none_efl_ui_zoom_zoom_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_web_region_show, _elm_web_none_elm_web_region_show),
      EFL_OBJECT_OP_FUNC(elm_obj_web_region_bring_in, _elm_web_none_elm_web_region_bring_in),
      EFL_OBJECT_OP_FUNC(elm_obj_web_inwin_mode_set, _elm_web_none_elm_web_inwin_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_web_inwin_mode_get, _elm_web_none_elm_web_inwin_mode_get),
      ELM_WEB_NONE_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_web_none_class_desc = {
   EO_VERSION,
   "Elm.Web.None",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Web_None_Data),
   _elm_web_none_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_web_none_class_get, &_elm_web_none_class_desc, ELM_WEB_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include "Elementary.h"
#include "elm_module_helper.h"
#include "elm_priv.h"
#include "elm_widget_web.h"

#define ELEMENTARY_BUILD
#undef ELM_MODULE_HELPER_H
#include "elm_module_helper.h"
#include "elm_web_none_eo.h"

#define MY_CLASS ELM_WEB_CLASS

#define MY_CLASS_NAME "Elm_Web_None"

typedef struct _Elm_Web_None_Data Elm_Web_None_Data;
struct _Elm_Web_None_Data
{
};

static int _none_log_dom = -1;

#undef CRI
#undef ERR
#undef WRN
#undef INF
#undef DBG
#define CRI(...)      EINA_LOG_DOM_CRIT(_none_log_dom, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_none_log_dom, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_none_log_dom, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_none_log_dom, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_none_log_dom, __VA_ARGS__)

EOLIAN static Eina_Bool
_elm_web_none_elm_web_tab_propagate_get(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *sd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static void
_elm_web_none_elm_web_tab_propagate_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *sd EINA_UNUSED, Eina_Bool propagate EINA_UNUSED)
{
}

EOLIAN static void
_elm_web_none_efl_canvas_group_group_add(Eo *obj, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   Evas_Object *resize_obj;

   resize_obj = elm_label_add(obj);
   elm_object_text_set(resize_obj, "WebKit not supported!");
   elm_widget_resize_object_set(obj, resize_obj);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);
}

EOLIAN static Evas_Object*
_elm_web_none_elm_web_webkit_view_get(const Eo *obj, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   (void)obj;
   ERR("Elementary not compiled with EWebKit support.");
   return NULL;
}

EOLIAN static void
_elm_web_none_elm_web_window_create_hook_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *sd EINA_UNUSED, Elm_Web_Window_Open func EINA_UNUSED, void *data EINA_UNUSED)
{
}

EOLIAN static void
_elm_web_none_elm_web_dialog_alert_hook_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *sd EINA_UNUSED, Elm_Web_Dialog_Alert func EINA_UNUSED, void *data EINA_UNUSED)
{
}

EOLIAN static void
_elm_web_none_elm_web_dialog_confirm_hook_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *sd EINA_UNUSED, Elm_Web_Dialog_Confirm func EINA_UNUSED, void *data EINA_UNUSED)
{
}

EOLIAN static void
_elm_web_none_elm_web_dialog_prompt_hook_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *sd EINA_UNUSED, Elm_Web_Dialog_Prompt func EINA_UNUSED, void *data EINA_UNUSED)
{
}

EOLIAN static void
_elm_web_none_elm_web_dialog_file_selector_hook_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, Elm_Web_Dialog_File_Selector func EINA_UNUSED, void *data EINA_UNUSED)
{
}

EOLIAN static void
_elm_web_none_elm_web_console_message_hook_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, Elm_Web_Console_Message func EINA_UNUSED, void *data EINA_UNUSED)
{
}

EOLIAN static void
_elm_web_none_elm_web_useragent_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, const char *user_agent EINA_UNUSED)
{
}

EOLIAN static const char*
_elm_web_none_elm_web_useragent_get(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_url_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, const char *url EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static const char*
_elm_web_none_elm_web_url_get(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_html_string_load(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, const char *html EINA_UNUSED, const char *base_url EINA_UNUSED, const char *unreachable_url EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static const char*
_elm_web_none_elm_web_title_get(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static void
_elm_web_none_elm_web_bg_color_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, int r EINA_UNUSED, int g EINA_UNUSED, int b EINA_UNUSED, int a EINA_UNUSED)
{
}

EOLIAN static void
_elm_web_none_elm_web_bg_color_get(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, int *r, int *g, int *b, int *a)
{
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;
}

EOLIAN static char*
_elm_web_none_elm_web_selection_get(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static void
_elm_web_none_elm_web_popup_selected_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, int idx EINA_UNUSED)
{
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_popup_destroy(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_text_search(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, const char *string EINA_UNUSED, Eina_Bool case_sensitive EINA_UNUSED, Eina_Bool forward EINA_UNUSED, Eina_Bool wrap EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static unsigned int
_elm_web_none_elm_web_text_matches_mark(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, const char *string EINA_UNUSED, Eina_Bool case_sensitive EINA_UNUSED, Eina_Bool highlight EINA_UNUSED, unsigned int limit EINA_UNUSED)
{
   return 0;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_text_matches_unmark_all(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_text_matches_highlight_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, Eina_Bool highlight EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_text_matches_highlight_get(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static double
_elm_web_none_elm_web_load_progress_get(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return -1.0;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_stop(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_reload(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_reload_full(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_back(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_forward(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_navigate(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, int steps EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_back_possible_get(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_forward_possible_get(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_navigate_possible_get(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, int steps EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_history_enabled_get(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static void
_elm_web_none_elm_web_history_enabled_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, Eina_Bool enable EINA_UNUSED)
{
}

EOLIAN static void
_elm_web_none_efl_ui_zoom_zoom_level_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, double zoom EINA_UNUSED)
{
}

EOLIAN static double
_elm_web_none_efl_ui_zoom_zoom_level_get(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return -1;
}

EOLIAN static void
_elm_web_none_efl_ui_zoom_zoom_mode_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, Efl_Ui_Zoom_Mode mode EINA_UNUSED)
{
}

EOLIAN static Efl_Ui_Zoom_Mode
_elm_web_none_efl_ui_zoom_zoom_mode_get(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED)
{
   return EFL_UI_ZOOM_MODE_LAST;
}

EOLIAN static void
_elm_web_none_elm_web_region_show(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
}

EOLIAN static void
_elm_web_none_elm_web_region_bring_in(Eo *obj EINA_UNUSED, Elm_Web_None_Data *_pd EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
}

EOLIAN static void
_elm_web_none_elm_web_inwin_mode_set(Eo *obj EINA_UNUSED, Elm_Web_None_Data *sd EINA_UNUSED, Eina_Bool value EINA_UNUSED)
{
}

EOLIAN static Eina_Bool
_elm_web_none_elm_web_inwin_mode_get(const Eo *obj EINA_UNUSED, Elm_Web_None_Data *sd EINA_UNUSED)
{
   return EINA_FALSE;
}

EAPI void
ewm_window_features_ref(Elm_Web_Window_Features *wf EINA_UNUSED)
{
}

EAPI void
ewm_window_features_unref(Elm_Web_Window_Features *wf EINA_UNUSED)
{
}

EAPI Eina_Bool
ewm_window_features_property_get(const Elm_Web_Window_Features *wf EINA_UNUSED,
                                 Elm_Web_Window_Feature_Flag flag EINA_UNUSED)
{
   return EINA_FALSE;
}

EAPI void
ewm_window_features_region_get(const Elm_Web_Window_Features *wf EINA_UNUSED,
                               Evas_Coord *x,
                               Evas_Coord *y,
                               Evas_Coord *w,
                               Evas_Coord *h)
{
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
}

EAPI void
ewm_unneed_web(void)
{
}

EAPI Eina_Bool
ewm_need_web(void)
{
   if (_none_log_dom == -1)
     _none_log_dom =  eina_log_domain_register("elm_none", EINA_COLOR_LIGHTBLUE);
   return EINA_TRUE;
}

EAPI const Efl_Class *
ewm_class_get(void)
{
   return elm_web_none_class_get();
}

#undef ELM_WEB_CLASS
#define ELM_WEB_CLASS elm_web_class_get()

/* Internal EO APIs and hidden overrides */

#define ELM_WEB_NONE_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(elm_web_none)

#include "elm_web_none_eo.c"

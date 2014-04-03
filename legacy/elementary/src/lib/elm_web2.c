#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_web.h"

#if defined(HAVE_ELEMENTARY_WEB) && defined(USE_WEBKIT2)
#include <EWebKit2.h>

#define MY_CLASS ELM_OBJ_WEB_CLASS

#define MY_CLASS_NAME "Elm_Web"
#define MY_CLASS_NAME_LEGACY "elm_web"

static Ewk_View_Smart_Class _ewk_view_parent_sc =
  EWK_VIEW_SMART_CLASS_INIT_NULL;

static const char SIG_URI_CHANGED[] = "uri,changed"; // deprecated, use "url,changed" instead.
static const char SIG_URL_CHANGED[] = "url,changed";

static const Evas_Smart_Cb_Description _elm_web_smart_callbacks[] = {
   { SIG_URI_CHANGED, "s" },
   { SIG_URL_CHANGED, "s" },
   { SIG_WIDGET_FOCUSED, ""}, /**< handled by elm_widget */
   { SIG_WIDGET_UNFOCUSED, ""}, /**< handled by elm_widget */
   { NULL, NULL }
};

static void
_view_smart_add(Evas_Object *obj)
{
   View_Smart_Data *sd;

   sd = calloc(1, sizeof(View_Smart_Data));
   evas_object_smart_data_set(obj, sd);

   _ewk_view_parent_sc.sc.add(obj);
}

static void
_view_smart_del(Evas_Object *obj)
{
   _ewk_view_parent_sc.sc.del(obj);
}

static Evas_Object *
_view_smart_window_create(Ewk_View_Smart_Data *vsd,
                          const Ewk_Window_Features *window_features)
{
   Evas_Object *new;
   Evas_Object *obj = evas_object_smart_parent_get(vsd->self);

   ELM_WEB_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   if (!sd->hook.window_create) return NULL;

   new = sd->hook.window_create
       (sd->hook.window_create_data, obj, EINA_TRUE,
       (const Elm_Web_Window_Features *)window_features);
   if (new) return elm_web_webkit_view_get(new);

   return NULL;
}

static void
_view_smart_window_close(Ewk_View_Smart_Data *sd)
{
   Evas_Object *obj = evas_object_smart_parent_get(sd->self);

   ELM_WEB_CHECK(obj);

   evas_object_smart_callback_call(obj, "windows,close,request", NULL);
}

static void
_fullscreen_accept(void *data, Evas_Object *obj EINA_UNUSED, void *ev EINA_UNUSED)
{
   Evas_Object *ewk = data;
   evas_object_del(evas_object_data_get(ewk, "_fullscreen_permission_popup"));
}

static void
_fullscreen_deny(void *data, Evas_Object *obj EINA_UNUSED, void *ev EINA_UNUSED)
{
   Evas_Object *ewk = data;
   ewk_view_fullscreen_exit(ewk);
   evas_object_del(evas_object_data_get(ewk, "_fullscreen_permission_popup"));
}

static Eina_Bool
_view_smart_fullscreen_enter(Ewk_View_Smart_Data *sd, Ewk_Security_Origin *origin)
{
   Evas_Object *btn, *popup, *top;
   const char *host;
   char buffer[2048];

   Evas_Object *obj = evas_object_smart_parent_get(sd->self);

   ELM_WEB_CHECK(obj) EINA_FALSE;

   top = elm_widget_top_get(obj);
   elm_win_fullscreen_set(top, EINA_TRUE);

   popup = elm_popup_add(top);
   elm_popup_orient_set(popup, ELM_POPUP_ORIENT_TOP);
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   host = ewk_security_origin_host_get(origin);
   snprintf(buffer, sizeof(buffer), "%s is now fullscreen.<br>Press ESC at any time to exit fullscreen,<br>Allow fullscreen?<br>", host);
   elm_object_text_set(popup, buffer);

   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Accept");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _fullscreen_accept, sd->self);

   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Deny");
   elm_object_part_content_set(popup, "button2", btn);
   evas_object_smart_callback_add(btn, "clicked", _fullscreen_deny, sd->self);

   evas_object_data_set(sd->self, "_fullscreen_permission_popup", popup);
   evas_object_show(popup);

   return EINA_TRUE;
}

static Eina_Bool
_view_smart_fullscreen_exit(Ewk_View_Smart_Data *sd)
{
   Evas_Object *obj = evas_object_smart_parent_get(sd->self);

   ELM_WEB_CHECK(obj) EINA_FALSE;

   Evas_Object *top = elm_widget_top_get(obj);
   elm_win_fullscreen_set(top, EINA_FALSE);

   return EINA_TRUE;
}

/**
 * Creates a new view object given the parent.
 *
 * @param parent object to use as parent.
 *
 * @return newly added Evas_Object or @c NULL on errors.
 */
Evas_Object *
_view_add(Evas_Object *parent)
{
   Evas *canvas = evas_object_evas_get(parent);
   static Evas_Smart *smart = NULL;
   Evas_Object *view;

   if (!smart)
     {
        static Ewk_View_Smart_Class api =
          EWK_VIEW_SMART_CLASS_INIT_NAME_VERSION("EWK_View_Elementary");

        ewk_view_smart_class_set(&api);
        ewk_view_smart_class_set(&_ewk_view_parent_sc);

        // TODO: check every api method and provide overrides with hooks!
        // TODO: hooks should provide extension points
        // TODO: extension should have some kind of "default implementation",
        // TODO: that can be replaced or controlled by hooks.
        // TODO: ie: run_javascript_alert() should present an elm_win
        // TODO: by default, but user could override it to show as inwin.
        api.sc.add = _view_smart_add;
        api.sc.del = _view_smart_del;
        api.window_create = _view_smart_window_create;
        api.window_close = _view_smart_window_close;
        api.fullscreen_enter = _view_smart_fullscreen_enter;
        api.fullscreen_exit = _view_smart_fullscreen_exit;

        smart = evas_smart_class_new(&api.sc);
        if (!smart)
          {
             CRI("Could not create smart class");
             return NULL;
          }
     }

   view = ewk_view_smart_add(canvas, smart, ewk_context_default_get(), ewk_page_group_create(0));
   if (!view)
     {
        ERR("Could not create smart object object for view");
        return NULL;
     }

   return view;
}

static void
_view_smart_url_changed_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info)
{
   evas_object_smart_callback_call(data, SIG_URI_CHANGED, event_info);
   evas_object_smart_callback_call(data, SIG_URL_CHANGED, event_info);
}

static void
_view_smart_callback_proxy(Evas_Object *view, Evas_Object *parent)
{
   evas_object_smart_callback_add(view, SIG_URL_CHANGED, _view_smart_url_changed_cb, parent);
}

static Eina_Bool _elm_need_web = EINA_FALSE;

void
_elm_unneed_web(void)
{
   if (!_elm_need_web) return;
   _elm_need_web = EINA_FALSE;
   ewk_shutdown();
}

EAPI Eina_Bool
elm_need_web(void)
{
   if (_elm_need_web) return EINA_TRUE;
   _elm_need_web = EINA_TRUE;
   ewk_init();
   return EINA_TRUE;
}

EOLIAN static void
_elm_web_evas_smart_add(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
#ifdef HAVE_ELEMENTARY_WEB
   Evas_Object *resize_obj;

   resize_obj = _view_add(obj);
   elm_widget_resize_object_set(obj, resize_obj, EINA_TRUE);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   //TODO: need a way to change theme
   ewk_view_theme_set(resize_obj, WEBKIT_DATADIR "/themes/default.edj");

   _view_smart_callback_proxy(resize_obj, obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);
#endif
}

EOLIAN static void
_elm_web_evas_smart_del(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_web_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_web_eo_base_constructor(Eo *obj, Elm_Web_Data *sd)
{
   sd->obj = obj;
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_elm_web_smart_callbacks, NULL));
}

EOLIAN static Evas_Object*
_elm_web_webkit_view_get(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);
   return wd->resize_obj;
}

EOLIAN static void
_elm_web_window_create_hook_set(Eo *obj EINA_UNUSED, Elm_Web_Data *sd, Elm_Web_Window_Open func, void *data)
{
   sd->hook.window_create = func;
   sd->hook.window_create_data = data;
}

EOLIAN static void
_elm_web_dialog_alert_hook_set(Eo *obj EINA_UNUSED, Elm_Web_Data *_pd EINA_UNUSED, Elm_Web_Dialog_Alert func, void *data)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)func;
   (void)data;
   (void)_pd;
#else
   (void)func;
   (void)data;
   (void)_pd;
#endif
}

EOLIAN static void
_elm_web_dialog_confirm_hook_set(Eo *obj EINA_UNUSED, Elm_Web_Data *_pd EINA_UNUSED, Elm_Web_Dialog_Confirm func, void *data)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)func;
   (void)data;
   (void)_pd;
#else
   (void)func;
   (void)data;
   (void)_pd;
#endif
}

EOLIAN static void
_elm_web_dialog_prompt_hook_set(Eo *obj EINA_UNUSED, Elm_Web_Data *_pd EINA_UNUSED, Elm_Web_Dialog_Prompt func, void *data)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)func;
   (void)data;
   (void)_pd;
#else
   (void)func;
   (void)data;
   (void)_pd;
#endif
}

EOLIAN static void
_elm_web_dialog_file_selector_hook_set(Eo *obj EINA_UNUSED, Elm_Web_Data *_pd EINA_UNUSED, Elm_Web_Dialog_File_Selector func, void *data)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)func;
   (void)data;
   (void)_pd;
#else
   (void)func;
   (void)data;
   (void)_pd;
#endif
}

EOLIAN static void
_elm_web_console_message_hook_set(Eo *obj EINA_UNUSED, Elm_Web_Data *_pd EINA_UNUSED, Elm_Web_Console_Message func, void *data)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)func;
   (void)data;
   (void)_pd;
#else
   (void)func;
   (void)data;
   (void)_pd;
#endif
}

EOLIAN static void
_elm_web_useragent_set(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, const char *user_agent)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   ewk_view_user_agent_set(wd->resize_obj, user_agent);
#else
   (void)user_agent;
   (void)obj;
#endif
}

EOLIAN static const char*
_elm_web_useragent_get(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   return ewk_view_user_agent_get(wd->resize_obj);
#else
   (void)obj;
   return NULL;
#endif
}

EAPI Eina_Bool
elm_web_uri_set(Evas_Object *obj,
                const char *url)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_url_set(url, &ret));
   return ret;
}

EOLIAN static Eina_Bool
_elm_web_url_set(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, const char *url)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   return ewk_view_url_set(wd->resize_obj, url);
}

EOLIAN static const char*
_elm_web_url_get(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);
   return ewk_view_url_get(wd->resize_obj);
}

EOLIAN static Eina_Bool
_elm_web_html_string_load(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, const char *html, const char *base_url, const char *unreachable_url)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   return ewk_view_html_string_load(wd->resize_obj,
                                             html, base_url, unreachable_url);
}

EOLIAN static const char*
_elm_web_title_get(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);
   return ewk_view_title_get(wd->resize_obj);
}

EOLIAN static void
_elm_web_bg_color_set(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, int r, int g, int b, int a)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)obj;
   (void)r;
   (void)g;
   (void)b;
   (void)a;
#else
   (void)obj;
   (void)r;
   (void)g;
   (void)b;
   (void)a;
#endif
}

EOLIAN static void
_elm_web_bg_color_get(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, int *r, int *g, int *b, int *a)
{
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;
#ifdef HAVE_ELEMENTARY_WEB
   (void)obj;
#else
   (void)obj;
#endif
}

EOLIAN static const char*
_elm_web_selection_get(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)obj;
   return NULL;
#else
   (void)obj;
   return NULL;
#endif
}

EOLIAN static void
_elm_web_popup_selected_set(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, int idx)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)idx;
   (void)obj;
#else
   (void)idx;
   (void)obj;
#endif
}

EOLIAN static Eina_Bool
_elm_web_popup_destroy(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)obj;
#else
   (void)obj;
#endif

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_text_search(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, const char *string, Eina_Bool case_sensitive, Eina_Bool forward, Eina_Bool wrap)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)string;
   (void)case_sensitive;
   (void)forward;
   (void)wrap;
   (void)obj;
#else
   (void)string;
   (void)case_sensitive;
   (void)forward;
   (void)wrap;
   (void)obj;
#endif

   return EINA_FALSE;
}

EOLIAN static unsigned int
_elm_web_text_matches_mark(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, const char *string, Eina_Bool case_sensitive, Eina_Bool highlight, unsigned int limit)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)string;
   (void)case_sensitive;
   (void)highlight;
   (void)limit;
   (void)obj;
#else
   (void)string;
   (void)case_sensitive;
   (void)highlight;
   (void)limit;
   (void)obj;
#endif

   return 0;
}

EOLIAN static Eina_Bool
_elm_web_text_matches_unmark_all(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)obj;
#else
   (void)obj;
#endif

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_text_matches_highlight_set(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, Eina_Bool highlight)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)obj;
   (void)highlight;
#else
   (void)obj;
   (void)highlight;
#endif

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_text_matches_highlight_get(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)obj;
#else
   (void)obj;
#endif

   return EINA_FALSE;
}

EOLIAN static double
_elm_web_load_progress_get(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
   double ret;
   ret = -1.0;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ret);

   ret = ewk_view_load_progress_get(wd->resize_obj);
#else
   (void)obj;
#endif

   return ret;
}

EOLIAN static Eina_Bool
_elm_web_stop(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
   Eina_Bool ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ret);

   ret = ewk_view_stop(wd->resize_obj);
#else
   (void)obj;
#endif

   return ret;
}

EOLIAN static Eina_Bool
_elm_web_reload(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
   Eina_Bool ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ret);

   ret = ewk_view_reload(wd->resize_obj);
#else
   (void)obj;
#endif

   return ret;
}

EOLIAN static Eina_Bool
_elm_web_reload_full(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
   Eina_Bool ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ret);

   ret = ewk_view_reload_bypass_cache(wd->resize_obj);
#else
   (void)obj;
#endif

   return ret;
}

EOLIAN static Eina_Bool
_elm_web_back(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
   Eina_Bool ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ret);

   ret = ewk_view_back(wd->resize_obj);
#else
   (void)obj;
#endif

   return ret;
}

EOLIAN static Eina_Bool
_elm_web_forward(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
   Eina_Bool ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   ret = ewk_view_forward(wd->resize_obj);
#else
   (void)obj;
#endif

   return ret;
}

EOLIAN static Eina_Bool
_elm_web_navigate(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, int steps)
{
   Eina_Bool ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   Ewk_Back_Forward_List *history;
   Ewk_Back_Forward_List_Item *item = NULL;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ret);

   history = ewk_view_back_forward_list_get(wd->resize_obj);
   if (history)
     {
        item = ewk_back_forward_list_item_at_index_get(history, steps);
        if (item) ret = ewk_view_navigate_to(wd->resize_obj, item);
     }
#else
   (void)steps;
   (void)obj;
#endif

   return ret;
}

EOLIAN static Eina_Bool
_elm_web_back_possible_get(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
   Eina_Bool ret;
   ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ret);

   ret = ewk_view_back_possible(wd->resize_obj);
#else
   (void)obj;
#endif

   return ret;
}

EOLIAN static Eina_Bool
_elm_web_forward_possible_get(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
   Eina_Bool ret;
   ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ret);

   ret = ewk_view_forward_possible(wd->resize_obj);
#else
   (void)obj;
#endif

   return ret;
}

EOLIAN static Eina_Bool
_elm_web_navigate_possible_get(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, int steps)
{
   Eina_Bool ret;
   ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   Ewk_Back_Forward_List *history;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ret);

   history = ewk_view_back_forward_list_get(wd->resize_obj);
   if (history && ewk_back_forward_list_item_at_index_get(history, steps))
     ret = EINA_TRUE;
#else
   (void)steps;
   (void)obj;
#endif

   return ret;
}

EOLIAN static Eina_Bool
_elm_web_history_enabled_get(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED)
{
   Eina_Bool ret;
   ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   (void)obj;
#else
   (void)obj;
#endif

   return ret;
}

EOLIAN static void
_elm_web_history_enabled_set(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, Eina_Bool enable)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)enable;
   (void)obj;
#else
   (void)enable;
   (void)obj;
#endif
}

EOLIAN static void
_elm_web_zoom_set(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, double zoom)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)obj;
   (void)_pd;
   (void)zoom;
#else
   (void)obj;
   (void)_pd;
   (void)zoom;
#endif
}

EOLIAN static double
_elm_web_zoom_get(Eo *obj EINA_UNUSED, Elm_Web_Data *_pd EINA_UNUSED)
{
   double ret;
   ret = -1.0;
#ifdef HAVE_ELEMENTARY_WEB
   (void)_pd;
#else
   (void)_pd;
#endif

   return ret;
}

EOLIAN static void
_elm_web_zoom_mode_set(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, Elm_Web_Zoom_Mode mode)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)obj;
   (void)_pd;
   (void)mode;
#else
   (void)obj;
   (void)_pd;
   (void)mode;
#endif
}

EOLIAN static Elm_Web_Zoom_Mode
_elm_web_zoom_mode_get(Eo *obj EINA_UNUSED, Elm_Web_Data *_pd EINA_UNUSED)
{
   Elm_Web_Zoom_Mode ret;
   ret = ELM_WEB_ZOOM_MODE_LAST;
#ifdef HAVE_ELEMENTARY_WEB
   (void)_pd;
#else
   (void)_pd;
#endif

   return ret;
}

EOLIAN static void
_elm_web_region_show(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, int x, int y, int w, int h)
{
   (void)w;
   (void)h;

#ifdef HAVE_ELEMENTARY_WEB
   (void)obj;
   (void)_pd;
   (void)x;
   (void)y;
#else
   (void)obj;
   (void)_pd;
   (void)x;
   (void)y;
#endif
}

EOLIAN static void
_elm_web_region_bring_in(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, int x, int y, int w, int h)
{
   (void)w;
   (void)h;

#ifdef HAVE_ELEMENTARY_WEB
   (void)obj;
   (void)_pd;
   (void)x;
   (void)y;
#else
   (void)obj;
   (void)_pd;
   (void)x;
   (void)y;
#endif
}

EOLIAN static void
_elm_web_inwin_mode_set(Eo *obj EINA_UNUSED, Elm_Web_Data *_pd EINA_UNUSED, Eina_Bool value)
{
#ifdef HAVE_ELEMENTARY_WEB
   (void)_pd;
   (void)value;
#else
   (void)_pd;
   (void)value;
#endif
}

EOLIAN static Eina_Bool
_elm_web_inwin_mode_get(Eo *obj EINA_UNUSED, Elm_Web_Data *_pd EINA_UNUSED)
{
   Eina_Bool ret;
   ret = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   (void)_pd;
#else
   (void)_pd;
#endif

   return ret;
}

EAPI void
elm_web_window_features_ref(Elm_Web_Window_Features *wf)
{
   ewk_object_ref((Ewk_Object *)wf);
}

EAPI void
elm_web_window_features_unref(Elm_Web_Window_Features *wf)
{
   ewk_object_unref((Ewk_Object *)wf);
}

EAPI Eina_Bool
elm_web_window_features_property_get(const Elm_Web_Window_Features *wf,
                                     Elm_Web_Window_Feature_Flag flag)
{
   const Ewk_Window_Features *ewf = (const Ewk_Window_Features *)wf;
   switch (flag)
     {
      case ELM_WEB_WINDOW_FEATURE_TOOLBAR:
        return ewk_window_features_toolbar_visible_get(ewf);

      case ELM_WEB_WINDOW_FEATURE_STATUSBAR:
        return ewk_window_features_statusbar_visible_get(ewf);

      case ELM_WEB_WINDOW_FEATURE_SCROLLBARS:
        return ewk_window_features_scrollbars_visible_get(ewf);

      case ELM_WEB_WINDOW_FEATURE_MENUBAR:
        return ewk_window_features_menubar_visible_get(ewf);

      case ELM_WEB_WINDOW_FEATURE_LOCATIONBAR:
        return ewk_window_features_locationbar_visible_get(ewf);

      case ELM_WEB_WINDOW_FEATURE_FULLSCREEN:
        return ewk_window_features_fullscreen_get(ewf);
     }

   return EINA_FALSE;
}

EAPI void
elm_web_window_features_region_get(const Elm_Web_Window_Features *wf,
                                   Evas_Coord *x,
                                   Evas_Coord *y,
                                   Evas_Coord *w,
                                   Evas_Coord *h)
{
   ewk_window_features_geometry_get
     ((const Ewk_Window_Features *)wf, x, y, w, h);
}

static void
_elm_web_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_web.eo.c"

#endif

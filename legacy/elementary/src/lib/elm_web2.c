#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_web.h"

#if defined(HAVE_ELEMENTARY_WEB) && defined(USE_WEBKIT2)
#include <EWebKit2.h>

EAPI const char ELM_WEB_SMART_NAME[] = "elm_web";

static Ewk_View_Smart_Class _ewk_view_parent_sc =
  EWK_VIEW_SMART_CLASS_INIT_NULL;

static const Evas_Smart_Cb_Description _elm_web_smart_callbacks[] = {
   { "url,changed", "s" },
   { NULL, NULL }
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_WEB_SMART_NAME, _elm_web, Elm_Web_Smart_Class, Elm_Widget_Smart_Class,
  elm_widget_smart_class_get, _elm_web_smart_callbacks);

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

        smart = evas_smart_class_new(&api.sc);
        if (!smart)
          {
             CRITICAL("Could not create smart class");
             return NULL;
          }
     }

   view = ewk_view_smart_add(canvas, smart, ewk_context_default_get());
   if (!view)
     {
        ERR("Could not create smart object object for view");
        return NULL;
     }

   return view;
}

static int _elm_need_web = 0;

EAPI Eina_Bool
elm_need_web(void)
{
   if (_elm_need_web++) return EINA_TRUE;
   ewk_init();
   return EINA_TRUE;
}

static void
_elm_web_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Web_Smart_Data);

   ELM_WIDGET_DATA(priv)->resize_obj = _view_add(obj);

   _elm_web_parent_sc->base.add(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);
}

static void
_elm_web_smart_del(Evas_Object *obj)
{
   _elm_web_parent_sc->base.del(obj); /* handles freeing sd */
}

static void
_elm_web_smart_set_user(Elm_Web_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_web_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_web_smart_del;
}

EAPI const Elm_Web_Smart_Class *
elm_web_smart_class_get(void)
{
   static Elm_Web_Smart_Class _sc =
     ELM_WEB_SMART_CLASS_INIT_NAME_VERSION(ELM_WEB_SMART_NAME);
   static const Elm_Web_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_web_smart_set(&_sc);
   esc->callbacks = _elm_web_smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_web_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_web_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI Evas_Object *
elm_web_webkit_view_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) NULL;

   ELM_WEB_DATA_GET(obj, sd);

   return ELM_WIDGET_DATA(sd)->resize_obj;
}

EAPI void
elm_web_window_create_hook_set(Evas_Object *obj,
                               Elm_Web_Window_Open func,
                               void *data)
{
   ELM_WEB_CHECK(obj);

   (void)func;
   (void)data;
}

EAPI void
elm_web_dialog_alert_hook_set(Evas_Object *obj,
                              Elm_Web_Dialog_Alert func,
                              void *data)
{
   ELM_WEB_CHECK(obj);

   (void)func;
   (void)data;
}

EAPI void
elm_web_dialog_confirm_hook_set(Evas_Object *obj,
                                Elm_Web_Dialog_Confirm func,
                                void *data)
{
   ELM_WEB_CHECK(obj);

   (void)func;
   (void)data;
}

EAPI void
elm_web_dialog_prompt_hook_set(Evas_Object *obj,
                               Elm_Web_Dialog_Prompt func,
                               void *data)
{
   ELM_WEB_CHECK(obj);

   (void)func;
   (void)data;
}

EAPI void
elm_web_dialog_file_selector_hook_set(Evas_Object *obj,
                                      Elm_Web_Dialog_File_Selector func,
                                      void *data)
{
   ELM_WEB_CHECK(obj);

   (void)func;
   (void)data;
}

EAPI void
elm_web_console_message_hook_set(Evas_Object *obj,
                                 Elm_Web_Console_Message func,
                                 void *data)
{
   ELM_WEB_CHECK(obj);

   (void)func;
   (void)data;
}

EAPI void
elm_web_useragent_set(Evas_Object *obj,
                      const char *user_agent)
{
   ELM_WEB_CHECK(obj);

   // FIXME : need to implement
   (void)user_agent;
}

EAPI const char *
elm_web_useragent_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) NULL;

   // FIXME : need to implement
   return NULL;
}

EAPI Eina_Bool
elm_web_uri_set(Evas_Object *obj,
                const char *uri)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_url_set(ELM_WIDGET_DATA(sd)->resize_obj, uri);
}

EAPI const char *
elm_web_uri_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) NULL;

   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_url_get(ELM_WIDGET_DATA(sd)->resize_obj);
}

EAPI const char *
elm_web_title_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) NULL;

   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_title_get(ELM_WIDGET_DATA(sd)->resize_obj);
}

EAPI void
elm_web_bg_color_set(Evas_Object *obj,
                     int r,
                     int g,
                     int b,
                     int a)
{
   ELM_WEB_CHECK(obj);

   (void)r;
   (void)g;
   (void)b;
   (void)a;
}

EAPI void
elm_web_bg_color_get(const Evas_Object *obj,
                     int *r,
                     int *g,
                     int *b,
                     int *a)
{
   ELM_WEB_CHECK(obj);

   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;
}

EAPI const char *
elm_web_selection_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) NULL;

   return NULL;
}

EAPI void
elm_web_popup_selected_set(Evas_Object *obj,
                           int idx)
{
   ELM_WEB_CHECK(obj);

   (void)idx;
}

EAPI Eina_Bool
elm_web_popup_destroy(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   return EINA_FALSE;
}

EAPI Eina_Bool
elm_web_text_search(const Evas_Object *obj,
                    const char *string,
                    Eina_Bool case_sensitive,
                    Eina_Bool forward,
                    Eina_Bool wrap)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   (void)string;
   (void)case_sensitive;
   (void)forward;
   (void)wrap;
   return EINA_FALSE;
}

EAPI unsigned int
elm_web_text_matches_mark(Evas_Object *obj,
                          const char *string,
                          Eina_Bool case_sensitive,
                          Eina_Bool highlight,
                          unsigned int limit)
{
   ELM_WEB_CHECK(obj) 0;

   (void)string;
   (void)case_sensitive;
   (void)highlight;
   (void)limit;
   return 0;
}

EAPI Eina_Bool
elm_web_text_matches_unmark_all(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   return EINA_FALSE;
}

EAPI Eina_Bool
elm_web_text_matches_highlight_set(Evas_Object *obj,
                                   Eina_Bool highlight)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   (void)highlight;
   return EINA_FALSE;
}

EAPI Eina_Bool
elm_web_text_matches_highlight_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   return EINA_FALSE;
}

EAPI double
elm_web_load_progress_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) - 1.0;

   return EINA_FALSE;
}

EAPI Eina_Bool
elm_web_stop(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   return EINA_FALSE;
}

EAPI Eina_Bool
elm_web_reload(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   return EINA_FALSE;
}

EAPI Eina_Bool
elm_web_reload_full(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   return EINA_FALSE;
}

EAPI Eina_Bool
elm_web_back(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   return EINA_FALSE;
}

EAPI Eina_Bool
elm_web_forward(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   return EINA_FALSE;
}

EAPI Eina_Bool
elm_web_navigate(Evas_Object *obj,
                 int steps)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   return EINA_FALSE;
   (void)steps;
}

EAPI Eina_Bool
elm_web_back_possible_get(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   return EINA_FALSE;
}

EAPI Eina_Bool
elm_web_forward_possible_get(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   return EINA_FALSE;
}

EAPI Eina_Bool
elm_web_navigate_possible_get(Evas_Object *obj,
                              int steps)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   (void)steps;
   return EINA_FALSE;
}

EAPI Eina_Bool
elm_web_history_enabled_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   return EINA_FALSE;
}

EAPI void
elm_web_history_enabled_set(Evas_Object *obj,
                            Eina_Bool enable)
{
   ELM_WEB_CHECK(obj);

   (void)enable;
}

EAPI void
elm_web_zoom_set(Evas_Object *obj,
                 double zoom)
{
   ELM_WEB_CHECK(obj);

   (void)zoom;
}

EAPI double
elm_web_zoom_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) - 1.0;

   return -1.0;
}

EAPI void
elm_web_zoom_mode_set(Evas_Object *obj,
                      Elm_Web_Zoom_Mode mode)
{
   ELM_WEB_CHECK(obj);

   (void)mode;
}

EAPI Elm_Web_Zoom_Mode
elm_web_zoom_mode_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) ELM_WEB_ZOOM_MODE_LAST;

   return ELM_WEB_ZOOM_MODE_LAST;
}

EAPI void
elm_web_region_show(Evas_Object *obj,
                    int x,
                    int y,
                    int w __UNUSED__,
                    int h __UNUSED__)
{
   ELM_WEB_CHECK(obj);

   (void)x;
   (void)y;
}

EAPI void
elm_web_region_bring_in(Evas_Object *obj,
                        int x,
                        int y,
                        int w __UNUSED__,
                        int h __UNUSED__)
{
   ELM_WEB_CHECK(obj);

   (void)x;
   (void)y;
}

EAPI void
elm_web_inwin_mode_set(Evas_Object *obj,
                       Eina_Bool value)
{
   ELM_WEB_CHECK(obj);

   (void)value;
}

EAPI Eina_Bool
elm_web_inwin_mode_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

   return EINA_FALSE;
}

EAPI void
elm_web_window_features_ref(Elm_Web_Window_Features *wf)
{
   // FIXME : need to implement
   (void)wf;
}

EAPI void
elm_web_window_features_unref(Elm_Web_Window_Features *wf)
{
   // FIXME : need to implement
   (void)wf;
}

EAPI Eina_Bool
elm_web_window_features_property_get(const Elm_Web_Window_Features *wf,
                                     Elm_Web_Window_Feature_Flag flag)
{
   // FIXME : need to implement
   (void)wf;
   (void)flag;
   return EINA_FALSE;
}

EAPI void
elm_web_window_features_region_get(const Elm_Web_Window_Features *wf,
                                   Evas_Coord *x,
                                   Evas_Coord *y,
                                   Evas_Coord *w,
                                   Evas_Coord *h)
{
   // FIXME : need to implement
   (void)wf;
   (void)x;
   (void)y;
   (void)w;
   (void)h;
   return;
}
#endif

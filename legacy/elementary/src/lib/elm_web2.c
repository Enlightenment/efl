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
_popup_item_selected(void *data,
                     Evas_Object *obj,
                     void *event_info EINA_UNUSED)
{
   Elm_Object_Item *list_it = elm_list_selected_item_get(obj);
   const Eina_List *itr, *list = elm_list_items_get(obj);
   Ewk_Popup_Menu *menu = data;
   int i = 0;
   void *d;

   EINA_LIST_FOREACH(list, itr, d)
     {
        if (d == list_it)
          break;

        i++;
     }

   ewk_popup_menu_selected_index_set(menu, i);
   ewk_popup_menu_close(menu);

   evas_object_del(evas_object_data_get(obj, "_notify"));
}

static void
_popup_dismiss_cb(void *data,
                  Evas_Object *obj,
                  void *event_info EINA_UNUSED)
{
   ewk_popup_menu_close(data);
   evas_object_del(obj);
}

static Eina_Bool
_view_smart_popup_menu_show(Ewk_View_Smart_Data *sd,
                            Eina_Rectangle r,
                            Ewk_Text_Direction dir EINA_UNUSED,
                            double scale EINA_UNUSED,
                            Ewk_Popup_Menu *menu)
{
   Evas_Object *notify, *list, *grid, *win;
   const Eina_List* items = ewk_popup_menu_items_get(menu);
   Evas_Object *obj = evas_object_smart_parent_get(sd->self);
   int h, ww, wh;

   Elm_Object_Item *lit;
   Eina_Bool disabled;
   const char *txt;
   Ewk_Popup_Menu_Item *it;
   const Eina_List *itr;

   Evas_Object *popup = evas_object_data_get(sd->self, "_select_popup");
   if (popup) evas_object_del(popup);

   win = elm_widget_top_get(obj);

   notify = elm_notify_add(win);
   elm_notify_allow_events_set(notify, EINA_FALSE);
   elm_notify_align_set(notify, 0.5, 1.0);

   list = elm_list_add(notify);
   evas_object_data_set(list, "_notify", notify);
   elm_list_select_mode_set(list, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_scroller_bounce_set(list, EINA_FALSE, EINA_FALSE);
   elm_list_mode_set(list, ELM_LIST_EXPAND);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(list);

   EINA_LIST_FOREACH(items, itr, it)
     {
        switch (ewk_popup_menu_item_type_get(it))
          {
           case EWK_POPUP_MENU_SEPARATOR:
             // TODO:
             break;
           case EWK_POPUP_MENU_ITEM:
             txt = ewk_popup_menu_item_text_get(it);
             if (ewk_popup_menu_item_is_label_get(it))
               {
                  lit = elm_list_item_append(list, txt, NULL, NULL, NULL, NULL);
                  disabled = EINA_TRUE;
               }
             else
               {
                  lit = elm_list_item_append(list, txt, NULL, NULL, _popup_item_selected, menu);
                  disabled = !ewk_popup_menu_item_enabled_get(it);
               }

             elm_object_item_disabled_set(lit, disabled);
             break;
           default:
             break;
          }
     }
   elm_list_go(list);

   grid = elm_grid_add(win);
   elm_grid_size_set(grid, 1, 1);
   elm_grid_pack(grid, list, 0, 0, 1, 1);
   evas_object_geometry_get(win, NULL, NULL, &ww, &wh);

   //FIXME: it should be the real height of items in the list.
   h = r.h * eina_list_count(items);
   evas_object_size_hint_min_set(grid, ww, h < wh / 2 ? h : wh / 2);
   elm_object_content_set(notify, grid);
   evas_object_show(grid);

   evas_object_show(notify);

   evas_object_data_set(sd->self, "_select_popup", notify);

   evas_object_smart_callback_add
     (notify, "block,clicked", _popup_dismiss_cb, menu);

   return EINA_TRUE;
}

static Eina_Bool
_view_smart_popup_menu_hide(Ewk_View_Smart_Data *sd)
{
   Evas_Object *popup = evas_object_data_get(sd->self, "_select_popup");
   if (!popup) return EINA_FALSE;

   evas_object_del(popup);
   evas_object_data_del(sd->self, "_select_popup");

   return EINA_TRUE;
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

static void
_bt_close(void *data,
          Evas_Object *obj,
          void *event_info EINA_UNUSED)
{
   Dialog_Data *d = data;

   if (d->type == DIALOG_ALERT) goto end;

   *d->response = (obj == d->bt_ok);
   if (d->type == DIALOG_CONFIRM) goto end;

   if (d->type == DIALOG_PROMPT)
     *d->entry_value = strdup(elm_entry_entry_get(d->entry));

end:
   evas_object_del(d->dialog);
}

static Dialog_Data *
_dialog_new(Evas_Object *web, Eina_Bool inwin_mode)
{
   Dialog_Data *d;

   d = calloc(1, sizeof(Dialog_Data));
   if (!d) return NULL;

   if (!web || inwin_mode)
     {
        Evas_Object *bg;

        d->dialog = elm_win_add(NULL, "elm-web-popup", ELM_WIN_DIALOG_BASIC);
        evas_object_smart_callback_add
          (d->dialog, "delete,request", _bt_close, d);

        bg = elm_bg_add(d->dialog);
        evas_object_size_hint_weight_set
          (bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_win_resize_object_add(d->dialog, bg);
        evas_object_show(bg);

        d->box = elm_box_add(d->dialog);
        evas_object_size_hint_weight_set
          (d->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_win_resize_object_add(d->dialog, d->box);
        evas_object_show(d->box);
     }
   else
     {
        Evas_Object *win = elm_widget_top_get(web);

        d->dialog = elm_win_inwin_add(win);
        elm_object_style_set(d->dialog, "minimal");
        evas_object_size_hint_weight_set
          (d->dialog, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

        d->box = elm_box_add(win);
        evas_object_size_hint_weight_set
          (d->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_win_inwin_content_set(d->dialog, d->box);
        evas_object_show(d->box);
     }

   return d;
}

static void
_dialog_ok_cancel_buttons_add(Dialog_Data *dialog_data)
{
   Evas_Object *bx, *bt;
   bx = elm_box_add(dialog_data->box);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_box_pack_end(dialog_data->box, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bx);

   dialog_data->bt_cancel = bt = elm_button_add(bx);
   elm_object_text_set(bt, "Cancel");
   elm_box_pack_end(bx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(bt, "clicked", _bt_close, dialog_data);
   evas_object_show(bt);

   dialog_data->bt_ok = bt = elm_button_add(bx);
   elm_object_text_set(bt, "Ok");
   elm_box_pack_end(bx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(bt, "clicked", _bt_close, dialog_data);
   evas_object_show(bt);
}

static void
_dialog_del_cb(void *data EINA_UNUSED,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_exec_dialog(Evas_Object *dialog)
{
   evas_object_event_callback_add
     (dialog, EVAS_CALLBACK_DEL, _dialog_del_cb, NULL);
   ecore_main_loop_begin();
}

static void
_view_smart_run_javascript_alert(Ewk_View_Smart_Data *vsd, const char *message)
{
   Evas_Object *obj = evas_object_smart_parent_get(vsd->self);
   Evas_Object *dialog = NULL;

   ELM_WEB_DATA_GET_OR_RETURN(obj, sd);

   if (sd->hook.alert)
     dialog = sd->hook.alert(sd->hook.alert_data, obj, message);
   else
     {
        Evas_Object *lb;
        Dialog_Data *dialog_data = _dialog_new(obj, sd->inwin_mode);
        dialog_data->type = DIALOG_ALERT;
        dialog = dialog_data->dialog;

        lb = elm_label_add(dialog_data->box);
        elm_object_text_set(lb, message);
        elm_box_pack_end(dialog_data->box, lb);
        evas_object_show(lb);

        dialog_data->bt_ok = elm_button_add(dialog_data->box);
        elm_object_text_set(dialog_data->bt_ok, "Close");
        elm_box_pack_end(dialog_data->box, dialog_data->bt_ok);
        evas_object_size_hint_align_set
          (dialog_data->bt_ok, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add
          (dialog_data->bt_ok, "clicked", _bt_close, dialog_data);
        evas_object_show(dialog_data->bt_ok);

        evas_object_show(dialog);
     }

   if (dialog) _exec_dialog(dialog);
}

static Eina_Bool
_view_smart_run_javascript_confirm(Ewk_View_Smart_Data *vsd, const char *message)
{
   Evas_Object *obj = evas_object_smart_parent_get(vsd->self);
   Eina_Bool response = EINA_FALSE;
   Evas_Object *dialog = NULL;

   ELM_WEB_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   if (sd->hook.confirm)
     dialog = sd->hook.confirm(sd->hook.confirm_data, obj, message, &response);
   else
     {
        Evas_Object *lb;
        Dialog_Data *dialog_data = _dialog_new(obj, sd->inwin_mode);
        dialog_data->type = DIALOG_CONFIRM;
        dialog_data->response = &response;
        dialog = dialog_data->dialog;

        lb = elm_label_add(dialog_data->box);
        elm_object_text_set(lb, message);
        elm_box_pack_end(dialog_data->box, lb);
        evas_object_show(lb);

        _dialog_ok_cancel_buttons_add(dialog_data);

        evas_object_show(dialog);
     }

   if (dialog) _exec_dialog(dialog);

   return response;
}

static const char *
_view_smart_run_javascript_prompt(Ewk_View_Smart_Data *vsd, const char *message, const char *default_value)
{
   Evas_Object *obj = evas_object_smart_parent_get(vsd->self);
   Eina_Bool response = EINA_FALSE;
   Evas_Object *dialog = NULL;
   char *value = NULL;
   const char *ret;

   ELM_WEB_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   if (sd->hook.prompt)
     dialog = sd->hook.prompt(sd->hook.prompt_data, obj, message, default_value, (const char**)&value, &response);
   else
     {
        Evas_Object *lb, *entry;
        Dialog_Data *dialog_data = _dialog_new(obj, sd->inwin_mode);
        dialog_data->type = DIALOG_PROMPT;
        dialog_data->response = &response;
        dialog_data->entry_value = (const char**)&value;
        dialog = dialog_data->dialog;

        lb = elm_label_add(dialog_data->box);
        elm_object_text_set(lb, message);
        elm_box_pack_end(dialog_data->box, lb);
        evas_object_show(lb);

        dialog_data->entry = entry = elm_entry_add(dialog_data->box);
        elm_entry_single_line_set(entry, EINA_TRUE);
        elm_entry_scrollable_set(entry, EINA_TRUE);
        elm_entry_entry_set(entry, default_value);
        evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_box_pack_end(dialog_data->box, entry);
        evas_object_show(entry);

        _dialog_ok_cancel_buttons_add(dialog_data);

        evas_object_show(dialog);
     }

   if (dialog) _exec_dialog(dialog);

   if (!value) return NULL;

   ret = eina_stringshare_add(value);
   free(value);
   return ret;
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
        api.popup_menu_show = _view_smart_popup_menu_show;
        api.popup_menu_hide = _view_smart_popup_menu_hide;
        api.fullscreen_enter = _view_smart_fullscreen_enter;
        api.fullscreen_exit = _view_smart_fullscreen_exit;
        api.run_javascript_alert = _view_smart_run_javascript_alert;
        api.run_javascript_confirm = _view_smart_run_javascript_confirm;
        api.run_javascript_prompt = _view_smart_run_javascript_prompt;

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

EOLIAN static Eina_Bool
_elm_web_elm_widget_theme_apply(Eo *obj, Elm_Web_Data *sd EINA_UNUSED)
{
   (void)obj;
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_web_elm_widget_on_focus(Eo *obj, Elm_Web_Data *sd)
{
   (void)obj;
   (void)sd;
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_web_elm_widget_event(Eo *obj, Elm_Web_Data *sd, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   (void)obj;
   (void)sd;
   (void)src;
   (void)type;
   (void)event_info;
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_web_tab_propagate_get(Eo *obj EINA_UNUSED, Elm_Web_Data *sd)
{
   return sd->tab_propagate;
}

EOLIAN static void
_elm_web_tab_propagate_set(Eo *obj EINA_UNUSED, Elm_Web_Data *sd, Eina_Bool propagate)
{
   sd->tab_propagate = propagate;
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
         evas_obj_smart_callbacks_descriptions_set(_elm_web_smart_callbacks));
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
_elm_web_dialog_alert_hook_set(Eo *obj EINA_UNUSED, Elm_Web_Data *sd, Elm_Web_Dialog_Alert func, void *data)
{
   sd->hook.alert = func;
   sd->hook.alert_data = data;
}

EOLIAN static void
_elm_web_dialog_confirm_hook_set(Eo *obj EINA_UNUSED, Elm_Web_Data *sd, Elm_Web_Dialog_Confirm func, void *data)
{
   sd->hook.confirm = func;
   sd->hook.confirm_data = data;
}

EOLIAN static void
_elm_web_dialog_prompt_hook_set(Eo *obj EINA_UNUSED, Elm_Web_Data *sd, Elm_Web_Dialog_Prompt func, void *data)
{
   sd->hook.prompt = func;
   sd->hook.prompt_data = data;
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
   eo_do(obj, ret = elm_obj_web_url_set(url));
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
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   ewk_view_bg_color_set(wd->resize_obj, r, g, b, a);
}

EOLIAN static void
_elm_web_bg_color_get(Eo *obj, Elm_Web_Data *_pd EINA_UNUSED, int *r, int *g, int *b, int *a)
{
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   return ewk_view_bg_color_get(wd->resize_obj, r, g, b, a);
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
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   ewk_view_page_zoom_set(wd->resize_obj, zoom);
}

EOLIAN static double
_elm_web_zoom_get(Eo *obj EINA_UNUSED, Elm_Web_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, -1);

   return ewk_view_page_zoom_get(wd->resize_obj);
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
_elm_web_inwin_mode_set(Eo *obj EINA_UNUSED, Elm_Web_Data *sd, Eina_Bool value)
{
   sd->inwin_mode = value;
}

EOLIAN static Eina_Bool
_elm_web_inwin_mode_get(Eo *obj EINA_UNUSED, Elm_Web_Data *sd)
{
   return sd->inwin_mode;
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

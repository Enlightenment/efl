#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_web.h"

// TODO:
//  1 - easy to use zoom like elm_photocam API
//  2 - review scrolling to match elm_scroller. Maybe in future use
//  elm_interface_scroller

#ifdef HAVE_ELEMENTARY_WEB
#include <EWebKit.h>

/* Similar to iPhone */
// TODO: switch between iPhone, iPad or Chrome/Safari based on some
// elm setting?

#define ELM_WEB_USER_AGENT                                          \
  "Mozilla/5.0 (iPhone; U; CPU like Mac OS X; en)"                  \
  " AppleWebKit/420+ (KHTML, like Gecko) Version/3.0 Mobile/1A543a" \
  " Safari/419.3 " PACKAGE_NAME "/" PACKAGE_VERSION
#endif

EAPI const char ELM_WEB_SMART_NAME[] = "elm_web";

#ifdef HAVE_ELEMENTARY_WEB
static Ewk_View_Smart_Class _ewk_view_parent_sc =
  EWK_VIEW_SMART_CLASS_INIT_NULL;

#endif

static const Evas_Smart_Cb_Description _elm_web_smart_callbacks[] = {
   { "download,request", "p" },
   { "editorclient,contents,changed", "" },
   { "editorclient,selection,changed", "" },
   { "frame,created", "p" },
   { "icon,received", "" },
   { "inputmethod,changed", "b" },
   { "js,windowobject,clear", "" },
   { "link,hover,in", "p" },
   { "link,hover,out", "" },
   { "load,document,finished", "p" },
   { "load,error", "p" },
   { "load,finished", "p" },
   { "load,newwindow,show", "" },
   { "load,progress", "d" },
   { "load,provisional", "" },
   { "load,started", "" },
   { "menubar,visible,get", "b" },
   { "menubar,visible,set", "b" },
   { "popup,created", "p" },
   { "popup,willdelete", "p" },
   { "ready", "" },
   { "scrollbars,visible,get", "b" },
   { "scrollbars,visible,set", "b" },
   { "statusbar,text,set", "s" },
   { "statusbar,visible,get", "b" },
   { "statusbar,visible,set", "b" },
   { "title,changed", "s" },
   { "toolbars,visible,get", "b" },
   { "toolbars,visible,set", "b" },
   { "tooltip,text,set", "s" },
   { "uri,changed", "s" },
   { "view,resized", "" },
   { "windows,close,request", ""},
   { "zoom,animated,end", "" },
   { NULL, NULL }
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_WEB_SMART_NAME, _elm_web, Elm_Web_Smart_Class, Elm_Widget_Smart_Class,
  elm_widget_smart_class_get, _elm_web_smart_callbacks);

#ifdef HAVE_ELEMENTARY_WEB
static char *
_webkit_theme_find(const Eina_List *list)
{
   const Eina_List *l;
   const char *th;

   EINA_LIST_FOREACH (list, l, th)
     {
        char *path = elm_theme_list_item_path_get(th, NULL);

        if (!path) continue;

        if (edje_file_group_exists(path, "webkit/base")) return path;

        free(path);
     }

   return NULL;
}

#endif

static Eina_Bool
_elm_web_smart_theme(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   Elm_Theme *theme;
   const Eina_List *themes;
   char *view_theme = NULL;

   ELM_WEB_DATA_GET(obj, sd);

   if (!_elm_web_parent_sc->theme(obj)) return EINA_FALSE;

   theme = elm_object_theme_get(obj);
   themes = elm_theme_overlay_list_get(theme);
   view_theme = _webkit_theme_find(themes);
   if (view_theme) goto set;

   themes = elm_theme_list_get(theme);
   view_theme = _webkit_theme_find(themes);
   if (view_theme) goto set;

   themes = elm_theme_extension_list_get(theme);
   view_theme = _webkit_theme_find(themes);

set:
   if (view_theme)
     {
        ewk_view_theme_set(ELM_WIDGET_DATA(sd)->resize_obj, view_theme);
        free(view_theme);
     }
   else
     ewk_view_theme_set
       (ELM_WIDGET_DATA(sd)->resize_obj, WEBKIT_DATADIR "/themes/default.edj");
#endif

   (void)obj;

   return EINA_TRUE;
}

static Eina_Bool
_elm_web_smart_on_focus(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   Evas_Object *top;

   ELM_WEB_DATA_GET(obj, sd);

   top = elm_widget_top_get(obj);

   if (elm_object_focus_get(obj))
     {
        evas_object_focus_set(ELM_WIDGET_DATA(sd)->resize_obj, EINA_TRUE);
        if (top) elm_win_keyboard_mode_set(top, sd->input_method);
     }
   else
     {
        evas_object_focus_set(ELM_WIDGET_DATA(sd)->resize_obj, EINA_FALSE);
        if (top) elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_OFF);
     }
#else
   (void)obj;
#endif

   return EINA_TRUE;
}

static Eina_Bool
_elm_web_smart_event(Evas_Object *obj,
                     Evas_Object *src __UNUSED__,
                     Evas_Callback_Type type,
                     void *event_info)
{
#ifdef HAVE_ELEMENTARY_WEB
   Evas_Event_Key_Down *ev = event_info;

   ELM_WEB_DATA_GET(obj, sd);

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   if ((!strcmp(ev->keyname, "Tab")) && (!sd->tab_propagate))
     {
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else return EINA_FALSE;
#else
   (void)obj;
   (void)type;
   (void)event_info;

   return EINA_FALSE;
#endif
}

#ifdef HAVE_ELEMENTARY_WEB
static Eina_Bool
_view_pan_animator(void *data)
{
   View_Smart_Data *sd = data;
   Evas_Coord x, y, dx, dy;

   evas_pointer_canvas_xy_get(sd->base.base.evas, &x, &y);

   dx = sd->mouse.x - x;
   dy = sd->mouse.y - y;

   if ((dx == 0) && (dy == 0))
     goto end;

   ewk_frame_scroll_add(sd->base.main_frame, dx, dy);

   sd->mouse.x = x;
   sd->mouse.y = y;

end:
   return EINA_TRUE;
}

static void
_view_smart_add(Evas_Object *obj)
{
   View_Smart_Data *sd;

   sd = calloc(1, sizeof(View_Smart_Data));
   evas_object_smart_data_set(obj, sd);

   _ewk_view_parent_sc.sc.add(obj);

   ewk_view_history_enable_set(obj, EINA_TRUE);
   ewk_history_limit_set(ewk_view_history_get(obj), 100);

   // TODO: auto toggle between smooth/nearest during bring-in animations
   //ewk_view_zoom_weak_smooth_scale_set(obj, EINA_TRUE);
}

static void
_view_smart_del(Evas_Object *obj)
{
   View_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);

   if (sd->mouse.pan_anim)
     ecore_animator_del(sd->mouse.pan_anim);

   _ewk_view_parent_sc.sc.del(obj);
}

static Eina_Bool
_view_longpress_timer(void *data)
{
   View_Smart_Data *sd = data;

   sd->mouse.move_count = 0;
   sd->mouse.longpress_timer = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_view_smart_mouse_down(Ewk_View_Smart_Data *esd,
                       const Evas_Event_Mouse_Down *event)
{
   // TODO: mimic elm_scroller and like
   // TODO-minor: offer hook?
   View_Smart_Data *sd = (View_Smart_Data *)esd;

   if (event->button != 1)
     return _ewk_view_parent_sc.mouse_down(esd, event);

   sd->mouse.pan_anim = ecore_animator_add(_view_pan_animator, sd);
   sd->mouse.longpress_timer = ecore_timer_add
       (_elm_config->longpress_timeout, _view_longpress_timer, sd);
   sd->mouse.move_count = 1;
   sd->mouse.x = event->canvas.x;
   sd->mouse.y = event->canvas.y;
   sd->mouse.event = *event;

   return EINA_TRUE;
}

static Eina_Bool
_view_smart_mouse_up(Ewk_View_Smart_Data *esd,
                     const Evas_Event_Mouse_Up *event)
{
   // TODO: mimic elm_scroller and like
   // TODO-minor: offer hook?
   View_Smart_Data *sd = (View_Smart_Data *)esd;

   if (sd->mouse.pan_anim)
     {
        ecore_animator_del(sd->mouse.pan_anim);
        sd->mouse.pan_anim = NULL;

        if (sd->mouse.longpress_timer)
          _ewk_view_parent_sc.mouse_down(esd, &sd->mouse.event);
        else
          return EINA_TRUE;
     }

   if (sd->mouse.longpress_timer)
     {
        ecore_timer_del(sd->mouse.longpress_timer);
        sd->mouse.longpress_timer = NULL;
     }

   sd->mouse.move_count = 0;
   return _ewk_view_parent_sc.mouse_up(esd, event);
}

static Eina_Bool
_view_smart_mouse_move(Ewk_View_Smart_Data *esd,
                       const Evas_Event_Mouse_Move *event)
{
   // TODO: mimic elm_scroller and like
   // TODO-minor: offer hook?
   View_Smart_Data *sd = (View_Smart_Data *)esd;

   sd->mouse.move_count++;

   if (sd->mouse.longpress_timer &&
       (((sd->mouse.x ^ sd->mouse.event.canvas.x) |
         (sd->mouse.y ^ sd->mouse.event.canvas.y)) & (~0x07)))
     {
        ecore_timer_del(sd->mouse.longpress_timer);
        sd->mouse.longpress_timer = NULL;
     }

   if (sd->mouse.pan_anim)
     {
        return EINA_FALSE;
     }

   return _ewk_view_parent_sc.mouse_move(esd, event);
}

static Evas_Object *
_view_smart_window_create(Ewk_View_Smart_Data *vsd,
                          Eina_Bool javascript,
                          const Ewk_Window_Features *window_features)
{
   Evas_Object *new;
   Evas_Object *obj = evas_object_smart_parent_get(vsd->self);

   ELM_WEB_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   if (!sd->hook.window_create) return NULL;

   new = sd->hook.window_create
       (sd->hook.window_create_data, obj, javascript,
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
_bt_close(void *data,
          Evas_Object *obj,
          void *event_info __UNUSED__)
{
   Dialog_Data *d = data;

   *d->response = (obj == d->bt_ok);
   if ((d->type == DIALOG_PROMPT) && (*d->response == EINA_TRUE))
     *d->entry_value = strdup(elm_entry_entry_get(d->entry));

   evas_object_del(d->dialog);
}

static void
_file_sel_done(void *data,
               Evas_Object *obj __UNUSED__,
               void *event_info)
{
   Dialog_Data *d = data;

   if (event_info)
     {
        *d->selected_files = eina_list_append(NULL, strdup(event_info));
        *d->response = EINA_TRUE;
     }
   else *d->response = EINA_FALSE;

   evas_object_del(d->dialog);
   free(d);
}

static Dialog_Data *
_dialog_new(Evas_Object *parent)
{
   Dialog_Data *d;

   ELM_WEB_DATA_GET(parent, sd);

   d = calloc(1, sizeof(Dialog_Data));
   if (!d) return NULL;

   if (!parent || !sd->inwin_mode)
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
        Evas_Object *win = elm_widget_top_get(parent);

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

static Evas_Object *
_run_dialog(Evas_Object *parent,
            enum Dialog_Type type,
            const char *message,
            const char *default_entry_value,
            const char **entry_value,
            Eina_Bool allows_multiple_files __UNUSED__,
            Eina_List *accept_types __UNUSED__,
            Eina_List **selected_filenames,
            Eina_Bool *response)
{
   Evas_Object *lb;
   Evas_Object *obj;
   Dialog_Data *dialog_data;

   EINA_SAFETY_ON_TRUE_RETURN_VAL
     ((type != DIALOG_PROMPT) && (!!default_entry_value), EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL
     ((type != DIALOG_PROMPT) && (!!entry_value), EINA_FALSE);

   obj = evas_object_smart_parent_get(parent);

   ELM_WEB_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   dialog_data = _dialog_new(obj);

   if (type != DIALOG_FILE_SELECTOR)
     {
        lb = elm_label_add(dialog_data->box);
        elm_object_text_set(lb, message);
        elm_box_pack_end(dialog_data->box, lb);
        evas_object_show(lb);
     }

   dialog_data->type = type;
   dialog_data->response = response;
   dialog_data->entry_value = entry_value;
   dialog_data->selected_files = selected_filenames;

   if (type == DIALOG_ALERT)
     {
        dialog_data->bt_ok = elm_button_add(dialog_data->box);
        elm_object_text_set(dialog_data->bt_ok, "Close");
        elm_box_pack_end(dialog_data->box, dialog_data->bt_ok);
        evas_object_size_hint_align_set
          (dialog_data->bt_ok, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add
          (dialog_data->bt_ok, "clicked", _bt_close, dialog_data);
        evas_object_show(dialog_data->bt_ok);
     }
   else if (type == DIALOG_FILE_SELECTOR)
     {
        dialog_data->file_sel = elm_fileselector_add(dialog_data->dialog);
        evas_object_size_hint_weight_set(dialog_data->file_sel,
                                         EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(dialog_data->file_sel, EVAS_HINT_FILL,
                                        EVAS_HINT_FILL);
        elm_box_pack_end(dialog_data->box, dialog_data->file_sel);
        evas_object_show(dialog_data->file_sel);

        elm_fileselector_path_set(dialog_data->file_sel, ".");
        elm_fileselector_is_save_set(dialog_data->file_sel, EINA_FALSE);
        elm_fileselector_folder_only_set(dialog_data->file_sel, EINA_FALSE);
        elm_fileselector_buttons_ok_cancel_set(dialog_data->file_sel,
                                               EINA_TRUE);
        elm_fileselector_expandable_set(dialog_data->file_sel, EINA_FALSE);
        evas_object_smart_callback_add(dialog_data->file_sel, "done",
                                       _file_sel_done, dialog_data);
        // fileselector can't set it's minimum size correctly
        evas_object_size_hint_min_set(dialog_data->file_sel, 300, 400);
     }
   else
     {
        if (type == DIALOG_PROMPT)
          {
             dialog_data->entry = elm_entry_add(dialog_data->box);
             elm_entry_single_line_set(dialog_data->entry, EINA_TRUE);
             elm_entry_scrollable_set(dialog_data->entry, EINA_TRUE);
             elm_entry_entry_set(dialog_data->entry, default_entry_value);
             evas_object_size_hint_align_set
               (dialog_data->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
             evas_object_size_hint_weight_set
               (dialog_data->entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             elm_box_pack_end(dialog_data->box, dialog_data->entry);
             evas_object_show(dialog_data->entry);
          }

        if (type == DIALOG_PROMPT || type == DIALOG_CONFIRM)
          {
             Evas_Object *bx_h = elm_box_add(dialog_data->box);
             elm_box_horizontal_set(bx_h, 1);
             elm_box_pack_end(dialog_data->box, bx_h);
             evas_object_size_hint_weight_set
               (bx_h, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set
               (bx_h, EVAS_HINT_FILL, EVAS_HINT_FILL);
             evas_object_show(bx_h);

             dialog_data->bt_cancel = elm_button_add(bx_h);
             elm_object_text_set(dialog_data->bt_cancel, "Cancel");
             elm_box_pack_end(bx_h, dialog_data->bt_cancel);
             evas_object_size_hint_weight_set
               (dialog_data->bt_cancel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set
               (dialog_data->bt_cancel, EVAS_HINT_FILL, EVAS_HINT_FILL);
             evas_object_smart_callback_add
               (dialog_data->bt_cancel, "clicked", _bt_close, dialog_data);
             evas_object_show(dialog_data->bt_cancel);

             dialog_data->bt_ok = elm_button_add(bx_h);
             elm_object_text_set(dialog_data->bt_ok, "Ok");
             elm_box_pack_end(bx_h, dialog_data->bt_ok);
             evas_object_size_hint_weight_set
               (dialog_data->bt_ok, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set
               (dialog_data->bt_ok, EVAS_HINT_FILL, EVAS_HINT_FILL);
             evas_object_smart_callback_add
               (dialog_data->bt_ok, "clicked", _bt_close, dialog_data);
             evas_object_show(dialog_data->bt_ok);
          }
        else return EINA_FALSE;
     }

   evas_object_show(dialog_data->dialog);

   return dialog_data->dialog;
}

static void
_dialog_del_cb(void *data __UNUSED__,
               Evas *e __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
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

/*
 * called by ewk_view when javascript called alert()
 */
static void
_view_smart_run_javascript_alert(Ewk_View_Smart_Data *esd,
                                 Evas_Object *frame __UNUSED__,
                                 const char *message)
{
   View_Smart_Data *vsd = (View_Smart_Data *)esd;
   Evas_Object *view = vsd->base.self;
   Evas_Object *obj = evas_object_smart_parent_get(view);
   Evas_Object *diag = NULL;
   Eina_Bool response = EINA_FALSE;

   ELM_WEB_DATA_GET_OR_RETURN(obj, sd);

   if (sd->hook.alert)
     diag = sd->hook.alert(sd->hook.alert_data, obj, message);
   else
     diag = _run_dialog(view, DIALOG_ALERT, message, NULL, NULL, EINA_FALSE,
                        NULL, NULL, &response);
   if (diag) _exec_dialog(diag);
}

/*
 * called by ewk_view when javascript called confirm()
 */
static Eina_Bool
_view_smart_run_javascript_confirm(Ewk_View_Smart_Data *esd,
                                   Evas_Object *frame __UNUSED__,
                                   const char *message)
{
   View_Smart_Data *vsd = (View_Smart_Data *)esd;
   Evas_Object *view = vsd->base.self;
   Evas_Object *obj = evas_object_smart_parent_get(view);
   Evas_Object *diag = NULL;
   Eina_Bool response = EINA_FALSE;

   ELM_WEB_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   if (sd->hook.confirm)
     diag = sd->hook.confirm(sd->hook.confirm_data, obj, message, &response);
   else
     diag = _run_dialog(view, DIALOG_CONFIRM, message, NULL, NULL, EINA_FALSE,
                        NULL, NULL, &response);
   if (diag) _exec_dialog(diag);
   return response;
}

/*
 * called by ewk_view when javascript called confirm()
 */
static Eina_Bool
_view_smart_run_javascript_prompt(Ewk_View_Smart_Data *esd,
                                  Evas_Object *frame __UNUSED__,
                                  const char *message,
                                  const char *default_value,
                                  const char **value)
{
   View_Smart_Data *vsd = (View_Smart_Data *)esd;
   Evas_Object *view = vsd->base.self;
   Evas_Object *obj = evas_object_smart_parent_get(view);
   Evas_Object *diag = NULL;
   Eina_Bool response = EINA_FALSE;

   ELM_WEB_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   if (sd->hook.prompt)
     diag = sd->hook.prompt(sd->hook.prompt_data, obj, message, default_value,
                            value, &response);
   else
     diag = _run_dialog(view, DIALOG_PROMPT, message, default_value, value,
                        EINA_FALSE, NULL, NULL, &response);
   if (diag) _exec_dialog(diag);
   if (!response)
     *value = NULL;

   return EINA_TRUE;
}

static Eina_Bool
_view_smart_run_open_panel(Ewk_View_Smart_Data *esd,
                           Evas_Object *frame __UNUSED__,
                           Eina_Bool allows_multiple_files,
                           Eina_List *accept_types,
                           Eina_List **selected_filenames)
{
   View_Smart_Data *vsd = (View_Smart_Data *)esd;
   Evas_Object *view = vsd->base.self;
   Evas_Object *obj = evas_object_smart_parent_get(view);
   Evas_Object *diag = NULL;
   Eina_Bool response = EINA_FALSE;

   ELM_WEB_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   if (sd->hook.file_selector)
     diag = sd->hook.file_selector(sd->hook.file_selector_data, obj,
                                   allows_multiple_files, accept_types,
                                   selected_filenames, &response);
   else
     diag = _run_dialog(view, DIALOG_FILE_SELECTOR, NULL, NULL, NULL,
                        allows_multiple_files, accept_types, selected_filenames,
                        &response);
   if (diag) _exec_dialog(diag);

   return response;
}

static void
_view_smart_add_console_message(Ewk_View_Smart_Data *esd,
                                const char *message,
                                unsigned int line_number,
                                const char *source_id)
{
   Evas_Object *obj = evas_object_smart_parent_get(esd->self);

   ELM_WEB_DATA_GET_OR_RETURN(obj, sd);

   if (sd->hook.console_message)
     sd->hook.console_message(sd->hook.console_message_data, obj, message,
                              line_number, source_id);
}

static Eina_Bool
_view_smart_focus_can_cycle(Ewk_View_Smart_Data *sd,
                            Ewk_Focus_Direction direction)
{
   Evas_Object *obj = evas_object_smart_parent_get(sd->self);

   ELM_WEB_CHECK(obj) EINA_FALSE;

   Elm_Focus_Direction dir;

   switch (direction)
     {
      case EWK_FOCUS_DIRECTION_FORWARD:
        dir = ELM_FOCUS_NEXT;
        break;

      case EWK_FOCUS_DIRECTION_BACKWARD:
        dir = ELM_FOCUS_PREVIOUS;
        break;

      default:
        return EINA_FALSE;
     }

   elm_widget_focus_cycle(elm_widget_parent_get(obj), dir);

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

#ifndef TILED_BACKING_STORE
        ewk_view_single_smart_set(&api);
#else
        ewk_view_tiled_smart_set(&api);
#endif

        _ewk_view_parent_sc = api;

        // TODO: check every api method and provide overrides with hooks!
        // TODO: hooks should provide extension points
        // TODO: extension should have some kind of "default implementation",
        // TODO: that can be replaced or controlled by hooks.
        // TODO: ie: run_javascript_alert() should present an elm_win
        // TODO: by default, but user could override it to show as inwin.
        api.sc.add = _view_smart_add;
        api.sc.del = _view_smart_del;
        //api.sc.calculate = _view_smart_calculate;
        api.mouse_down = _view_smart_mouse_down;
        api.mouse_up = _view_smart_mouse_up;
        api.mouse_move = _view_smart_mouse_move;
        api.add_console_message = _view_smart_add_console_message;
        api.window_create = _view_smart_window_create;
        api.window_close = _view_smart_window_close;
        api.run_javascript_alert = _view_smart_run_javascript_alert;
        api.run_javascript_confirm = _view_smart_run_javascript_confirm;
        api.run_javascript_prompt = _view_smart_run_javascript_prompt;
        api.run_open_panel = _view_smart_run_open_panel;
        api.focus_can_cycle = _view_smart_focus_can_cycle;

        smart = evas_smart_class_new(&api.sc);
        if (!smart)
          {
             CRITICAL("Could not create smart class");
             return NULL;
          }
     }

   view = evas_object_smart_add(canvas, smart);
   if (!view)
     {
        ERR("Could not create smart object object for view");
        return NULL;
     }

   return view;
}

static void
_ewk_view_inputmethod_change_cb(void *data,
                                Evas_Object *obj __UNUSED__,
                                void *event_info)
{
   Elm_Web_Smart_Data *sd = data;
   Evas_Object *top = elm_widget_top_get(ELM_WIDGET_DATA(sd)->obj);
   if (!top) return;

   if (event_info)
     sd->input_method = ELM_WIN_KEYBOARD_ON;
   else
     sd->input_method = ELM_WIN_KEYBOARD_OFF;
   elm_win_keyboard_mode_set(top, sd->input_method);
}

static void
_ewk_view_load_started_cb(void *data,
                          Evas_Object *obj,
                          void *event_info __UNUSED__)
{
   _ewk_view_inputmethod_change_cb(data, obj, (void *)(long)EINA_FALSE);
}

static void
_ewk_view_load_finished_cb(void *data,
                           Evas_Object *obj __UNUSED__,
                           void *event_info)
{
   Elm_Web_Smart_Data *sd = data;

   if (event_info) return;

   if (sd->zoom.mode != ELM_WEB_ZOOM_MODE_MANUAL)
     {
        float tz = sd->zoom.current;
        sd->zoom.current = 0.0;
        elm_web_zoom_set(ELM_WIDGET_DATA(sd)->obj, tz);
     }
}

static void
_ewk_view_viewport_changed_cb(void *data,
                              Evas_Object *obj,
                              void *event_info __UNUSED__)
{
   Elm_Web_Smart_Data *sd = data;

   if (sd->zoom.mode != ELM_WEB_ZOOM_MODE_MANUAL)
     {
        ewk_view_zoom_set(obj, 1.0, 0, 0);
        sd->zoom.no_anim = EINA_TRUE;
     }
}

static Eina_Bool
_restore_zoom_mode_timer_cb(void *data)
{
   Elm_Web_Smart_Data *sd = data;
   float tz = sd->zoom.current;

   sd->zoom.timer = NULL;
   sd->zoom.current = 0.0;
   sd->zoom.no_anim = EINA_TRUE;
   elm_web_zoom_set(ELM_WIDGET_DATA(sd)->obj, tz);

   return EINA_FALSE;
}

static Eina_Bool
_reset_zoom_timer_cb(void *data)
{
   Elm_Web_Smart_Data *sd = data;

   sd->zoom.timer = ecore_timer_add(0.0, _restore_zoom_mode_timer_cb, sd);
   ewk_view_zoom_set(ELM_WIDGET_DATA(sd)->resize_obj, 1.0, 0, 0);

   return EINA_FALSE;
}

static void
_ewk_view_resized_cb(void *data,
                     Evas_Object *obj __UNUSED__,
                     void *event_info __UNUSED__)
{
   Elm_Web_Smart_Data *sd = data;

   if (!(sd->zoom.mode != ELM_WEB_ZOOM_MODE_MANUAL))
     return;

   if (sd->zoom.timer)
     ecore_timer_del(sd->zoom.timer);
   sd->zoom.timer = ecore_timer_add(0.5, _reset_zoom_timer_cb, sd);
}

static void
_popup_del_job(void *data)
{
   evas_object_del(data);
}

static void
_popup_will_delete(void *data,
                   Evas_Object *obj,
                   void *event_info __UNUSED__)
{
   ecore_job_add(_popup_del_job, data);
   evas_object_smart_callback_del(obj, "popup,willdelete", _popup_will_delete);
}

static void
_popup_item_selected(void *data,
                     Evas_Object *obj,
                     void *event_info __UNUSED__)
{
   Elm_Object_Item *list_it = elm_list_selected_item_get(obj);
   const Eina_List *itr, *list = elm_list_items_get(obj);
   Evas_Object *view = data;
   int i = 0;
   void *d;

   EINA_LIST_FOREACH (list, itr, d)
     {
        if (d == list_it)
          break;

        i++;
     }

   ewk_view_popup_selected_set(view, i);
   ewk_view_popup_destroy(view);
}

static void
_popup_dismiss_cb(void *data,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   ewk_view_popup_destroy(data);
}

static void
_ewk_view_popup_create_cb(void *data,
                          Evas_Object *obj,
                          void *event_info)
{
   Elm_Web_Smart_Data *sd = data;
   Evas_Object *notify, *list;
   Ewk_Menu *m = event_info;
   Ewk_Menu_Item *it;
   Elm_Web_Menu m2;
   Eina_List *itr;

   m2.items = m->items;
   m2.x = m->x;
   m2.y = m->y;
   m2.width = m->width;
   m2.height = m->height;
   m2.handled = EINA_FALSE;
   evas_object_smart_callback_call
     (ELM_WIDGET_DATA(sd)->obj, "popup,create", &m2);
   if (m2.handled) return;

   notify = elm_notify_add(obj);
   elm_notify_allow_events_set(notify, EINA_FALSE);
   elm_notify_orient_set(notify, ELM_NOTIFY_ORIENT_BOTTOM);

   list = elm_list_add(obj);
   elm_list_select_mode_set(data, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_scroller_bounce_set(list, EINA_FALSE, EINA_FALSE);
   elm_list_mode_set(list, ELM_LIST_EXPAND);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(notify, list);
   evas_object_show(list);

   EINA_LIST_FOREACH (m->items, itr, it)
     elm_list_item_append(list, it->text, NULL, NULL, _popup_item_selected,
                          obj);
   elm_list_go(list);

   evas_object_show(notify);

   evas_object_smart_callback_add
     (obj, "popup,willdelete", _popup_will_delete, notify);
   evas_object_smart_callback_add
     (notify, "block,clicked", _popup_dismiss_cb, obj);
}

static void
_view_smart_callback_proxy_free_cb(void *data,
                                   Evas *e __UNUSED__,
                                   Evas_Object *obj __UNUSED__,
                                   void *event_info __UNUSED__)
{
   free(data);
}

static void
_view_smart_callback_proxy_cb(void *data,
                              Evas_Object *obj __UNUSED__,
                              void *event_info)
{
   Elm_Web_Callback_Proxy_Context *ctxt = data;

   evas_object_smart_callback_call(ctxt->obj, ctxt->name, event_info);
}

static void
_view_smart_callback_proxy(Evas_Object *view,
                           Evas_Object *parent)
{
   const Evas_Smart_Cb_Description **cls_descs, **inst_descs;
   unsigned int cls_count, inst_count, total;
   Elm_Web_Callback_Proxy_Context *ctxt;

   evas_object_smart_callbacks_descriptions_get
     (view, &cls_descs, &cls_count, &inst_descs, &inst_count);
   total = cls_count + inst_count;
   if (!total) return;

   ctxt = malloc(sizeof(Elm_Web_Callback_Proxy_Context) * total);
   if (!ctxt) return;

   evas_object_event_callback_add
     (view, EVAS_CALLBACK_FREE, _view_smart_callback_proxy_free_cb, ctxt);

   for (; cls_count > 0; cls_count--, cls_descs++, ctxt++)
     {
        const Evas_Smart_Cb_Description *d = *cls_descs;
        if (!strcmp(d->name, "popup,create"))
          continue;
        ctxt->name = d->name;
        ctxt->obj = parent;
        evas_object_smart_callback_add
          (view, d->name, _view_smart_callback_proxy_cb, ctxt);
     }

   for (; inst_count > 0; inst_count--, inst_descs++, ctxt++)
     {
        const Evas_Smart_Cb_Description *d = *inst_descs;
        ctxt->name = d->name;
        ctxt->obj = parent;
        evas_object_smart_callback_add
          (view, d->name, _view_smart_callback_proxy_cb, ctxt);
     }
}

static Eina_Bool
_bring_in_anim_cb(void *data,
                  double pos)
{
   Elm_Web_Smart_Data *sd = data;
   Evas_Object *frame =
     ewk_view_frame_main_get(ELM_WIDGET_DATA(sd)->resize_obj);
   int sx, sy, rx, ry;

   sx = sd->bring_in.start.x;
   sy = sd->bring_in.start.y;
   rx = (sd->bring_in.end.x - sx) * pos;
   ry = (sd->bring_in.end.y - sy) * pos;

   ewk_frame_scroll_set(frame, rx + sx, ry + sy);

   if (pos == 1.0)
     {
        sd->bring_in.end.x = sd->bring_in.end.y = sd->bring_in.start.x =
              sd->bring_in.start.y = 0;
        sd->bring_in.animator = NULL;
     }

   return EINA_TRUE;
}

#endif

#ifdef HAVE_ELEMENTARY_WEB
static int _elm_need_web = 0;
#endif

void
_elm_unneed_web(void)
{
#ifdef HAVE_ELEMENTARY_WEB
   if (--_elm_need_web) return;

   _elm_need_web = 0;
   ewk_shutdown();
#endif
}

EAPI Eina_Bool
elm_need_web(void)
{
#ifdef HAVE_ELEMENTARY_WEB
   if (_elm_need_web++) return EINA_TRUE;
   ewk_init();
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

static void
_elm_web_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Web_Smart_Data);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA(priv)->resize_obj = _view_add(obj);
#else
   ELM_WIDGET_DATA(priv)->resize_obj = elm_label_add(obj);
   elm_object_text_set
     (ELM_WIDGET_DATA(priv)->resize_obj, "WebKit not supported!");
   evas_object_show(ELM_WIDGET_DATA(priv)->resize_obj);
#endif

   _elm_web_parent_sc->base.add(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ewk_view_setting_user_agent_set
     (ELM_WIDGET_DATA(priv)->resize_obj, ELM_WEB_USER_AGENT);

   priv->input_method = ELM_WIN_KEYBOARD_OFF;
   evas_object_smart_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "inputmethod,changed",
     _ewk_view_inputmethod_change_cb, priv);
   evas_object_smart_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "load,started",
     _ewk_view_load_started_cb, priv);
   evas_object_smart_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "popup,create",
     _ewk_view_popup_create_cb, priv);
   evas_object_smart_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "load,finished",
     _ewk_view_load_finished_cb, priv);
   evas_object_smart_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "viewport,changed",
     _ewk_view_viewport_changed_cb, priv);
   evas_object_smart_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "view,resized",
     _ewk_view_resized_cb, priv);

   priv->tab_propagate = EINA_FALSE;
   priv->inwin_mode = _elm_config->inwin_dialogs_enable;
   priv->zoom.min =
     ewk_view_zoom_range_min_get(ELM_WIDGET_DATA(priv)->resize_obj);
   priv->zoom.max =
     ewk_view_zoom_range_max_get(ELM_WIDGET_DATA(priv)->resize_obj);
   priv->zoom.current = 1.0;

   _view_smart_callback_proxy(ELM_WIDGET_DATA(priv)->resize_obj, obj);
   _elm_web_smart_theme(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);
#endif
}

static void
_elm_web_smart_del(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   if (sd->zoom.timer)
     {
        ecore_timer_del(sd->zoom.timer);
        sd->zoom.timer = NULL;
     }

   if (sd->bring_in.animator)
     {
        ecore_animator_del(sd->bring_in.animator);
        sd->bring_in.animator = NULL;
     }
#endif

   _elm_web_parent_sc->base.del(obj); /* handles freeing sd */
}

static void
_elm_web_smart_set_user(Elm_Web_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_web_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_web_smart_del;

   ELM_WIDGET_CLASS(sc)->on_focus = _elm_web_smart_on_focus;
   ELM_WIDGET_CLASS(sc)->theme = _elm_web_smart_theme;
   ELM_WIDGET_CLASS(sc)->event = _elm_web_smart_event;
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

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ELM_WIDGET_DATA(sd)->resize_obj;
#else
   ERR("Elementary not compiled with EWebKit support.");
   return NULL;
#endif
}

EAPI void
elm_web_window_create_hook_set(Evas_Object *obj,
                               Elm_Web_Window_Open func,
                               void *data)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   sd->hook.window_create = func;
   sd->hook.window_create_data = data;
#else
   (void)func;
   (void)data;
#endif
}

EAPI void
elm_web_dialog_alert_hook_set(Evas_Object *obj,
                              Elm_Web_Dialog_Alert func,
                              void *data)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   sd->hook.alert = func;
   sd->hook.alert_data = data;
#else
   (void)func;
   (void)data;
#endif
}

EAPI void
elm_web_dialog_confirm_hook_set(Evas_Object *obj,
                                Elm_Web_Dialog_Confirm func,
                                void *data)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   sd->hook.confirm = func;
   sd->hook.confirm_data = data;
#else
   (void)func;
   (void)data;
#endif
}

EAPI void
elm_web_dialog_prompt_hook_set(Evas_Object *obj,
                               Elm_Web_Dialog_Prompt func,
                               void *data)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   sd->hook.prompt = func;
   sd->hook.prompt_data = data;
#else
   (void)func;
   (void)data;
#endif
}

EAPI void
elm_web_dialog_file_selector_hook_set(Evas_Object *obj,
                                      Elm_Web_Dialog_File_Selector func,
                                      void *data)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   sd->hook.file_selector = func;
   sd->hook.file_selector_data = data;
#else
   (void)func;
   (void)data;
#endif
}

EAPI void
elm_web_console_message_hook_set(Evas_Object *obj,
                                 Elm_Web_Console_Message func,
                                 void *data)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   sd->hook.console_message = func;
   sd->hook.console_message_data = data;
#else
   (void)func;
   (void)data;
#endif
}

EAPI void
elm_web_useragent_set(Evas_Object *obj,
                      const char *user_agent)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   ewk_view_setting_user_agent_set
     (ELM_WIDGET_DATA(sd)->resize_obj, user_agent);
#else
   (void)user_agent;
#endif
}

EAPI const char *
elm_web_useragent_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) NULL;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_setting_user_agent_get(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return NULL;
#endif
}

EAPI Eina_Bool
elm_web_tab_propagate_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return sd->tab_propagate;
#else
   return EINA_FALSE;
#endif
}

EAPI void
elm_web_tab_propagate_set(Evas_Object *obj,
                          Eina_Bool propagate)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   sd->tab_propagate = propagate;
#else
   (void)propagate;
#endif
}

EAPI Eina_Bool
elm_web_uri_set(Evas_Object *obj,
                const char *uri)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_uri_set(ELM_WIDGET_DATA(sd)->resize_obj, uri);
#else
   (void)uri;
   return EINA_FALSE;
#endif
}

EAPI const char *
elm_web_uri_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) NULL;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_uri_get(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return NULL;
#endif
}

EAPI const char *
elm_web_title_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) NULL;

#ifdef HAVE_ELEMENTARY_WEB
   const Ewk_Text_With_Direction *txt;

   ELM_WEB_DATA_GET(obj, sd);

   txt = ewk_view_title_get(ELM_WIDGET_DATA(sd)->resize_obj);
   if (txt) return txt->string;

   return NULL;
#else
   return NULL;
#endif
}

EAPI void
elm_web_bg_color_set(Evas_Object *obj,
                     int r,
                     int g,
                     int b,
                     int a)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   ewk_view_bg_color_set(ELM_WIDGET_DATA(sd)->resize_obj, r, g, b, a);
#else
   (void)r;
   (void)g;
   (void)b;
   (void)a;
#endif
}

EAPI void
elm_web_bg_color_get(const Evas_Object *obj,
                     int *r,
                     int *g,
                     int *b,
                     int *a)
{
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;

   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_bg_color_get(ELM_WIDGET_DATA(sd)->resize_obj, r, g, b, a);
#endif
}

EAPI const char *
elm_web_selection_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) NULL;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_selection_get(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return NULL;
#endif
}

EAPI void
elm_web_popup_selected_set(Evas_Object *obj,
                           int idx)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);
   ewk_view_popup_selected_set(ELM_WIDGET_DATA(sd)->resize_obj, idx);
#else
   (void)idx;
#endif
}

EAPI Eina_Bool
elm_web_popup_destroy(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);
   return ewk_view_popup_destroy(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_text_search(const Evas_Object *obj,
                    const char *string,
                    Eina_Bool case_sensitive,
                    Eina_Bool forward,
                    Eina_Bool wrap)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_text_search
            (ELM_WIDGET_DATA(sd)->resize_obj, string,
            case_sensitive, forward, wrap);
#else
   (void)string;
   (void)case_sensitive;
   (void)forward;
   (void)wrap;
   return EINA_FALSE;
#endif
}

EAPI unsigned int
elm_web_text_matches_mark(Evas_Object *obj,
                          const char *string,
                          Eina_Bool case_sensitive,
                          Eina_Bool highlight,
                          unsigned int limit)
{
   ELM_WEB_CHECK(obj) 0;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);
   if (!sd) return 0;
   return ewk_view_text_matches_mark
            (ELM_WIDGET_DATA(sd)->resize_obj, string,
            case_sensitive, highlight, limit);
#else
   (void)string;
   (void)case_sensitive;
   (void)highlight;
   (void)limit;
   return 0;
#endif
}

EAPI Eina_Bool
elm_web_text_matches_unmark_all(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_text_matches_unmark_all(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_text_matches_highlight_set(Evas_Object *obj,
                                   Eina_Bool highlight)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_text_matches_highlight_set
            (ELM_WIDGET_DATA(sd)->resize_obj, highlight);
#else
   (void)highlight;
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_text_matches_highlight_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_text_matches_highlight_get(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return EINA_FALSE;
#endif
}

EAPI double
elm_web_load_progress_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) - 1.0;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_load_progress_get(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_stop(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_stop(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_reload(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_reload(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_reload_full(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_reload_full(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_back(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_back(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_forward(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_forward(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_navigate(Evas_Object *obj,
                 int steps)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_navigate(ELM_WIDGET_DATA(sd)->resize_obj, steps);
#else
   return EINA_FALSE;
   (void)steps;
#endif
}

EINA_DEPRECATED EAPI Eina_Bool
elm_web_back_possible(Evas_Object *obj)
{
   return elm_web_back_possible_get(obj);
}

EAPI Eina_Bool
elm_web_back_possible_get(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_back_possible(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_forward_possible_get(Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_forward_possible(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_navigate_possible_get(Evas_Object *obj,
                              int steps)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_navigate_possible(ELM_WIDGET_DATA(sd)->resize_obj, steps);
#else
   return EINA_FALSE;
   (void)steps;
#endif
}

EAPI Eina_Bool
elm_web_history_enabled_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return ewk_view_history_enable_get(ELM_WIDGET_DATA(sd)->resize_obj);
#else
   return EINA_FALSE;
#endif
}

EAPI void
elm_web_history_enabled_set(Evas_Object *obj,
                            Eina_Bool enable)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   ewk_view_history_enable_set(ELM_WIDGET_DATA(sd)->resize_obj, enable);
#else
   (void)enable;
#endif
}

//EAPI Ewk_History *ewk_view_history_get(const Evas_Object *obj); // TODO:

EAPI void
elm_web_zoom_set(Evas_Object *obj,
                 double zoom)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   int vw, vh, cx, cy;
   float z = 1.0;

   evas_object_geometry_get
     (ELM_WIDGET_DATA(sd)->resize_obj, NULL, NULL, &vw, &vh);
   cx = vw / 2;
   cy = vh / 2;
   if (zoom > sd->zoom.max) zoom = sd->zoom.max;
   else if (zoom < sd->zoom.min)
     zoom = sd->zoom.min;
   if (zoom == sd->zoom.current) return;

   sd->zoom.current = zoom;
   if (sd->zoom.mode == ELM_WEB_ZOOM_MODE_MANUAL) z = zoom;
   else if (sd->zoom.mode == ELM_WEB_ZOOM_MODE_AUTO_FIT)
     {
        Evas_Object *frame =
          ewk_view_frame_main_get(ELM_WIDGET_DATA(sd)->resize_obj);
        Evas_Coord fw, fh, pw, ph;

        if (!ewk_frame_contents_size_get(frame, &fw, &fh)) return;
        z = ewk_frame_page_zoom_get(frame);
        fw /= z;
        fh /= z;
        if ((fw > 0) && (fh > 0))
          {
             ph = (fh * vw) / fw;
             if (ph > vh)
               {
                  pw = (fw * vh) / fh;
                  ph = vh;
               }
             else
               pw = vw;
             if (fw > fh)
               z = (float)pw / fw;
             else
               z = (float)ph / fh;
          }
     }
   else if (sd->zoom.mode == ELM_WEB_ZOOM_MODE_AUTO_FILL)
     {
        Evas_Object *frame =
          ewk_view_frame_main_get(ELM_WIDGET_DATA(sd)->resize_obj);
        Evas_Coord fw, fh, pw, ph;

        if (!ewk_frame_contents_size_get(frame, &fw, &fh)) return;
        z = ewk_frame_page_zoom_get(frame);
        fw /= z;
        fh /= z;
        if ((fw > 0) && (fh > 0))
          {
             ph = (fh * vw) / fw;
             if (ph < vh)
               {
                  pw = (fw * vh) / fh;
                  ph = vh;
               }
             else
               pw = vw;
             if (fw > fh)
               z = (float)pw / fw;
             else
               z = (float)ph / fh;
          }
     }
   if (sd->zoom.no_anim)
     ewk_view_zoom_set(ELM_WIDGET_DATA(sd)->resize_obj, z, cx, cy);
   else
     ewk_view_zoom_animated_set
       (ELM_WIDGET_DATA(sd)->resize_obj, z,
       _elm_config->zoom_friction, cx, cy);
   sd->zoom.no_anim = EINA_FALSE;
#else
   (void)zoom;
#endif
}

EAPI double
elm_web_zoom_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) - 1.0;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return sd->zoom.current;
#else
   return -1.0;
#endif
}

EAPI void
elm_web_zoom_mode_set(Evas_Object *obj,
                      Elm_Web_Zoom_Mode mode)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   float tz;

   if (mode >= ELM_WEB_ZOOM_MODE_LAST)
     return;
   if (mode == sd->zoom.mode)
     return;

   sd->zoom.mode = mode;
   tz = sd->zoom.current;
   sd->zoom.current = 0.0;
   elm_web_zoom_set(obj, tz);
#else
   (void)mode;
#endif
}

EAPI Elm_Web_Zoom_Mode
elm_web_zoom_mode_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) ELM_WEB_ZOOM_MODE_LAST;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return sd->zoom.mode;
#else
   return ELM_WEB_ZOOM_MODE_LAST;
#endif
}

EAPI void
elm_web_region_show(Evas_Object *obj,
                    int x,
                    int y,
                    int w __UNUSED__,
                    int h __UNUSED__)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   Evas_Object *frame =
     ewk_view_frame_main_get(ELM_WIDGET_DATA(sd)->resize_obj);
   int fw, fh, zw, zh, rx, ry;
   float zoom;

   ewk_frame_contents_size_get(frame, &fw, &fh);
   zoom = ewk_frame_page_zoom_get(frame);
   zw = fw / zoom;
   zh = fh / zoom;
   rx = (x * fw) / zw;
   ry = (y * fh) / zh;
   if (sd->bring_in.animator)
     {
        ecore_animator_del(sd->bring_in.animator);
        sd->bring_in.animator = NULL;
     }
   ewk_frame_scroll_set(frame, rx, ry);
#else
   (void)x;
   (void)y;
#endif
}

EAPI void
elm_web_region_bring_in(Evas_Object *obj,
                        int x,
                        int y,
                        int w __UNUSED__,
                        int h __UNUSED__)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   Evas_Object *frame =
     ewk_view_frame_main_get(ELM_WIDGET_DATA(sd)->resize_obj);
   int fw, fh, zw, zh, rx, ry, sx, sy;
   float zoom;

   ewk_frame_contents_size_get(frame, &fw, &fh);
   ewk_frame_scroll_pos_get(frame, &sx, &sy);
   zoom = ewk_frame_page_zoom_get(frame);
   zw = fw / zoom;
   zh = fh / zoom;
   rx = (x * fw) / zw;
   ry = (y * fh) / zh;
   if ((sd->bring_in.end.x == rx) && (sd->bring_in.end.y == ry))
     return;
   sd->bring_in.start.x = sx;
   sd->bring_in.start.y = sy;
   sd->bring_in.end.x = rx;
   sd->bring_in.end.y = ry;
   if (sd->bring_in.animator)
     ecore_animator_del(sd->bring_in.animator);
   sd->bring_in.animator = ecore_animator_timeline_add(
       _elm_config->bring_in_scroll_friction, _bring_in_anim_cb, sd);
#else
   (void)x;
   (void)y;
#endif
}

EAPI void
elm_web_inwin_mode_set(Evas_Object *obj,
                       Eina_Bool value)
{
   ELM_WEB_CHECK(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   sd->inwin_mode = value;
#else
   (void)value;
#endif
}

EAPI Eina_Bool
elm_web_inwin_mode_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_DATA_GET(obj, sd);

   return sd->inwin_mode;
#else
   return EINA_FALSE;
#endif
}

EAPI void
elm_web_window_features_ref(Elm_Web_Window_Features *wf)
{
#ifdef HAVE_ELEMENTARY_WEB
   ewk_window_features_ref((Ewk_Window_Features *)wf);
#else
   (void)wf;
#endif
}

EAPI void
elm_web_window_features_unref(Elm_Web_Window_Features *wf)
{
#ifdef HAVE_ELEMENTARY_WEB
   ewk_window_features_unref((Ewk_Window_Features *)wf);
#else
   (void)wf;
#endif
}

EAPI Eina_Bool
elm_web_window_features_property_get(const Elm_Web_Window_Features *wf,
                                     Elm_Web_Window_Feature_Flag flag)
{
#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool toolbar_visible, statusbar_visible;
   Eina_Bool scrollbars_visible, menubar_visible;
   Eina_Bool locationbar_visible, fullscreen;

   ewk_window_features_bool_property_get
     ((const Ewk_Window_Features *)wf, &toolbar_visible, &statusbar_visible,
     &scrollbars_visible, &menubar_visible, &locationbar_visible,
     &fullscreen);

   //TODO : Improve ewk API usage to get one value.
   switch (flag)
     {
      case ELM_WEB_WINDOW_FEATURE_TOOLBAR:
        return toolbar_visible;
        break;

      case ELM_WEB_WINDOW_FEATURE_STATUSBAR:
        return statusbar_visible;
        break;

      case ELM_WEB_WINDOW_FEATURE_SCROLLBARS:
        return scrollbars_visible;
        break;

      case ELM_WEB_WINDOW_FEATURE_MENUBAR:
        return menubar_visible;
        break;

      case ELM_WEB_WINDOW_FEATURE_LOCATIONBAR:
        return locationbar_visible;
        break;

      case ELM_WEB_WINDOW_FEATURE_FULLSCREEN:
        return fullscreen;
        break;
     }
#else

   if (!wf) return EINA_FALSE;
   if (flag == ELM_WEB_WINDOW_FEATURE_TOOLBAR)
     {
        //TODO Handle unused variable warnings when
        //ewk is not installed.
     }

   /* What to do with these ?
      (void)wf;
      (void)toolbar_visible;
      (void)statusbar_visible;
      (void)scrollbars_visible;
      (void)menubar_visible;
      (void)locationbar_visible;
      (void)fullscreen;
    */

#endif
   return EINA_FALSE;
}

EAPI void
elm_web_window_features_region_get(const Elm_Web_Window_Features *wf,
                                   Evas_Coord *x,
                                   Evas_Coord *y,
                                   Evas_Coord *w,
                                   Evas_Coord *h)
{
#ifdef HAVE_ELEMENTARY_WEB
   ewk_window_features_int_property_get
     ((const Ewk_Window_Features *)wf, x, y, w, h);
#else

   if (!wf || !x || !y || !w || !h) return;
   /* What to do with these ?
      (void)wf;
      (void)x;
      (void)y;
      (void)w;
      (void)h;
    */

#endif
}

// TODO: use all ewk_view_zoom stuff to implement bring-in and
// animated zoom like elm_photocam. Should be simple to use, must not
// expose every single bit to users!

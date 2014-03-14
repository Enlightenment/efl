#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_web.h"

// TODO:
//  1 - easy to use zoom like elm_photocam API
//  2 - review scrolling to match elm_scroller. Maybe in future use
//  elm_interface_scroller

#if !defined(HAVE_ELEMENTARY_WEB) || !defined(USE_WEBKIT2)

EAPI Eo_Op ELM_OBJ_WEB_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_WEB_CLASS

#define MY_CLASS_NAME "Elm_Web"
#define MY_CLASS_NAME_LEGACY "elm_web"

#ifdef HAVE_ELEMENTARY_WEB
#include <EWebKit.h>

/* Similar to iPhone */
// TODO: switch between iPhone, iPad or Chrome/Safari based on some
// elm setting?

#define ELM_WEB_USER_AGENT                                          \
  "Mozilla/5.0 (iPhone; U; CPU like Mac OS X; en)"                  \
  " AppleWebKit/420+ (KHTML, like Gecko) Version/3.0 Mobile/1A543a" \
  " Safari/419.3 " PACKAGE_NAME "/" PACKAGE_VERSION

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
   { "url,changed", "s" },
   { "view,resized", "" },
   { "windows,close,request", ""},
   { "zoom,animated,end", "" },
   { SIG_WIDGET_FOCUSED, ""}, /**< handled by elm_widget */
   { SIG_WIDGET_UNFOCUSED, ""}, /**< handled by elm_widget */
   { NULL, NULL }
};

#ifdef HAVE_ELEMENTARY_WEB
static char *
_webkit_theme_find(const Eina_List *list)
{
   const Eina_List *l;
   const char *th;

   EINA_LIST_FOREACH(list, l, th)
     {
        char *path = elm_theme_list_item_path_get(th, NULL);

        if (!path) continue;

        if (edje_file_group_exists(path, "webkit/base")) return path;

        free(path);
     }

   return NULL;
}

#endif

static void
_elm_web_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool int_ret = EINA_FALSE;
   Elm_Theme *theme;
   const Eina_List *themes;
   char *view_theme = NULL;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

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
        ewk_view_theme_set(wd->resize_obj, view_theme);
        free(view_theme);
     }
   else
     ewk_view_theme_set
       (wd->resize_obj, WEBKIT_DATADIR "/themes/default.edj");
#else
   (void)obj;
#endif

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_web_smart_on_focus(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_TRUE;
#ifdef HAVE_ELEMENTARY_WEB
   Evas_Object *top;

   Elm_Web_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_on_focus(&int_ret));
   if (!int_ret) return;

   top = elm_widget_top_get(obj);

   if (elm_object_focus_get(obj))
     {
        evas_object_focus_set(wd->resize_obj, EINA_TRUE);
        if (top) elm_win_keyboard_mode_set(top, sd->input_method);
     }
   else
     {
        evas_object_focus_set(wd->resize_obj, EINA_FALSE);
        if (top) elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_OFF);
     }
#else
   (void)obj;
   (void)_pd;
   (void)list;
#endif
}

static void
_elm_web_smart_event(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *src = va_arg(*list, Evas_Object *);
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   Evas_Event_Key_Down *ev = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   if (ret) *ret = EINA_FALSE;
   (void) src;

#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

   if (type != EVAS_CALLBACK_KEY_DOWN) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (elm_widget_disabled_get(obj)) return;

   if ((!strcmp(ev->key, "Tab")) && (!sd->tab_propagate))
     {
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        if (ret) *ret = EINA_TRUE;
     }
#else
   (void)obj;
   (void)_pd;
   (void)type;
   (void)ev;
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
   return ECORE_CALLBACK_RENEW;
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
        ELM_SAFE_FREE(sd->mouse.pan_anim, ecore_animator_del);

        if (sd->mouse.longpress_timer)
          _ewk_view_parent_sc.mouse_down(esd, &sd->mouse.event);
        else
          return EINA_TRUE;
     }

   ELM_SAFE_FREE(sd->mouse.longpress_timer, ecore_timer_del);

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

   if (((sd->mouse.x ^ sd->mouse.event.canvas.x) |
        (sd->mouse.y ^ sd->mouse.event.canvas.y)) & (~0x07))
     {
        ELM_SAFE_FREE(sd->mouse.longpress_timer, ecore_timer_del);
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
          void *event_info EINA_UNUSED)
{
   Dialog_Data *d = data;

   *d->response = (obj == d->bt_ok);
   if ((d->type == DIALOG_PROMPT) && (*d->response == EINA_TRUE))
     *d->entry_value = strdup(elm_entry_entry_get(d->entry));

   evas_object_del(d->dialog);
}

static void
_file_sel_done(void *data,
               Evas_Object *obj EINA_UNUSED,
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
            Eina_Bool allows_multiple_files EINA_UNUSED,
            Eina_List *accept_types EINA_UNUSED,
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
             elm_box_horizontal_set(bx_h, EINA_TRUE);
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

/*
 * called by ewk_view when javascript called alert()
 */
static void
_view_smart_run_javascript_alert(Ewk_View_Smart_Data *esd,
                                 Evas_Object *frame EINA_UNUSED,
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
                                   Evas_Object *frame EINA_UNUSED,
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
                                  Evas_Object *frame EINA_UNUSED,
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
                           Evas_Object *frame EINA_UNUSED,
                           Ewk_File_Chooser *request,
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
                                   ewk_file_chooser_allows_multiple_files_get(request),
                                   ewk_file_chooser_accept_mimetypes_get(request),
                                   selected_filenames, &response);
   else
     diag = _run_dialog(view, DIALOG_FILE_SELECTOR, NULL, NULL, NULL,
                        ewk_file_chooser_allows_multiple_files_get(request),
                        ewk_file_chooser_accept_mimetypes_get(request),
                        selected_filenames, &response);
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

        ewk_view_smart_set(&api);

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
             CRI("Could not create smart class");
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
                                Evas_Object *obj EINA_UNUSED,
                                void *event_info)
{
   ELM_WEB_DATA_GET(data, sd);
   Evas_Object *top = elm_widget_top_get(sd->obj);
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
                          void *event_info EINA_UNUSED)
{
   _ewk_view_inputmethod_change_cb(data, obj, (void *)(long)EINA_FALSE);
}

static void
_ewk_view_load_finished_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info)
{
   ELM_WEB_DATA_GET(data, sd);

   if (event_info) return;

   if (sd->zoom.mode != ELM_WEB_ZOOM_MODE_MANUAL)
     {
        float tz = sd->zoom.current;
        sd->zoom.current = 0.0;
        elm_web_zoom_set(sd->obj, tz);
     }
}

static void
_ewk_view_viewport_changed_cb(void *data,
                              Evas_Object *obj,
                              void *event_info EINA_UNUSED)
{
   ELM_WEB_DATA_GET(data, sd);

   if (sd->zoom.mode != ELM_WEB_ZOOM_MODE_MANUAL)
     {
        ewk_view_zoom_set(obj, 1.0, 0, 0);
        sd->zoom.no_anim = EINA_TRUE;
     }
}

static Eina_Bool
_restore_zoom_mode_timer_cb(void *data)
{
   ELM_WEB_DATA_GET(data, sd);
   float tz = sd->zoom.current;

   sd->zoom.timer = NULL;
   sd->zoom.current = 0.0;
   sd->zoom.no_anim = EINA_TRUE;
   elm_web_zoom_set(sd->obj, tz);

   return EINA_FALSE;
}

static Eina_Bool
_reset_zoom_timer_cb(void *data)
{
   ELM_WEB_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd, EINA_FALSE);

   sd->zoom.timer = ecore_timer_add(0.0, _restore_zoom_mode_timer_cb, data);
   ewk_view_zoom_set(wd->resize_obj, 1.0, 0, 0);

   return EINA_FALSE;
}

static void
_ewk_view_resized_cb(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   ELM_WEB_DATA_GET(data, sd);

   if (!(sd->zoom.mode != ELM_WEB_ZOOM_MODE_MANUAL))
     return;

   ecore_timer_del(sd->zoom.timer);
   sd->zoom.timer = ecore_timer_add(0.5, _reset_zoom_timer_cb, data);
}

static void
_popup_del_job(void *data)
{
   evas_object_del(data);
}

static void
_popup_will_delete(void *data,
                   Evas_Object *obj,
                   void *event_info EINA_UNUSED)
{
   ecore_job_add(_popup_del_job, data);
   evas_object_smart_callback_del(obj, "popup,willdelete", _popup_will_delete);
}

static void
_popup_item_selected(void *data,
                     Evas_Object *obj,
                     void *event_info EINA_UNUSED)
{
   Elm_Object_Item *list_it = elm_list_selected_item_get(obj);
   const Eina_List *itr, *list = elm_list_items_get(obj);
   Evas_Object *view = data;
   int i = 0;
   void *d;

   EINA_LIST_FOREACH(list, itr, d)
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
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   ewk_view_popup_destroy(data);
}

static void
_ewk_view_popup_create_cb(void *data,
                          Evas_Object *obj,
                          void *event_info)
{
   ELM_WEB_DATA_GET(data, sd);
   Evas_Object *notify, *list, *grid, *win;
   Ewk_Menu *m = event_info;
   Ewk_Menu_Item *it;
   Elm_Web_Menu m2;
   Eina_List *itr;

   int h, ww, wh;

   m2.items = m->items;
   m2.x = m->x;
   m2.y = m->y;
   m2.width = m->width;
   m2.height = m->height;
   m2.handled = EINA_FALSE;
   evas_object_smart_callback_call
     (sd->obj, "popup,create", &m2);
   if (m2.handled) return;

   win = elm_widget_top_get(data);

   notify = elm_notify_add(win);
   elm_notify_allow_events_set(notify, EINA_FALSE);
   elm_notify_align_set(notify, 0.5, 1.0);

   list = elm_list_add(data);
   elm_list_select_mode_set(data, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_scroller_bounce_set(list, EINA_FALSE, EINA_FALSE);
   elm_list_mode_set(list, ELM_LIST_EXPAND);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(list);

   EINA_LIST_FOREACH(m->items, itr, it)
     elm_list_item_append(list, it->text, NULL, NULL, _popup_item_selected,
                          obj);
   elm_list_go(list);

   grid = elm_grid_add(data);
   elm_grid_size_set(grid, 1, 1);
   elm_grid_pack(grid, list, 0, 0, 1, 1);
   evas_object_geometry_get(win, NULL, NULL, &ww, &wh);
   //FIXME: it should be the real height of items in the list.
   h = m->height * eina_list_count(m->items);
   evas_object_size_hint_min_set(grid, ww, h < wh / 2 ? h : wh / 2);
   elm_object_content_set(notify, grid);
   evas_object_show(grid);

   evas_object_show(notify);

   evas_object_smart_callback_add
     (obj, "popup,willdelete", _popup_will_delete, notify);
   evas_object_smart_callback_add
     (notify, "block,clicked", _popup_dismiss_cb, obj);
}

static void
_view_smart_callback_proxy_free_cb(void *data,
                                   Evas *e EINA_UNUSED,
                                   Evas_Object *obj EINA_UNUSED,
                                   void *event_info EINA_UNUSED)
{
   free(data);
}

static void
_view_smart_callback_proxy_cb(void *data,
                              Evas_Object *obj EINA_UNUSED,
                              void *event_info)
{
   Elm_Web_Callback_Proxy_Context *ctxt = data;

   if (!strcmp(ctxt->name, "uri,changed"))
     evas_object_smart_callback_call(ctxt->obj, "url,changed", event_info);

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
   ELM_WEB_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd, ECORE_CALLBACK_CANCEL);

   Evas_Object *frame =
     ewk_view_frame_main_get(wd->resize_obj);
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

   return ECORE_CALLBACK_RENEW;
}

#endif

#ifdef HAVE_ELEMENTARY_WEB
static Eina_Bool _elm_need_web = EINA_FALSE;
#endif

void
_elm_unneed_web(void)
{
#ifdef HAVE_ELEMENTARY_WEB
   if (!_elm_need_web) return;
   _elm_need_web = EINA_FALSE;
   ewk_shutdown();
#endif
}

EAPI Eina_Bool
elm_need_web(void)
{
#ifdef HAVE_ELEMENTARY_WEB
   if (_elm_need_web) return EINA_TRUE;
   _elm_need_web = EINA_TRUE;
   ewk_init();
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

static void
_elm_web_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Web_Smart_Data *priv = _pd;
   Evas_Object *resize_obj;

#ifdef HAVE_ELEMENTARY_WEB
   resize_obj = _view_add(obj);
#else
   resize_obj = elm_label_add(obj);
   elm_object_text_set(resize_obj, "WebKit not supported!");
#endif

   elm_widget_resize_object_set(obj, resize_obj, EINA_TRUE);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

#ifdef HAVE_ELEMENTARY_WEB
   ewk_view_setting_user_agent_set
     (resize_obj, ELM_WEB_USER_AGENT);

   priv->input_method = ELM_WIN_KEYBOARD_OFF;
   evas_object_smart_callback_add
     (resize_obj, "inputmethod,changed",
     _ewk_view_inputmethod_change_cb, obj);
   evas_object_smart_callback_add
     (resize_obj, "load,started",
     _ewk_view_load_started_cb, obj);
   evas_object_smart_callback_add
     (resize_obj, "popup,create",
     _ewk_view_popup_create_cb, obj);
   evas_object_smart_callback_add
     (resize_obj, "load,finished",
     _ewk_view_load_finished_cb, obj);
   evas_object_smart_callback_add
     (resize_obj, "viewport,changed",
     _ewk_view_viewport_changed_cb, obj);
   evas_object_smart_callback_add
     (resize_obj, "view,resized",
     _ewk_view_resized_cb, obj);

   priv->inwin_mode = _elm_config->inwin_dialogs_enable;
   priv->zoom.min =
     ewk_view_zoom_range_min_get(resize_obj);
   priv->zoom.max =
     ewk_view_zoom_range_max_get(resize_obj);
   priv->zoom.current = 1.0;

   _view_smart_callback_proxy(resize_obj, obj);
   eo_do(obj, elm_obj_widget_theme_apply(NULL));

   elm_widget_can_focus_set(obj, EINA_TRUE);
#else
   (void)priv;
#endif
}

static void
_elm_web_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

   ecore_timer_del(sd->zoom.timer);
   ecore_animator_del(sd->bring_in.animator);

#else
   (void)_pd;
#endif

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

static void
_constructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{

   Elm_Web_Smart_Data *sd = _pd;
   sd->obj = obj;
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_elm_web_smart_callbacks, NULL));
}

EAPI Evas_Object *
elm_web_webkit_view_get(const Evas_Object *obj)
{
   ELM_WEB_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_obj_web_webkit_view_get(&ret));
   return ret;
}

static void
_webkit_view_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   *ret = wd->resize_obj;
#else
   ERR("Elementary not compiled with EWebKit support.");
   *ret =  NULL;
   (void)obj;
#endif
}

EAPI void
elm_web_window_create_hook_set(Evas_Object *obj,
                               Elm_Web_Window_Open func,
                               void *data)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_window_create_hook_set(func, data));
#else
   (void)obj;
   (void)func;
   (void)data;
#endif
}

static void
_window_create_hook_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Web_Window_Open func = va_arg(*list, Elm_Web_Window_Open);
   void *data = va_arg(*list, void *);
#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

   sd->hook.window_create = func;
   sd->hook.window_create_data = data;
#else
   (void)_pd;
   (void)func;
   (void)data;
#endif
}

EAPI void
elm_web_dialog_alert_hook_set(Evas_Object *obj,
                              Elm_Web_Dialog_Alert func,
                              void *data)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_dialog_alert_hook_set(func, data));
#else
   (void)obj;
   (void)func;
   (void)data;
#endif
}

static void
_dialog_alert_hook_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Web_Dialog_Alert func = va_arg(*list, Elm_Web_Dialog_Alert);
   void *data = va_arg(*list, void *);

#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

   sd->hook.alert = func;
   sd->hook.alert_data = data;
#else
   (void)func;
   (void)data;
   (void)_pd;
#endif
}

EAPI void
elm_web_dialog_confirm_hook_set(Evas_Object *obj,
                                Elm_Web_Dialog_Confirm func,
                                void *data)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_dialog_confirm_hook_set(func, data));
#else
   (void)obj;
   (void)func;
   (void)data;
#endif
}

static void
_dialog_confirm_hook_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Web_Dialog_Confirm func = va_arg(*list, Elm_Web_Dialog_Confirm);
   void *data = va_arg(*list, void *);

#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

   sd->hook.confirm = func;
   sd->hook.confirm_data = data;
#else
   (void)func;
   (void)data;
   (void)_pd;
#endif
}

EAPI void
elm_web_dialog_prompt_hook_set(Evas_Object *obj,
                               Elm_Web_Dialog_Prompt func,
                               void *data)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_dialog_prompt_hook_set(func, data));
#else
   (void)obj;
   (void)func;
   (void)data;
#endif
}

static void
_dialog_prompt_hook_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Web_Dialog_Prompt func = va_arg(*list, Elm_Web_Dialog_Prompt);
   void *data = va_arg(*list, void *);
#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

   sd->hook.prompt = func;
   sd->hook.prompt_data = data;
#else
   (void)func;
   (void)data;
   (void)_pd;
#endif
}

EAPI void
elm_web_dialog_file_selector_hook_set(Evas_Object *obj,
                                      Elm_Web_Dialog_File_Selector func,
                                      void *data)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_dialog_file_selector_hook_set(func, data));
#else
   (void)obj;
   (void)func;
   (void)data;
#endif
}

static void
_dialog_file_selector_hook_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Web_Dialog_File_Selector func = va_arg(*list, Elm_Web_Dialog_File_Selector);
   void *data = va_arg(*list, void *);
#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

   sd->hook.file_selector = func;
   sd->hook.file_selector_data = data;
#else
   (void)func;
   (void)data;
   (void)_pd;
#endif
}

EAPI void
elm_web_console_message_hook_set(Evas_Object *obj,
                                 Elm_Web_Console_Message func,
                                 void *data)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_console_message_hook_set(func, data));
#else
   (void)obj;
   (void)func;
   (void)data;
#endif
}

static void
_console_message_hook_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Web_Console_Message func = va_arg(*list, Elm_Web_Console_Message);
   void *data = va_arg(*list, void *);

#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

   sd->hook.console_message = func;
   sd->hook.console_message_data = data;
#else
   (void)func;
   (void)data;
   (void)_pd;
#endif
}

EAPI void
elm_web_useragent_set(Evas_Object *obj,
                      const char *user_agent)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_useragent_set(user_agent));
#else
   (void)user_agent;
   (void)obj;
#endif
}

static void
_useragent_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *user_agent = va_arg(*list, const char *);
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   ewk_view_setting_user_agent_set
     (wd->resize_obj, user_agent);
#else
   (void)user_agent;
   (void)obj;
#endif
}

EAPI const char *
elm_web_useragent_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_web_useragent_get(&ret));
   return ret;
#else
   (void)obj;
   return NULL;
#endif
}

static void
_useragent_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char **ret = va_arg(*list, const char **);

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   *ret = ewk_view_setting_user_agent_get(wd->resize_obj);
#else
   *ret = NULL;
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_tab_propagate_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_web_tab_propagate_get(&ret));
   return ret;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

static void
_tab_propagate_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

   *ret = sd->tab_propagate;
#else
   *ret = EINA_FALSE;
   (void)_pd;
#endif
}

EAPI void
elm_web_tab_propagate_set(Evas_Object *obj,
                          Eina_Bool propagate)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_tab_propagate_set(propagate));
#else
   (void)obj;
   (void)propagate;
#endif
}

static void
_tab_propagate_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool propagate = va_arg(*list, int);

#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

   sd->tab_propagate = propagate;
#else
   (void)propagate;
   (void)_pd;
#endif
}

EAPI Eina_Bool
elm_web_uri_set(Evas_Object *obj,
                const char *url)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_url_set(url, &ret));
   return ret;
#else
   (void)obj;
   (void)url;
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_url_set(Evas_Object *obj,
                const char *url)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_url_set(url, &ret));
   return ret;
#else
   (void)obj;
   (void)url;
   return EINA_FALSE;
#endif
}

static void
_url_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *url = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool int_ret = EINA_FALSE;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   int_ret = ewk_view_uri_set(wd->resize_obj, url);
   if (ret) *ret = int_ret;
#else
   (void)url;
   (void)obj;
#endif
}

EAPI const char *
elm_web_uri_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_web_url_get(&ret));
   return ret;
#else
   (void)obj;
   return NULL;
#endif
}

EAPI const char *
elm_web_url_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_web_url_get(&ret));
   return ret;
#else
   (void)obj;
   return NULL;
#endif
}

static void
_url_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   *ret = ewk_view_uri_get(wd->resize_obj);
#else
   *ret = NULL;
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_html_string_load(Evas_Object *obj, const char *html, const char *base_url, const char *unreachable_url)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_html_string_load(html, base_url, unreachable_url, &ret));
   return ret;
#else
   (void)obj;
   (void)html;
   (void)base_url;
   (void)unreachable_url;
   return EINA_FALSE;
#endif
}

static void
_html_string_load(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *html = va_arg(*list, const char *);
   const char *base_url = va_arg(*list, const char *);
   const char *unreachable_url = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool int_ret;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!unreachable_url)
     int_ret = ewk_frame_contents_set(
                  ewk_view_frame_main_get(wd->resize_obj),
                  html, 0, NULL, NULL,
                  base_url);
   else
     int_ret = ewk_frame_contents_alternate_set(
                  ewk_view_frame_main_get(wd->resize_obj),
                  html, 0, NULL, NULL,
                  base_url, unreachable_url);

   if (ret) *ret = int_ret;
#else
   (void)obj;
   (void)html;
   (void)base_url;
   (void)unreachable_url;
#endif
}

EAPI const char *
elm_web_title_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_web_title_get(&ret));
   return ret;
#else
   (void)obj;
   return NULL;
#endif
}

static void
_title_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   *ret = NULL;
#ifdef HAVE_ELEMENTARY_WEB
   const Ewk_Text_With_Direction *txt;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   txt = ewk_view_title_get(wd->resize_obj);
   if (txt) *ret = txt->string;
#else
   (void)obj;
#endif
}

EAPI void
elm_web_bg_color_set(Evas_Object *obj,
                     int r,
                     int g,
                     int b,
                     int a)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_bg_color_set(r, g, b, a));
#else
   (void)obj;
   (void)r;
   (void)g;
   (void)b;
   (void)a;
#endif
}

static void
_bg_color_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   int r = va_arg(*list, int);
   int g = va_arg(*list, int);
   int b = va_arg(*list, int);
   int a = va_arg(*list, int);
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   ewk_view_bg_color_set(wd->resize_obj, r, g, b, a);
#else
   (void)obj;
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
   ELM_WEB_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_web_bg_color_get(r, g, b, a));
}

static void
_bg_color_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   int *r = va_arg(*list, int *);
   int *g = va_arg(*list, int *);
   int *b = va_arg(*list, int *);
   int *a = va_arg(*list, int *);

   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   return ewk_view_bg_color_get(wd->resize_obj, r, g, b, a);
#else
   (void)obj;
#endif
}

EAPI const char *
elm_web_selection_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_web_selection_get(&ret));
   return ret;
#else
   (void)obj;
   return NULL;
#endif
}

static void
_selection_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   *ret = ewk_view_selection_get(wd->resize_obj);
#else
   *ret = NULL;
   (void)obj;
#endif
}

EAPI void
elm_web_popup_selected_set(Evas_Object *obj,
                           int idx)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_popup_selected_set(idx));
#else
   (void)obj;
   (void)idx;
#endif
}

static void
_popup_selected_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   int idx = va_arg(*list, int);
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   ewk_view_popup_selected_set(wd->resize_obj, idx);
#else
   (void)idx;
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_popup_destroy(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_popup_destroy(&ret));
   return ret;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

static void
_popup_destroy(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool int_ret = EINA_FALSE;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   int_ret = ewk_view_popup_destroy(wd->resize_obj);
   if (ret) *ret = int_ret;
#else
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_text_search(const Evas_Object *obj,
                    const char *string,
                    Eina_Bool case_sensitive,
                    Eina_Bool forward,
                    Eina_Bool wrap)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_web_text_search(string, case_sensitive, forward, wrap, &ret));
   return ret;
#else
   (void)obj;
   (void)string;
   (void)case_sensitive;
   (void)forward;
   (void)wrap;
   return EINA_FALSE;
#endif
}

static void
_text_search(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *string = va_arg(*list, const char *);
   Eina_Bool case_sensitive = va_arg(*list, int);
   Eina_Bool forward = va_arg(*list, int);
   Eina_Bool wrap = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool int_ret = EINA_FALSE;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   int_ret = ewk_view_text_search
            (wd->resize_obj, string,
            case_sensitive, forward, wrap);
   if (ret) *ret = int_ret;
#else
   (void)string;
   (void)case_sensitive;
   (void)forward;
   (void)wrap;
   (void)obj;
#endif
}

EAPI unsigned int
elm_web_text_matches_mark(Evas_Object *obj,
                          const char *string,
                          Eina_Bool case_sensitive,
                          Eina_Bool highlight,
                          unsigned int limit)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) 0;
   unsigned int ret = 0;
   eo_do(obj, elm_obj_web_text_matches_mark(string, case_sensitive, highlight, limit, &ret));
   return ret;
#else
   (void)obj;
   (void)string;
   (void)case_sensitive;
   (void)highlight;
   (void)limit;
   return 0;
#endif
}

static void
_text_matches_mark(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *string = va_arg(*list, const char *);
   Eina_Bool case_sensitive = va_arg(*list, int);
   Eina_Bool highlight = va_arg(*list, int);
   unsigned int limit = va_arg(*list, unsigned int);
   unsigned int *ret = va_arg(*list, unsigned int *);
   if (ret) *ret = 0;

#ifdef HAVE_ELEMENTARY_WEB
   unsigned int int_ret = 0;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   int_ret = ewk_view_text_matches_mark
            (wd->resize_obj, string,
            case_sensitive, highlight, limit);
   if (ret) *ret = int_ret;
#else
   (void)string;
   (void)case_sensitive;
   (void)highlight;
   (void)limit;
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_text_matches_unmark_all(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_text_matches_unmark_all(&ret));
   return ret;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

static void
_text_matches_unmark_all(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool int_ret = EINA_FALSE;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   int_ret = ewk_view_text_matches_unmark_all(wd->resize_obj);
   if (ret) *ret = int_ret;
#else
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_text_matches_highlight_set(Evas_Object *obj,
                                   Eina_Bool highlight)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_text_matches_highlight_set(highlight, &ret));
   return ret;
#else
   (void)obj;
   (void)highlight;
   return EINA_FALSE;
#endif
}

static void
_text_matches_highlight_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool highlight = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool int_ret;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   int_ret = ewk_view_text_matches_highlight_set
            (wd->resize_obj, highlight);
   if (ret) *ret = int_ret;
#else
   (void)obj;
   (void)highlight;
#endif
}

EAPI Eina_Bool
elm_web_text_matches_highlight_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_web_text_matches_highlight_get(&ret));
   return ret;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

static void
_text_matches_highlight_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool int_ret = EINA_FALSE;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   int_ret = ewk_view_text_matches_highlight_get(wd->resize_obj);
   if (ret) *ret = int_ret;
#else
   (void)obj;
#endif
}

EAPI double
elm_web_load_progress_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) -1.0;
   double ret = -1.0;
   eo_do((Eo *) obj, elm_obj_web_load_progress_get(&ret));
   return ret;
#else
   (void)obj;
   return -1.0;
#endif
}

static void
_load_progress_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   double *ret = va_arg(*list, double *);
   *ret = -1.0;
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   *ret = ewk_view_load_progress_get(wd->resize_obj);
#else
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_stop(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_stop(&ret));
   return ret;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

static void
_stop(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool int_ret;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   int_ret = ewk_view_stop(wd->resize_obj);
   if (ret) *ret = int_ret;
#else
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_reload(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_reload(&ret));
   return ret;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

static void
_reload(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool int_ret;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   int_ret = ewk_view_reload(wd->resize_obj);
   if (ret) *ret = int_ret;
#else
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_reload_full(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_reload_full(&ret));
   return ret;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

static void
_reload_full(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool int_ret;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   int_ret = ewk_view_reload_full(wd->resize_obj);
   if (ret) *ret = int_ret;
#else
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_back(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_back(&ret));
   return ret;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

static void
_back(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool int_ret;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   int_ret = ewk_view_back(wd->resize_obj);
   if (ret) *ret = int_ret;
#else
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_forward(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_forward(&ret));
   return ret;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

static void
_forward(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool int_ret;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   int_ret = ewk_view_forward(wd->resize_obj);
   if (ret) *ret = int_ret;
#else
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_navigate(Evas_Object *obj,
                 int steps)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_navigate(steps, &ret));
   return ret;
#else
   (void)obj;
   (void)steps;
   return EINA_FALSE;
#endif
}

static void
_navigate(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   int steps = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   Eina_Bool int_ret;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   int_ret = ewk_view_navigate(wd->resize_obj, steps);
   if (ret) *ret = int_ret;
#else
   (void)steps;
   (void)obj;
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
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_back_possible_get(&ret));
   return ret;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

static void
_back_possible_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   *ret = ewk_view_back_possible(wd->resize_obj);
#else
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_forward_possible_get(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_forward_possible_get(&ret));
   return ret;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

static void
_forward_possible_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   *ret = ewk_view_forward_possible(wd->resize_obj);
#else
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_navigate_possible_get(Evas_Object *obj,
                              int steps)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_web_navigate_possible_get(steps, &ret));
   return ret;
#else
   (void)obj;
   (void)steps;
   return EINA_FALSE;
#endif
}

static void
_navigate_possible_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   int steps = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   *ret = ewk_view_navigate_possible(wd->resize_obj, steps);
#else
   (void)steps;
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_web_history_enabled_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_web_history_enabled_get(&ret));
   return ret;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

static void
_history_enabled_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   *ret = ewk_view_history_enable_get(wd->resize_obj);
#else
   (void)obj;
#endif
}

EAPI void
elm_web_history_enabled_set(Evas_Object *obj,
                            Eina_Bool enable)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_history_enabled_set(enable));
#else
   (void)obj;
   (void)enable;
#endif
}

static void
_history_enabled_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool enable = va_arg(*list, int);
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   ewk_view_history_enable_set(wd->resize_obj, enable);
#else
   (void)enable;
   (void)obj;
#endif
}

//EAPI Ewk_History *ewk_view_history_get(const Evas_Object *obj); // TODO:

EAPI void
elm_web_zoom_set(Evas_Object *obj,
                 double zoom)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_zoom_set(zoom));
#else
   (void)obj;
   (void)zoom;
#endif
}

static void
_zoom_set(Eo *obj, void *_pd, va_list *list)
{
   double zoom = va_arg(*list, double);

#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   int vw, vh, cx, cy;
   float z = 1.0;

   evas_object_geometry_get
     (wd->resize_obj, NULL, NULL, &vw, &vh);
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
          ewk_view_frame_main_get(wd->resize_obj);
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
          ewk_view_frame_main_get(wd->resize_obj);
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
     ewk_view_zoom_set(wd->resize_obj, z, cx, cy);
   else
     ewk_view_zoom_animated_set
       (wd->resize_obj, z,
       _elm_config->zoom_friction, cx, cy);
   sd->zoom.no_anim = EINA_FALSE;
#else
   (void)obj;
   (void)_pd;
   (void)zoom;
#endif
}

EAPI double
elm_web_zoom_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) -1.0;
   double ret = -1.0;
   eo_do((Eo *) obj, elm_obj_web_zoom_get(&ret));
   return ret;
#else
   (void)obj;
   return -1.0;
#endif
}

static void
_zoom_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   *ret = -1.0;
#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

   *ret = sd->zoom.current;
#else
   (void)_pd;
#endif
}

EAPI void
elm_web_zoom_mode_set(Evas_Object *obj,
                      Elm_Web_Zoom_Mode mode)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_zoom_mode_set(mode));
#else
   (void)obj;
   (void)mode;
#endif
}

static void
_zoom_mode_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Web_Zoom_Mode mode = va_arg(*list, Elm_Web_Zoom_Mode);

#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

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
   (void)obj;
   (void)_pd;
   (void)mode;
#endif
}

EAPI Elm_Web_Zoom_Mode
elm_web_zoom_mode_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) ELM_WEB_ZOOM_MODE_LAST;
   Elm_Web_Zoom_Mode ret = ELM_WEB_ZOOM_MODE_LAST;
   eo_do((Eo *) obj, elm_obj_web_zoom_mode_get(&ret));
   return ret;
#else
   (void)obj;
   return ELM_WEB_ZOOM_MODE_LAST;
#endif
}

static void
_zoom_mode_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Web_Zoom_Mode *ret = va_arg(*list, Elm_Web_Zoom_Mode *);
   *ret = ELM_WEB_ZOOM_MODE_LAST;
#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

   *ret = sd->zoom.mode;
#else
   (void)_pd;
#endif
}

EAPI void
elm_web_region_show(Evas_Object *obj,
                    int x,
                    int y,
                    int w,
                    int h)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_region_show(x, y, w, h));
#else
   (void)obj;
   (void)x;
   (void)y;
   (void)w;
   (void)h;
#endif
}

static void
_region_show(Eo *obj, void *_pd, va_list *list)
{
   int x = va_arg(*list, int);
   int y = va_arg(*list, int);
   int w = va_arg(*list, int);
   int h = va_arg(*list, int);
   (void)w;
   (void)h;

#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Evas_Object *frame =
     ewk_view_frame_main_get(wd->resize_obj);
   int fw, fh, zw, zh, rx, ry;
   float zoom;

   ewk_frame_contents_size_get(frame, &fw, &fh);
   zoom = ewk_frame_page_zoom_get(frame);
   zw = fw / zoom;
   zh = fh / zoom;
   rx = (x * fw) / zw;
   ry = (y * fh) / zh;
   ELM_SAFE_FREE(sd->bring_in.animator, ecore_animator_del);
   ewk_frame_scroll_set(frame, rx, ry);
#else
   (void)obj;
   (void)_pd;
   (void)x;
   (void)y;
#endif
}

EAPI void
elm_web_region_bring_in(Evas_Object *obj,
                        int x,
                        int y,
                        int w,
                        int h)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_region_bring_in(x, y, w, h));
#else
   (void)obj;
   (void)x;
   (void)y;
   (void)w;
   (void)h;
#endif
}

static void
_region_bring_in(Eo *obj, void *_pd, va_list *list)
{
   int x = va_arg(*list, int);
   int y = va_arg(*list, int);
   int w = va_arg(*list, int);
   int h = va_arg(*list, int);
   (void)w;
   (void)h;

#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Evas_Object *frame =
     ewk_view_frame_main_get(wd->resize_obj);
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
   ecore_animator_del(sd->bring_in.animator);
   sd->bring_in.animator = ecore_animator_timeline_add(
       _elm_config->bring_in_scroll_friction, _bring_in_anim_cb, obj);
#else
   (void)obj;
   (void)_pd;
   (void)x;
   (void)y;
#endif
}

EAPI void
elm_web_inwin_mode_set(Evas_Object *obj,
                       Eina_Bool value)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj);
   eo_do(obj, elm_obj_web_inwin_mode_set(value));
#else
   (void)obj;
   (void)value;
#endif
}

static void
_inwin_mode_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool value = va_arg(*list, int);
#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

   sd->inwin_mode = value;
#else
   (void)_pd;
   (void)value;
#endif
}

EAPI Eina_Bool
elm_web_inwin_mode_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   ELM_WEB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_web_inwin_mode_get(&ret));
   return ret;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

static void
_inwin_mode_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Elm_Web_Smart_Data *sd = _pd;

   *ret = sd->inwin_mode;
#else
   (void)_pd;
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
   (void)wf;
   (void)flag;
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
   (void)wf;
   (void)x;
   (void)y;
   (void)w;
   (void)h;
#endif
}

// TODO: use all ewk_view_zoom stuff to implement bring-in and
// animated zoom like elm_photocam. Should be simple to use, must not
// expose every single bit to users!

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_web_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_web_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_web_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ON_FOCUS), _elm_web_smart_on_focus),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_EVENT), _elm_web_smart_event),

        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_WEBKIT_VIEW_GET), _webkit_view_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_WINDOW_CREATE_HOOK_SET), _window_create_hook_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_DIALOG_ALERT_HOOK_SET), _dialog_alert_hook_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_DIALOG_CONFIRM_HOOK_SET), _dialog_confirm_hook_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_DIALOG_PROMPT_HOOK_SET), _dialog_prompt_hook_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_DIALOG_FILE_SELECTOR_HOOK_SET), _dialog_file_selector_hook_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_CONSOLE_MESSAGE_HOOK_SET), _console_message_hook_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_USERAGENT_SET), _useragent_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_USERAGENT_GET), _useragent_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TAB_PROPAGATE_GET), _tab_propagate_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TAB_PROPAGATE_SET), _tab_propagate_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_URL_SET), _url_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_URL_GET), _url_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_HTML_STRING_LOAD), _html_string_load),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TITLE_GET), _title_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_BG_COLOR_SET), _bg_color_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_BG_COLOR_GET), _bg_color_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_SELECTION_GET), _selection_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_POPUP_SELECTED_SET), _popup_selected_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_POPUP_DESTROY), _popup_destroy),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TEXT_SEARCH), _text_search),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_MARK), _text_matches_mark),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_UNMARK_ALL), _text_matches_unmark_all),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_HIGHLIGHT_SET), _text_matches_highlight_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_HIGHLIGHT_GET), _text_matches_highlight_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_LOAD_PROGRESS_GET), _load_progress_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_STOP), _stop),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_RELOAD), _reload),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_RELOAD_FULL), _reload_full),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_BACK), _back),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_FORWARD), _forward),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_NAVIGATE), _navigate),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_BACK_POSSIBLE_GET), _back_possible_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_FORWARD_POSSIBLE_GET), _forward_possible_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_NAVIGATE_POSSIBLE_GET), _navigate_possible_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_HISTORY_ENABLED_GET), _history_enabled_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_HISTORY_ENABLED_SET), _history_enabled_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_ZOOM_SET), _zoom_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_ZOOM_GET), _zoom_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_ZOOM_MODE_SET), _zoom_mode_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_ZOOM_MODE_GET), _zoom_mode_get),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_REGION_SHOW), _region_show),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_REGION_BRING_IN), _region_bring_in),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_INWIN_MODE_SET), _inwin_mode_set),
        EO_OP_FUNC(ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_INWIN_MODE_GET), _inwin_mode_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_WEBKIT_VIEW_GET, "Get internal ewk_view object from web object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_WINDOW_CREATE_HOOK_SET, "Sets the function to call when a new window is requested."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_DIALOG_ALERT_HOOK_SET, "Sets the function to call when an alert dialog."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_DIALOG_CONFIRM_HOOK_SET, "Sets the function to call when an confirm dialog."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_DIALOG_PROMPT_HOOK_SET, "Sets the function to call when an prompt dialog."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_DIALOG_FILE_SELECTOR_HOOK_SET, "Sets the function to call when an file selector dialog."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_CONSOLE_MESSAGE_HOOK_SET, "Sets the function to call when a console message is emitted from JS."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_USERAGENT_SET, "Change useragent of a elm_web object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_USERAGENT_GET, "Return current useragent of elm_web object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_TAB_PROPAGATE_GET, "Get the status of the tab propagation."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_TAB_PROPAGATE_SET, "Sets whether to use tab propagation."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_URL_SET, "Sets the URL for the web object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_URL_GET, "Get the current URL for the object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_HTML_STRING_LOAD, "Loads the specified html string as the content of the object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_TITLE_GET, "Get the current title."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_BG_COLOR_SET, "Sets the background color to be used by the web object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_BG_COLOR_GET, "Get the background color to be used by the web object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_SELECTION_GET, "Get a copy of the currently selected text."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_POPUP_SELECTED_SET, "Tells the web object which index in the currently open popup was selected."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_POPUP_DESTROY, "Dismisses an open dropdown popup."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_TEXT_SEARCH, "Searches the given string in a document."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_MARK, "Marks matches of the given string in a document."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_UNMARK_ALL, "Clears all marked matches in the document."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_HIGHLIGHT_SET, "Sets whether to highlight the matched marks."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_HIGHLIGHT_GET, "Get whether highlighting marks is enabled."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_LOAD_PROGRESS_GET, "Get the overall loading progress of the page."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_STOP, "Stops loading the current page."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_RELOAD, "Requests a reload of the current document in the object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_RELOAD_FULL, "Requests a reload of the current document, avoiding any existing caches."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_BACK, "Goes back one step in the browsing history."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_FORWARD, "Goes forward one step in the browsing history."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_NAVIGATE, "Jumps the given number of steps in the browsing history."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_BACK_POSSIBLE_GET, "Queries whether it's possible to go back in history."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_FORWARD_POSSIBLE_GET, "Queries whether it's possible to go forward in history."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_NAVIGATE_POSSIBLE_GET, "Queries whether it's possible to jump the given number of steps."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_HISTORY_ENABLED_GET, "Get whether browsing history is enabled for the given object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_HISTORY_ENABLED_SET, "Enables or disables the browsing history."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_ZOOM_SET, "Sets the zoom level of the web object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_ZOOM_GET, "Get the current zoom level set on the web object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_ZOOM_MODE_SET, "Sets the zoom mode to use."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_ZOOM_MODE_GET, "Get the currently set zoom mode."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_REGION_SHOW, "Shows the given region in the web object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_REGION_BRING_IN, "Brings in the region to the visible area."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_INWIN_MODE_SET, "Sets the default dialogs to use an Inwin instead of a normal window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WEB_SUB_ID_INWIN_MODE_GET, "Get whether Inwin mode is set for the current object."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_WEB_BASE_ID, op_desc, ELM_OBJ_WEB_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Web_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_web_class_get, &class_desc, ELM_OBJ_WIDGET_CLASS, NULL);
#endif

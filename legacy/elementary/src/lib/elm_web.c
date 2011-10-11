#include <Elementary.h>
#include "elm_priv.h"

// TODO:
//  1 - easy to use zoom like elm_photocam API
//  2 - review scrolling to match elm_scroller. Maybe in future use elm_scroller

#ifdef HAVE_ELEMENTARY_WEB
#include <EWebKit.h>

/* Similar to iPhone */
// TODO: switch between iPhone, iPad or Chrome/Safari based on some elm setting?
#define ELM_WEB_USER_AGENT "Mozilla/5.0 (iPhone; U; CPU like Mac OS X; en) AppleWebKit/420+ (KHTML, like Gecko) Version/3.0 Mobile/1A543a Safari/419.3 " PACKAGE_NAME "/" PACKAGE_VERSION

static Ewk_View_Smart_Class _parent_sc = EWK_VIEW_SMART_CLASS_INIT_NULL;

typedef struct _View_Smart_Data View_Smart_Data;
struct _View_Smart_Data
{
   Ewk_View_Smart_Data base;
   struct {
      Evas_Event_Mouse_Down event;
      Evas_Coord x, y;
      unsigned int move_count;
      Ecore_Timer *longpress_timer;
      Ecore_Animator *pan_anim;
   } mouse;
};
#endif

typedef struct _Widget_Data Widget_Data;
struct _Widget_Data
{
   Evas_Object *self;
#ifdef HAVE_ELEMENTARY_WEB
   Evas_Object *ewk_view;
   struct {
      Elm_Web_Window_Open window_create;
      void *window_create_data;
      Elm_Web_Dialog_Alert alert;
      void *alert_data;
      Elm_Web_Dialog_Confirm confirm;
      void *confirm_data;
      Elm_Web_Dialog_Prompt prompt;
      void *prompt_data;
      Elm_Web_Dialog_File_Selector file_selector;
      void *file_selector_data;
      Elm_Web_Console_Message console_message;
      void *console_message_data;
   } hook;
   Elm_Win_Keyboard_Mode input_method;
   struct {
        Elm_Web_Zoom_Mode mode;
        float current;
        float min, max;
        Eina_Bool no_anim;
        Ecore_Timer *timer;
   } zoom;
   struct {
        struct {
             int x, y;
        } start, end;
        Ecore_Animator *animator;
   } bring_in;
   Eina_Bool tab_propagate : 1;
   Eina_Bool inwin_mode : 1;
#else
   Evas_Object *label;
#endif
};

enum Dialog_Type
{
   DIALOG_ALERT,
   DIALOG_CONFIRM,
   DIALOG_PROMPT,
   DIALOG_FILE_SELECTOR
};

typedef struct _Dialog_Data Dialog_Data;
struct _Dialog_Data
{
   enum Dialog_Type type;
   Evas_Object *dialog;
   Evas_Object *box;
   Evas_Object *bt_ok, *bt_cancel;
   Evas_Object *entry;
   Evas_Object *file_sel;

   Eina_Bool   *response;
   char       **entry_value;
   Eina_List  **selected_files;
};

struct _Elm_Web_Callback_Proxy_Context
{
   const char *name;
   Evas_Object *obj;
};
typedef struct _Elm_Web_Callback_Proxy_Context Elm_Web_Callback_Proxy_Context;

static const char *widtype = NULL;
static const Evas_Smart_Cb_Description _elm_web_callback_names[] = {
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

static void
_theme_hook(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   Elm_Theme *theme = elm_object_theme_get(obj);
   Widget_Data *wd = elm_widget_data_get(obj);
   const Eina_List *themes, *l;
   const char *th;
   char *view_theme = NULL;

   themes = elm_theme_list_get(theme);
   EINA_LIST_FOREACH(themes, l, th)
     {
        char *path = elm_theme_list_item_path_get(th, NULL);
        if (!path) continue;
        if (edje_file_group_exists(path, "webkit/base"))
          {
             view_theme = path;
             break;
          }
        free(path);
     }

   if (view_theme)
     {
        ewk_view_theme_set(wd->ewk_view, view_theme);
        free(view_theme);
     }
   else
     ewk_view_theme_set(wd->ewk_view, WEBKIT_DATADIR"/themes/default.edj");
#else
   (void)obj;
#endif
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *top = elm_widget_top_get(obj);

   if (!wd) return;

   if (elm_object_focus_get(obj))
     {
        evas_object_focus_set(wd->ewk_view, EINA_TRUE);
        if (top) elm_win_keyboard_mode_set(top, wd->input_method);
     }
   else
     {
        evas_object_focus_set(wd->ewk_view, EINA_FALSE);
        if (top) elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_OFF);
     }
#else
   (void)obj;
#endif
}

static Eina_Bool
_event_hook(Evas_Object *obj, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
#ifdef HAVE_ELEMENTARY_WEB
   Evas_Event_Key_Down *ev = event_info;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   if ((!strcmp(ev->keyname, "Tab")) && (!wd->tab_propagate))
     {
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else
     return EINA_FALSE;
#else
   return EINA_FALSE;
   (void)obj;
   (void)type;
   (void)event_info;
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

   _parent_sc.sc.add(obj);

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

   _parent_sc.sc.del(obj);
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
_view_smart_mouse_down(Ewk_View_Smart_Data *esd, const Evas_Event_Mouse_Down *event)
{
   // TODO: mimic elm_scroller and like
   // TODO-minor: offer hook?
   View_Smart_Data *sd = (View_Smart_Data *)esd;

   if (event->button != 1)
     return _parent_sc.mouse_down(esd, event);

   sd->mouse.pan_anim = ecore_animator_add(_view_pan_animator, sd);
   sd->mouse.longpress_timer = ecore_timer_add(_elm_config->longpress_timeout, _view_longpress_timer, sd);
   sd->mouse.move_count = 1;
   sd->mouse.x = event->canvas.x;
   sd->mouse.y = event->canvas.y;
   sd->mouse.event = *event;

   return EINA_TRUE;
}

static Eina_Bool
_view_smart_mouse_up(Ewk_View_Smart_Data *esd, const Evas_Event_Mouse_Up *event)
{
   // TODO: mimic elm_scroller and like
   // TODO-minor: offer hook?
   View_Smart_Data *sd = (View_Smart_Data *)esd;

   if (sd->mouse.pan_anim)
     {
        ecore_animator_del(sd->mouse.pan_anim);
        sd->mouse.pan_anim = NULL;

        if (sd->mouse.longpress_timer)
          _parent_sc.mouse_down(esd, &sd->mouse.event);
        else
          return EINA_TRUE;
     }

   if (sd->mouse.longpress_timer)
     {
        ecore_timer_del(sd->mouse.longpress_timer);
        sd->mouse.longpress_timer = NULL;
     }

   sd->mouse.move_count = 0;
   return _parent_sc.mouse_up(esd, event);
}

static Eina_Bool
_view_smart_mouse_move(Ewk_View_Smart_Data *esd, const Evas_Event_Mouse_Move *event)
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

   return _parent_sc.mouse_move(esd, event);
}

static Evas_Object *
_view_smart_window_create(Ewk_View_Smart_Data *sd, Eina_Bool javascript, const Ewk_Window_Features *window_features)
{
   Evas_Object *new;
   Evas_Object *obj = evas_object_smart_parent_get(sd->self);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd->hook.window_create) return NULL;
   new = wd->hook.window_create(wd->hook.window_create_data, obj, javascript,
                        (const Elm_Web_Window_Features *)window_features);
   if (new) return elm_web_webkit_view_get(new);

   return NULL;
}

static void
_view_smart_window_close(Ewk_View_Smart_Data *sd)
{
   Evas_Object *obj = evas_object_smart_parent_get(sd->self);
   evas_object_smart_callback_call(obj, "windows,close,request", NULL);
}

static void
_bt_close(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Dialog_Data *d = data;

   *d->response = (obj == d->bt_ok);
   if ((d->type == DIALOG_PROMPT) && (*d->response == EINA_TRUE))
     *d->entry_value = strdup(elm_entry_entry_get(d->entry));
   evas_object_del(d->dialog);
}

static void
_file_sel_done(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Dialog_Data *d = data;
   if (event_info)
     {
        *d->selected_files = eina_list_append(NULL, strdup(event_info));
        *d->response = EINA_TRUE;
     }
   else
     *d->response = EINA_FALSE;
   evas_object_del(d->dialog);
   free(d);
}

static Dialog_Data *
_dialog_new(Evas_Object *parent)
{
   Dialog_Data *d;
   Widget_Data *wd = elm_widget_data_get(parent);

   d = calloc(1, sizeof(Dialog_Data));
   if (!d) return NULL;

   if (!parent || wd->inwin_mode)
     {
        Evas_Object *bg;

        d->dialog = elm_win_add(NULL, "elm-web-popup", ELM_WIN_DIALOG_BASIC);
        evas_object_smart_callback_add(d->dialog, "delete,request",
                                       _bt_close, d);

        bg = elm_bg_add(d->dialog);
        evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        elm_win_resize_object_add(d->dialog, bg);
        evas_object_show(bg);

        d->box = elm_box_add(d->dialog);
        evas_object_size_hint_weight_set(d->box, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        elm_win_resize_object_add(d->dialog, d->box);
        evas_object_show(d->box);
     }
   else
     {
        Evas_Object *win = elm_widget_top_get(parent);
        d->dialog = elm_win_inwin_add(win);
        elm_object_style_set(d->dialog, "minimal");
        evas_object_size_hint_weight_set(d->dialog, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);

        d->box = elm_box_add(win);
        evas_object_size_hint_weight_set(d->box, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        elm_win_inwin_content_set(d->dialog, d->box);
        evas_object_show(d->box);
     }

   return d;
}

static Evas_Object *
_run_dialog(Evas_Object *parent, enum Dialog_Type type, const char *message, const char *default_entry_value, char **entry_value, Eina_Bool allows_multiple_files __UNUSED__, const char *accept_types __UNUSED__, Eina_List **selected_filenames, Eina_Bool *response)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL((type != DIALOG_PROMPT) && (!!default_entry_value), EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((type != DIALOG_PROMPT) && (!!entry_value), EINA_FALSE);

   Dialog_Data *dialog_data = _dialog_new(evas_object_smart_parent_get(parent));
   Evas_Object *lb;

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
        evas_object_size_hint_align_set(dialog_data->bt_ok, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(dialog_data->bt_ok, "clicked", _bt_close, dialog_data);
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
             evas_object_size_hint_align_set(dialog_data->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
             evas_object_size_hint_weight_set(dialog_data->entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             elm_box_pack_end(dialog_data->box, dialog_data->entry);
             evas_object_show(dialog_data->entry);
          }

        if (type == DIALOG_PROMPT || type == DIALOG_CONFIRM)
          {
             Evas_Object *bx_h = elm_box_add(dialog_data->box);
             elm_box_horizontal_set(bx_h, 1);
             elm_box_pack_end(dialog_data->box, bx_h);
             evas_object_size_hint_weight_set(bx_h, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set(bx_h, EVAS_HINT_FILL, EVAS_HINT_FILL);
             evas_object_show(bx_h);

             dialog_data->bt_cancel = elm_button_add(bx_h);
             elm_object_text_set(dialog_data->bt_cancel, "Cancel");
             elm_box_pack_end(bx_h, dialog_data->bt_cancel);
             evas_object_size_hint_weight_set(dialog_data->bt_cancel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set(dialog_data->bt_cancel, EVAS_HINT_FILL, EVAS_HINT_FILL);
             evas_object_smart_callback_add(dialog_data->bt_cancel, "clicked", _bt_close, dialog_data);
             evas_object_show(dialog_data->bt_cancel);

             dialog_data->bt_ok = elm_button_add(bx_h);
             elm_object_text_set(dialog_data->bt_ok, "Ok");
             elm_box_pack_end(bx_h, dialog_data->bt_ok);
             evas_object_size_hint_weight_set(dialog_data->bt_ok, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set(dialog_data->bt_ok, EVAS_HINT_FILL, EVAS_HINT_FILL);
             evas_object_smart_callback_add(dialog_data->bt_ok, "clicked", _bt_close, dialog_data);
             evas_object_show(dialog_data->bt_ok);
          }
        else
           return EINA_FALSE;
     }

   evas_object_show(dialog_data->dialog);

   return dialog_data->dialog;
}

static void
_dialog_del_cb(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   ecore_main_loop_quit();
}

static void
_exec_dialog(Evas_Object *dialog)
{
   evas_object_event_callback_add(dialog, EVAS_CALLBACK_DEL, _dialog_del_cb,
                                  NULL);
   ecore_main_loop_begin();
}

/* called by ewk_view when javascript called alert()
 *
 */
static void
_view_smart_run_javascript_alert(Ewk_View_Smart_Data *esd, Evas_Object *frame __UNUSED__, const char *message)
{
   View_Smart_Data *sd = (View_Smart_Data *)esd;
   Evas_Object *view = sd->base.self;
   Evas_Object *obj = evas_object_smart_parent_get(view);
   Evas_Object *diag = NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Bool response = EINA_FALSE;

   if (wd->hook.alert)
     diag = wd->hook.alert(wd->hook.alert_data, obj, message);
   else
     diag = _run_dialog(view, DIALOG_ALERT, message, NULL, NULL, EINA_FALSE,
                        NULL, NULL, &response);
   if (diag) _exec_dialog(diag);
}

/* called by ewk_view when javascript called confirm()
 *
 */
static Eina_Bool
_view_smart_run_javascript_confirm(Ewk_View_Smart_Data *esd, Evas_Object *frame __UNUSED__, const char *message)
{
   View_Smart_Data *sd = (View_Smart_Data *)esd;
   Evas_Object *view = sd->base.self;
   Evas_Object *obj = evas_object_smart_parent_get(view);
   Evas_Object *diag = NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Bool response = EINA_FALSE;

   if (wd->hook.confirm)
     diag = wd->hook.confirm(wd->hook.confirm_data, obj, message, &response);
   else
    diag = _run_dialog(view, DIALOG_CONFIRM, message, NULL, NULL, EINA_FALSE,
                       NULL, NULL, &response);
   if (diag) _exec_dialog(diag);
   return response;
}

/* called by ewk_view when javascript called confirm()
 *
 */
static Eina_Bool
_view_smart_run_javascript_prompt(Ewk_View_Smart_Data *esd, Evas_Object *frame __UNUSED__, const char *message, const char *default_value, char **value)
{
   View_Smart_Data *sd = (View_Smart_Data *)esd;
   Evas_Object *view = sd->base.self;
   Evas_Object *obj = evas_object_smart_parent_get(view);
   Evas_Object *diag = NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Bool response = EINA_FALSE;

   if (wd->hook.prompt)
     diag = wd->hook.prompt(wd->hook.prompt_data, obj, message, default_value,
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
_view_smart_run_open_panel(Ewk_View_Smart_Data *esd, Evas_Object *frame __UNUSED__, Eina_Bool allows_multiple_files, const char *accept_types, Eina_List **selected_filenames)
{
   View_Smart_Data *sd = (View_Smart_Data *)esd;
   Evas_Object *view = sd->base.self;
   Evas_Object *obj = evas_object_smart_parent_get(view);
   Evas_Object *diag = NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Bool response = EINA_FALSE;

   if (wd->hook.file_selector)
     diag = wd->hook.file_selector(wd->hook.file_selector_data, obj,
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
_view_smart_add_console_message(Ewk_View_Smart_Data *esd, const char *message, unsigned int line_number, const char *source_id)
{
   Evas_Object *obj = evas_object_smart_parent_get(esd->self);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (wd->hook.console_message)
     wd->hook.console_message(wd->hook.console_message_data, obj, message,
                              line_number, source_id);
}

static Eina_Bool
_view_smart_focus_can_cycle(Ewk_View_Smart_Data *sd, Ewk_Focus_Direction direction)
{
   Evas_Object *obj = evas_object_smart_parent_get(sd->self);
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
   static Evas_Smart *smart = NULL;
   Evas *canvas = evas_object_evas_get(parent);
   Evas_Object *view;

   if (!smart)
     {
        static Ewk_View_Smart_Class api = EWK_VIEW_SMART_CLASS_INIT_NAME_VERSION("EWK_View_Elementary");

#ifndef TILED_BACKING_STORE
        ewk_view_single_smart_set(&api);
#else
        ewk_view_tiled_smart_set(&api);
#endif

        _parent_sc = api;

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
_ewk_view_inputmethod_change_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = data;
   Evas_Object *top = elm_widget_top_get(wd->self);
   if (!top) return;

   if (event_info)
     wd->input_method = ELM_WIN_KEYBOARD_ON;
   else
     wd->input_method = ELM_WIN_KEYBOARD_OFF;
   elm_win_keyboard_mode_set(top, wd->input_method);
}

static void
_ewk_view_load_started_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   _ewk_view_inputmethod_change_cb(data, obj, (void *)(long)EINA_FALSE);
}

static void
_ewk_view_load_finished_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = data;

   if (event_info)
     return;

   if (wd->zoom.mode != ELM_WEB_ZOOM_MODE_MANUAL)
     {
        float tz = wd->zoom.current;
        wd->zoom.current = 0.0;
        elm_web_zoom_set(wd->self, tz);
     }
}

static void
_ewk_view_viewport_changed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = data;

   if (wd->zoom.mode != ELM_WEB_ZOOM_MODE_MANUAL)
     {
        ewk_view_zoom_set(obj, 1.0, 0, 0);
        wd->zoom.no_anim = EINA_TRUE;
     }
}

static Eina_Bool
_restore_zoom_mode_timer_cb(void *data)
{
   Widget_Data *wd = data;
   float tz = wd->zoom.current;
   wd->zoom.timer = NULL;
   wd->zoom.current = 0.0;
   wd->zoom.no_anim = EINA_TRUE;
   elm_web_zoom_set(wd->self, tz);
   return EINA_FALSE;
}

static Eina_Bool
_reset_zoom_timer_cb(void *data)
{
   Widget_Data *wd = data;
   wd->zoom.timer = ecore_timer_add(0.0, _restore_zoom_mode_timer_cb, wd);
   ewk_view_zoom_set(wd->ewk_view, 1.0, 0, 0);
   return EINA_FALSE;
}

static void
_ewk_view_resized_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = data;
   if (!(wd->zoom.mode != ELM_WEB_ZOOM_MODE_MANUAL))
     return;
   if (wd->zoom.timer)
     ecore_timer_del(wd->zoom.timer);
   wd->zoom.timer = ecore_timer_add(0.5, _reset_zoom_timer_cb, wd);
}

static void
_popup_del_job(void *data)
{
   evas_object_del(data);
}

static void
_popup_will_delete(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   ecore_job_add(_popup_del_job, data);
   evas_object_smart_callback_del(obj, "popup,willdelete", _popup_will_delete);
}

static void
_popup_item_selected(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *view = data;
   Elm_List_Item *it = elm_list_selected_item_get(obj);
   const Eina_List *itr, *list = elm_list_items_get(obj);
   void *d;
   int i = 0;

   EINA_LIST_FOREACH(list, itr, d)
   {
      if (d == it)
         break;

      i++;
   }

   ewk_view_popup_selected_set(view, i);
   ewk_view_popup_destroy(view);
}

static void
_popup_dismiss_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   ewk_view_popup_destroy(data);
}

static void
_ewk_view_popup_create_cb(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = data;
   Ewk_Menu *m = event_info;
   Elm_Web_Menu m2;
   Ewk_Menu_Item *it;
   Eina_List *itr;
   Evas_Object *notify, *list;

   m2.items = m->items;
   m2.x = m->x;
   m2.y = m->y;
   m2.width = m->width;
   m2.height = m->height;
   m2.handled = EINA_FALSE;
   evas_object_smart_callback_call(wd->self, "popup,create", &m2);
   if (m2.handled)
     return;

   notify = elm_notify_add(obj);
   elm_notify_repeat_events_set(notify, EINA_FALSE);
   elm_notify_orient_set(notify, ELM_NOTIFY_ORIENT_BOTTOM);

   list = elm_list_add(obj);
   elm_list_always_select_mode_set(list, EINA_TRUE);
   elm_list_bounce_set(list, EINA_FALSE, EINA_FALSE);
   elm_list_mode_set(list, ELM_LIST_EXPAND);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_notify_content_set(notify, list);
   evas_object_show(list);

   EINA_LIST_FOREACH(m->items, itr, it)
      elm_list_item_append(list, it->text, NULL, NULL, _popup_item_selected,
                           obj);
   elm_list_go(list);

   evas_object_show(notify);

   evas_object_smart_callback_add(obj, "popup,willdelete", _popup_will_delete,
                                  notify);
   evas_object_smart_callback_add(notify, "block,clicked", _popup_dismiss_cb,
                                  obj);
}

static void
_view_smart_callback_proxy_free_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   free(data);
}

static void
_view_smart_callback_proxy_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Web_Callback_Proxy_Context *ctxt = data;

   evas_object_smart_callback_call(ctxt->obj, ctxt->name, event_info);
}

static void
_view_smart_callback_proxy(Evas_Object *view, Evas_Object *parent)
{
   const Evas_Smart_Cb_Description **cls_descs, **inst_descs;
   unsigned int cls_count, inst_count, total;
   Elm_Web_Callback_Proxy_Context *ctxt;

   evas_object_smart_callbacks_descriptions_get(view, &cls_descs, &cls_count,
                                                &inst_descs, &inst_count);
   total = cls_count + inst_count;
   if (!total) return;
   ctxt = malloc(sizeof(Elm_Web_Callback_Proxy_Context) * total);
   if (!ctxt) return;
   evas_object_event_callback_add(view, EVAS_CALLBACK_FREE,
                                  _view_smart_callback_proxy_free_cb, ctxt);

   for (; cls_count > 0; cls_count--, cls_descs++, ctxt++)
     {
        const Evas_Smart_Cb_Description *d = *cls_descs;
        if (!strcmp(d->name, "popup,create"))
          continue;
        ctxt->name = d->name;
        ctxt->obj = parent;
        evas_object_smart_callback_add(view, d->name,
                                       _view_smart_callback_proxy_cb, ctxt);
     }

   for (; inst_count > 0; inst_count--, inst_descs++, ctxt++)
     {
        const Evas_Smart_Cb_Description *d = *inst_descs;
        ctxt->name = d->name;
        ctxt->obj = parent;
        evas_object_smart_callback_add(view, d->name,
                                       _view_smart_callback_proxy_cb, ctxt);
     }
}

static Eina_Bool
_bring_in_anim_cb(void *data, double pos)
{
   Widget_Data *wd = data;
   Evas_Object *frame = ewk_view_frame_main_get(wd->ewk_view);
   int sx, sy, rx, ry;

   sx = wd->bring_in.start.x;
   sy = wd->bring_in.start.y;
   rx = (wd->bring_in.end.x - sx) * pos;
   ry = (wd->bring_in.end.y - sy) * pos;

   ewk_frame_scroll_set(frame, rx + sx, ry + sy);

   if (pos == 1.0)
     {
        wd->bring_in.end.x = wd->bring_in.end.y = wd->bring_in.start.x =
           wd->bring_in.start.y = 0;
        wd->bring_in.animator = NULL;
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

EAPI Evas_Object *
elm_web_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Widget_Data *wd;
   Evas *e;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   wd = calloc(1, sizeof(Widget_Data));
   e = evas_object_evas_get(parent);
   if (!e)
     return NULL;
   obj = elm_widget_add(e);
   wd->self = obj;

   if (!widtype)
     {
        widtype = eina_stringshare_add("web");
        elm_widget_type_register(&widtype);
     }

   elm_widget_type_set(obj, widtype);
   elm_widget_sub_object_add(parent, obj);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_event_hook_set(obj, _event_hook);
   elm_widget_data_set(obj, wd);
   elm_widget_can_focus_set(obj, EINA_TRUE);

#ifdef HAVE_ELEMENTARY_WEB
   wd->ewk_view = _view_add(obj);
   ewk_view_setting_user_agent_set(wd->ewk_view, ELM_WEB_USER_AGENT);

   wd->input_method = ELM_WIN_KEYBOARD_OFF;
   evas_object_smart_callback_add(wd->ewk_view, "inputmethod,changed",
                                  _ewk_view_inputmethod_change_cb, wd);
   evas_object_smart_callback_add(wd->ewk_view, "load,started",
                                  _ewk_view_load_started_cb, wd);
   evas_object_smart_callback_add(wd->ewk_view, "popup,create",
                                  _ewk_view_popup_create_cb, wd);
   evas_object_smart_callback_add(wd->ewk_view, "load,finished",
                                  _ewk_view_load_finished_cb, wd);
   evas_object_smart_callback_add(wd->ewk_view, "viewport,changed",
                                  _ewk_view_viewport_changed_cb, wd);
   evas_object_smart_callback_add(wd->ewk_view, "view,resized",
                                  _ewk_view_resized_cb, wd);

   elm_widget_resize_object_set(obj, wd->ewk_view);

   wd->tab_propagate = EINA_FALSE;
   wd->inwin_mode = _elm_config->inwin_dialogs_enable;
   wd->zoom.min = ewk_view_zoom_range_min_get(wd->ewk_view);
   wd->zoom.max = ewk_view_zoom_range_max_get(wd->ewk_view);
   wd->zoom.current = 1.0;

   _view_smart_callback_proxy(wd->ewk_view, wd->self);
   evas_object_smart_callbacks_descriptions_set(obj, _elm_web_callback_names);

   _theme_hook(obj);

#else
   wd->label = elm_label_add(obj);
   elm_object_text_set(wd->label, "WebKit not supported!");
   evas_object_show(wd->label);
   elm_widget_resize_object_set(obj, wd->label);
#endif

   return obj;
}

EAPI Evas_Object *
elm_web_webkit_view_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->ewk_view;
#else
   ERR("Elementary not compiled with EWebKit support.");
   return NULL;
#endif
}

EAPI void
elm_web_window_create_hook_set(Evas_Object *obj, Elm_Web_Window_Open func, void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->hook.window_create = func;
   wd->hook.window_create_data = data;
#else
   (void)func;
   (void)data;
#endif
}

EAPI void
elm_web_dialog_alert_hook_set(Evas_Object *obj, Elm_Web_Dialog_Alert func, void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->hook.alert = func;
   wd->hook.alert_data = data;
#else
   (void)func;
   (void)data;
#endif
}

EAPI void
elm_web_dialog_confirm_hook_set(Evas_Object *obj, Elm_Web_Dialog_Confirm func, void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->hook.confirm = func;
   wd->hook.confirm_data = data;
#else
   (void)func;
   (void)data;
#endif
}

EAPI void
elm_web_dialog_prompt_hook_set(Evas_Object *obj, Elm_Web_Dialog_Prompt func, void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->hook.prompt = func;
   wd->hook.prompt_data = data;
#else
   (void)func;
   (void)data;
#endif
}

EAPI void
elm_web_dialog_file_selector_hook_set(Evas_Object *obj, Elm_Web_Dialog_File_Selector func, void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->hook.file_selector = func;
   wd->hook.file_selector_data = data;
#else
   (void)func;
   (void)data;
#endif
}

EAPI void
elm_web_console_message_hook_set(Evas_Object *obj, Elm_Web_Console_Message func, void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->hook.console_message = func;
   wd->hook.console_message_data = data;
#else
   (void)func;
   (void)data;
#endif
}

EAPI Eina_Bool
elm_web_tab_propagate_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->tab_propagate;
#else
   return EINA_FALSE;
#endif
}

EAPI void
elm_web_tab_propagate_set(Evas_Object *obj, Eina_Bool propagate)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->tab_propagate = propagate;
#else
   (void)propagate;
#endif
}

EAPI Eina_Bool
elm_web_uri_set(Evas_Object *obj, const char *uri)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_uri_set(wd->ewk_view, uri);
#else
   (void)uri;
   return EINA_FALSE;
#endif
}

EAPI const char *
elm_web_uri_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return ewk_view_uri_get(wd->ewk_view);
#else
   return NULL;
#endif
}

EAPI const char *
elm_web_title_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return ewk_view_title_get(wd->ewk_view);
#else
   return NULL;
#endif
}

EAPI void
elm_web_bg_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   ewk_view_bg_color_set(wd->ewk_view, r, g, b, a);
#else
   (void)r;
   (void)g;
   (void)b;
   (void)a;
#endif
}

EAPI void
elm_web_bg_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a)
{
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   return ewk_view_bg_color_get(wd->ewk_view, r, g, b, a);
#endif
}

EAPI char *
elm_web_selection_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return ewk_view_selection_get(wd->ewk_view);
#else
   return NULL;
#endif
}

EAPI void
elm_web_popup_selected_set(Evas_Object *obj, int index)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   ewk_view_popup_selected_set(wd->ewk_view, index);
#else
   (void)index;
#endif
}

EAPI Eina_Bool
elm_web_popup_destroy(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   return ewk_view_popup_destroy(wd->ewk_view);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_text_search(const Evas_Object *obj, const char *string, Eina_Bool case_sensitive, Eina_Bool forward, Eina_Bool wrap)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_text_search
     (wd->ewk_view, string, case_sensitive, forward, wrap);
#else
   (void)string;
   (void)case_sensitive;
   (void)forward;
   (void)wrap;
   return EINA_FALSE;
#endif
}

EAPI unsigned int
elm_web_text_matches_mark(Evas_Object *obj, const char *string, Eina_Bool case_sensitive, Eina_Bool highlight, unsigned int limit)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return ewk_view_text_matches_mark
     (wd->ewk_view, string, case_sensitive, highlight, limit);
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
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_text_matches_unmark_all(wd->ewk_view);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_text_matches_highlight_set(Evas_Object *obj, Eina_Bool highlight)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_text_matches_highlight_set(wd->ewk_view, highlight);
#else
   (void)highlight;
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_text_matches_highlight_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_text_matches_highlight_get(wd->ewk_view);
#else
   return EINA_FALSE;
#endif
}

EAPI double
elm_web_load_progress_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) -1.0;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return -1.0;
   return ewk_view_load_progress_get(wd->ewk_view);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_stop(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_stop(wd->ewk_view);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_reload(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_reload(wd->ewk_view);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_reload_full(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_reload_full(wd->ewk_view);
#else
   return EINA_FALSE;
#endif
}


EAPI Eina_Bool
elm_web_back(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_back(wd->ewk_view);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_forward(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_forward(wd->ewk_view);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_navigate(Evas_Object *obj, int steps)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_navigate(wd->ewk_view, steps);
#else
   return EINA_FALSE;
   (void)steps;
#endif
}

EAPI Eina_Bool
elm_web_back_possible(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_back_possible(wd->ewk_view);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_forward_possible(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_forward_possible(wd->ewk_view);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_web_navigate_possible(Evas_Object *obj, int steps)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_navigate_possible(wd->ewk_view, steps);
#else
   return EINA_FALSE;
   (void)steps;
#endif
}

EAPI Eina_Bool
elm_web_history_enable_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_history_enable_get(wd->ewk_view);
#else
   return EINA_FALSE;
#endif
}

EAPI void
elm_web_history_enable_set(Evas_Object *obj, Eina_Bool enable)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   ewk_view_history_enable_set(wd->ewk_view, enable);
#else
   (void)enable;
#endif
}

//EAPI Ewk_History *ewk_view_history_get(const Evas_Object *obj); // TODO:

EAPI void
elm_web_zoom_set(Evas_Object *obj, double zoom)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   int vw, vh, cx, cy;
   float z = 1.0;
   evas_object_geometry_get(wd->ewk_view, NULL, NULL, &vw, &vh);
   cx = vw / 2;
   cy = vh / 2;
   if (zoom > wd->zoom.max)
     zoom = wd->zoom.max;
   else if (zoom < wd->zoom.min)
     zoom = wd->zoom.min;
   if (zoom == wd->zoom.current) return;
   wd->zoom.current = zoom;
   if (wd->zoom.mode == ELM_WEB_ZOOM_MODE_MANUAL)
     z = zoom;
   else if (wd->zoom.mode == ELM_WEB_ZOOM_MODE_AUTO_FIT)
     {
        Evas_Object *frame = ewk_view_frame_main_get(wd->ewk_view);
        Evas_Coord fw, fh, pw, ph;
        if (!ewk_frame_contents_size_get(frame, &fw, &fh))
          return;
        z = ewk_frame_zoom_get(frame);
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
   else if (wd->zoom.mode == ELM_WEB_ZOOM_MODE_AUTO_FILL)
     {
        Evas_Object *frame = ewk_view_frame_main_get(wd->ewk_view);
        Evas_Coord fw, fh, pw, ph;
        if (!ewk_frame_contents_size_get(frame, &fw, &fh))
          return;
        z = ewk_frame_zoom_get(frame);
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
   if (wd->zoom.no_anim)
     ewk_view_zoom_set(wd->ewk_view, z, cx, cy);
   else
     ewk_view_zoom_animated_set(wd->ewk_view, z, _elm_config->zoom_friction,
                                cx, cy);
   wd->zoom.no_anim = EINA_FALSE;
#else
   (void)zoom;
#endif
}

EAPI double
elm_web_zoom_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) -1.0;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->zoom.current;
#else
   return -1.0;
#endif
}

EAPI void
elm_web_zoom_mode_set(Evas_Object *obj, Elm_Web_Zoom_Mode mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   float tz;
   if (mode >= ELM_WEB_ZOOM_MODE_LAST)
     return;
   if (mode == wd->zoom.mode)
     return;
   wd->zoom.mode = mode;
   tz = wd->zoom.current;
   wd->zoom.current = 0.0;
   elm_web_zoom_set(obj, tz);
#else
   (void)mode;
#endif
}

EAPI Elm_Web_Zoom_Mode
elm_web_zoom_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_WEB_ZOOM_MODE_LAST;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->zoom.mode;
#else
   return ELM_WEB_ZOOM_MODE_LAST;
#endif
}

EAPI Eina_Bool
elm_web_zoom_text_only_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return ewk_view_zoom_text_only_get(wd->ewk_view);
#else
   return EINA_FALSE;
#endif
}

EAPI void
elm_web_zoom_text_only_set(Evas_Object *obj, Eina_Bool setting)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   ewk_view_zoom_text_only_set(wd->ewk_view, setting);
#else
   (void)setting;
#endif
}

EAPI void
elm_web_region_show(Evas_Object *obj, int x, int y, int w __UNUSED__, int h __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *frame = ewk_view_frame_main_get(wd->ewk_view);
   int fw, fh, zw, zh, rx, ry;
   float zoom;
   ewk_frame_contents_size_get(frame, &fw, &fh);
   zoom = ewk_frame_zoom_get(frame);
   zw = fw / zoom;
   zh = fh / zoom;
   rx = (x * fw) / zw;
   ry = (y * fh) / zh;
   if (wd->bring_in.animator)
     {
        ecore_animator_del(wd->bring_in.animator);
        wd->bring_in.animator = NULL;
     }
   ewk_frame_scroll_set(frame, rx, ry);
#else
   (void)x;
   (void)y;
#endif
}

EAPI void
elm_web_region_bring_in(Evas_Object *obj, int x, int y, int w __UNUSED__, int h __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *frame = ewk_view_frame_main_get(wd->ewk_view);
   int fw, fh, zw, zh, rx, ry, sx, sy;
   float zoom;
   ewk_frame_contents_size_get(frame, &fw, &fh);
   ewk_frame_scroll_pos_get(frame, &sx, &sy);
   zoom = ewk_frame_zoom_get(frame);
   zw = fw / zoom;
   zh = fh / zoom;
   rx = (x * fw) / zw;
   ry = (y * fh) / zh;
   if ((wd->bring_in.end.x == rx) && (wd->bring_in.end.y == ry))
     return;
   wd->bring_in.start.x = sx;
   wd->bring_in.start.y = sy;
   wd->bring_in.end.x = rx;
   wd->bring_in.end.y = ry;
   if (wd->bring_in.animator)
     ecore_animator_del(wd->bring_in.animator);
   wd->bring_in.animator = ecore_animator_timeline_add(
      _elm_config->bring_in_scroll_friction, _bring_in_anim_cb, wd);
#else
   (void)x;
   (void)y;
#endif
}

EAPI void
elm_web_inwin_mode_set(Evas_Object *obj, Eina_Bool value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);

   wd->inwin_mode = value;
#else
   (void)value;
#endif
}

EAPI Eina_Bool
elm_web_inwin_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
#ifdef HAVE_ELEMENTARY_WEB
   Widget_Data *wd = elm_widget_data_get(obj);

   return wd->inwin_mode;
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

EAPI void
elm_web_window_features_bool_property_get(const Elm_Web_Window_Features *wf, Eina_Bool *toolbar_visible, Eina_Bool *statusbar_visible, Eina_Bool *scrollbars_visible, Eina_Bool *menubar_visible, Eina_Bool *locationbar_visible, Eina_Bool *fullscreen)
{
#ifdef HAVE_ELEMENTARY_WEB
   ewk_window_features_bool_property_get((const Ewk_Window_Features *)wf,
                                         toolbar_visible, statusbar_visible,
                                         scrollbars_visible, menubar_visible,
                                         locationbar_visible, fullscreen);
#else
   (void)wf;
   (void)toolbar_visible;
   (void)statusbar_visible;
   (void)scrollbars_visible;
   (void)menubar_visible;
   (void)locationbar_visible;
   (void)fullscreen;
#endif
}

EAPI void
elm_web_window_features_int_property_get(const Elm_Web_Window_Features *wf, int *x, int *y, int *w, int *h)
{
#ifdef HAVE_ELEMENTARY_WEB
   ewk_window_features_int_property_get((const Ewk_Window_Features *)wf,
                                        x, y, w, h);
#else
   (void)wf;
   (void)x;
   (void)y;
   (void)w;
   (void)h;
#endif
}

// TODO: use all ewk_view_zoom stuff to implement bring-in and animated zoom like elm_photocam. Should be simple to use, must not expose every single bit to users!

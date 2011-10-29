#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

typedef struct
{
   Evas_Object *web;
   Evas_Object *btn_back;
   Evas_Object *btn_fwd;
   Evas_Object *url_entry;
   Eina_List *sub_wins;
   Eina_Bool js_hooks : 1;
} Web_Test;

static void
_btn_back_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *web = data;

   elm_web_back(web);
}

static void
_btn_fwd_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *web = data;

   elm_web_forward(web);
}

static void
_btn_reload_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *web = data;

   elm_web_reload(web);
}

static void
_url_change_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *web = data;
   const char *uri = elm_object_text_get(obj);

   elm_web_uri_set(web, uri);
}

static void
_toggle_inwin_mode_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_web_inwin_mode_set(data, !elm_web_inwin_mode_get(data));
}

static void
_title_changed_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   char buf[512];
   snprintf(buf, sizeof(buf), "Web - %s", (const char *)event_info);
   elm_win_title_set(data, buf);
}

static void
_uri_changed_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Web_Test *wt = data;

   elm_entry_entry_set(wt->url_entry, event_info);

   elm_object_disabled_set(wt->btn_back, !elm_web_back_possible(wt->web));
   elm_object_disabled_set(wt->btn_fwd, !elm_web_forward_possible(wt->web));
}

static void
_new_win_del_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Web_Test *wt = data;
   wt->sub_wins = eina_list_remove(wt->sub_wins, obj);
}

static void
_web_win_close_request_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_del(data);
}

static Evas_Object *
_new_window_hook(void *data, Evas_Object *obj __UNUSED__, Eina_Bool js __UNUSED__, const Elm_Web_Window_Features *wf __UNUSED__)
{
   Web_Test *wt = data;
   Evas_Object *new_win, *new_web, *bg;

   new_win = elm_win_add(NULL, "elm-web-test-popup", ELM_WIN_BASIC);
   elm_win_autodel_set(new_win, EINA_TRUE);
   evas_object_resize(new_win, 300, 300);
   evas_object_show(new_win);

   bg = elm_bg_add(new_win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(new_win, bg);
   evas_object_show(bg);

   new_web = elm_web_add(new_win);
   evas_object_size_hint_weight_set(new_web, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   elm_win_resize_object_add(new_win, new_web);
   evas_object_show(new_web);

   evas_object_smart_callback_add(new_win, "delete,request", _new_win_del_cb,
                                  wt);
   evas_object_smart_callback_add(new_web, "windows,close,request",
                                  _web_win_close_request_cb, new_win);
   wt->sub_wins = eina_list_append(wt->sub_wins, new_win);

   return new_web;
}

static void
_alert_del(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   evas_object_del(obj);
}

static Evas_Object *
_alert_hook(void *data __UNUSED__, Evas_Object *obj, const char *message)
{
   Evas_Object *popup, *label;

   popup = elm_notify_add(obj);
   elm_notify_orient_set(popup, ELM_NOTIFY_ORIENT_CENTER);
   // Using the timeout doesn't seem to go well with the second main loop
   //elm_notify_timeout_set(popup, 2.0);
   elm_notify_repeat_events_set(popup, EINA_FALSE);
   evas_object_show(popup);

   evas_object_smart_callback_add(popup, "block,clicked", _alert_del, NULL);

   label = elm_label_add(obj);
   elm_object_text_set(label, message);
   elm_object_content_set(popup, label);
   evas_object_show(label);

   return popup;
}

static void
_confirm_ok_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Eina_Bool *response = data;
   *response = EINA_TRUE;
}

static void
_confirm_cancel_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Eina_Bool *response = data;
   *response = EINA_FALSE;
}

static void
_confirm_dismiss_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_del(data);
}

static Evas_Object *
_confirm_hook(void *data __UNUSED__, Evas_Object *obj, const char *message, Eina_Bool *response)
{
   Evas_Object *popup, *box, *box2, *label, *btn_ok, *btn_cancel;

   popup = elm_notify_add(obj);
   elm_notify_orient_set(popup, ELM_NOTIFY_ORIENT_CENTER);
   elm_notify_repeat_events_set(popup, EINA_FALSE);
   evas_object_show(popup);

   box = elm_box_add(obj);
   elm_object_content_set(popup, box);
   evas_object_show(box);

   label = elm_label_add(obj);
   elm_object_text_set(label, message);
   elm_box_pack_end(box, label);
   evas_object_show(label);

   box2 = elm_box_add(obj);
   elm_box_horizontal_set(box2, EINA_TRUE);
   elm_box_pack_end(box, box2);
   evas_object_show(box2);

   btn_ok = elm_button_add(obj);
   elm_object_text_set(btn_ok, "Ok");
   elm_box_pack_end(box2, btn_ok);
   evas_object_show(btn_ok);

   btn_cancel = elm_button_add(obj);
   elm_object_text_set(btn_cancel, "Cancel");
   elm_box_pack_end(box2, btn_cancel);
   evas_object_show(btn_cancel);

   evas_object_smart_callback_add(btn_ok, "clicked", _confirm_dismiss_cb,
                                  popup);
   evas_object_smart_callback_add(btn_cancel, "clicked", _confirm_dismiss_cb,
                                  popup);
   evas_object_smart_callback_add(btn_ok, "clicked", _confirm_ok_cb, response);
   evas_object_smart_callback_add(btn_cancel, "clicked", _confirm_cancel_cb,
                                  response);

   return popup;
}

static Evas_Object *
_prompt_hook(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *message __UNUSED__, const char *default_value, char **value, Eina_Bool *response)
{
   *response = EINA_TRUE;
   *value = default_value ? strdup(default_value) : "No default!";
   return NULL;
}

static Evas_Object *
_file_selector_hook(void *data __UNUSED__, Evas_Object *obj __UNUSED__, Eina_Bool allow_multiple __UNUSED__, Eina_List *accept_types __UNUSED__, Eina_List **selected_files, Eina_Bool *response)
{
   *selected_files = eina_list_append(NULL,
                                      strdup("/path/to/non_existing_file"));
   *response = EINA_TRUE;
   return NULL;
}

static void
_console_message_hook(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *message, unsigned int line_number, const char *source_id)
{
   printf("CONSOLE: %s:%d:%s\n", source_id, line_number, message);
}

static void
_js_popup_hooks_set(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Web_Test *wt = data;

   wt->js_hooks = !wt->js_hooks;
   if (wt->js_hooks)
     {
        elm_web_dialog_alert_hook_set(wt->web, _alert_hook, NULL);
        elm_web_dialog_confirm_hook_set(wt->web, _confirm_hook, NULL);
        elm_web_dialog_prompt_hook_set(wt->web, _prompt_hook, NULL);
        elm_web_dialog_file_selector_hook_set(wt->web, _file_selector_hook,
                                              NULL);
        elm_web_console_message_hook_set(wt->web, _console_message_hook, NULL);
     }
   else
     {
        elm_web_dialog_alert_hook_set(wt->web, NULL, NULL);
        elm_web_dialog_confirm_hook_set(wt->web, NULL, NULL);
        elm_web_dialog_prompt_hook_set(wt->web, NULL, NULL);
        elm_web_dialog_file_selector_hook_set(wt->web, NULL, NULL);
        elm_web_console_message_hook_set(wt->web, NULL, NULL);
     }
}

static void
_zoom_out_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Web_Test *wt = data;
   double zoom;

   zoom = elm_web_zoom_get(wt->web);
   if (zoom > 1)
     zoom -= .5;
   else
     zoom /= 2;
   if (zoom < .05)
     zoom = .05;
   elm_web_zoom_set(wt->web, zoom);
}

static void
_zoom_in_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Web_Test *wt = data;
   double zoom;

   zoom = elm_web_zoom_get(wt->web);

   if (zoom < 1)
     zoom *= 2;
   else
     zoom += .5;
   if (zoom > 4)
     zoom = 4;
   elm_web_zoom_set(wt->web, zoom);
}

static void
_zoom_mode_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Web_Test *wt = data;
   Elm_Hoversel_Item *it = event_info;
   const char *lbl = elm_hoversel_item_label_get(it);

   if (!strcmp(lbl, "Manual"))
     elm_web_zoom_mode_set(wt->web, ELM_WEB_ZOOM_MODE_MANUAL);
   else if (!strcmp(lbl, "Fit"))
     elm_web_zoom_mode_set(wt->web, ELM_WEB_ZOOM_MODE_AUTO_FIT);
   else
     elm_web_zoom_mode_set(wt->web, ELM_WEB_ZOOM_MODE_AUTO_FILL);
}

static void
_show_region_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Web_Test *wt = data;
   elm_web_region_show(wt->web, 300, 300, 1, 1);
}

static void
_bring_in_region_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Web_Test *wt = data;
   elm_web_region_bring_in(wt->web, 50, 0, 1, 1);
}

static void
_main_web_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Web_Test *wt = data;
   Evas_Object *sub_win;

   EINA_LIST_FREE(wt->sub_wins, sub_win)
      evas_object_del(sub_win);

   free(wt);
}

void
test_web(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bx2, *bt, *web, *url;
   Web_Test *wt;

   elm_need_web();

   wt = calloc(1, sizeof(*wt));

   win = elm_win_add(NULL, "web", ELM_WIN_BASIC);
   elm_win_title_set(win, "Web");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   web = elm_web_add(win);
   evas_object_size_hint_weight_set(web, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(web, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, web);
   evas_object_show(web);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "<");
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   evas_object_smart_callback_add(bt, "clicked", _btn_back_cb, web);
   wt->btn_back = bt;

   bt = elm_button_add(win);
   elm_object_text_set(bt, "R");
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   evas_object_smart_callback_add(bt, "clicked", _btn_reload_cb, web);

   bt = elm_button_add(win);
   elm_object_text_set(bt, ">");
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   evas_object_smart_callback_add(bt, "clicked", _btn_fwd_cb, web);
   wt->btn_fwd = bt;

   url = elm_entry_add(win);
   elm_entry_single_line_set(url, EINA_TRUE);
   elm_entry_scrollable_set(url, EINA_TRUE);
   evas_object_size_hint_weight_set(url, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(url, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx2, url);
   evas_object_show(url);

   evas_object_smart_callback_add(url, "activated", _url_change_cb, web);
   wt->url_entry = url;

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Inwin Mode");
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   evas_object_smart_callback_add(bt, "clicked", _toggle_inwin_mode_cb, web);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Custom Hooks");
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   evas_object_smart_callback_add(bt, "clicked", _js_popup_hooks_set, wt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "-");
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   evas_object_smart_callback_add(bt, "clicked", _zoom_out_cb, wt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "+");
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   evas_object_smart_callback_add(bt, "clicked", _zoom_in_cb, wt);

   bt = elm_hoversel_add(win);
   elm_object_text_set(bt, "Zoom Mode");
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_hoversel_item_add(bt, "Manual", NULL, ELM_ICON_NONE, _zoom_mode_cb, wt);
   elm_hoversel_item_add(bt, "Fit", NULL, ELM_ICON_NONE, _zoom_mode_cb, wt);
   elm_hoversel_item_add(bt, "Fill", NULL, ELM_ICON_NONE, _zoom_mode_cb, wt);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Show 300, 300");
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   evas_object_smart_callback_add(bt, "clicked", _show_region_cb, wt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Bring in 50, 0");
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   evas_object_smart_callback_add(bt, "clicked", _bring_in_region_cb, wt);

   evas_object_smart_callback_add(web, "title,changed", _title_changed_cb, win);
   evas_object_smart_callback_add(web, "uri,changed", _uri_changed_cb, wt);

   evas_object_event_callback_add(web, EVAS_CALLBACK_DEL, _main_web_del_cb, wt);

   wt->web = web;

   elm_web_uri_set(web, "http://www.enlightenment.org");

   elm_web_window_create_hook_set(web, _new_window_hook, wt);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
#endif

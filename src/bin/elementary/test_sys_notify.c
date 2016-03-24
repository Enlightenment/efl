#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define WIDTH  320
#define HEIGHT 160

struct _Sys_Notify_Data {
    Evas_Object *l;
    Evas_Object *n;
    Ecore_Event_Handler *handlers[2];
};
typedef struct _Sys_Notify_Data Sys_Notify_Data;

static Eina_Bool
_ev_handler(void *data EINA_UNUSED,
            int type,
            void *event)
{
   Elm_Sys_Notify_Notification_Closed *closed;
   Elm_Sys_Notify_Action_Invoked *action;

   if (type == ELM_EVENT_SYS_NOTIFY_NOTIFICATION_CLOSED)
     {
        closed = event;
        printf("Notification Closed Event: %u %d.\n",
               closed->id, closed->reason);
     }
   else if (type == ELM_EVENT_SYS_NOTIFY_ACTION_INVOKED)
     {
        action = event;
        printf("Notification Action Event: %u %s.\n",
               action->id, action->action_key);
     }
   else
     return ECORE_CALLBACK_PASS_ON;

   return ECORE_CALLBACK_DONE;
}

static void _sys_notify_cb(void *data, unsigned int id EINA_UNUSED)
{
   Sys_Notify_Data *notify_data = data;

   elm_object_text_set(notify_data->l, "notify is done");
   evas_object_show(notify_data->n);
}

static void
_bt_clicked(void *data,
            Evas_Object *obj,
            void *event_info EINA_UNUSED)
{
   Evas_Object *s, *b;
   s = evas_object_data_get(obj, "summary");
   b = evas_object_data_get(obj, "body");

   elm_sys_notify_send(0, "", elm_entry_entry_get(s), elm_entry_entry_get(b),
                       ELM_SYS_NOTIFY_URGENCY_NORMAL,
                       -1, _sys_notify_cb, data);
}

static void
_test_sys_notify_win_del_cb(void *data,
                            Evas *e EINA_UNUSED,
                            Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   Sys_Notify_Data *notify_data = data;
   ecore_event_handler_del(notify_data->handlers[0]);
   ecore_event_handler_del(notify_data->handlers[1]);

   free(notify_data);
}

void
test_sys_notify(void *data EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *it, *btn;
   Evas_Object *s, *b, *n, *l;
   Sys_Notify_Data *notify_data;

   elm_need_sys_notify();

   notify_data = malloc(sizeof(Sys_Notify_Data));

   win = elm_win_add(NULL, "Sys Notify", ELM_WIN_BASIC);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_title_set(win, "System Notification");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL,
                                  _test_sys_notify_win_del_cb, notify_data);

   it = elm_bg_add(win);
   elm_win_resize_object_add(win, it);
   evas_object_size_hint_min_set(it, WIDTH, HEIGHT);
   evas_object_size_hint_max_set(it, WIDTH, HEIGHT);
   evas_object_show(it);

   notify_data->n = n = elm_notify_add(win);
   evas_object_size_hint_weight_set(n, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_notify_align_set(n, 0.5, 0.0);
   elm_notify_timeout_set(n, 2.0);

   notify_data->l = l = elm_label_add(win);
   elm_object_content_set(n, l);
   evas_object_show(l);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   s = elm_entry_add(win);
   elm_entry_single_line_set(s, EINA_TRUE);
   elm_entry_scrollable_set(s, EINA_TRUE);
   elm_entry_entry_set(s, "Summary");
   evas_object_size_hint_align_set(s, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, s);
   evas_object_show(s);

   b = elm_entry_add(win);
   elm_entry_single_line_set(b, EINA_TRUE);
   elm_entry_scrollable_set(b, EINA_TRUE);
   elm_entry_entry_set(b, "Body long description.");
   evas_object_size_hint_align_set(b, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, b);
   evas_object_show(b);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Send Notification");
   evas_object_smart_callback_add(btn, "clicked", _bt_clicked, notify_data);
   evas_object_data_set(btn, "summary", s);
   evas_object_data_set(btn, "body", b);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   evas_object_resize(win, WIDTH, HEIGHT);
   evas_object_show(win);

   notify_data->handlers[0] = ecore_event_handler_add(ELM_EVENT_SYS_NOTIFY_NOTIFICATION_CLOSED,
                                                      _ev_handler, notify_data);
   notify_data->handlers[1] = ecore_event_handler_add(ELM_EVENT_SYS_NOTIFY_ACTION_INVOKED,
                                                      _ev_handler, notify_data);
}

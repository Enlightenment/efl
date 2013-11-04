#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>


#define WIDTH  320
#define HEIGHT 160

static Evas_Object *s = NULL;
static Evas_Object *b = NULL;
static Evas_Object *l = NULL;
static Evas_Object *n = NULL;

static Eina_Bool
_ev_handler(void *data EINA_UNUSED,
            int type,
            void *event)
{
   char msg[256];

   Elm_Sys_Notify_Notification_Closed *closed;
   Elm_Sys_Notify_Action_Invoked *action;

   if (type == ELM_EVENT_SYS_NOTIFY_NOTIFICATION_CLOSED)
     {
        closed = event;
        sprintf(msg, "Notification Closed Event: %u %d.",
               closed->id, closed->reason);
     }
   else if (type == ELM_EVENT_SYS_NOTIFY_ACTION_INVOKED)
     {
        action = event;
        sprintf(msg, "Notification Action Event: %u %s.",
               action->id, action->action_key);
     }
   else
     return ECORE_CALLBACK_PASS_ON;

   elm_object_text_set(l, msg);
   evas_object_show(n);

   return ECORE_CALLBACK_DONE;
}

static void
_bt_clicked(void *data EINA_UNUSED,
            Evas_Object *obj EINA_UNUSED,
            void *event_info EINA_UNUSED)
{
   elm_sys_notify_simple_send
      ("", elm_entry_entry_get(s), elm_entry_entry_get(b));
}

void
test_sys_notify(void *data EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *it;

   elm_need_sys_notify();

   ecore_event_handler_add(ELM_EVENT_SYS_NOTIFY_NOTIFICATION_CLOSED,
                           _ev_handler, NULL);

   ecore_event_handler_add(ELM_EVENT_SYS_NOTIFY_ACTION_INVOKED,
                           _ev_handler, NULL);

   win = elm_win_add(NULL, "Sys Notify", ELM_WIN_BASIC);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_title_set(win, "System Notification");
   elm_win_autodel_set(win, EINA_TRUE);

   it = elm_bg_add(win);
   elm_win_resize_object_add(win, it);
   evas_object_size_hint_min_set(it, WIDTH, HEIGHT);
   evas_object_size_hint_max_set(it, WIDTH, HEIGHT);
   evas_object_show(it);

   n = elm_notify_add(win);
   evas_object_size_hint_weight_set(n, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_notify_align_set(n, 0.5, 0.0);
   elm_notify_timeout_set(n, 2.0);

   l = elm_label_add(win);
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

   it = elm_button_add(win);
   elm_object_text_set(it, "Send Notification");
   evas_object_smart_callback_add(it, "clicked", _bt_clicked, NULL);
   elm_box_pack_end(bx, it);
   evas_object_show(it);

   evas_object_resize(win, WIDTH, HEIGHT);
   evas_object_show(win);
}

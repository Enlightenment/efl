/*
 * gcc -o button_example_01 button_example_01.c `pkg-config --cflags --libs elementary`
 */
#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

typedef struct
{
   Evas_Object *mid;
   Evas_Object *icon_still;
   struct {
      Evas_Object *up;
      Evas_Object *down;
      Evas_Object *left;
      Evas_Object *right;
   } cursors;
} App_Data;

static void
_btn_cursors_release_cb(void *data, Evas_Object *btn __UNUSED__, void *ev __UNUSED__)
{
   App_Data *app = data;
   elm_object_content_set(app->mid, app->icon_still);
   app->icon_still = NULL;
}

static void
_btn_cursors_move_cb(void *data, Evas_Object *btn, void *ev __UNUSED__)
{
   App_Data *app = data;
   double ax, ay;

   if (!app->icon_still)
     {
        Evas_Object *icon;
        app->icon_still = elm_object_content_unset(app->mid);
        icon = elm_icon_add(app->mid);
        elm_icon_standard_set(icon, "chat");
        elm_object_content_set(app->mid, icon);
     }

   evas_object_size_hint_align_get(app->mid, &ax, &ay);
   if (btn == app->cursors.up)
     {
        ay -= 0.05;
        if (ay < 0.0)
          ay = 0.0;
     }
   else if (btn == app->cursors.down)
     {
        ay += 0.05;
        if (ay > 1.0)
          ay = 1.0;
     }
   else if (btn == app->cursors.left)
     {
        ax -= 0.05;
        if (ax < 0.0)
          ax = 0.0;
     }
   else if (btn == app->cursors.right)
     {
        ax += 0.05;
        if (ax > 1.0)
          ax = 1.0;
     }
   evas_object_size_hint_align_set(app->mid, ax, ay);
}

static void
_btn_options_cb(void *data, Evas_Object *btn, void *ev __UNUSED__)
{
   char *ptr;
   double t;
   App_Data *app = data;
   const char *lbl = elm_button_label_get(btn);

   ptr = strchr(lbl, ':');
   ptr += 2;
   t = strtod(ptr, NULL);

   if (!strncmp(lbl, "Initial", 7))
     {
        elm_button_autorepeat_initial_timeout_set(app->cursors.up, t);
        elm_button_autorepeat_initial_timeout_set(app->cursors.down, t);
        elm_button_autorepeat_initial_timeout_set(app->cursors.left, t);
        elm_button_autorepeat_initial_timeout_set(app->cursors.right, t);
     }
   else if (!strncmp(lbl, "Gap", 3))
     {
        elm_button_autorepeat_gap_timeout_set(app->cursors.up, t);
        elm_button_autorepeat_gap_timeout_set(app->cursors.down, t);
        elm_button_autorepeat_gap_timeout_set(app->cursors.left, t);
        elm_button_autorepeat_gap_timeout_set(app->cursors.right, t);
     }
}

int
elm_main(int argc __UNUSED__, char *argv[] __UNUSED__)
{
   Evas_Object *win, *bg, *box, *box2, *btn, *icon;
   static App_Data data;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_add(NULL, "Button example", ELM_WIN_BASIC);
   elm_win_title_set(win, "Button example");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 300, 320);
   evas_object_show(win);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   box2 = elm_box_add(win);
   elm_box_horizontal_set(box2, EINA_TRUE);
   evas_object_size_hint_weight_set(box2, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(box, box2);
   evas_object_show(box2);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Initial: 0.0");
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _btn_options_cb, &data);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Initial: 1.0");
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _btn_options_cb, &data);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Initial: 5.0");
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _btn_options_cb, &data);

   box2 = elm_box_add(win);
   elm_box_horizontal_set(box2, EINA_TRUE);
   evas_object_size_hint_weight_set(box2, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(box, box2);
   evas_object_show(box2);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Gap: 0.1");
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _btn_options_cb, &data);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Gap: 0.5");
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _btn_options_cb, &data);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Gap: 1.0");
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _btn_options_cb, &data);

   btn = elm_button_add(win);
   elm_button_autorepeat_set(btn, EINA_TRUE);
   elm_button_autorepeat_initial_timeout_set(btn, 1.0);
   elm_button_autorepeat_gap_timeout_set(btn, 0.5);
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "repeated", _btn_cursors_move_cb, &data);
   evas_object_smart_callback_add(btn, "unpressed", _btn_cursors_release_cb,
                                  &data);

   icon = elm_icon_add(win);
   elm_icon_standard_set(icon, "arrow_up");
   elm_object_content_set(btn, icon);

   data.cursors.up = btn;

   box2 = elm_box_add(win);
   elm_box_horizontal_set(box2, EINA_TRUE);
   evas_object_size_hint_weight_set(box2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, box2);
   evas_object_show(box2);

   btn = elm_button_add(win);
   elm_button_autorepeat_set(btn, EINA_TRUE);
   elm_button_autorepeat_initial_timeout_set(btn, 1.0);
   elm_button_autorepeat_gap_timeout_set(btn, 0.5);
   evas_object_size_hint_weight_set(btn, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn, 0.0, EVAS_HINT_FILL);
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "repeated", _btn_cursors_move_cb, &data);
   evas_object_smart_callback_add(btn, "unpressed", _btn_cursors_release_cb,
                                  &data);

   icon = elm_icon_add(win);
   elm_icon_standard_set(icon, "arrow_left");
   elm_object_content_set(btn, icon);

   data.cursors.left = btn;

   btn = elm_button_add(win);
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);

   icon = elm_icon_add(win);
   elm_icon_standard_set(icon, "close");
   elm_object_content_set(btn, icon);

   data.mid = btn;

   btn = elm_button_add(win);
   elm_button_autorepeat_set(btn, EINA_TRUE);
   elm_button_autorepeat_initial_timeout_set(btn, 1.0);
   elm_button_autorepeat_gap_timeout_set(btn, 0.5);
   evas_object_size_hint_weight_set(btn, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn, 0.0, EVAS_HINT_FILL);
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "repeated", _btn_cursors_move_cb, &data);
   evas_object_smart_callback_add(btn, "unpressed", _btn_cursors_release_cb,
                                  &data);

   icon = elm_icon_add(win);
   elm_icon_standard_set(icon, "arrow_right");
   elm_object_content_set(btn, icon);

   data.cursors.right = btn;

   btn = elm_button_add(win);
   elm_button_autorepeat_set(btn, EINA_TRUE);
   elm_button_autorepeat_initial_timeout_set(btn, 1.0);
   elm_button_autorepeat_gap_timeout_set(btn, 0.5);
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "repeated", _btn_cursors_move_cb, &data);
   evas_object_smart_callback_add(btn, "unpressed", _btn_cursors_release_cb,
                                  &data);

   icon = elm_icon_add(win);
   elm_icon_standard_set(icon, "arrow_down");
   elm_object_content_set(btn, icon);

   data.cursors.down = btn;

   elm_run();

   return 0;
}
ELM_MAIN();

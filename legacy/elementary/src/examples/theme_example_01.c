/*
 * gcc -o theme_example_01 theme_example_01.c `pkg-config --cflags --libs elementary`
 */
#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static void
btn_extension_click_cb(void *data __UNUSED__, Evas_Object *btn, void *ev __UNUSED__)
{
   const char *lbl = elm_button_label_get(btn);

   if (!strncmp(lbl, "Load", 4))
     {
        elm_theme_extension_add(NULL, "./theme_example.edj");
        elm_button_label_set(btn, "Unload extension");
     }
   else if (!strncmp(lbl, "Unload", 6))
     {
        elm_theme_extension_del(NULL, "./theme_example.edj");
        elm_button_label_set(btn, "Load extension");
     }
}

static void
btn_style_click_cb(void *data __UNUSED__, Evas_Object *btn, void *ev __UNUSED__)
{
   const char *styles[] = {
        "chucknorris",
        "default",
        "anchor"
   };
   static int sel_style = 0;

   sel_style = (sel_style + 1) % 3;
   elm_object_style_set(btn, styles[sel_style]);
}

int
elm_main(int argc __UNUSED__, char *argv[] __UNUSED__)
{
   Evas_Object *win, *bg, *box, *btn;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   elm_theme_extension_add(NULL, "./theme_example.edj");

   win = elm_win_add(NULL, "Theme example", ELM_WIN_BASIC);
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

   btn = elm_button_add(win);
   elm_button_label_set(btn, "Unload extension");
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", btn_extension_click_cb, NULL);

   btn = elm_button_add(win);
   elm_button_label_set(btn, "Switch style");
   elm_object_style_set(btn, "chucknorris");
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", btn_style_click_cb, NULL);

   elm_run();

   return 0;
}
ELM_MAIN();

/*
 * gcc -o theme_example_02 theme_example_02.c `pkg-config --cflags --libs elememntaru`
 */
#include <Elementary.h>

static void
_btn_clicked_cb(void *data, Evas_Object *obj, void *ev)
{
   static int loaded = 1;
   if (loaded)
     elm_theme_overlay_del(NULL, "./theme_example.edj");
   else
     elm_theme_overlay_add(NULL, "./theme_example.edj");
   loaded = 1 - loaded;
}

EAPI_MAIN int
elm_main(int argc, char *argv[])
{
   Evas_Object *win, *bg, *box, *btn;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   elm_theme_overlay_add(NULL, "./theme_example.edj");

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
   elm_object_text_set(btn, "Button 1");
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _btn_clicked_cb, NULL);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Button 2");
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _btn_clicked_cb, NULL);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Button 3");
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _btn_clicked_cb, NULL);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Button 4");
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _btn_clicked_cb, NULL);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()

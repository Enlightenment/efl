/*
 * gcc -o theme_example_01 theme_example_01.c `pkg-config --cflags --libs elementary`
 */
#include <Elementary.h>

static void
btn_extension_click_cb(void *data, Evas_Object *btn, void *ev)
{
   const char *lbl = elm_object_text_get(btn);

   if (!strncmp(lbl, "Load", 4))
     {
        elm_theme_extension_add(NULL, "./theme_example.edj");
        elm_object_text_set(btn, "Unload extension");
     }
   else if (!strncmp(lbl, "Unload", 6))
     {
        elm_theme_extension_del(NULL, "./theme_example.edj");
        elm_object_text_set(btn, "Load extension");
     }
}

static void
btn_style_click_cb(void *data, Evas_Object *btn, void *ev)
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

EAPI_MAIN int
elm_main(int argc, char *argv[])
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
   elm_object_text_set(btn, "Unload extension");
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", btn_extension_click_cb, NULL);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Switch style");
   elm_object_style_set(btn, "chucknorris");
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", btn_style_click_cb, NULL);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()

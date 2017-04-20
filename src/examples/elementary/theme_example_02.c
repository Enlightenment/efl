/*
 * gcc -o theme_example_02 theme_example_02.c `pkg-config --cflags --libs elementary`
 */
#include <Elementary.h>

char edj_path[PATH_MAX];

static void
_btn_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *ev EINA_UNUSED)
{
   static int loaded = 1;
   if (loaded)
     elm_theme_overlay_del(NULL, edj_path);
   else
     elm_theme_overlay_add(NULL, edj_path);
   loaded = 1 - loaded;
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *box, *btn;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

#ifdef PACKAGE_DATA_DIR
   elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
#endif
   if (ecore_file_exists("./theme_example.edj"))
     {
        strcpy(edj_path, "./theme_example.edj");
     }
   else
     {
        elm_app_info_set(elm_main, "elementary", "examples/theme_example.edj");
        snprintf(edj_path, sizeof(edj_path), "%s/examples/theme_example.edj",
                 elm_app_data_dir_get());
     }
   elm_theme_overlay_add(NULL, edj_path);

   win = elm_win_util_standard_add("theme", "Theme example");
   elm_win_autodel_set(win, EINA_TRUE);

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

   evas_object_resize(win, 300, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

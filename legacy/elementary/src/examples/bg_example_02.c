//Compile with:
//gcc -o bg_example_02 bg_example_02.c -g `pkg-config --cflags --libs elementary`
//where directory is the a path where images/plant_01.jpg can be found.

#include <Elementary.h>

static void
on_done(void *data, Evas_Object *obj, void *event_info)
{
   /* quit the mainloop (elm_run) */
   elm_exit();
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg;
   char buf[PATH_MAX];

   elm_app_info_set(elm_main, "elementary", "images/plant_01.jpg");
   win = elm_win_add(NULL, "bg-image", ELM_WIN_BASIC);
   elm_win_title_set(win, "Bg Image");
   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_bg_load_size_set(bg, 20, 20);
   elm_bg_option_set(bg, ELM_BG_OPTION_CENTER);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_bg_file_set(bg, buf, NULL);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()

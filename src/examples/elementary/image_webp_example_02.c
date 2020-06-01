//Compile with:
//gcc -g image_webp_example_02.c -o image_webp_example_02 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *image;
   char buf[PATH_MAX];

   elm_app_info_set(elm_main, "elementary", "images/animated_webp_image.webp");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("WebP Image", "WebP Image");
   elm_win_autodel_set(win, EINA_TRUE);

   snprintf(buf, sizeof(buf), "%s/images/animated_webp_image.webp", elm_app_data_dir_get());

   image = elm_image_add(win);
   if (!elm_image_file_set(image, buf, NULL))
     {
        printf("error: could not load image \"%s\"\n", buf);
        return -1;
     }

   elm_image_animated_set(image, EINA_TRUE);
   elm_image_animated_play_set(image, EINA_TRUE);

   evas_object_size_hint_weight_set(image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, image);
   evas_object_show(image);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

//Compile with:
//gcc -g image_example_01.c -o image_example_01 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *image;
   char buf[PATH_MAX];

   elm_app_info_set(elm_main, "elementary", "images/plant_01.jpg");
   win = elm_win_add(NULL, "image", ELM_WIN_BASIC);
   elm_win_title_set(win, "Image");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_bg_color_set(bg, 255,255 ,255);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());

   image = elm_image_add(win);
   if (!elm_image_file_set(image, buf, NULL))
     {
	printf("error: could not load image \"%s\"\n", buf);
	return -1;
     }

   elm_image_no_scale_set(image, EINA_TRUE);
   elm_image_resizable_set(image, EINA_FALSE, EINA_TRUE);
   elm_image_smooth_set(image, EINA_FALSE);
   elm_image_orient_set(image, ELM_IMAGE_FLIP_HORIZONTAL);
   elm_image_aspect_fixed_set(image, EINA_TRUE);
   elm_image_fill_outside_set(image, EINA_TRUE);
   elm_image_editable_set(image, EINA_TRUE);

   evas_object_size_hint_weight_set(image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, image);
   evas_object_show(image);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()

//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` -DPACKAGE_DATA_DIR="\"<directory>\"" photocam_example_01.c -o photocam_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

static void _fit(void *data, Evas_Object *obj, void *event_info);
static void _unfit(void *data, Evas_Object *obj, void *event_info);
static void _zoom(void *data, Evas_Object *obj, void *event_info);
static void _bring_in(void *data, Evas_Object *obj, void *event_info);

EAPI int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *bx, *obj, *photocam;

   win = elm_win_add(NULL, "photocam", ELM_WIN_BASIC);
   elm_win_title_set(win, "Photocam");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   photocam = elm_photocam_add(win);
   elm_photocam_file_set(photocam, PACKAGE_DATA_DIR"/images/insanely_huge_test_image.jpg");
   elm_photocam_bounce_set(photocam, EINA_FALSE, EINA_TRUE);
   evas_object_smart_callback_add(photocam, "loaded,detail", _bring_in, NULL);
   evas_object_resize(photocam, 500, 400);
   evas_object_show(photocam);

   obj = elm_button_add(win);
   elm_object_text_set(obj, "Fit");
   evas_object_show(obj);
   evas_object_resize(obj, 50, 30);
   evas_object_move(obj, 10, 410);
   evas_object_smart_callback_add(obj, "clicked", _fit, photocam);

   obj = elm_button_add(win);
   elm_object_text_set(obj, "Unfit");
   evas_object_show(obj);
   evas_object_resize(obj, 70, 30);
   evas_object_move(obj, 70, 410);
   evas_object_smart_callback_add(obj, "clicked", _unfit, photocam);

   obj = elm_slider_add(win);
   elm_object_text_set(obj, "Zoom");
   evas_object_show(obj);
   evas_object_resize(obj, 300, 30);
   evas_object_move(obj, 150, 410);
   evas_object_smart_callback_add(obj, "changed", _zoom, photocam);

   evas_object_resize(win, 500, 440);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

static void
_bring_in(void *data, Evas_Object *obj, void *event_info)
{
   int w, h;
   elm_photocam_image_size_get(obj, &w, &h);
   elm_photocam_image_region_bring_in(obj, w/2, h/2, 500, 400);
}

static void
_fit(void *data, Evas_Object *obj, void *event_info)
{
   int x, y, w, h;
   elm_photocam_region_get(data, &x, &y, &w, &h);
   printf("region: {%d, %d, %d, %d}\n", x, y, w, h);
   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
}

static void
_unfit(void *data, Evas_Object *obj, void *event_info)
{
   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
}

static void
_zoom(void *data, Evas_Object *obj, void *event_info)
{
   double z = elm_slider_value_get(obj) * 8;
   elm_photocam_zoom_set(data, z);
}

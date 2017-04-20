//Compile with:
//gcc -o photocam_example_01 photocam_example_01.c -g `pkg-config --cflags --libs elementary` -DDATA_DIR="\"<directory>\""
//where directory is the path where images/insanely_huge_test_image.jpg can be found.

#include <Elementary.h>

static void _fit(void *data, Evas_Object *obj, void *event_info);
static void _unfit(void *data, Evas_Object *obj, void *event_info);
static void _zoom(void *data, Evas_Object *obj, void *event_info);
static void _bring_in(void *data, Evas_Object *obj, void *event_info);
static void _orient(void *data, Evas_Object *obj, void *event_info);
static void _orient_do(void *data, Evas_Object *obj, void *event_info);

typedef struct _Orient_Data Orient_Data;
struct _Orient_Data
{
   Evas_Object *ph;
   Evas_Image_Orient orient;
};

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *obj, *photocam, *menu;
   Orient_Data *orient_data;
   char buf[PATH_MAX];

   elm_app_info_set(elm_main, "elementary", "images/insanely_huge_test_image.jpg");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("photocam", "Photocam");
   elm_win_autodel_set(win, EINA_TRUE);

   photocam = elm_photocam_add(win);
   snprintf(buf, sizeof(buf), "%s/images/insanely_huge_test_image.jpg", elm_app_data_dir_get());
   elm_photocam_file_set(photocam, buf);
   elm_scroller_bounce_set(photocam, EINA_FALSE, EINA_TRUE);
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

   orient_data = (Orient_Data *)malloc(sizeof(Orient_Data));
   orient_data->ph = photocam;
   orient_data->orient = 0;
   menu = elm_menu_add(win);
   elm_menu_item_add(menu, NULL, NULL, "orient 0", _orient_do, orient_data);
   orient_data = (Orient_Data *)malloc(sizeof(Orient_Data));
   orient_data->ph = photocam;
   orient_data->orient = 1;
   elm_menu_item_add(menu, NULL, NULL, "orient 90", _orient_do, orient_data);
   orient_data = (Orient_Data *)malloc(sizeof(Orient_Data));
   orient_data->ph = photocam;
   orient_data->orient = 2;
   elm_menu_item_add(menu, NULL, NULL, "orient 180", _orient_do, orient_data);
   orient_data = (Orient_Data *)malloc(sizeof(Orient_Data));
   orient_data->ph = photocam;
   orient_data->orient = 3;
   elm_menu_item_add(menu, NULL, NULL, "orient 270", _orient_do, orient_data);
   orient_data = (Orient_Data *)malloc(sizeof(Orient_Data));
   orient_data->ph = photocam;
   orient_data->orient = 4;
   elm_menu_item_add(menu, NULL, NULL, "flip horizontal", _orient_do, orient_data);
   orient_data = (Orient_Data *)malloc(sizeof(Orient_Data));
   orient_data->ph = photocam;
   orient_data->orient = 5;
   elm_menu_item_add(menu, NULL, NULL, "flip vertical", _orient_do, orient_data);
   orient_data = (Orient_Data *)malloc(sizeof(Orient_Data));
   orient_data->ph = photocam;
   orient_data->orient = 6;
   elm_menu_item_add(menu, NULL, NULL, "flip transverse", _orient_do, orient_data);
   orient_data = (Orient_Data *)malloc(sizeof(Orient_Data));
   orient_data->ph = photocam;
   orient_data->orient = 7;
   elm_menu_item_add(menu, NULL, NULL, "flip transpose", _orient_do, orient_data);

   obj = elm_button_add(win);
   elm_object_text_set(obj, "Orient");
   evas_object_show(obj);
   evas_object_resize(obj, 70, 30);
   evas_object_move(obj, 470, 410);
   evas_object_smart_callback_add(obj, "clicked", _orient, menu);

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
_bring_in(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int w, h;
   elm_photocam_image_size_get(obj, &w, &h);
   elm_photocam_image_region_bring_in(obj, w/2, h/2, 500, 400);
}

static void
_fit(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int x, y, w, h;
   elm_photocam_image_region_get(data, &x, &y, &w, &h);
   printf("region: {%d, %d, %d, %d}\n", x, y, w, h);
   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
}

static void
_unfit(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
}

static void
_zoom(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   double z = elm_slider_value_get(obj) * 8;
   elm_photocam_zoom_set(data, z);
}

static void
_orient(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *mn = data;
   if (!mn) return;

   evas_object_show(mn);
}

static void
_orient_do(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Orient_Data *orient_data = data;
   if (!orient_data) return;

   elm_photocam_image_orient_set(orient_data->ph, orient_data->orient);
}

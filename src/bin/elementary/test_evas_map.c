#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_map_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Evas_Map *map;

   map = evas_map_new(8);
   //1st rect
   evas_map_point_coord_set(map, 0, 100, 0, 0);
   evas_map_point_coord_set(map, 1, 200, 0, 0);
   evas_map_point_coord_set(map, 2, 200, 100, 0);
   evas_map_point_coord_set(map, 3, 100, 100, 0);
   //2nd rect
   evas_map_point_coord_set(map, 4, 200, 0, 0);
   evas_map_point_coord_set(map, 5, 100, 200, 0);
   evas_map_point_coord_set(map, 6, 100, 300, 0);
   evas_map_point_coord_set(map, 7, 200, 100, 0);

   //uv: 1st rect
   evas_map_point_image_uv_set(map, 0, 0, 0);
   evas_map_point_image_uv_set(map, 1, w / 2, 0);
   evas_map_point_image_uv_set(map, 2, w / 2, h);
   evas_map_point_image_uv_set(map, 3, 0, h);
   //uv: 2nd rect
   evas_map_point_image_uv_set(map, 4, w / 2, 0);
   evas_map_point_image_uv_set(map, 5, w, 0);
   evas_map_point_image_uv_set(map, 6, w, h);
   evas_map_point_image_uv_set(map, 7, w / 2, h);

   evas_object_map_enable_set(obj, EINA_TRUE);
   evas_object_map_set(obj, map);
   evas_map_free(map);
}

static void
_image_resize_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;

   efl_gfx_geometry_get(obj, NULL, NULL, &w, &h);
   _map_set(obj, w, h);
}

void
test_evas_map(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   const Evas_Coord W = 300, H = 300;
   Evas_Object *win, *img;
   char buf[PATH_MAX];

   win = efl_add(EFL_UI_WIN_CLASS, NULL,
                 efl_text_set(efl_added, "Evas Map"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   /* image with a min size */
   snprintf(buf, sizeof(buf), "%s/images/rock_02.jpg", elm_app_data_dir_get());
   img = efl_add(EFL_UI_IMAGE_CLASS, win,
                 efl_gfx_size_hint_align_set(efl_added, EFL_GFX_SIZE_HINT_FILL, EFL_GFX_SIZE_HINT_FILL),
                 efl_gfx_size_hint_weight_set(efl_added, EFL_GFX_SIZE_HINT_EXPAND, EFL_GFX_SIZE_HINT_EXPAND),
                 efl_gfx_size_hint_min_set(efl_added, 64, 64),
                 efl_file_set(efl_added, buf, NULL),
                 efl_gfx_visible_set(efl_added, EINA_TRUE));
   efl_ui_image_scale_type_set(img, EFL_UI_IMAGE_SCALE_TYPE_FILL);
   evas_object_event_callback_add(img, EVAS_CALLBACK_RESIZE,
                                  _image_resize_cb, NULL);

   _map_set(img, W, H);

   efl_content_set(win, img);
   efl_gfx_size_set(win, W, H);
   efl_gfx_visible_set(win, EINA_TRUE);
}

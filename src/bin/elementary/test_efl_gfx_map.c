#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_map_set(Evas_Object *obj)
{
   efl_gfx_map_reset(obj);
   efl_gfx_map_point_count_set(obj, 8);
   //1st rect
   efl_gfx_map_coord_absolute_set(obj, 0, 100, 0, 0);
   efl_gfx_map_coord_absolute_set(obj, 1, 200, 0, 0);
   efl_gfx_map_coord_absolute_set(obj, 2, 200, 100, 0);
   efl_gfx_map_coord_absolute_set(obj, 3, 100, 100, 0);
   //2nd rect
   efl_gfx_map_coord_absolute_set(obj, 4, 200, 0, 0);
   efl_gfx_map_coord_absolute_set(obj, 5, 100, 200, 0);
   efl_gfx_map_coord_absolute_set(obj, 6, 100, 300, 0);
   efl_gfx_map_coord_absolute_set(obj, 7, 200, 100, 0);

   //uv: 1st rect: uv: [0-1]
   efl_gfx_map_uv_set(obj, 0, 0, 0);
   efl_gfx_map_uv_set(obj, 1, 0.5, 0);
   efl_gfx_map_uv_set(obj, 2, 0.5, 1);
   efl_gfx_map_uv_set(obj, 3, 0, 1);
   //uv: 2nd rect
   efl_gfx_map_uv_set(obj, 4, 0.5, 0);
   efl_gfx_map_uv_set(obj, 5, 1, 0);
   efl_gfx_map_uv_set(obj, 6, 1, 1);
   efl_gfx_map_uv_set(obj, 7, 0.5, 1);
}

static void
_image_resize_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   _map_set(obj);
}

void
test_efl_gfx_map(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   const Evas_Coord W = 300, H = 300;
   Evas_Object *win, *img;
   char buf[PATH_MAX];

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_text_set(efl_added, "Efl Gfx Map"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   /* image with a min size */
   snprintf(buf, sizeof(buf), "%s/images/rock_02.jpg", elm_app_data_dir_get());
   img = efl_add(EFL_UI_IMAGE_CLASS, win,
                 efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(64, 64)),
                 efl_file_set(efl_added, buf, NULL));
   efl_gfx_image_scale_type_set(img, EFL_GFX_IMAGE_SCALE_TYPE_FILL);
   evas_object_event_callback_add(img, EVAS_CALLBACK_RESIZE,
                                  _image_resize_cb, NULL);

   _map_set(img);

   efl_content_set(win, img);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(W, H));
}

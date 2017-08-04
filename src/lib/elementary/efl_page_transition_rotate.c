#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#include "efl_page_transition_rotate.h"
#include "efl_ui_widget_pagecontrol.h"



static void
_map_uv_set(Evas_Object *obj, Evas_Map *map)
{
   Evas_Coord x, y, w, h;

   // FIXME: only handles filled obj
   if (efl_isa(obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS) &&
       !evas_object_image_source_get(obj))
     {
        int iw, ih;
        evas_object_image_size_get(obj, &iw, &ih);
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        evas_map_util_points_populate_from_geometry(map, x, y, w, h, 0);
        evas_map_point_image_uv_set(map, 0, 0, 0);
        evas_map_point_image_uv_set(map, 1, iw, 0);
        evas_map_point_image_uv_set(map, 2, iw, ih);
        evas_map_point_image_uv_set(map, 3, 0, ih);
     }
   else
     {
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        evas_map_util_points_populate_from_geometry(map, x, y, w, h, 0);
     }
}

static void
_rotate_effect(Evas_Object *obj, Evas_Object *target, double t, int c)
{
   EFL_UI_PAGECONTROL_DATA_GET(obj, sd);

   Evas_Map *map;
   double deg;
   Evas_Coord cx, cy, px, py, foc;
   int lx, ly, lz, lr, lg, lb, lar, lag, lab;

   cx = sd->x + (sd->w / 2);
   cy = sd->y + (sd->h / 2);

   px = sd->x + (sd->w / 2);
   py = sd->y + (sd->h / 2);
   foc = 2048;

   lx = cx;
   ly = cy;
   lz = -10000;
   lr = 255;
   lg = 255;
   lb = 255;
   lar = 0;
   lag = 0;
   lab = 0;

   if (c == 1) deg = 360 - (180.0 * t);
   else deg = 180 * (1 - t);

   map = evas_map_new(4);
   _map_uv_set(target, map);
   evas_map_util_3d_rotate(map, 0.0, deg, 0.0, cx, cy, 0);
   evas_map_util_3d_lighting(map, lx, ly, lz, lr, lg, lb, lar, lag, lab);
   evas_map_util_3d_perspective(map, px, py, 0, foc);
   evas_object_map_set(target, map);
   evas_object_map_enable_set(target, EINA_TRUE);

   if (c == 1)
     {
        if (evas_map_util_clockwise_get(map)) evas_object_show(sd->viewport.foreclip);
        else evas_object_hide(sd->viewport.foreclip);
     }
   else
     {
        if (evas_map_util_clockwise_get(map)) evas_object_show(sd->viewport.backclip);
        else evas_object_hide(sd->viewport.backclip);
     }

   evas_map_free(map);
}

EOLIAN static void
_efl_page_transition_rotate_efl_page_transition_update(Eo *obj EINA_UNUSED,
                                                       Efl_Page_Transition_Rotate_Data *_pd EINA_UNUSED,
                                                       Efl_Canvas_Object *target,
                                                       double position)
{
   if (!target) return;

   Evas_Object *parent = efl_parent_get(target);

   if (position <= 0) _rotate_effect(parent, target, (-1) * position, 1);
   else _rotate_effect(parent, target, position, 2);
}

#include "efl_page_transition_rotate.eo.c"

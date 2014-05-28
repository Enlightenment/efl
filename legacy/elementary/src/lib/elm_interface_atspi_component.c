#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_COMPONENT_PROTECTED
#include "atspi/atspi-constants.h"

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_interface_atspi_component.eo.h"
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"


EOLIAN static void
_elm_interface_atspi_component_position_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, Eina_Bool type, int *x, int *y)
{
   eo_do(obj, elm_interface_atspi_component_extents_get(type, x, y, NULL, NULL));
}

EOLIAN static Eina_Bool
_elm_interface_atspi_component_position_set(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, Eina_Bool type, int x, int y)
{
   Eina_Bool ret = EINA_FALSE;
   int c_w, c_h;

   eo_do(obj, elm_interface_atspi_component_extents_get(type, NULL, NULL, &c_w, &c_h));
   eo_do(obj, ret = elm_interface_atspi_component_extents_set(type, x, y, c_w, c_h));

   return ret;
}

EOLIAN static Eina_Bool
_elm_interface_atspi_component_size_set(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, int w, int h)
{
   Eina_Bool ret;
   int c_x = 0, c_y = 0;

   eo_do(obj, elm_interface_atspi_component_extents_get(EINA_FALSE, &c_x, &c_y, NULL, NULL));
   eo_do(obj, ret = elm_interface_atspi_component_extents_set(EINA_FALSE, c_x, c_y, w, h));
   return ret;
}

EOLIAN static void
_elm_interface_atspi_component_size_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, int *w, int *h)
{
   eo_do(obj, elm_interface_atspi_component_extents_get(EINA_FALSE, NULL,  NULL, w, h));
}

EOLIAN static Eina_Bool
_elm_interface_atspi_component_contains(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, Eina_Bool type, int x, int y)
{
   int w_x = 0, w_y = 0, w_w = 0, w_h = 0;

   eo_do(obj, elm_interface_atspi_component_extents_get(type, &w_x, &w_y, &w_w, &w_h));

   if ((x >= w_x) && (x <= w_x + w_w) && (y >= w_y) && (y <= w_y + w_h))
     return EINA_TRUE;
   return EINA_FALSE;
}

EOLIAN static double
_elm_interface_atspi_component_alpha_get(Eo *obj, void *_pd EINA_UNUSED)
{
   int alpha;

   evas_object_color_get(obj, NULL, NULL, NULL, &alpha);
   return (double)alpha / 255.0;
}

EOLIAN static Evas_Object *
_elm_interface_atspi_component_accessible_at_point_get(Eo *obj, void *_pd EINA_UNUSED, Eina_Bool screen_coords, int x, int y)
{
   Evas_Object *evobj;
   int ee_x, ee_y;
   Eina_List *l, *objs;
   Evas_Object *ret = NULL;

   if (screen_coords)
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
        if (!ee) return NULL;
        ecore_evas_geometry_get(ee, &ee_x, &ee_y, NULL, NULL);
        x -= ee_x;
        y -= ee_y;
     }
   objs = evas_objects_at_xy_get(evas_object_evas_get(obj), x, y, EINA_TRUE, EINA_TRUE);
   EINA_LIST_FOREACH(objs, l, evobj)
     {
        // return first only, test if there is atspi interface for eo
        if (eo_isa(evobj, ELM_INTERFACE_ATSPI_ACCESSIBLE_CLASS))
          {
             ret = evobj;
             break;
          }
     }
   eina_list_free(objs);
   return ret;
}

EOLIAN static void
_elm_interface_atspi_component_extents_get(Eo *obj, void *_pd EINA_UNUSED, Eina_Bool screen_coords, int *x, int *y, int *w, int *h)
{
   int ee_x, ee_y;

   evas_object_geometry_get(obj, x, y, w, h);
   if (screen_coords)
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
        if (!ee) return;
        ecore_evas_geometry_get(ee, &ee_x, &ee_y, NULL, NULL);
        if (x) *x += ee_x;
        if (y) *y += ee_y;
     }
}

EOLIAN static Eina_Bool
_elm_interface_atspi_component_extents_set(Eo *obj, void *_pd EINA_UNUSED, Eina_Bool screen_coords, int x, int y, int w, int h)
{
   int wx, wy;

   if ((x < 0) || (y < 0) || (w < 0) || (h < 0)) return EINA_FALSE;

   if (screen_coords)
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
        if (!ee) return EINA_FALSE;
        evas_object_geometry_get(obj, &wx, &wy, NULL, NULL);
        ecore_evas_move(ee, x - wx, y - wy);
     }
   else
     evas_object_move(obj, x, y);

   evas_object_resize(obj, w, h);
   return EINA_TRUE;
}

EOLIAN static int
_elm_interface_atspi_component_layer_get(Eo *obj, void *_pd EINA_UNUSED)
{
   return evas_object_layer_get(obj);
}

EOLIAN static int
_elm_interface_atspi_component_z_order_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   // Currently not used.
   return 0;
}

EOLIAN static Eina_Bool
_elm_interface_atspi_component_focus_grab(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   evas_object_focus_set(obj, EINA_TRUE);
   return evas_object_focus_get(obj);
}

#include "elm_interface_atspi_component.eo.c"

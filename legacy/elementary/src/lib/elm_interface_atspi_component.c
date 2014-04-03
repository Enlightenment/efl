#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_COMPONENT_PROTECTED
#include "atspi/atspi-constants.h"

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_interface_atspi_component.eo.h"

EOLIAN static void
_elm_interface_atspi_component_position_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED,int *x, int *y, AtspiCoordType type)
{
   eo_do(obj, elm_interface_atspi_component_extents_get(x, y, NULL, NULL, type));
}

EOLIAN static Eina_Bool
_elm_interface_atspi_component_position_set(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, int x, int y, AtspiCoordType type)
{
   Eina_Bool ret = EINA_FALSE;
   int c_w, c_h;

   eo_do(obj, elm_interface_atspi_component_extents_get(NULL, NULL, &c_w, &c_h, type));
   eo_do(obj, ret = elm_interface_atspi_component_extents_set(x, y, c_w, c_h, type));
   return ret;
}

EOLIAN static Eina_Bool
_elm_interface_atspi_component_size_set(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, int w, int h)
{
   Eina_Bool ret = EINA_FALSE;
   int c_x = 0, c_y = 0;

   eo_do(obj, elm_interface_atspi_component_extents_get(&c_x, &c_y, NULL, NULL, ATSPI_COORD_TYPE_WINDOW));
   eo_do(obj, ret = elm_interface_atspi_component_extents_set(c_x, c_y, w, h, ATSPI_COORD_TYPE_WINDOW));
   return ret;
}

EOLIAN static void
_elm_interface_atspi_component_size_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, int *w, int *h)
{
   eo_do(obj, elm_interface_atspi_component_extents_get(NULL,  NULL, w, h, ATSPI_COORD_TYPE_WINDOW));
}

EOLIAN static Eina_Bool
_elm_interface_atspi_component_contains(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED,
      int x, int y, AtspiCoordType type)
{
   int w_x = 0, w_y = 0, w_w = 0, w_h = 0;

   eo_do(obj, elm_interface_atspi_component_extents_get(&w_x, &w_y, &w_w, &w_h, type));

   if ((x >= w_x) && (x <= w_x + w_w) && (y >= w_y) && (y <= w_y + w_h))
     return EINA_TRUE;
   return EINA_FALSE;
}

#include "elm_interface_atspi_component.eo.c"

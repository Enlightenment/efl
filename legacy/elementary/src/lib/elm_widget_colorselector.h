#ifndef ELM_WIDGET_COLORSELECTOR_H
#define ELM_WIDGET_COLORSELECTOR_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-colorselector-class The Elementary Colorselector Class
 *
 * Elementary, besides having the @ref Colorselector widget, exposes its
 * foundation -- the Elementary Colorselector Class -- in order to create other
 * widgets which are a colorselector with some more logic on top.
 */

typedef struct _Color_Bar_Data Color_Bar_Data;

/**
 * Base layout smart data extended with colorselector instance data.
 */
typedef struct _Elm_Colorselector_Smart_Data Elm_Colorselector_Smart_Data;
struct _Elm_Colorselector_Smart_Data
{
   /* for the 3 displaying modes of the widget */
   Evas_Object           *col_bars_area;
   Evas_Object           *palette_box;
   Evas_Object           *picker;
   Evas_Object           *picker_display;
   Evas_Object           *entries[4];
   Evas_Object           *button;

   struct {
      int                 x, y;
      Eina_Bool           in;
#ifdef HAVE_ELEMENTARY_X
      Ecore_X_Window       xroot;
      Ecore_Event_Handler *mouse_motion;
      Ecore_Event_Handler *key_up;
      Ecore_Event_Handler *mouse_up;
#endif
   } grab;

   Eina_List             *items, *selected;
   Color_Bar_Data        *cb_data[4];

   Ecore_Timer           *longpress_timer;
   const char            *palette_name;
   Evas_Coord             _x, _y, _w, _h;

   /* color components */
   int                    r, g, b, a;
   int                    er, eg, eb;

   double                 h, s, l;
   Elm_Colorselector_Mode mode, focused;
   int                    sel_color_type;

   Eina_Bool              config_load : 1;
};

typedef enum _Color_Type
{
   HUE,
   SATURATION,
   LIGHTNESS,
   ALPHA
} Color_Type;

struct _Color_Bar_Data
{
   Evas_Object *parent;
   Evas_Object *colorbar;
   Evas_Object *bar;
   Evas_Object *lbt;
   Evas_Object *rbt;
   Evas_Object *bg_rect;
   Evas_Object *arrow;
   Evas_Object *touch_area;
   Evas_Object *access_obj;
   Color_Type   color_type;
};

typedef struct _Elm_Color_Item Elm_Color_Item;
struct _Elm_Color_Item
{
   ELM_WIDGET_ITEM;

   Evas_Object    *color_obj;
   Elm_Color_RGBA *color;
};

/**
 * @}
 */

#define ELM_COLORSELECTOR_DATA_GET(o, sd) \
  Elm_Colorselector_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_COLORSELECTOR_CLASS)

#define ELM_COLORSELECTOR_DATA_GET_OR_RETURN(o, ptr) \
  ELM_COLORSELECTOR_DATA_GET(o, ptr);                \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_COLORSELECTOR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_COLORSELECTOR_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                    \
    {                                                         \
       CRI("No widget data for object %p (%s)",               \
           o, evas_object_type_get(o));                       \
       return val;                                            \
    }

#define ELM_COLORSELECTOR_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_COLORSELECTOR_CLASS))) \
    return

#define ELM_COLORSELECTOR_ITEM_CHECK(it)                    \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_COLORSELECTOR_CHECK(it->base.widget);

#define ELM_COLORSELECTOR_ITEM_CHECK_OR_RETURN(it, ...)                \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_COLORSELECTOR_CHECK(it->base.widget) __VA_ARGS__;

#endif

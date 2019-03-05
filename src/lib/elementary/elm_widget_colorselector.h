#ifndef ELM_WIDGET_COLORSELECTOR_H
#define ELM_WIDGET_COLORSELECTOR_H

#include "Elementary.h"
#include "elm_color_item_eo.h"
#include "elm_colorselector_eo.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

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
typedef struct _Elm_Colorselector_Data Elm_Colorselector_Data;
struct _Elm_Colorselector_Data
{
   /* for the 3 displaying modes of the widget */
   Evas_Object           *col_bars_area;
   Evas_Object           *palette_box;
   Evas_Object           *picker;
   Evas_Object           *picker_display;
   Evas_Object           *spinners[4];
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

   /* focus support data */
   Elm_Object_Item       *focused_item;
   Eina_List             *focus_items;

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

typedef struct _Elm_Color_Item_Data Elm_Color_Item_Data;
struct _Elm_Color_Item_Data
{
   Elm_Widget_Item_Data *base;

   Evas_Object    *color_obj;
   Elm_Color_RGBA *color;

   Eina_Bool       still_in : 1;
};

typedef struct _Elm_Color_Name Elm_Color_Name;
struct _Elm_Color_Name
{
   Elm_Color_RGBA color;
   const char *name;
};

/**
 * @}
 */

#define ELM_COLORSELECTOR_DATA_GET(o, sd) \
  Elm_Colorselector_Data * sd = efl_data_scope_get(o, ELM_COLORSELECTOR_CLASS)

#define ELM_COLORSELECTOR_DATA_GET_OR_RETURN(o, ptr) \
  ELM_COLORSELECTOR_DATA_GET(o, ptr);                \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_COLORSELECTOR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_COLORSELECTOR_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                    \
    {                                                         \
       ERR("No widget data for object %p (%s)",               \
           o, evas_object_type_get(o));                       \
       return val;                                            \
    }

#define ELM_COLOR_ITEM_DATA_GET(o, sd) \
  Elm_Color_Item_Data * sd = efl_data_scope_get(o, ELM_COLOR_ITEM_CLASS)

#define ELM_COLORSELECTOR_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_COLORSELECTOR_CLASS))) \
    return

#define ELM_COLORSELECTOR_ITEM_CHECK(it)                       \
  if (EINA_UNLIKELY(!efl_isa(it->base->eo_obj, ELM_COLOR_ITEM_CLASS))) \
    return

#define ELM_COLORSELECTOR_ITEM_CHECK_OR_RETURN(it, ...)        \
  if (EINA_UNLIKELY(!efl_isa(it->base->eo_obj, ELM_COLOR_ITEM_CLASS))) \
    return __VA_ARGS__;

#endif

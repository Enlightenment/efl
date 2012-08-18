#ifndef ELM_WIDGET_COLORSELECTOR_H
#define ELM_WIDGET_COLORSELECTOR_H

#include "elm_widget_layout.h"

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

/**
 * @def ELM_COLORSELECTOR_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Colorselector_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_COLORSELECTOR_CLASS(x) ((Elm_Colorselector_Smart_Class *)x)

/**
 * @def ELM_COLORSELECTOR_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Colorselector_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_COLORSELECTOR_DATA(x)  ((Elm_Colorselector_Smart_Data *)x)

/**
 * @def ELM_COLORSELECTOR_SMART_CLASS_VERSION
 *
 * Current version for Elementary colorselector @b base smart class, a value
 * which goes to _Elm_Colorselector_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_COLORSELECTOR_SMART_CLASS_VERSION 1

/**
 * @def ELM_COLORSELECTOR_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Colorselector_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_COLORSELECTOR_SMART_CLASS_INIT_NULL
 * @see ELM_COLORSELECTOR_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_COLORSELECTOR_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_COLORSELECTOR_SMART_CLASS_VERSION}

/**
 * @def ELM_COLORSELECTOR_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Colorselector_Smart_Class structure.
 *
 * @see ELM_COLORSELECTOR_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_COLORSELECTOR_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_COLORSELECTOR_SMART_CLASS_INIT_NULL \
  ELM_COLORSELECTOR_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_COLORSELECTOR_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Colorselector_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_COLORSELECTOR_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Colorselector_Smart_Class (base field)
 * to the latest #ELM_COLORSELECTOR_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_COLORSELECTOR_SMART_CLASS_INIT_NULL
 * @see ELM_COLORSELECTOR_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_COLORSELECTOR_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_COLORSELECTOR_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary colorselector base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a colorselector.
 *
 * All of the functions listed on @ref Colorselector namespace will work for
 * objects deriving from #Elm_Colorselector_Smart_Class.
 */
typedef struct _Elm_Colorselector_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Colorselector_Smart_Class;

typedef struct _Color_Bar_Data Color_Bar_Data;

/**
 * Base layout smart data extended with colorselector instance data.
 */
typedef struct _Elm_Colorselector_Smart_Data Elm_Colorselector_Smart_Data;
struct _Elm_Colorselector_Smart_Data
{
   Elm_Layout_Smart_Data  base;

   /* for the 2 displaying modes of the widget */
   Evas_Object           *col_bars_area;
   Evas_Object           *palette_box;

   Eina_List             *items, *selected;
   Color_Bar_Data        *cb_data[4];

   Ecore_Timer           *longpress_timer;
   const char            *palette_name;
   Evas_Coord             _x, _y, _w, _h;

   /* color components */
   int                    r, g, b, a;
   int                    er, eg, eb;
   int                    sr, sg, sb;
   int                    lr, lg, lb;

   double                 h, s, l;
   Elm_Colorselector_Mode mode, focused;
   int                    sel_color_type;

   Eina_Bool              longpressed : 1;
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

EAPI extern const char ELM_COLORSELECTOR_SMART_NAME[];
EAPI const Elm_Colorselector_Smart_Class
*elm_colorselector_smart_class_get(void);

#define ELM_COLORSELECTOR_DATA_GET(o, sd) \
  Elm_Colorselector_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_COLORSELECTOR_DATA_GET_OR_RETURN(o, ptr) \
  ELM_COLORSELECTOR_DATA_GET(o, ptr);                \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_COLORSELECTOR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_COLORSELECTOR_DATA_GET(o, ptr);                         \
  if (!ptr)                                                   \
    {                                                         \
       CRITICAL("No widget data for object %p (%s)",          \
                o, evas_object_type_get(o));                  \
       return val;                                            \
    }

#define ELM_COLORSELECTOR_CHECK(obj)                     \
  if (!obj || !elm_widget_type_check                     \
        ((obj), ELM_COLORSELECTOR_SMART_NAME, __func__)) \
    return

#define ELM_COLORSELECTOR_ITEM_CHECK(it)                    \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_COLORSELECTOR_CHECK(it->base.widget);

#define ELM_COLORSELECTOR_ITEM_CHECK_OR_RETURN(it, ...)                \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_COLORSELECTOR_CHECK(it->base.widget) __VA_ARGS__;

#endif

#ifndef ELM_WIDGET_DISKSELECTOR_H
#define ELM_WIDGET_DISKSELECTOR_H

#include "elm_interface_scrollable.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-diskselector-class The Elementary Diskselector Class
 *
 * Elementary, besides having the @ref Diskselector widget, exposes its
 * foundation -- the Elementary Diskselector Class -- in order to create other
 * widgets which are a diskselector with some more logic on top.
 */

/**
 * @def ELM_DISKSELECTOR_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Diskselector_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_DISKSELECTOR_CLASS(x) ((Elm_Diskselector_Smart_Class *)x)

/**
 * @def ELM_DISKSELECTOR_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Diskselector_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_DISKSELECTOR_DATA(x)  ((Elm_Diskselector_Smart_Data *)x)

/**
 * @def ELM_DISKSELECTOR_SMART_CLASS_VERSION
 *
 * Current version for Elementary diskselector @b base smart class, a value
 * which goes to _Elm_Diskselector_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_DISKSELECTOR_SMART_CLASS_VERSION 1

/**
 * @def ELM_DISKSELECTOR_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Diskselector_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_DISKSELECTOR_SMART_CLASS_INIT_NULL
 * @see ELM_DISKSELECTOR_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_DISKSELECTOR_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_DISKSELECTOR_SMART_CLASS_VERSION}

/**
 * @def ELM_DISKSELECTOR_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Diskselector_Smart_Class structure.
 *
 * @see ELM_DISKSELECTOR_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_DISKSELECTOR_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_DISKSELECTOR_SMART_CLASS_INIT_NULL \
  ELM_DISKSELECTOR_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_DISKSELECTOR_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Diskselector_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_DISKSELECTOR_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Diskselector_Smart_Class (base field)
 * to the latest #ELM_DISKSELECTOR_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_DISKSELECTOR_SMART_CLASS_INIT_NULL
 * @see ELM_DISKSELECTOR_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_DISKSELECTOR_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_DISKSELECTOR_SMART_CLASS_INIT(ELM_WIDGET_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary diskselector base smart class. This inherits directly from
 * #Elm_Widget_Smart_Class and is meant to build widgets extending the
 * behavior of a diskselector.
 *
 * All of the functions listed on @ref Diskselector namespace will work for
 * objects deriving from #Elm_Diskselector_Smart_Class.
 */
typedef struct _Elm_Diskselector_Smart_Class
{
   Elm_Widget_Smart_Class base;

   int                    version; /**< Version of this smart class definition */
} Elm_Diskselector_Smart_Class;

/**
 * Base widget smart data extended with diskselector instance data.
 */
typedef struct _Elm_Diskselector_Smart_Data Elm_Diskselector_Smart_Data;
typedef struct _Elm_Diskselector_Item       Elm_Diskselector_Item;

struct _Elm_Diskselector_Smart_Data
{
   Elm_Widget_Smart_Data                 base; /* base widget smart data as
                                                * first member obligatory, as
                                                * we're inheriting from it */

   Evas_Object                          *hit_rect;
   const Elm_Scrollable_Smart_Interface *s_iface;

   Evas_Object                          *main_box;
   Elm_Diskselector_Item                *selected_item;
   Elm_Diskselector_Item                *first;
   Elm_Diskselector_Item                *second;
   Elm_Diskselector_Item                *s_last;
   Elm_Diskselector_Item                *last;
   Eina_List                            *items;
   Eina_List                            *r_items;
   Eina_List                            *over_items;
   Eina_List                            *under_items;
   Eina_List                            *left_blanks;
   Eina_List                            *right_blanks;
   Ecore_Idle_Enterer                   *idler;
   Ecore_Idle_Enterer                   *check_idler;

   int                                   item_count, len_threshold, len_side,
                                         display_item_num;
   Evas_Coord                            minw, minh;

   Eina_Bool                             init : 1;
   Eina_Bool                             round : 1;
   Eina_Bool                             display_item_num_by_api : 1;
   Eina_Bool                             left_boundary_reached:1;
   Eina_Bool                             right_boundary_reached:1;
};

struct _Elm_Diskselector_Item
{
   ELM_WIDGET_ITEM;

   Eina_List    *node;
   Evas_Object  *icon;
   const char   *label;
   Evas_Smart_Cb func;
};

/**
 * @}
 */

EAPI extern const char ELM_DISKSELECTOR_SMART_NAME[];
EAPI const Elm_Diskselector_Smart_Class
*elm_diskselector_smart_class_get(void);

#define ELM_DISKSELECTOR_DATA_GET(o, sd) \
  Elm_Diskselector_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_DISKSELECTOR_DATA_GET_OR_RETURN(o, ptr)  \
  ELM_DISKSELECTOR_DATA_GET(o, ptr);                 \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_DISKSELECTOR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_DISKSELECTOR_DATA_GET(o, ptr);                         \
  if (!ptr)                                                  \
    {                                                        \
       CRITICAL("No widget data for object %p (%s)",         \
                o, evas_object_type_get(o));                 \
       return val;                                           \
    }

#define ELM_DISKSELECTOR_CHECK(obj)                                          \
  if (!obj || !elm_widget_type_check((obj),                                  \
                                     ELM_DISKSELECTOR_SMART_NAME, __func__)) \
    return

#define ELM_DISKSELECTOR_ITEM_CHECK(it)                     \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_DISKSELECTOR_CHECK(it->base.widget);

#define ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it, ...)                 \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_DISKSELECTOR_CHECK(it->base.widget) __VA_ARGS__;

#define ELM_DISKSELECTOR_ITEM_CHECK_OR_GOTO(it, label)         \
  ELM_WIDGET_ITEM_CHECK_OR_GOTO((Elm_Widget_Item *)it, label); \
  if (!it->base.widget || !elm_widget_type_check               \
        ((it->base.widget), ELM_DISKSELECTOR_SMART_NAME, __func__)) goto label;

#endif

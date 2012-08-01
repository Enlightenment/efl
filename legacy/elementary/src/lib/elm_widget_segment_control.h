#ifndef ELM_WIDGET_SEGMENT_CONTROL_H
#define ELM_WIDGET_SEGMENT_CONTROL_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-segment-control-class The Elementary Segment control Class
 *
 * Elementary, besides having the @ref SegmentControl widget, exposes
 * its foundation -- the Elementary Segment control Class -- in order
 * to create other widgets which are a segment control with some more
 * logic on top.
 */

/**
 * @def ELM_SEGMENT_CONTROL_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Segment_Control_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_SEGMENT_CONTROL_CLASS(x) ((Elm_Segment_Control_Smart_Class *)x)

/**
 * @def ELM_SEGMENT_CONTROL_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Segment_Control_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_SEGMENT_CONTROL_DATA(x)  ((Elm_Segment_Control_Smart_Data *)x)

/**
 * @def ELM_SEGMENT_CONTROL_SMART_CLASS_VERSION
 *
 * Current version for Elementary segment control @b base smart class, a value
 * which goes to _Elm_Segment_Control_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_SEGMENT_CONTROL_SMART_CLASS_VERSION 1

/**
 * @def ELM_SEGMENT_CONTROL_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Segment_Control_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_SEGMENT_CONTROL_SMART_CLASS_INIT_NULL
 * @see ELM_SEGMENT_CONTROL_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_SEGMENT_CONTROL_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_SEGMENT_CONTROL_SMART_CLASS_VERSION}

/**
 * @def ELM_SEGMENT_CONTROL_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Segment_Control_Smart_Class structure.
 *
 * @see ELM_SEGMENT_CONTROL_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_SEGMENT_CONTROL_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_SEGMENT_CONTROL_SMART_CLASS_INIT_NULL \
  ELM_SEGMENT_CONTROL_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_SEGMENT_CONTROL_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Segment_Control_Smart_Class
 * structure and set its name and version.
 *
 * This is similar to #ELM_SEGMENT_CONTROL_SMART_CLASS_INIT_NULL, but
 * it will also set the version field of
 * #Elm_Segment_Control_Smart_Class (base field) to the latest
 * #ELM_SEGMENT_CONTROL_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_SEGMENT_CONTROL_SMART_CLASS_INIT_NULL
 * @see ELM_SEGMENT_CONTROL_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_SEGMENT_CONTROL_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_SEGMENT_CONTROL_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary segment control base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a segment control.
 *
 * All of the functions listed on @ref SegmentControl namespace will work for
 * objects deriving from #Elm_Segment_Control_Smart_Class.
 */
typedef struct _Elm_Segment_Control_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Segment_Control_Smart_Class;

/**
 * Base layout smart data extended with segment control instance data.
 */
typedef struct _Elm_Segment_Control_Smart_Data Elm_Segment_Control_Smart_Data;
typedef struct _Elm_Segment_Item               Elm_Segment_Item;

struct _Elm_Segment_Control_Smart_Data
{
   Elm_Layout_Smart_Data base;

   Eina_List            *items;
   Elm_Segment_Item     *selected_item;

   int                   item_width;
};

struct _Elm_Segment_Item
{
   ELM_WIDGET_ITEM;

   Evas_Object *icon;
   const char  *label;
   int          seg_index;
};

/**
 * @}
 */

EAPI extern const char ELM_SEGMENT_CONTROL_SMART_NAME[];
EAPI const Elm_Segment_Control_Smart_Class
*elm_segment_control_smart_class_get(void);


#define ELM_SEGMENT_CONTROL_DATA_GET(o, sd) \
  Elm_Segment_Control_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_SEGMENT_CONTROL_DATA_GET_OR_RETURN(o, ptr) \
  ELM_SEGMENT_CONTROL_DATA_GET(o, ptr);                \
  if (!ptr)                                            \
    {                                                  \
       CRITICAL("No widget data for object %p (%s)",   \
                o, evas_object_type_get(o));           \
       return;                                         \
    }

#define ELM_SEGMENT_CONTROL_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_SEGMENT_CONTROL_DATA_GET(o, ptr);                         \
  if (!ptr)                                                     \
    {                                                           \
       CRITICAL("No widget data for object %p (%s)",            \
                o, evas_object_type_get(o));                    \
       return val;                                              \
    }

#define ELM_SEGMENT_CONTROL_CHECK(obj)                     \
  if (!obj || !elm_widget_type_check                       \
        ((obj), ELM_SEGMENT_CONTROL_SMART_NAME, __func__)) \
    return

#define ELM_SEGMENT_CONTROL_ITEM_CHECK(it)                  \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_SEGMENT_CONTROL_CHECK(it->base.widget);

#define ELM_SEGMENT_CONTROL_ITEM_CHECK_OR_RETURN(it, ...)              \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_SEGMENT_CONTROL_CHECK(it->base.widget) __VA_ARGS__;

#endif

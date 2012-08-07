#ifndef ELM_WIDGET_FLIPSELECTOR_H
#define ELM_WIDGET_FLIPSELECTOR_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-flipselector-class The Elementary Flipselector Class
 *
 * Elementary, besides having the @ref Flipselector widget, exposes its
 * foundation -- the Elementary Flipselector Class -- in order to create other
 * widgets which are a flipselector with some more logic on top.
 */

/**
 * @def ELM_FLIPSELECTOR_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Flipselector_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_FLIPSELECTOR_CLASS(x) ((Elm_Flipselector_Smart_Class *)x)

/**
 * @def ELM_FLIPSELECTOR_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Flipselector_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_FLIPSELECTOR_DATA(x)  ((Elm_Flipselector_Smart_Data *)x)

/**
 * @def ELM_FLIPSELECTOR_SMART_CLASS_VERSION
 *
 * Current version for Elementary flipselector @b base smart class, a value
 * which goes to _Elm_Flipselector_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_FLIPSELECTOR_SMART_CLASS_VERSION 1

/**
 * @def ELM_FLIPSELECTOR_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Flipselector_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_FLIPSELECTOR_SMART_CLASS_INIT_NULL
 * @see ELM_FLIPSELECTOR_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_FLIPSELECTOR_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_FLIPSELECTOR_SMART_CLASS_VERSION}

/**
 * @def ELM_FLIPSELECTOR_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Flipselector_Smart_Class structure.
 *
 * @see ELM_FLIPSELECTOR_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_FLIPSELECTOR_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_FLIPSELECTOR_SMART_CLASS_INIT_NULL \
  ELM_FLIPSELECTOR_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_FLIPSELECTOR_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Flipselector_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_FLIPSELECTOR_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Flipselector_Smart_Class (base field)
 * to the latest #ELM_FLIPSELECTOR_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_FLIPSELECTOR_SMART_CLASS_INIT_NULL
 * @see ELM_FLIPSELECTOR_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_FLIPSELECTOR_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_FLIPSELECTOR_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary flipselector base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a flipselector.
 *
 * All of the functions listed on @ref Flipselector namespace will work for
 * objects deriving from #Elm_Flipselector_Smart_Class.
 */
typedef struct _Elm_Flipselector_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Flipselector_Smart_Class;

/**
 * Base layout smart data extended with flipselector instance data.
 */
typedef struct _Elm_Flipselector_Smart_Data Elm_Flipselector_Smart_Data;
struct _Elm_Flipselector_Smart_Data
{
   Elm_Layout_Smart_Data base;

   Eina_List            *items;
   Eina_List            *current;
   Eina_List            *sentinel; /* item containing the largest
                                    * label string */
   Ecore_Timer          *spin;

   unsigned int          max_len;
   double                interval, first_interval;

   int                   walking;
   Eina_Bool             evaluating : 1;
   Eina_Bool             deleting : 1;
};

typedef struct _Elm_Flipselector_Item       Elm_Flipselector_Item;
struct _Elm_Flipselector_Item
{
   ELM_WIDGET_ITEM;

   const char   *label;
   Evas_Smart_Cb func;
   void         *data;
   int           deleted : 1;
};

/**
 * @}
 */

EAPI extern const char ELM_FLIPSELECTOR_SMART_NAME[];
EAPI const Elm_Flipselector_Smart_Class *elm_flipselector_smart_class_get(void);

#define ELM_FLIPSELECTOR_DATA_GET(o, sd) \
  Elm_Flipselector_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_FLIPSELECTOR_DATA_GET_OR_RETURN(o, ptr)  \
  ELM_FLIPSELECTOR_DATA_GET(o, ptr);                 \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_FLIPSELECTOR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_FLIPSELECTOR_DATA_GET(o, ptr);                         \
  if (!ptr)                                                  \
    {                                                        \
       CRITICAL("No widget data for object %p (%s)",         \
                o, evas_object_type_get(o));                 \
       return val;                                           \
    }

#define ELM_FLIPSELECTOR_CHECK(obj)                     \
  if (!obj || !elm_widget_type_check                    \
        ((obj), ELM_FLIPSELECTOR_SMART_NAME, __func__)) \
    return

#define ELM_FLIPSELECTOR_ITEM_CHECK(it)                     \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_FLIPSELECTOR_CHECK(it->base.widget);

#define ELM_FLIPSELECTOR_ITEM_CHECK_OR_RETURN(it, ...)                 \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_FLIPSELECTOR_CHECK(it->base.widget) __VA_ARGS__;

#endif

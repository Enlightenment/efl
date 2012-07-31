#ifndef ELM_WIDGET_SLIDESHOW_H
#define ELM_WIDGET_SLIDESHOW_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-slideshow-class The Elementary Slideshow Class
 *
 * Elementary, besides having the @ref Slideshow widget, exposes its
 * foundation -- the Elementary Slideshow Class -- in order to create other
 * widgets which are a slideshow with some more logic on top.
 */

/**
 * @def ELM_SLIDESHOW_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Slideshow_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_SLIDESHOW_CLASS(x) ((Elm_Slideshow_Smart_Class *)x)

/**
 * @def ELM_SLIDESHOW_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Slideshow_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_SLIDESHOW_DATA(x)  ((Elm_Slideshow_Smart_Data *)x)

/**
 * @def ELM_SLIDESHOW_SMART_CLASS_VERSION
 *
 * Current version for Elementary slideshow @b base smart class, a value
 * which goes to _Elm_Slideshow_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_SLIDESHOW_SMART_CLASS_VERSION 1

/**
 * @def ELM_SLIDESHOW_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Slideshow_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_SLIDESHOW_SMART_CLASS_INIT_NULL
 * @see ELM_SLIDESHOW_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_SLIDESHOW_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_SLIDESHOW_SMART_CLASS_VERSION}

/**
 * @def ELM_SLIDESHOW_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Slideshow_Smart_Class structure.
 *
 * @see ELM_SLIDESHOW_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_SLIDESHOW_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_SLIDESHOW_SMART_CLASS_INIT_NULL \
  ELM_SLIDESHOW_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_SLIDESHOW_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Slideshow_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_SLIDESHOW_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Slideshow_Smart_Class (base field)
 * to the latest #ELM_SLIDESHOW_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_SLIDESHOW_SMART_CLASS_INIT_NULL
 * @see ELM_SLIDESHOW_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_SLIDESHOW_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_SLIDESHOW_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary slideshow base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a slideshow.
 *
 * All of the functions listed on @ref Slideshow namespace will work for
 * objects deriving from #Elm_Slideshow_Smart_Class.
 */
typedef struct _Elm_Slideshow_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Slideshow_Smart_Class;

/**
 * Base layout smart data extended with slideshow instance data.
 */
typedef struct _Elm_Slideshow_Smart_Data Elm_Slideshow_Smart_Data;
typedef struct _Elm_Slideshow_Item       Elm_Slideshow_Item;

struct _Elm_Slideshow_Item
{
   ELM_WIDGET_ITEM;

   Eina_List                      *l, *l_built;

   const Elm_Slideshow_Item_Class *itc;
};

struct _Elm_Slideshow_Smart_Data
{
   Elm_Layout_Smart_Data base;

   // list of Elm_Slideshow_Item*
   Eina_List            *items;
   Eina_List            *items_built;

   Elm_Slideshow_Item   *current;
   Elm_Slideshow_Item   *previous;

   Eina_List            *transitions;
   const char           *transition;

   int                   count_item_pre_before;
   int                   count_item_pre_after;
   Ecore_Timer          *timer;
   double                timeout;
   Eina_Bool             loop : 1;

   struct
   {
      const char *current;
      Eina_List  *list;  //list of const char *
   } layout;
};

/**
 * @}
 */

EAPI extern const char ELM_SLIDESHOW_SMART_NAME[];
EAPI const Elm_Slideshow_Smart_Class *elm_slideshow_smart_class_get(void);

#define ELM_SLIDESHOW_DATA_GET(o, sd) \
  Elm_Slideshow_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_SLIDESHOW_DATA_GET_OR_RETURN(o, ptr)     \
  ELM_SLIDESHOW_DATA_GET(o, ptr);                    \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_SLIDESHOW_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_SLIDESHOW_DATA_GET(o, ptr);                         \
  if (!ptr)                                               \
    {                                                     \
       CRITICAL("No widget data for object %p (%s)",      \
                o, evas_object_type_get(o));              \
       return val;                                        \
    }

#define ELM_SLIDESHOW_CHECK(obj)                     \
  if (!obj || !elm_widget_type_check                 \
        ((obj), ELM_SLIDESHOW_SMART_NAME, __func__)) \
    return

#define ELM_SLIDESHOW_ITEM_CHECK(it)                        \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_SLIDESHOW_CHECK(it->base.widget);

#define ELM_SLIDESHOW_ITEM_CHECK_OR_RETURN(it, ...)                    \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_SLIDESHOW_CHECK(it->base.widget) __VA_ARGS__;

#endif

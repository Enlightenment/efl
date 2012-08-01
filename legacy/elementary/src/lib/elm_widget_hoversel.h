#ifndef ELM_WIDGET_HOVERSEL_H
#define ELM_WIDGET_HOVERSEL_H

#include "elm_widget_button.h"

#ifdef HAVE_EIO
# include <Eio.h>
#endif

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-hoversel-class The Elementary Hoversel Class
 *
 * Elementary, besides having the @ref Hoversel widget, exposes its
 * foundation -- the Elementary Hoversel Class -- in order to create other
 * widgets which are a hoversel with some more logic on top.
 */

/**
 * @def ELM_HOVERSEL_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Hoversel_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_HOVERSEL_CLASS(x) ((Elm_Hoversel_Smart_Class *)x)

/**
 * @def ELM_HOVERSEL_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Hoversel_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_HOVERSEL_DATA(x)  ((Elm_Hoversel_Smart_Data *)x)

/**
 * @def ELM_HOVERSEL_SMART_CLASS_VERSION
 *
 * Current version for Elementary hoversel @b base smart class, a value
 * which goes to _Elm_Hoversel_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_HOVERSEL_SMART_CLASS_VERSION 1

/**
 * @def ELM_HOVERSEL_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Hoversel_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_HOVERSEL_SMART_CLASS_INIT_NULL
 * @see ELM_HOVERSEL_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_HOVERSEL_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_HOVERSEL_SMART_CLASS_VERSION}

/**
 * @def ELM_HOVERSEL_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Hoversel_Smart_Class structure.
 *
 * @see ELM_HOVERSEL_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_HOVERSEL_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_HOVERSEL_SMART_CLASS_INIT_NULL \
  ELM_HOVERSEL_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_HOVERSEL_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Hoversel_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_HOVERSEL_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Hoversel_Smart_Class (base field)
 * to the latest #ELM_HOVERSEL_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_HOVERSEL_SMART_CLASS_INIT_NULL
 * @see ELM_HOVERSEL_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_HOVERSEL_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_HOVERSEL_SMART_CLASS_INIT                          \
    (ELM_BUTTON_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary hoversel base smart class. This inherits directly from
 * #Elm_Button_Smart_Class and is meant to build widgets extending the
 * behavior of a hoversel.
 *
 * All of the functions listed on @ref Hoversel namespace will work for
 * objects deriving from #Elm_Hoversel_Smart_Class.
 */
typedef struct _Elm_Hoversel_Smart_Class
{
   Elm_Button_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Hoversel_Smart_Class;

/**
 * Base button smart data extended with hoversel instance data.
 */
typedef struct _Elm_Hoversel_Smart_Data Elm_Hoversel_Smart_Data;
struct _Elm_Hoversel_Smart_Data
{
   Elm_Button_Smart_Data base;

   /* aggregates a hover */
   Evas_Object          *hover;
   Evas_Object          *hover_parent;

   Eina_List            *items;

   Eina_Bool             horizontal : 1;
   Eina_Bool             expanded   : 1;
};

typedef struct _Elm_Hoversel_Item Elm_Hoversel_Item;
struct _Elm_Hoversel_Item
{
   ELM_WIDGET_ITEM;

   const char   *label;
   const char   *icon_file;
   const char   *icon_group;

   Elm_Icon_Type icon_type;
   Evas_Smart_Cb func;
};

/**
 * @}
 */

EAPI extern const char ELM_HOVERSEL_SMART_NAME[];
EAPI const Elm_Hoversel_Smart_Class *elm_hoversel_smart_class_get(void);

#define ELM_HOVERSEL_DATA_GET(o, sd) \
  Elm_Hoversel_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_HOVERSEL_DATA_GET_OR_RETURN(o, ptr)      \
  ELM_HOVERSEL_DATA_GET(o, ptr);                     \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_HOVERSEL_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_HOVERSEL_DATA_GET(o, ptr);                         \
  if (!ptr)                                              \
    {                                                    \
       CRITICAL("No widget data for object %p (%s)",     \
                o, evas_object_type_get(o));             \
       return val;                                       \
    }

#define ELM_HOVERSEL_CHECK(obj)                     \
  if (!obj || !elm_widget_type_check                \
        ((obj), ELM_HOVERSEL_SMART_NAME, __func__)) \
    return

#define ELM_HOVERSEL_ITEM_CHECK(it)                         \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_HOVERSEL_CHECK(it->base.widget);

#define ELM_HOVERSEL_ITEM_CHECK_OR_RETURN(it, ...)                     \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_HOVERSEL_CHECK(it->base.widget) __VA_ARGS__;

#endif

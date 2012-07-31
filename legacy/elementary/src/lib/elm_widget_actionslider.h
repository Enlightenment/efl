#ifndef ELM_WIDGET_ACTIONSLIDER_H
#define ELM_WIDGET_ACTIONSLIDER_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-actionslider-class The Elementary Actionslider Class
 *
 * Elementary, besides having the @ref Actionslider widget, exposes its
 * foundation -- the Elementary Actionslider Class -- in order to create other
 * widgets which are a actionslider with some more logic on top.
 */

/**
 * @def ELM_ACTIONSLIDER_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Actionslider_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_ACTIONSLIDER_CLASS(x) ((Elm_Actionslider_Smart_Class *)x)

/**
 * @def ELM_ACTIONSLIDER_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Actionslider_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_ACTIONSLIDER_DATA(x)  ((Elm_Actionslider_Smart_Data *)x)

/**
 * @def ELM_ACTIONSLIDER_SMART_CLASS_VERSION
 *
 * Current version for Elementary actionslider @b base smart class, a value
 * which goes to _Elm_Actionslider_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_ACTIONSLIDER_SMART_CLASS_VERSION 1

/**
 * @def ELM_ACTIONSLIDER_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Actionslider_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_ACTIONSLIDER_SMART_CLASS_INIT_NULL
 * @see ELM_ACTIONSLIDER_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_ACTIONSLIDER_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_ACTIONSLIDER_SMART_CLASS_VERSION}

/**
 * @def ELM_ACTIONSLIDER_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Actionslider_Smart_Class structure.
 *
 * @see ELM_ACTIONSLIDER_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_ACTIONSLIDER_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_ACTIONSLIDER_SMART_CLASS_INIT_NULL \
  ELM_ACTIONSLIDER_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_ACTIONSLIDER_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Actionslider_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_ACTIONSLIDER_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Actionslider_Smart_Class (base field)
 * to the latest #ELM_ACTIONSLIDER_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_ACTIONSLIDER_SMART_CLASS_INIT_NULL
 * @see ELM_ACTIONSLIDER_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_ACTIONSLIDER_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_ACTIONSLIDER_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary actionslider base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a actionslider.
 *
 * All of the functions listed on @ref Actionslider namespace will work for
 * objects deriving from #Elm_Actionslider_Smart_Class.
 */
typedef struct _Elm_Actionslider_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Actionslider_Smart_Class;

/**
 * Base layout smart data extended with actionslider instance data.
 */
typedef struct _Elm_Actionslider_Smart_Data Elm_Actionslider_Smart_Data;
struct _Elm_Actionslider_Smart_Data
{
   Elm_Layout_Smart_Data base;

   Evas_Object          *drag_button_base;
   Elm_Actionslider_Pos  magnet_position, enabled_position;
   Ecore_Animator       *button_animator;
   double                final_position;

   Eina_Bool             mouse_down : 1;
};

/**
 * @}
 */

EAPI extern const char ELM_ACTIONSLIDER_SMART_NAME[];
EAPI const Elm_Actionslider_Smart_Class *elm_actionslider_smart_class_get(void);

#define ELM_ACTIONSLIDER_DATA_GET(o, sd) \
  Elm_Actionslider_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_ACTIONSLIDER_DATA_GET_OR_RETURN(o, ptr)     \
  ELM_ACTIONSLIDER_DATA_GET(o, ptr);                    \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_ACTIONSLIDER_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_ACTIONSLIDER_DATA_GET(o, ptr);                         \
  if (!ptr)                                               \
    {                                                     \
       CRITICAL("No widget data for object %p (%s)",      \
                o, evas_object_type_get(o));              \
       return val;                                        \
    }

#define ELM_ACTIONSLIDER_CHECK(obj)                     \
  if (!obj || !elm_widget_type_check                 \
        ((obj), ELM_ACTIONSLIDER_SMART_NAME, __func__)) \
    return

#endif

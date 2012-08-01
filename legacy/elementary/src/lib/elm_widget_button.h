#ifndef ELM_WIDGET_BUTTON_H
#define ELM_WIDGET_BUTTON_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-button-class The Elementary Button Class
 *
 * Elementary, besides having the @ref Button widget, exposes its
 * foundation -- the Elementary Button Class -- in order to create
 * other widgets which are, basically, a button with some more logic
 * on top.
 */

/**
 * @def ELM_BUTTON_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Button_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_BUTTON_CLASS(x) ((Elm_Button_Smart_Class *)x)

/**
 * @def ELM_BUTTON_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Button_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_BUTTON_DATA(x)  ((Elm_Button_Smart_Data *)x)

/**
 * @def ELM_BUTTON_SMART_CLASS_VERSION
 *
 * Current version for Elementary button @b base smart class, a value
 * which goes to _Elm_Button_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_BUTTON_SMART_CLASS_VERSION 1

/**
 * @def ELM_BUTTON_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Button_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_BUTTON_SMART_CLASS_INIT_NULL
 * @see ELM_BUTTON_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_BUTTON_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_BUTTON_SMART_CLASS_VERSION, EINA_TRUE}

/**
 * @def ELM_BUTTON_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Button_Smart_Class structure.
 *
 * @see ELM_BUTTON_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_BUTTON_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_BUTTON_SMART_CLASS_INIT_NULL \
  ELM_BUTTON_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_BUTTON_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Button_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_BUTTON_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Button_Smart_Class (base field)
 * to the latest #ELM_BUTTON_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_BUTTON_SMART_CLASS_INIT_NULL
 * @see ELM_BUTTON_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_BUTTON_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_BUTTON_SMART_CLASS_INIT(ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary button base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a button.
 *
 * All of the functions listed on @ref Button namespace will work for
 * objects deriving from #Elm_Button_Smart_Class.
 */
typedef struct _Elm_Button_Smart_Class
{
   Elm_Layout_Smart_Class base; /**< Layout widget class struct, since we're inheriting from it */

   int                    version; /**< Version of this smart class definition */

   Eina_Bool              admits_autorepeat : 1; /**< Whether the button objects of this class admit auto-repetition of click events, when one holds the click on them. This is true by default. */
} Elm_Button_Smart_Class;

/**
 * Base widget smart data extended with button instance data.
 */
typedef struct _Elm_Button_Smart_Data
{
   Elm_Layout_Smart_Data base; /**< Base widget smart data as first member obligatory, as we're inheriting from it */

   /* auto-repeat stuff */
   double                ar_threshold; /**< Time to wait until first auto-repeated click is generated */
   double                ar_interval; /**< Time frame for subsequent auto-repeated clicks, after the first automatic one is triggerred */

   Ecore_Timer          *timer; /**< Internal timer object for auto-repeat behavior */

   Eina_Bool             autorepeat : 1; /**< Whether auto-repetition of clicks is enabled or not (bound to _Elm_Button_Smart_Class::admits_autorepeat) */
   Eina_Bool             repeating : 1; /**< Whether auto-repetition is going on */
} Elm_Button_Smart_Data;

/**
 * @}
 */

EAPI const Elm_Button_Smart_Class *elm_button_smart_class_get(void);

#define ELM_BUTTON_DATA_GET(o, sd) \
  Elm_Button_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_BUTTON_DATA_GET_OR_RETURN(o, ptr)        \
  ELM_BUTTON_DATA_GET(o, ptr);                       \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_BUTTON_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_BUTTON_DATA_GET(o, ptr);                         \
  if (!ptr)                                            \
    {                                                  \
       CRITICAL("No widget data for object %p (%s)",   \
                o, evas_object_type_get(o));           \
       return val;                                     \
    }

#define ELM_BUTTON_CHECK(obj)                     \
  if (!obj || !elm_widget_type_check              \
        ((obj), ELM_BUTTON_SMART_NAME, __func__)) \
    return

#endif

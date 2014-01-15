#ifndef ELM_WIDGET_BUTTON_H
#define ELM_WIDGET_BUTTON_H

#include "Elementary.h"

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
 * Base widget smart data extended with button instance data.
 */
typedef struct _Elm_Button_Smart_Data
{
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

#define ELM_BUTTON_DATA_GET(o, sd) \
  Elm_Button_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_BUTTON_CLASS)

#define ELM_BUTTON_DATA_GET_OR_RETURN(o, ptr)        \
  ELM_BUTTON_DATA_GET(o, ptr);                       \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_BUTTON_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_BUTTON_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                             \
    {                                                  \
       CRI("No widget data for object %p (%s)",        \
           o, evas_object_type_get(o));                \
       return val;                                     \
    }

#define ELM_BUTTON_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_BUTTON_CLASS))) \
    return

#endif

#ifndef ELM_WIDGET_ACTIONSLIDER_H
#define ELM_WIDGET_ACTIONSLIDER_H

#include "Elementary.h"

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
 * Base layout smart data extended with actionslider instance data.
 */
typedef struct _Elm_Actionslider_Smart_Data Elm_Actionslider_Smart_Data;
struct _Elm_Actionslider_Smart_Data
{
   Evas_Object          *drag_button_base;
   Elm_Actionslider_Pos  magnet_position, enabled_position;
   Ecore_Animator       *button_animator;
   double                final_position;

   Eina_Bool             mouse_down : 1;
};

/**
 * @}
 */

#define ELM_ACTIONSLIDER_DATA_GET(o, sd) \
  Elm_Actionslider_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_ACTIONSLIDER_CLASS)

#define ELM_ACTIONSLIDER_DATA_GET_OR_RETURN(o, ptr)  \
  ELM_ACTIONSLIDER_DATA_GET(o, ptr);                 \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_ACTIONSLIDER_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_ACTIONSLIDER_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                   \
    {                                                        \
       CRI("No widget data for object %p (%s)",              \
           o, evas_object_type_get(o));                      \
       return val;                                           \
    }

#define ELM_ACTIONSLIDER_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_ACTIONSLIDER_CLASS))) \
    return

#endif

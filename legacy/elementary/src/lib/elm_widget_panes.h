#ifndef ELM_WIDGET_PANES_H
#define ELM_WIDGET_PANES_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-panes-class The Elementary Panes Class
 *
 * Elementary, besides having the @ref Panes widget, exposes its
 * foundation -- the Elementary Panes Class -- in order to create other
 * widgets which are a panes with some more logic on top.
 */

/**
 * Base layout smart data extended with panes instance data.
 */
typedef struct _Elm_Panes_Smart_Data Elm_Panes_Smart_Data;
struct _Elm_Panes_Smart_Data
{
   Evas_Object *event;
   struct
   {
      int       x_diff;
      int       y_diff;
      Eina_Bool move;
   } move;

   Eina_Bool             double_clicked : 1;
   Eina_Bool             horizontal : 1;
   Eina_Bool             fixed : 1;
};

/**
 * @}
 */

#define ELM_PANES_DATA_GET(o, sd) \
  Elm_Panes_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_PANES_CLASS)

#define ELM_PANES_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_PANES_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_PANES_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_PANES_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_PANES_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_PANES_CLASS))) \
    return

#endif

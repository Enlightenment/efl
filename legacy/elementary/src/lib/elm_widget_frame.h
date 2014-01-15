#ifndef ELM_WIDGET_FRAME_H
#define ELM_WIDGET_FRAME_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-frame-class The Elementary Frame Class
 *
 * Elementary, besides having the @ref Frame widget, exposes its
 * foundation -- the Elementary Frame Class -- in order to create other
 * widgets which are a frame with some more logic on top.
 */

/**
 * Base layout smart data extended with frame instance data.
 */
typedef struct _Elm_Frame_Smart_Data Elm_Frame_Smart_Data;
struct _Elm_Frame_Smart_Data
{
   Eina_Bool             collapsed : 1;
   Eina_Bool             collapsible : 1;
   Eina_Bool             anim : 1;
};

/**
 * @}
 */

#define ELM_FRAME_DATA_GET(o, sd) \
  Elm_Frame_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_FRAME_CLASS)

#define ELM_FRAME_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_FRAME_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_FRAME_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_FRAME_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_FRAME_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_FRAME_CLASS))) \
    return

#endif

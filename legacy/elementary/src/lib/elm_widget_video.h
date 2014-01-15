#ifndef ELM_WIDGET_VIDEO_H
#define ELM_WIDGET_VIDEO_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-video-class The Elementary Video Class
 *
 * Elementary, besides having the @ref Video widget, exposes its
 * foundation -- the Elementary Video Class -- in order to create other
 * widgets which are a video with some more logic on top.
 */

/**
 * Base layout smart data extended with video instance data.
 */
typedef struct _Elm_Video_Smart_Data Elm_Video_Smart_Data;
struct _Elm_Video_Smart_Data
{
   Evas_Object          *emotion;
   Ecore_Timer          *timer;

   Eina_Bool             stop : 1;
   Eina_Bool             remember : 1;
};

/**
 * @}
 */

#define ELM_VIDEO_DATA_GET(o, sd) \
  Elm_Video_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_VIDEO_CLASS)

#define ELM_VIDEO_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_VIDEO_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_VIDEO_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_VIDEO_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_VIDEO_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_VIDEO_CLASS))) \
    return

#endif

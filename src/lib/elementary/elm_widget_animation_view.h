#ifndef ELM_WIDGET_ANIMATION_VIEW_H
#define ELM_WIDGET_ANIMATION_VIEW_H

#include "Elementary.h"
#include "elm_animation_view.eo.h"

typedef struct _Elm_Animation_View_Data Elm_Animation_View_Data;

struct _Elm_Animation_View_Data
{
   Eo* obj;                           //Elm_Animation_View Object
   Eo* vg;                            //Evas_Object_Vg
   Elm_Animation_View_State state;
   Elm_Transit *transit;
   Eina_Stringshare *file;
   double speed;
   double keyframe;
   double frame_cnt;
   int repeat_times;
   double frame_duration;

   Eina_Bool play_back : 1;
   Eina_Bool auto_play : 1;
   Eina_Bool auto_play_pause: 1;
   Eina_Bool auto_repeat : 1;
};

#define ELM_ANIMATION_VIEW_DATA_GET(o, sd) \
  Elm_Animation_View_Data * sd = efl_data_scope_get(o, ELM_ANIMATION_VIEW_CLASS)

#define ELM_ANIMATION_VIEW_DATA_GET_OR_RETURN(o, ptr)   \
  ELM_ANIMATION_VIEW_DATA_GET(o, ptr);                  \
  if (EINA_UNLIKELY(!ptr))                              \
    {                                                   \
       ERR("No widget data for object %p (%s)",         \
           o, evas_object_type_get(o));                 \
       return;                                          \
    }

#endif

#ifndef EFL_UI_ANIMATION_VIEW_PRIVATE_H
#define EFL_UI_ANIMATION_VIEW_PRIVATE_H

#include "Elementary.h"
#include "efl_ui_animation_view.eo.h"

typedef struct _Efl_Ui_Animation_View_Data Efl_Ui_Animation_View_Data;

struct _Efl_Ui_Animation_View_Data
{
   Eo* obj;                           //Efl_Ui_Animation_View Object
   Eo* vg;                            //Evas_Object_Vg
   Efl_Ui_Animation_View_State state;
   Elm_Transit *transit;
   Eina_Stringshare *file;
   double speed;
   double keyframe;
   double frame_cnt;
   int repeat_times;
   double frame_duration;
   double min_progress;
   double max_progress;

   Eina_Bool play_back : 1;
   Eina_Bool auto_play : 1;
   Eina_Bool auto_play_pause: 1;
   Eina_Bool auto_repeat : 1;
};

#define EFL_UI_ANIMATION_VIEW_DATA_GET(o, sd) \
  Efl_Ui_Animation_View_Data * sd = efl_data_scope_get(o, EFL_UI_ANIMATION_VIEW_CLASS)

#define EFL_UI_ANIMATION_VIEW_DATA_GET_OR_RETURN(o, ptr)   \
  EFL_UI_ANIMATION_VIEW_DATA_GET(o, ptr);                  \
  if (EINA_UNLIKELY(!ptr))                              \
    {                                                   \
       ERR("No widget data for object %p (%s)",         \
           o, evas_object_type_get(o));                 \
       return;                                          \
    }

#endif

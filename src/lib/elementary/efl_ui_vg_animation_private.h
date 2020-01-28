#ifndef EFL_UI_VG_ANIMATION_PRIVATE_H
#define EFL_UI_VG_ANIMATION_PRIVATE_H

#include "Elementary.h"
#include "efl_ui_vg_animation.eo.h"

typedef struct _Efl_Ui_Vg_Animation_Data Efl_Ui_Vg_Animation_Data;

struct _Efl_Ui_Vg_Animation_Data
{
   Eo* vg;                            //Evas_Object_Vg
   Efl_Ui_Vg_Animation_State state;
   Elm_Transit *transit;
   Eina_Stringshare *file;
   double playback_speed;
   double progress;
   double frame_cnt;
   int repeat_times;
   double frame_duration;
   double min_progress;
   double max_progress;
   Eina_List *vp_list;

   Eina_Bool playing_reverse : 1;
   Eina_Bool autoplay : 1;
   Eina_Bool autoplay_pause: 1;
   Eina_Bool loop : 1;
   Eina_Bool playback_direction_changed : 1;
};

#define EFL_UI_VG_ANIMATION_DATA_GET(o, sd) \
  Efl_Ui_Vg_Animation_Data * sd = efl_data_scope_safe_get(o, EFL_UI_VG_ANIMATION_CLASS)

#define EFL_UI_VG_ANIMATION_DATA_GET_OR_RETURN(o, ptr)   \
  EFL_UI_VG_ANIMATION_DATA_GET(o, ptr);                  \
  if (EINA_UNLIKELY(!ptr))                              \
    {                                                   \
       ERR("No widget data for object %p (%s)",         \
           o, evas_object_type_get(o));                 \
       return;                                          \
    }

#endif

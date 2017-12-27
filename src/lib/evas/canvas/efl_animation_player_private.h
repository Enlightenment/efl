#define EFL_ANIMATION_PLAYER_PROTECTED

#include "evas_common_private.h"
#include <Ecore.h>

#define MY_CLASS EFL_ANIMATION_PLAYER_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#if 0
typedef struct _Target_State
{
   Evas_Coord x, y, w, h;
   int        r, g, b, a;

   Evas_Map  *map;
   Eina_Bool  enable_map : 1;
} Target_State;
#endif

typedef struct _Efl_Animation_Player_Data
{
   Ecore_Animator                  *animator;
   Ecore_Timer                     *start_delay_timer;

   struct {
        double prev;
        double begin;
        double current;
        double pause_begin;
     } time;

   Efl_Animation                   *animation;
   Efl_Canvas_Object               *target;

   double                           progress;
   double                           play_speed;

   int                              remaining_repeat_count;

   Efl_Interpolator                *interpolator;

   Eina_Bool                        auto_del : 1;
   Eina_Bool                        is_play : 1;
   Eina_Bool                        keep_final_state : 1;
   Eina_Bool                        is_direction_forward : 1;
} Efl_Animation_Player_Data;

#define EFL_ANIMATION_PLAYER_DATA_GET(o, pd) \
   Efl_Animation_Player_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_PLAYER_CLASS)

#define EFL_ANIMATION_PLAYER_ANIMATION_GET(o, anim) \
   Efl_Animation *anim = efl_animation_player_animation_get(o)

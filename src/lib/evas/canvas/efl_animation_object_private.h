#define EFL_ANIMATION_OBJECT_PROTECTED

#include "evas_common_private.h"
#include <Ecore.h>

#define MY_CLASS EFL_ANIMATION_OBJECT_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

typedef struct _Target_State
{
   Evas_Coord x, y, w, h;
   int        r, g, b, a;

   Evas_Map  *map;
   Eina_Bool  enable_map : 1;
} Target_State;

typedef struct _Efl_Animation_Object_Data
{
   Ecore_Animator    *animator;

   struct {
        double begin;
        double current;
     } time;

   Efl_Canvas_Object *target;
   Target_State      *target_state;

   double             progress;

   double             duration;
   double             total_duration;

   Eina_Bool          auto_del : 1;
   Eina_Bool          is_deleted : 1;
   Eina_Bool          is_cancelled : 1;
   Eina_Bool          keep_final_state : 1;
} Efl_Animation_Object_Data;

#define EFL_ANIMATION_OBJECT_CHECK_OR_RETURN(anim_obj, ...) \
   do { \
      if (!anim_obj) { \
         CRI("Efl_Animation_Object " # anim_obj " is NULL!"); \
         return __VA_ARGS__; \
      } \
      if (efl_animation_object_is_deleted(anim_obj)) { \
         ERR("Efl_Animation_Object " # anim_obj " has already been deleted!"); \
         return __VA_ARGS__; \
      } \
   } while (0)

#define EFL_ANIMATION_OBJECT_DATA_GET(o, pd) \
   Efl_Animation_Object_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_OBJECT_CLASS)

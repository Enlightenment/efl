#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"

#define MY_CLASS EFL_ANIMATION_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

typedef struct _Efl_Animation_Data
{
   Efl_Canvas_Object *target;

   double             duration;
   double             total_duration;

   Eina_Bool          is_deleted : 1;
   Eina_Bool          keep_final_state : 1;
} Efl_Animation_Data;

#define EFL_ANIMATION_CHECK_OR_RETURN(anim, ...) \
   do { \
      if (!anim) { \
         CRI("Efl_Animation " # anim " is NULL!"); \
         return __VA_ARGS__; \
      } \
      if (efl_animation_is_deleted(anim)) { \
         ERR("Efl_Animation " # anim " has already been deleted!"); \
         return __VA_ARGS__; \
      } \
   } while (0)

#define EFL_ANIMATION_DATA_GET(o, pd) \
   Efl_Animation_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_CLASS)

#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"

#define MY_CLASS EFL_ANIMATION_TRANSLATE_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(anim, ...) \
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

#define EFL_ANIMATION_TRANSLATE_DATA_GET(o, pd) \
   Efl_Animation_Translate_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_TRANSLATE_CLASS)

typedef struct _Efl_Animation_Translate_Property
{
   Evas_Coord move_x, move_y;
   Evas_Coord x, y;
} Efl_Animation_Translate_Property;

typedef struct _Efl_Animation_Translate_Data
{
   Efl_Animation_Translate_Property from;
   Efl_Animation_Translate_Property to;

   Eina_Bool use_rel_move;
} Efl_Animation_Translate_Data;

#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include <Ecore.h>

#define MY_CLASS EFL_ANIMATION_ROTATE_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(anim, ...) \
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

#define EFL_ANIMATION_ROTATE_DATA_GET(o, pd) \
   Efl_Animation_Rotate_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_ROTATE_CLASS)

typedef struct _Efl_Animation_Rotate_Property
{
   double degree;
} Efl_Animation_Rotate_Property;

typedef struct _Efl_Animation_Rotate_Absolute_Pivot
{
   Evas_Coord cx, cy;
} Efl_Animation_Rotate_Absolute_Pivot;

typedef struct _Efl_Animation_Rotate_Relative_Pivot
{
   Efl_Canvas_Object *obj;
   double             cx, cy;
} Efl_Animation_Rotate_Relative_Pivot;

typedef struct _Efl_Animation_Rotate_Data
{
   Efl_Animation_Rotate_Property       from;
   Efl_Animation_Rotate_Property       to;

   Efl_Animation_Rotate_Absolute_Pivot abs_pivot;
   Efl_Animation_Rotate_Relative_Pivot rel_pivot;

   Eina_Bool use_rel_pivot;
} Efl_Animation_Rotate_Data;

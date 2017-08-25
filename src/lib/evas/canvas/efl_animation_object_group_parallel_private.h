#define EFL_ANIMATION_OBJECT_PROTECTED

#include "evas_common_private.h"

#define MY_CLASS EFL_ANIMATION_OBJECT_GROUP_PARALLEL_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_OBJECT_GROUP_PARALLEL_CHECK_OR_RETURN(anim_obj, ...) \
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

#define EFL_ANIMATION_OBJECT_GROUP_PARALLEL_DATA_GET(o, pd) \
   Efl_Animation_Object_Group_Parallel_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_OBJECT_GROUP_PARALLEL_CLASS)

typedef struct _Efl_Animation_Object_Group_Parallel_Data
{
} Efl_Animation_Object_Group_Parallel_Data;

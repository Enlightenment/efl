#define EFL_ANIMATION_OBJECT_PROTECTED

#include "evas_common_private.h"
#include <Ecore.h>

#define MY_CLASS EFL_ANIMATION_OBJECT_GROUP_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_OBJECT_GROUP_DATA_GET(o, pd) \
   Efl_Animation_Object_Group_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_OBJECT_GROUP_CLASS)

typedef struct _Efl_Animation_Object_Group_Data
{
   Eina_List *anim_objs;
} Efl_Animation_Object_Group_Data;

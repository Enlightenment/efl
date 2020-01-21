#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include <Ecore.h>

#define MY_CLASS EFL_CANVAS_GROUP_ANIMATION_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_GROUP_ANIMATION_DATA_GET(o, pd) \
   Efl_Canvas_Group_Animation_Data *pd = efl_data_scope_get(o, EFL_CANVAS_GROUP_ANIMATION_CLASS)

typedef struct _Efl_Canvas_Group_Animation_Data
{
   Eina_List *animations;
   Eina_Bool is_duration_set : 1; //EINA_TRUE if duration is set
} Efl_Canvas_Group_Animation_Data;

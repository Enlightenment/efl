#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include <Ecore.h>
#include "efl_canvas_animation_private.h"

#define EFL_ANIMATION_SCALE_DATA_GET(o, pd) \
   Efl_Canvas_Animation_Scale_Data *pd = efl_data_scope_get(o, EFL_CANVAS_ANIMATION_SCALE_CLASS)

typedef struct _Efl_Canvas_Animation_Scale_Relative_Pivot
{
   Efl_Canvas_Object *obj;
   Eina_Vector2 pos;
} Efl_Canvas_Animation_Scale_Relative_Pivot;

typedef struct _Efl_Canvas_Animation_Scale_Data
{
   Eina_Vector2 from;
   Eina_Vector2 to;

   Eina_Position2D pos;
   Efl_Canvas_Animation_Scale_Relative_Pivot rel_pivot;

   Eina_Bool use_rel_pivot;
} Efl_Canvas_Animation_Scale_Data;

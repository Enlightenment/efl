#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include <Ecore.h>
#include "efl_canvas_animation_private.h"

#define EFL_ROTATE_ANIMATION_DATA_GET(o, pd) \
   Efl_Canvas_Rotate_Animation_Data *pd = efl_data_scope_get(o, EFL_CANVAS_ROTATE_ANIMATION_CLASS)

typedef struct _Efl_Canvas_Rotate_Animation_Property
{
   double degree;
} Efl_Canvas_Rotate_Animation_Property;

typedef struct _Efl_Canvas_Rotate_Animation_Relative_Pivot
{
   Efl_Canvas_Object *obj;
   Eina_Vector2       pos;
} Efl_Canvas_Rotate_Animation_Relative_Pivot;

typedef struct _Efl_Canvas_Rotate_Animation_Data
{
   Efl_Canvas_Rotate_Animation_Property       from;
   Efl_Canvas_Rotate_Animation_Property       to;

   Eina_Position2D abs_pivot;
   Efl_Canvas_Rotate_Animation_Relative_Pivot rel_pivot;

   Eina_Bool use_rel_pivot;
} Efl_Canvas_Rotate_Animation_Data;

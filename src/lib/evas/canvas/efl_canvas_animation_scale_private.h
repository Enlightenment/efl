#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include <Ecore.h>
#include "efl_canvas_animation_private.h"

#define EFL_ANIMATION_SCALE_DATA_GET(o, pd) \
   Efl_Canvas_Animation_Scale_Data *pd = efl_data_scope_get(o, EFL_CANVAS_ANIMATION_SCALE_CLASS)

typedef struct _Efl_Canvas_Animation_Scale_Property
{
   double scale_x, scale_y;
} Efl_Canvas_Animation_Scale_Property;

typedef struct _Efl_Canvas_Animation_Scale_Absolute_Pivot
{
   Evas_Coord cx, cy;
} Efl_Canvas_Animation_Scale_Absolute_Pivot;

typedef struct _Efl_Canvas_Animation_Scale_Relative_Pivot
{
   Efl_Canvas_Object *obj;
   double             cx, cy;
} Efl_Canvas_Animation_Scale_Relative_Pivot;

typedef struct _Efl_Canvas_Animation_Scale_Data
{
   Efl_Canvas_Animation_Scale_Property from;
   Efl_Canvas_Animation_Scale_Property to;

   Efl_Canvas_Animation_Scale_Absolute_Pivot abs_pivot;
   Efl_Canvas_Animation_Scale_Relative_Pivot rel_pivot;

   Eina_Bool use_rel_pivot;
} Efl_Canvas_Animation_Scale_Data;

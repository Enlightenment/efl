#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include <Ecore.h>
#include "efl_animation_private.h"

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

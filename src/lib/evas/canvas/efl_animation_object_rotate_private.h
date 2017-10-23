#define EFL_ANIMATION_OBJECT_PROTECTED
#define EFL_ANIMATION_OBJECT_ROTATE_PROTECTED

#include "evas_common_private.h"

#define MY_CLASS EFL_ANIMATION_OBJECT_ROTATE_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_OBJECT_ROTATE_DATA_GET(o, pd) \
   Efl_Animation_Object_Rotate_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_OBJECT_ROTATE_CLASS)

typedef struct _Efl_Animation_Object_Rotate_Property
{
   double degree;
} Efl_Animation_Object_Rotate_Property;

typedef struct _Efl_Animation_Object_Rotate_Absolute_Pivot
{
   Evas_Coord cx, cy;
} Efl_Animation_Object_Rotate_Absolute_Pivot;

typedef struct _Efl_Animation_Object_Rotate_Relative_Pivot
{
   Efl_Canvas_Object *obj;
   double             cx, cy;
} Efl_Animation_Object_Rotate_Relative_Pivot;

typedef struct _Efl_Animation_Object_Rotate_Data
{
   Efl_Animation_Object_Rotate_Property       from;
   Efl_Animation_Object_Rotate_Property       to;

   Efl_Animation_Object_Rotate_Absolute_Pivot abs_pivot;
   Efl_Animation_Object_Rotate_Relative_Pivot rel_pivot;

   Eina_Bool use_rel_pivot;
} Efl_Animation_Object_Rotate_Data;

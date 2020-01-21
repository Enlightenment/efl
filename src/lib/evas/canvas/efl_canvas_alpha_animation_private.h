#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include <Ecore.h>
#include "efl_canvas_animation_private.h"

#define EFL_ALPHA_ANIMATION_DATA_GET(o, pd) \
   Efl_Canvas_Alpha_Animation_Data *pd = efl_data_scope_get(o, EFL_CANVAS_ALPHA_ANIMATION_CLASS)

typedef struct _Efl_Canvas_Alpha_Animation_Property
{
   double alpha;
} Efl_Canvas_Alpha_Animation_Property;

typedef struct _Efl_Canvas_Alpha_Animation_Data
{
   Efl_Canvas_Alpha_Animation_Property from;
   Efl_Canvas_Alpha_Animation_Property to;
} Efl_Canvas_Alpha_Animation_Data;

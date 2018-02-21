#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include <Ecore.h>
#include "efl_canvas_animation_private.h"

#define EFL_ANIMATION_ALPHA_DATA_GET(o, pd) \
   Efl_Canvas_Animation_Alpha_Data *pd = efl_data_scope_get(o, EFL_CANVAS_ANIMATION_ALPHA_CLASS)

typedef struct _Efl_Canvas_Animation_Alpha_Property
{
   double alpha;
} Efl_Canvas_Animation_Alpha_Property;

typedef struct _Efl_Canvas_Animation_Alpha_Data
{
   Efl_Canvas_Animation_Alpha_Property from;
   Efl_Canvas_Animation_Alpha_Property to;
} Efl_Canvas_Animation_Alpha_Data;

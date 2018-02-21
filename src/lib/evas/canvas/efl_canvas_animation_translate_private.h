#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include <Ecore.h>
#include "efl_canvas_animation_private.h"

#define EFL_ANIMATION_TRANSLATE_DATA_GET(o, pd) \
   Efl_Canvas_Animation_Translate_Data *pd = efl_data_scope_get(o, EFL_CANVAS_ANIMATION_TRANSLATE_CLASS)

typedef struct _Efl_Canvas_Animation_Translate_Property
{
   Evas_Coord move_x, move_y;
   Evas_Coord x, y;
} Efl_Canvas_Animation_Translate_Property;

typedef struct _Efl_Canvas_Animation_Translate_Data
{
   Efl_Canvas_Animation_Translate_Property from;
   Efl_Canvas_Animation_Translate_Property to;

   Eina_Bool use_rel_move;
} Efl_Canvas_Animation_Translate_Data;

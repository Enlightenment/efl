#define EFL_ANIMATION_PROTECTED

#include "evas_common_private.h"
#include <Ecore.h>
#include "efl_canvas_animation_private.h"

#define EFL_TRANSLATE_ANIMATION_DATA_GET(o, pd) \
   Efl_Canvas_Translate_Animation_Data *pd = efl_data_scope_get(o, EFL_CANVAS_TRANSLATE_ANIMATION_CLASS)

typedef struct _Efl_Canvas_Translate_Animation_Data
{
   Eina_Position2D from;
   Eina_Position2D to;

   Eina_Bool use_rel_move;
} Efl_Canvas_Translate_Animation_Data;

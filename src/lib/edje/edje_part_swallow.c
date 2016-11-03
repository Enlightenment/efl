#include "edje_private.h"
#include "edje_part_helper.h"

#define EFL_CANVAS_LAYOUT_INTERNAL_SWALLOW_PROTECTED
#include "efl_canvas_layout_internal_swallow.eo.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_INTERNAL_SWALLOW_CLASS

PROXY_IMPLEMENTATION(swallow, SWALLOW, Edje_Swallow_Data)
#undef PROXY_IMPLEMENTATION

/* Swallow parts */
EOLIAN static Efl_Gfx *
_efl_canvas_layout_internal_swallow_efl_container_content_get(Eo *obj, Edje_Swallow_Data *pd)
{
   RETURN_VAL(_edje_efl_container_content_get(pd->ed, pd->part));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_swallow_efl_container_content_set(Eo *obj, Edje_Swallow_Data *pd, Efl_Gfx *content)
{
   RETURN_VAL(_edje_efl_container_content_set(pd->ed, pd->part, content));
}

EOLIAN static Efl_Gfx *
_efl_canvas_layout_internal_swallow_efl_container_content_unset(Eo *obj, Edje_Swallow_Data *pd)
{
   Efl_Gfx *content = _edje_efl_container_content_get(pd->ed, pd->part);
   if (!content) RETURN_VAL(NULL);
   PROXY_CALL(efl_content_remove(obj, content));
   RETURN_VAL(content);
}

#include "efl_canvas_layout_internal_swallow.eo.c"

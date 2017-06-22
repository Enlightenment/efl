#include "edje_private.h"
#include "edje_part_helper.h"
#include "efl_canvas_layout_internal_swallow.eo.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_INTERNAL_SWALLOW_CLASS

PROXY_IMPLEMENTATION(swallow, MY_CLASS, EINA_FALSE)
#undef PROXY_IMPLEMENTATION

/* Swallow parts */
EOLIAN static Efl_Gfx *
_efl_canvas_layout_internal_swallow_efl_container_content_get(Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   RETURN_VAL(_edje_efl_container_content_get(pd->ed, pd->part));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_swallow_efl_container_content_set(Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx *content)
{
   PROXY_DATA_GET(obj, pd);
   RETURN_VAL(_edje_efl_container_content_set(pd->ed, pd->part, content));
}

EOLIAN static Efl_Gfx *
_efl_canvas_layout_internal_swallow_efl_container_content_unset(Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   Efl_Gfx *content = _edje_efl_container_content_get(pd->ed, pd->part);
   if (!content) RETURN_VAL(NULL);
   PROXY_CALL(efl_content_remove(obj, content));
   RETURN_VAL(content);
}

#include "efl_canvas_layout_internal_swallow.eo.c"

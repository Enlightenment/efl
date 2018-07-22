#include "edje_private.h"
#include "edje_part_helper.h"
#include "efl_canvas_layout_part_swallow.eo.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_PART_SWALLOW_CLASS

PROXY_IMPLEMENTATION(swallow, MY_CLASS, EINA_FALSE)
#undef PROXY_IMPLEMENTATION

/* Swallow parts */
EOLIAN static Efl_Gfx_Entity *
_efl_canvas_layout_part_swallow_efl_content_content_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_efl_content_content_get(pd->ed, pd->part);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_swallow_efl_content_content_set(Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx_Entity *content)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_efl_content_content_set(pd->ed, pd->part, content);
}

EOLIAN static Efl_Gfx_Entity *
_efl_canvas_layout_part_swallow_efl_content_content_unset(Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   Efl_Gfx_Entity *content = _edje_efl_content_content_get(pd->ed, pd->part);
   if (!content) return NULL;
   efl_content_remove(obj, content);
   return content;
}

#include "efl_canvas_layout_part_swallow.eo.c"

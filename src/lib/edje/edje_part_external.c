#include "edje_private.h"
#include "edje_part_helper.h"

#define MY_CLASS EFL_CANVAS_LAYOUT_PART_EXTERNAL_CLASS

static void _external_compose(Eo *obj, Edje *ed, const char *part);

PROXY_IMPLEMENTATION(external, MY_CLASS, EINA_TRUE, _external_compose(proxy, ed, rp->part->name))
#undef PROXY_IMPLEMENTATION

static void
_external_compose(Eo *obj, Edje *ed, const char *part)
{
   Eo *ext_obj = _edje_object_part_external_object_get(ed, part);
   efl_composite_attach(obj, ext_obj);
}

EOLIAN static Efl_Gfx_Entity *
_efl_canvas_layout_part_external_efl_content_content_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_object_part_external_object_get(pd->ed, pd->part);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_external_efl_content_content_set(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Gfx_Entity *content EINA_UNUSED)
{
   ERR("Setting of content is not permitted on this part");
   return EINA_FALSE;
}

EOLIAN static Efl_Gfx_Entity*
_efl_canvas_layout_part_external_efl_content_content_unset(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   ERR("Unsetting of content is not permitted on this part");
   return NULL;
}

#include "efl_canvas_layout_part_external.eo.c"

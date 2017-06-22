#include "edje_private.h"
#include "edje_part_helper.h"

#define MY_CLASS EFL_CANVAS_LAYOUT_EXTERNAL_CLASS

static void _external_compose(Eo *obj, Edje *ed, const char *part);

PROXY_IMPLEMENTATION(external, MY_CLASS, EINA_TRUE, _external_compose(proxy, ed, rp->part->name))
#undef PROXY_IMPLEMENTATION

static void
_external_compose(Eo *obj, Edje *ed, const char *part)
{
   Eo *ext_obj = _edje_object_part_external_object_get(ed, part);
   efl_composite_attach(obj, ext_obj);
}

EOLIAN static Efl_Gfx *
_efl_canvas_layout_external_efl_container_content_get(Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   RETURN_VAL(_edje_object_part_external_object_get(pd->ed, pd->part));
}

#include "efl_canvas_layout_external.eo.c"

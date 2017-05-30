#include "edje_private.h"
#include "edje_part_helper.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_INTERNAL_CLASS

PROXY_IMPLEMENTATION(other, INTERNAL, EINA_FALSE)
#undef PROXY_IMPLEMENTATION

PROXY_INIT(box)
PROXY_INIT(table)
PROXY_INIT(swallow)
PROXY_INIT(external)
PROXY_INIT(other)

void
_edje_internal_proxy_shutdown(void)
{
   _box_shutdown();
   _table_shutdown();
   _swallow_shutdown();
   _external_shutdown();
   _other_shutdown();
}

void
_edje_real_part_set(Eo *obj, void *ed, void *rp, const char *part)
{
   PROXY_DATA_GET(obj, pd);
   pd->ed = ed;
   pd->rp = rp;
   pd->part = part;
   pd->temp = 1;
   efl_parent_set(obj, pd->ed->obj);
}

EOLIAN static Efl_Object *
_efl_canvas_layout_internal_efl_object_finalize(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(pd->rp && pd->ed && pd->part, NULL);
   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN void
_efl_canvas_layout_internal_efl_gfx_geometry_get(Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Internal_Data *pd, int *x, int *y, int *w, int *h)
{
   Edje_Real_Part *rp = pd->rp;

   _edje_recalc_do(pd->ed);
   if (!rp)
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (w) *w = 0;
        if (h) *h = 0;
     }

   if (x) *x = rp->x;
   if (y) *y = rp->y;
   if (w) *w = rp->w;
   if (h) *h = rp->h;
   RETURN_VOID;
}

EOLIAN static void
_efl_canvas_layout_internal_state_get(Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Internal_Data *pd, const char **name, double *val)
{
   const char *str;

   if (!name && !val) return;
   str = _edje_object_part_state_get(pd->ed, pd->part, val);
   if (name) *name = str;
   RETURN_VOID;
}

#include "efl_canvas_layout_internal.eo.c"

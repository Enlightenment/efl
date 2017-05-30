#include "edje_private.h"
#include "edje_part_helper.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_INTERNAL_CLASS

PROXY_IMPLEMENTATION(other, INTERNAL, EINA_FALSE)
#undef PROXY_IMPLEMENTATION

PROXY_INIT(box)
PROXY_INIT(table)
PROXY_INIT(swallow)
PROXY_INIT(external)
PROXY_INIT(text)
PROXY_INIT(other)

void
_edje_internal_proxy_shutdown(void)
{
   _box_shutdown();
   _table_shutdown();
   _swallow_shutdown();
   _external_shutdown();
   _text_shutdown();
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
_efl_canvas_layout_internal_efl_gfx_geometry_get(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, int *x, int *y, int *w, int *h)
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
_efl_canvas_layout_internal_state_get(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, const char **name, double *val)
{
   const char *str;

   if (!name && !val) return;
   str = _edje_object_part_state_get(pd->ed, pd->part, val);
   if (name) *name = str;
   RETURN_VOID;
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_value_set(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double dx, double dy)
{
   RETURN_VAL(_edje_object_part_drag_value_set(pd->ed, pd->part, dx, dy));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_value_get(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double *dx, double *dy)
{
   RETURN_VAL(_edje_object_part_drag_value_get(pd->ed, pd->part, dx, dy));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_size_set(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double dw, double dh)
{
   RETURN_VAL(_edje_object_part_drag_size_set(pd->ed, pd->part, dw, dh));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_size_get(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double *dw, double *dh)
{
   RETURN_VAL(_edje_object_part_drag_size_get(pd->ed, pd->part, dw, dh));
}

EOLIAN static Efl_Ui_Drag_Dir
_efl_canvas_layout_internal_efl_ui_drag_drag_dir_get(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd)
{
   RETURN_VAL(_edje_object_part_drag_dir_get(pd->ed, pd->part));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_step_set(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double dx, double dy)
{
   RETURN_VAL(_edje_object_part_drag_step_set(pd->ed, pd->part, dx, dy));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_step_get(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double *dx, double *dy)
{
   RETURN_VAL(_edje_object_part_drag_step_get(pd->ed, pd->part, dx, dy));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_step_move(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double dx, double dy)
{
   RETURN_VAL(_edje_object_part_drag_step(pd->ed, pd->part, dx, dy));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_page_set(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double dx, double dy)
{
   RETURN_VAL(_edje_object_part_drag_page_set(pd->ed, pd->part, dx, dy));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_page_get(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double *dx, double *dy)
{
   RETURN_VAL(_edje_object_part_drag_page_get(pd->ed, pd->part, dx, dy));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_page_move(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double dx, double dy)
{
   RETURN_VAL(_edje_object_part_drag_page(pd->ed, pd->part, dx, dy));
}

#include "efl_canvas_layout_internal.eo.c"

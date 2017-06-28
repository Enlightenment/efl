#include "edje_private.h"
#include "edje_part_helper.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_INTERNAL_CLASS

PROXY_IMPLEMENTATION(other, MY_CLASS, EINA_FALSE)
#undef PROXY_IMPLEMENTATION

void
_part_reuse_error(Efl_Canvas_Layout_Internal_Data *pd)
{
   const char *typestr = "UNKNOWN";
   Edje *ed;

   // TODO: Enable full debug only for eo_debug?
   // Don't trust pd->ed as it may be invalid now.
   ed = efl_data_scope_safe_get(pd->obj, EDJE_OBJECT_CLASS);
   if (!ed)
     {
        ERR("A previous misuse of efl_part has been detected. Handles returned "
            "by efl_part() are valid for a single function call. Did you call "
            "a non implemented function? obj: %p has been deleted!", pd->obj);
        return;
     }

   switch (pd->rp->part->type)
     {
      case EDJE_PART_TYPE_RECTANGLE: typestr = "RECTANGLE"; break;
      case EDJE_PART_TYPE_TEXT: typestr = "TEXT"; break;
      case EDJE_PART_TYPE_IMAGE: typestr = "IMAGE"; break;
      case EDJE_PART_TYPE_SWALLOW: typestr = "SWALLOW"; break;
      case EDJE_PART_TYPE_TEXTBLOCK: typestr = "TEXTBLOCK"; break;
      case EDJE_PART_TYPE_GRADIENT: typestr = "GRADIENT"; break;
      case EDJE_PART_TYPE_GROUP: typestr = "GROUP"; break;
      case EDJE_PART_TYPE_BOX: typestr = "BOX"; break;
      case EDJE_PART_TYPE_TABLE: typestr = "TABLE"; break;
      case EDJE_PART_TYPE_EXTERNAL: typestr = "EXTERNAL"; break;
      case EDJE_PART_TYPE_PROXY: typestr = "PROXY"; break;
      case EDJE_PART_TYPE_SPACER: typestr = "SPACER"; break;
      case EDJE_PART_TYPE_MESH_NODE: typestr = "MESH_NODE"; break;
      case EDJE_PART_TYPE_LIGHT: typestr = "LIGHT"; break;
      case EDJE_PART_TYPE_CAMERA: typestr = "CAMERA"; break;
      case EDJE_PART_TYPE_SNAPSHOT: typestr = "SNAPSHOT"; break;
      case EDJE_PART_TYPE_VECTOR: typestr = "VECTOR"; break;
      default: break;
     }

   ERR("A previous misuse of efl_part has been detected. Handles returned "
       "by efl_part() are valid for a single function call. Did you call "
       "a non implemented function? obj: %p group: '%s' part: '%s' type: %s%s",
       pd->obj, ed->group, pd->part, typestr,
       ed->delete_me ? ". This object is already deleted." : "");
}

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
_edje_real_part_set(Eo *obj, Edje *ed, Edje_Real_Part *rp, const char *part)
{
   Efl_Canvas_Layout_Internal_Data *pd;

   pd = efl_data_scope_get(obj, EFL_CANVAS_LAYOUT_INTERNAL_CLASS);
   pd->obj = ed->obj;
   pd->ed = ed;
   pd->rp = rp;
   pd->part = part;
   pd->temp = 1;
   pd->in_use = 1;
   pd->in_call = 0;
   efl_parent_set(obj, ed->obj);
}

EOLIAN static Efl_Object *
_efl_canvas_layout_internal_efl_object_finalize(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(pd->rp && pd->ed && pd->part, NULL);
   // Do not use RETURN_VAL() here!
   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN void
_efl_canvas_layout_internal_efl_gfx_geometry_get(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, int *x, int *y, int *w, int *h)
{
   Edje_Real_Part *rp = pd->rp;
   PROXY_CALL_BEGIN(pd);

   _edje_recalc_do(pd->ed);
   if (!rp)
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (w) *w = 0;
        if (h) *h = 0;
        RETURN_VOID;
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
   PROXY_CALL_BEGIN(pd);

   if (!name && !val)
     RETURN_VOID;
   str = _edje_object_part_state_get(pd->ed, pd->part, val);
   if (name) *name = str;
   RETURN_VOID;
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_value_set(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double dx, double dy)
{
   PROXY_CALL_BEGIN(pd);
   RETURN_VAL(_edje_object_part_drag_value_set(pd->ed, pd->part, dx, dy));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_value_get(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double *dx, double *dy)
{
   PROXY_CALL_BEGIN(pd);
   RETURN_VAL(_edje_object_part_drag_value_get(pd->ed, pd->part, dx, dy));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_size_set(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double dw, double dh)
{
   PROXY_CALL_BEGIN(pd);
   RETURN_VAL(_edje_object_part_drag_size_set(pd->ed, pd->part, dw, dh));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_size_get(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double *dw, double *dh)
{
   PROXY_CALL_BEGIN(pd);
   RETURN_VAL(_edje_object_part_drag_size_get(pd->ed, pd->part, dw, dh));
}

EOLIAN static Efl_Ui_Drag_Dir
_efl_canvas_layout_internal_efl_ui_drag_drag_dir_get(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd)
{
   PROXY_CALL_BEGIN(pd);
   RETURN_VAL(_edje_object_part_drag_dir_get(pd->ed, pd->part));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_step_set(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double dx, double dy)
{
   PROXY_CALL_BEGIN(pd);
   RETURN_VAL(_edje_object_part_drag_step_set(pd->ed, pd->part, dx, dy));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_step_get(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double *dx, double *dy)
{
   PROXY_CALL_BEGIN(pd);
   RETURN_VAL(_edje_object_part_drag_step_get(pd->ed, pd->part, dx, dy));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_step_move(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double dx, double dy)
{
   PROXY_CALL_BEGIN(pd);
   RETURN_VAL(_edje_object_part_drag_step(pd->ed, pd->part, dx, dy));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_page_set(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double dx, double dy)
{
   PROXY_CALL_BEGIN(pd);
   RETURN_VAL(_edje_object_part_drag_page_set(pd->ed, pd->part, dx, dy));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_page_get(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double *dx, double *dy)
{
   PROXY_CALL_BEGIN(pd);
   RETURN_VAL(_edje_object_part_drag_page_get(pd->ed, pd->part, dx, dy));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_efl_ui_drag_drag_page_move(Eo *obj, Efl_Canvas_Layout_Internal_Data *pd, double dx, double dy)
{
   PROXY_CALL_BEGIN(pd);
   RETURN_VAL(_edje_object_part_drag_page(pd->ed, pd->part, dx, dy));
}

#include "efl_canvas_layout_internal.eo.c"

#include "edje_private.h"
#include "edje_part_helper.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_PART_CLASS

PROXY_IMPLEMENTATION(other, MY_CLASS, EINA_FALSE)
#undef PROXY_IMPLEMENTATION

const char *
_part_type_to_string(unsigned char type)
{
   const char *typestr = "UNKNOWN";

   switch (type)
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

   return typestr;
}

void
_part_reuse_error(Efl_Canvas_Layout_Part_Data *pd)
{
   Edje *ed;

   // TODO: Enable full debug only for eo_debug?
   // Don't trust pd->ed as it may be invalid now.
   ed = efl_data_scope_safe_get(pd->obj, EFL_CANVAS_LAYOUT_CLASS);
   if (!ed)
     {
        ERR("A previous misuse of efl_part has been detected. Handles returned "
            "by efl_part() are valid for a single function call. Did you call "
            "a non implemented function? obj: %p has been deleted!", pd->obj);
        return;
     }

   ERR("A previous misuse of efl_part has been detected. Handles returned "
       "by efl_part() are valid for a single function call. Did you call "
       "a non implemented function? obj: %p group: '%s' part: '%s' type: %s%s",
       pd->obj, ed->group, pd->part, _part_type_to_string(pd->rp->part->type),
       ed->delete_me ? ". This object is already deleted." : "");
}

PROXY_INIT(box)
PROXY_INIT(table)
PROXY_INIT(swallow)
PROXY_INIT(external)
PROXY_INIT(text)
PROXY_INIT(other)
PROXY_INIT(invalid)

void
_edje_internal_proxy_shutdown(void)
{
   _box_shutdown();
   _table_shutdown();
   _swallow_shutdown();
   _external_shutdown();
   _text_shutdown();
   _invalid_shutdown();
   _other_shutdown();
}

void
_edje_real_part_set(Eo *obj EINA_UNUSED, Edje *ed, Edje_Real_Part *rp, const char *part)
{
   Efl_Canvas_Layout_Part_Data *pd;

   pd = efl_data_scope_get(obj, EFL_CANVAS_LAYOUT_PART_CLASS);
   pd->obj = ed->obj;
   pd->ed = ed;
   pd->rp = rp;
   pd->part = part;
   efl_parent_set(obj, ed->obj);
}

EOLIAN static Efl_Object *
_efl_canvas_layout_part_efl_object_finalize(Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(pd->rp && pd->ed && pd->part, NULL);
   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN Eina_Rect
_efl_canvas_layout_part_efl_gfx_entity_geometry_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd)
{
   Edje_Real_Part *rp = pd->rp;
   Eina_Rect r = EINA_RECT_ZERO();

   _edje_recalc_do(pd->ed);
   if (!rp) return r;

   return (Eina_Rect) rp->rect;
}

EOLIAN static void
_efl_canvas_layout_part_state_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd, const char **name, double *val)
{
   const char *str;

   if (!name && !val) return;
   str = _edje_object_part_state_get(pd->ed, pd->part, val);
   if (name) *name = str;
}

EOLIAN static Efl_Canvas_Layout_Part_Type
_efl_canvas_layout_part_part_type_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd)
{
   Edje_Real_Part *rp;

   rp = _edje_real_part_recursive_get(&pd->ed, pd->part);
   if (!rp || !rp->part) return EFL_CANVAS_LAYOUT_PART_TYPE_NONE;

   return (Efl_Canvas_Layout_Part_Type) rp->part->type;
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_efl_ui_drag_drag_value_set(Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd, double dx, double dy)
{
   return _edje_object_part_drag_value_set(pd->ed, pd->part, dx, dy);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_efl_ui_drag_drag_value_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd, double *dx, double *dy)
{
   return _edje_object_part_drag_value_get(pd->ed, pd->part, dx, dy);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_efl_ui_drag_drag_size_set(Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd, double dw, double dh)
{
   return _edje_object_part_drag_size_set(pd->ed, pd->part, dw, dh);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_efl_ui_drag_drag_size_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd, double *dw, double *dh)
{
   return _edje_object_part_drag_size_get(pd->ed, pd->part, dw, dh);
}

EOLIAN static Efl_Ui_Drag_Dir
_efl_canvas_layout_part_efl_ui_drag_drag_dir_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd)
{
   return _edje_object_part_drag_dir_get(pd->ed, pd->part);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_efl_ui_drag_drag_step_set(Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd, double dx, double dy)
{
   return _edje_object_part_drag_step_set(pd->ed, pd->part, dx, dy);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_efl_ui_drag_drag_step_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd, double *dx, double *dy)
{
   return _edje_object_part_drag_step_get(pd->ed, pd->part, dx, dy);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_efl_ui_drag_drag_step_move(Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd, double dx, double dy)
{
   return _edje_object_part_drag_step(pd->ed, pd->part, dx, dy);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_efl_ui_drag_drag_page_set(Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd, double dx, double dy)
{
   return _edje_object_part_drag_page_set(pd->ed, pd->part, dx, dy);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_efl_ui_drag_drag_page_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd, double *dx, double *dy)
{
   return _edje_object_part_drag_page_get(pd->ed, pd->part, dx, dy);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_efl_ui_drag_drag_page_move(Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd, double dx, double dy)
{
   return _edje_object_part_drag_page(pd->ed, pd->part, dx, dy);
}

static Eo *
_edje_invalid_part_efl_content_get(Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd)
{
   ERR("Part '%s' (type: %s) of group '%s' is not a SWALLOW part!", pd->part, _part_type_to_string(pd->rp->type), pd->ed->group);
   return NULL;
}

static Eina_Bool
_edje_invalid_part_efl_content_set(Eo *obj EINA_UNUSED, Efl_Canvas_Layout_Part_Data *pd, Eo *subobj EINA_UNUSED)
{
   ERR("Part '%s' (type: %s) of group '%s' is not a SWALLOW part!", pd->part, _part_type_to_string(pd->rp->type), pd->ed->group);
   return EINA_FALSE;
}

/* Internal EO APIs and hidden overrides */

#define EFL_CANVAS_LAYOUT_PART_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_content_get, _edje_invalid_part_efl_content_get), \
   EFL_OBJECT_OP_FUNC(efl_content_set, _edje_invalid_part_efl_content_set), \

#include "efl_canvas_layout_part.eo.c"

#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"
#include "efl_vg_root_node.eo.h"

#include <string.h>

#define MY_CLASS EFL_VG_ROOT_NODE_CLASS

typedef struct _Efl_VG_Root_Node_Data Efl_VG_Root_Node_Data;
struct _Efl_VG_Root_Node_Data
{
   Evas_Object *parent;
   Evas_Object_Protected_Data *data;
};

static void
_evas_vg_root_node_render_pre(Eo *obj EINA_UNUSED,
                              Eina_Matrix3 *parent,
                              Ector_Surface *s,
                              void *data,
                              Efl_VG_Base_Data *nd)
{
   Efl_VG_Container_Data *pd = data;
   Eina_List *l;
   Eo *child;

   EFL_VG_COMPUTE_MATRIX(current, parent, nd);

   EINA_LIST_FOREACH(pd->children, l, child)
     _evas_vg_render_pre(child, s, current);
}

static Eina_Bool
_evas_vg_root_node_changed(void *data, Eo *obj,
                           const Eo_Event_Description *desc EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Efl_VG_Root_Node_Data *pd = data;
   Efl_VG_Base_Data *bd = eo_data_scope_get(obj, EFL_VG_BASE_CLASS);

   if (bd->changed) return EINA_TRUE;
   bd->changed = EINA_TRUE;

   if (pd->parent) evas_object_change(pd->parent, pd->data);
   return EINA_TRUE;
}

static void
_efl_vg_root_node_eo_base_parent_set(Eo *obj,
                                     Efl_VG_Root_Node_Data *pd,
                                     Eo *parent)
{
   // Nice little hack, jump over parent parent_set in Efl_VG_Root
   eo_do_super(obj, EFL_VG_BASE_CLASS, eo_parent_set(parent));
   if (parent && !eo_isa(parent, EVAS_VG_CLASS))
     {
        ERR("Parent of VG_ROOT_NODE must be a VG_CLASS");
     }
   else
     {
        pd->parent = parent;
        pd->data = parent ? eo_data_scope_get(parent, EVAS_OBJECT_CLASS) : NULL;
     }
}

static Eo *
_efl_vg_root_node_eo_base_constructor(Eo *obj,
                                      Efl_VG_Root_Node_Data *pd)
{
   Efl_VG_Container_Data *cd;
   Efl_VG_Base_Data *nd;
   Eo *parent;

   // Nice little hack, jump over parent constructor in Efl_VG_Root
   obj = eo_do_super_ret(obj, EFL_VG_BASE_CLASS, obj, eo_constructor());
   eo_do(obj, parent = eo_parent_get());
   if (!eo_isa(parent, EVAS_VG_CLASS)) {
        ERR("Parent of VG_ROOT_NODE must be a VG_CLASS");
        return NULL;
   }

   cd = eo_data_scope_get(obj, EFL_VG_CONTAINER_CLASS);
   cd->children = NULL;

   nd = eo_data_scope_get(obj, EFL_VG_BASE_CLASS);
   nd->render_pre = _evas_vg_root_node_render_pre;
   nd->data = cd;

   eo_do(obj, eo_event_callback_add(EFL_GFX_CHANGED, _evas_vg_root_node_changed, pd));

   return obj;
}

#include "efl_vg_root_node.eo.c"

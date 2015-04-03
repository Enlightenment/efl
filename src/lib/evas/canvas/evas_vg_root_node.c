#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"
#include "evas_vg_root_node.eo.h"

#include <string.h>

#define MY_CLASS EVAS_VG_ROOT_NODE_CLASS

typedef struct _Evas_VG_Root_Node_Data Evas_VG_Root_Node_Data;
struct _Evas_VG_Root_Node_Data
{
   Evas_Object *parent;
   Evas_Object_Protected_Data *data;
};

static void
_evas_vg_root_node_render_pre(Eo *obj EINA_UNUSED,
                              Eina_Matrix3 *parent,
                              Ector_Surface *s,
                              void *data,
                              Evas_VG_Node_Data *nd)
{
   Evas_VG_Container_Data *pd = data;
   Eina_List *l;
   Eo *child;

   if (!nd->changed) return ;
   nd->changed = EINA_FALSE;

   EVAS_VG_COMPUTE_MATRIX(current, parent, nd);

   EINA_LIST_FOREACH(pd->children, l, child)
     _evas_vg_render_pre(child, s, current);
}

static Eina_Bool
_evas_vg_root_node_changed(void *data, Eo *obj EINA_UNUSED,
                           const Eo_Event_Description *desc EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Evas_VG_Root_Node_Data *pd = data;

   if (pd->parent) evas_object_change(pd->parent, pd->data);
   return EINA_TRUE;
}

void
_evas_vg_root_node_eo_base_parent_set(Eo *obj,
                                      Evas_VG_Root_Node_Data *pd,
                                      Eo *parent)
{
   // Nice little hack, jump over parent parent_set in Evas_VG_Root
   eo_do_super(obj, EVAS_VG_NODE_CLASS, eo_parent_set(parent));
   if (parent && !eo_isa(parent, EVAS_VG_CLASS))
     {
        eo_error_set(obj);
     }
   else
     {
        pd->parent = parent;
        pd->data = parent ? eo_data_scope_get(parent, EVAS_OBJECT_CLASS) : NULL;
     }
}

void
_evas_vg_root_node_eo_base_constructor(Eo *obj,
                                       Evas_VG_Root_Node_Data *pd)
{
   Evas_VG_Container_Data *cd;
   Evas_VG_Node_Data *nd;
   Eo *parent;

   // Nice little hack, jump over parent constructor in Efl_VG_Root
   eo_do_super(obj, EFL_VG_BASE_CLASS, eo_constructor());
   eo_do(obj, parent = eo_parent_get());
   if (!eo_isa(parent, EVAS_VG_CLASS))
     eo_error_set(obj);

   cd = eo_data_scope_get(obj, EVAS_VG_CONTAINER_CLASS);
   cd->children = NULL;

   nd = eo_data_scope_get(obj, EVAS_VG_NODE_CLASS);
   nd->render_pre = _evas_vg_root_node_render_pre;
   nd->data = cd;

   eo_do(obj, eo_event_callback_add(EFL_GFX_CHANGED, _evas_vg_root_node_changed, pd));
}

#include "evas_vg_root_node.eo.c"

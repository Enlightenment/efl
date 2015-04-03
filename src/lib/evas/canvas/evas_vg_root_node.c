#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_root_node.eo.h"

#include <string.h>

#define MY_CLASS EVAS_VG_ROOT_NODE_CLASS

typedef struct _Evas_VG_Root_Node_Data Evas_VG_Root_Node_Data;
struct _Evas_VG_Root_Node_Data
{
   Evas_Object *parent;
   Evas_Object_Protected_Data *data;
};

static Eina_Bool
_evas_vg_root_node_changed(void *data, Eo *obj EINA_UNUSED,
                           const Eo_Event_Description *desc EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Evas_VG_Root_Node_Data *pd = data;

   evas_object_change(pd->parent, pd->data);
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
        pd->data = eo_data_scope_get(parent, EVAS_OBJECT_CLASS);
     }
}

void
_evas_vg_root_node_eo_base_constructor(Eo *obj,
                                       Evas_VG_Root_Node_Data *pd)
{
   Eo *parent;

   // Nice little hack, jump over parent constructor in Efl_VG_Root
   eo_do_super(obj, EFL_VG_BASE_CLASS, eo_constructor());
   eo_do(obj, parent = eo_parent_get());
   if (!eo_isa(parent, EVAS_VG_CLASS))
     eo_error_set(obj);

   eo_do(obj, eo_event_callback_add(EFL_GFX_CHANGED, _evas_vg_root_node_changed, pd));
}

#include "evas_vg_root_node.eo.c"

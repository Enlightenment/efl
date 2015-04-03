#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_root_node.eo.h"

#include <string.h>

#define MY_CLASS EVAS_VG_ROOT_NODE_CLASS

typedef struct _Evas_VG_Root_Node_Data Evas_VG_Root_Node_Data;
struct _Evas_VG_Root_Node_Data
{
};

void
_evas_vg_root_node_eo_base_parent_set(Eo *obj,
                                      Evas_VG_Root_Node_Data *pd EINA_UNUSED,
                                      Eo *parent)
{
   // Nice little hack, jump over parent parent_set in Evas_VG_Root
   eo_do_super(obj, EVAS_VG_NODE_CLASS, eo_parent_set(parent));
   if (parent && !eo_isa(parent, EVAS_VG_CLASS))
     eo_error_set(obj);
}

void
_evas_vg_root_node_eo_base_constructor(Eo *obj,
                                       Evas_VG_Root_Node_Data *pd EINA_UNUSED)
{
   Eo *parent;

   // Nice little hack, jump over parent constructor in Evas_VG_Root
   eo_do_super(obj, EVAS_VG_NODE_CLASS, eo_constructor());
   eo_do(obj, parent = eo_parent_get());
   if (!eo_isa(parent, EVAS_VG_CLASS))
     eo_error_set(obj);
}

#include "evas_vg_root_node.eo.c"

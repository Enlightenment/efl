#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#include <string.h>

#define MY_CLASS EVAS_VG_NODE_CLASS

// FIXME:
// - share private structure with evas_object_vg
// - mark parent canvas evas_object dirty after any change on the object
// - add a virtual render function as part of the private data field

void
_evas_vg_node_transformation_set(Eo *obj EINA_UNUSED,
                                 Evas_VG_Node_Data *pd,
                                 const Eina_Matrix3 *m)
{
   if (!pd->m)
     {
        pd->m = malloc(sizeof (Eina_Matrix3));
        if (!pd->m) return ;
     }
   memcpy(pd->m, m, sizeof (Eina_Matrix3));
}

const Eina_Matrix3 *
_evas_vg_node_transformation_get(Eo *obj EINA_UNUSED, Evas_VG_Node_Data *pd)
{
   return pd->m;
}

void
_evas_vg_node_origin_set(Eo *obj EINA_UNUSED,
                         Evas_VG_Node_Data *pd,
                         double x, double y)
{
   pd->x = x;
   pd->y = y;
}

void
_evas_vg_node_origin_get(Eo *obj EINA_UNUSED,
                         Evas_VG_Node_Data *pd,
                         double *x, double *y)
{
   if (x) *x = pd->x;
   if (y) *y = pd->y;
}

void
_evas_vg_node_visibility_set(Eo *obj EINA_UNUSED,
                             Evas_VG_Node_Data *pd,
                             Eina_Bool v)
{
   pd->visibility = v;
}

Eina_Bool
_evas_vg_node_visibility_get(Eo *obj EINA_UNUSED, Evas_VG_Node_Data *pd)
{
   return pd->visibility;
}

void
_evas_vg_node_color_set(Eo *obj EINA_UNUSED,
                        Evas_VG_Node_Data *pd,
                        int r, int g, int b, int a)
{
   pd->r = r;
   pd->g = g;
   pd->b = b;
   pd->a = a;
}

void
_evas_vg_node_color_get(Eo *obj EINA_UNUSED,
                        Evas_VG_Node_Data *pd,
                        int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->r;
   if (g) *g = pd->g;
   if (b) *b = pd->b;
   if (a) *a = pd->a;
}

void
_evas_vg_node_mask_set(Eo *obj EINA_UNUSED,
                       Evas_VG_Node_Data *pd,
                       Evas_VG_Node *r)
{
   Evas_VG_Node *tmp = pd->mask;

   pd->mask = eo_ref(r);
   eo_unref(tmp);
}

Evas_VG_Node*
_evas_vg_node_mask_get(Eo *obj EINA_UNUSED, Evas_VG_Node_Data *pd)
{
   return pd->mask;
}

// Parent should be a container otherwise dismissing the stacking operation
void
_evas_vg_node_eo_base_constructor(Eo *obj, Evas_VG_Node_Data *pd EINA_UNUSED)
{
   Evas_VG_Container_Data *cd;
   Eo *parent;

   eo_do_super(obj, MY_CLASS, eo_constructor());

   eo_do(obj, parent = eo_parent_get());
   cd = eo_data_scope_get(parent, EVAS_VG_CONTAINER_CLASS);
   if (!cd)
     {
        eo_error_set(obj);
        return ;
     }
   cd->children = eina_list_append(cd->children, obj);
}

void
_evas_vg_node_eo_base_parent_set(Eo *obj,
                                 Evas_VG_Node_Data *pd EINA_UNUSED,
                                 Eo *parent)
{
   Evas_VG_Container_Data *cd;
   Evas_VG_Container_Data *old_cd;
   Eo *old_parent;

   cd = eo_data_scope_get(parent, EVAS_VG_CONTAINER_CLASS);
   if (!cd)
     {
        eo_error_set(obj);
        return ;
     }

   eo_do(obj, old_parent = eo_parent_get());
   old_cd = eo_data_scope_get(old_parent, EVAS_VG_CONTAINER_CLASS);
   if (!old_cd)
     {
        eo_error_set(obj);
        return ;
     }

   // FIXME: this may become slow with to much object
   old_cd->children = eina_list_remove(old_cd->children, obj);

   eo_do_super(obj, MY_CLASS, eo_parent_set(parent));
   cd->children = eina_list_append(cd->children, obj);
}

void
_evas_vg_node_raise(Eo *obj, Evas_VG_Node_Data *pd EINA_UNUSED)
{
   Evas_VG_Container_Data *cd;
   Eina_List *lookup, *next;
   Eo *parent;

   eo_do(obj, parent = eo_parent_get());
   cd = eo_data_scope_get(parent, EVAS_VG_CONTAINER_CLASS);
   if (!cd) goto on_error;

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   next = eina_list_next(lookup);
   if (!next) return ;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_append_relative_list(cd->children, obj, next);

   return ;

 on_error:
   eo_error_set(obj);
}

void
_evas_vg_node_stack_above(Eo *obj,
                          Evas_VG_Node_Data *pd EINA_UNUSED,
                          Evas_VG_Node *above)
{
   Evas_VG_Container_Data *cd;
   Eina_List *lookup, *ref;
   Eo *parent;

   eo_do(obj, parent = eo_parent_get());
   cd = eo_data_scope_get(parent, EVAS_VG_CONTAINER_CLASS);
   if (!cd) goto on_error;

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   ref = eina_list_data_find_list(cd->children, above);
   if (!ref) goto on_error;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_append_relative_list(cd->children, obj, ref);

   return ;

 on_error:
   eo_error_set(obj);
}

void
_evas_vg_node_stack_below(Eo *obj,
                          Evas_VG_Node_Data *pd EINA_UNUSED,
                          Evas_Object *below)
{
   Evas_VG_Container_Data *cd;
   Eina_List *lookup, *ref;
   Eo *parent;

   eo_do(obj, parent = eo_parent_get());
   cd = eo_data_scope_get(parent, EVAS_VG_CONTAINER_CLASS);
   if (!cd) goto on_error;

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   ref = eina_list_data_find_list(cd->children, below);
   if (!ref) goto on_error;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_prepend_relative_list(cd->children, obj, ref);

   return ;

 on_error:
   eo_error_set(obj);
}

void
_evas_vg_node_lower(Eo *obj, Evas_VG_Node_Data *pd EINA_UNUSED)
{
   Evas_VG_Container_Data *cd;
   Eina_List *lookup, *prev;
   Eo *parent;

   eo_do(obj, parent = eo_parent_get());
   cd = eo_data_scope_get(parent, EVAS_VG_CONTAINER_CLASS);
   if (!cd) goto on_error;

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   prev = eina_list_prev(lookup);
   if (!prev) return ;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_prepend_relative_list(cd->children, obj, prev);

   return ;

 on_error:
   eo_error_set(obj);
}

#include "evas_vg_node.eo.c"

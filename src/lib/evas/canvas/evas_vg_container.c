#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EVAS_VG_CONTAINER_CLASS

static void
_evas_vg_container_render_pre(Eo *obj EINA_UNUSED,
                              Eina_Matrix3 *parent,
                              Ector_Surface *s,
                              void *data,
                              Evas_VG_Node_Data *nd)
{
   Evas_VG_Container_Data *pd = data;
   Eina_List *l;
   Eo *child;
   EVAS_VG_COMPUTE_MATRIX(current, parent, nd);

   EINA_LIST_FOREACH(pd->children, l, child)
     _evas_vg_render_pre(child, s, current);
}

static void
_evas_vg_container_eo_base_constructor(Eo *obj,
                                       Evas_VG_Container_Data *pd)
{
   Evas_VG_Node_Data *nd;

   eo_do_super(obj, MY_CLASS, eo_constructor());

   nd = eo_data_scope_get(obj, EVAS_VG_NODE_CLASS);
   nd->render_pre = _evas_vg_container_render_pre;
   nd->data = pd;
}

static void
_evas_vg_container_eo_base_destructor(Eo *obj,
                                      Evas_VG_Container_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Eina_Bool
_evas_vg_container_evas_vg_node_bound_get(Eo *obj EINA_UNUSED,
                                          Evas_VG_Container_Data *pd,
                                          Eina_Rectangle *r)
{
   Eina_Rectangle s;
   Eina_Bool first = EINA_TRUE;
   Eina_List *l;
   Eo *child;

   if (!r) return EINA_FALSE;

   EINA_RECTANGLE_SET(&s, -1, -1, 0, 0);

   EINA_LIST_FOREACH(pd->children, l, child)
     {
        if (first)
          {
             eo_do(child, evas_vg_node_bound_get(r));
             first = EINA_FALSE;
          }
        else
          {
             eo_do(child, evas_vg_node_bound_get(&s));
             eina_rectangle_union(r, &s);
          }
     }
   // returning EINA_FALSE if no bouding box was found
   return first ? EINA_FALSE : EINA_TRUE;
}


#include "evas_vg_container.eo.c"

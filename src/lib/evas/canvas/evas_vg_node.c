#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"
#include "efl_vg_root_node.eo.h"

#include <string.h>
#include <math.h>

#define MY_CLASS EFL_VG_BASE_CLASS

static Eina_Bool
_efl_vg_base_property_changed(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info)
{
   Efl_VG_Base_Data *pd = data;
   Eo *parent;

   if (pd->changed) return EINA_TRUE;
   pd->changed = EINA_TRUE;

   eo_do(obj, parent = eo_parent_get());
   eo_do(parent, eo_event_callback_call(desc, event_info));
   return EINA_TRUE;
}

static void
_efl_vg_base_transformation_set(Eo *obj,
                                Efl_VG_Base_Data *pd,
                                const Eina_Matrix3 *m)
{
   if (m)
     {
        if (!pd->m)
          {
             pd->m = malloc(sizeof (Eina_Matrix3));
             if (!pd->m) return;
          }
        memcpy(pd->m, m, sizeof (Eina_Matrix3));
     }
   else
     {
        free(pd->m);
        pd->m = NULL;
     }

   _efl_vg_base_changed(obj);
}

const Eina_Matrix3 *
_efl_vg_base_transformation_get(Eo *obj EINA_UNUSED, Efl_VG_Base_Data *pd)
{
   return pd->m;
}

static void
_efl_vg_base_origin_set(Eo *obj,
                        Efl_VG_Base_Data *pd,
                        double x, double y)
{
   pd->x = x;
   pd->y = y;

   _efl_vg_base_changed(obj);
}

static void
_efl_vg_base_origin_get(Eo *obj EINA_UNUSED,
                        Efl_VG_Base_Data *pd,
                        double *x, double *y)
{
   if (x) *x = pd->x;
   if (y) *y = pd->y;
}

static void
_efl_vg_base_efl_gfx_base_position_set(Eo *obj EINA_UNUSED,
                                       Efl_VG_Base_Data *pd,
                                       int x, int y)
{
   pd->x = lrint(x);
   pd->y = lrint(y);

   _efl_vg_base_changed(obj);
}

static void
_efl_vg_base_efl_gfx_base_position_get(Eo *obj EINA_UNUSED,
                                       Efl_VG_Base_Data *pd,
                                       int *x, int *y)
{
   if (x) *x = pd->x;
   if (y) *y = pd->y;
}

static void
_efl_vg_base_efl_gfx_base_visible_set(Eo *obj EINA_UNUSED,
                                      Efl_VG_Base_Data *pd, Eina_Bool v)
{
   pd->visibility = v;

   _efl_vg_base_changed(obj);
}


static Eina_Bool
_efl_vg_base_efl_gfx_base_visible_get(Eo *obj EINA_UNUSED,
                                      Efl_VG_Base_Data *pd)
{
   return pd->visibility;
}

static void
_efl_vg_base_efl_gfx_base_color_set(Eo *obj EINA_UNUSED,
                                    Efl_VG_Base_Data *pd,
                                    int r, int g, int b, int a)
{
   if (r > 255) r = 255; if (r < 0) r = 0;
   if (g > 255) g = 255; if (g < 0) g = 0;
   if (b > 255) b = 255; if (b < 0) b = 0;
   if (a > 255) a = 255; if (a < 0) a = 0;
   if (r > a)
     {
        r = a;
        ERR("Evas only handles pre multiplied colors!");
     }
   if (g > a)
     {
        g = a;
        ERR("Evas only handles pre multiplied colors!");
     }
   if (b > a)
     {
        b = a;
        ERR("Evas only handles pre multiplied colors!");
     }

   pd->r = r;
   pd->g = g;
   pd->b = b;
   pd->a = a;

   _efl_vg_base_changed(obj);
}

static Eina_Bool
_efl_vg_base_efl_gfx_base_color_part_set(Eo *obj, Efl_VG_Base_Data *pd,
                                         const char *part,
                                         int r, int g, int b, int a)
{
   if (part) return EINA_FALSE;

   _efl_vg_base_efl_gfx_base_color_set(obj, pd, r, g, b, a);
   return EINA_TRUE;
}

static void
_efl_vg_base_efl_gfx_base_color_get(Eo *obj EINA_UNUSED,
                                    Efl_VG_Base_Data *pd,
                                    int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->r;
   if (g) *g = pd->g;
   if (b) *b = pd->b;
   if (a) *a = pd->a;
}

static Eina_Bool
_efl_vg_base_efl_gfx_base_color_part_get(Eo *obj, Efl_VG_Base_Data *pd,
                                         const char *part,
                                         int *r, int *g, int *b, int *a)
{
   if (part) return EINA_FALSE;

   _efl_vg_base_efl_gfx_base_color_get(obj, pd, r, g, b, a);
   return EINA_TRUE;
}

static void
_efl_vg_base_mask_set(Eo *obj EINA_UNUSED,
                       Efl_VG_Base_Data *pd,
                       Efl_VG_Base *r)
{
   Efl_VG_Base *tmp = pd->mask;

   pd->mask = eo_ref(r);
   eo_unref(tmp);

   _efl_vg_base_changed(obj);
}

static Efl_VG_Base*
_efl_vg_base_mask_get(Eo *obj EINA_UNUSED, Efl_VG_Base_Data *pd)
{
   return pd->mask;
}

static void
_efl_vg_base_efl_gfx_base_size_get(Eo *obj,
                                   Efl_VG_Base_Data *pd EINA_UNUSED,
                                   int *w, int *h)
{
   Eina_Rectangle r = { 0, 0, 0, 0 };

   eo_do(obj, efl_vg_bounds_get(&r));
   if (w) *w = r.w;
   if (h) *h = r.h;
}

// Parent should be a container otherwise dismissing the stacking operation
static Eina_Bool
_efl_vg_base_parent_checked_get(Eo *obj,
                                Eo **parent,
                                Efl_VG_Container_Data **cd)
{
   *cd = NULL;
   eo_do(obj, *parent = eo_parent_get());

   if (eo_isa(*parent, EFL_VG_CONTAINER_CLASS))
     {
        *cd = eo_data_scope_get(*parent, EFL_VG_CONTAINER_CLASS);
        if (!*cd)
          {
             ERR("Can't get EFL_VG_CONTAINER_CLASS data.");
             goto on_error;
          }
     }
   else if (*parent != NULL)
     {
        ERR("Parent of unauthorized class.");
        goto on_error;
     }

   return EINA_TRUE;

 on_error:
   *parent = NULL;
   *cd = NULL;
   return EINA_FALSE;
}

static Eo *
_efl_vg_base_eo_base_constructor(Eo *obj,
                                 Efl_VG_Base_Data *pd)
{
   Efl_VG_Container_Data *cd = NULL;
   Eo *parent;

   obj = eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());

   if (!_efl_vg_base_parent_checked_get(obj, &parent, &cd)) {
        ERR("Failed");
        return NULL;
   }

   eo_do(obj, eo_event_callback_add(EFL_GFX_CHANGED, _efl_vg_base_property_changed, pd));
   pd->changed = EINA_TRUE;

   return obj;
}

static void
_efl_vg_base_eo_base_destructor(Eo *obj, Efl_VG_Base_Data *pd)
{
   if (pd->m)
     {
        free(pd->m);
        pd->m = NULL;
     }

   if (pd->renderer)
     {
        eo_del(pd->renderer);
        pd->renderer = NULL;
     }

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_efl_vg_base_eo_base_parent_set(Eo *obj,
                                Efl_VG_Base_Data *pd EINA_UNUSED,
                                Eo *parent)
{
   Efl_VG_Container_Data *cd = NULL;
   Efl_VG_Container_Data *old_cd = NULL;
   Eo *old_parent;

   if (eo_isa(parent, EFL_VG_CONTAINER_CLASS))
     {
        cd = eo_data_scope_get(parent, EFL_VG_CONTAINER_CLASS);
        if (!cd)
          {
             ERR("Can't get EFL_VG_CONTAINER_CLASS data from %p.", parent);
             goto on_error;
          }
     }
   else if (parent != NULL)
     {
        ERR("%p not even an EVAS_VG_CLASS.", parent);
        goto on_error;
     }

   if (!_efl_vg_base_parent_checked_get(obj, &old_parent, &old_cd))
     goto on_error;

   // FIXME: this may become slow with to much object
   if (old_cd)
     old_cd->children = eina_list_remove(old_cd->children, obj);

   eo_do_super(obj, MY_CLASS, eo_parent_set(parent));
   if (cd)
     cd->children = eina_list_append(cd->children, obj);

   _efl_vg_base_changed(old_parent);
   _efl_vg_base_changed(obj);
   _efl_vg_base_changed(parent);

   return ;

 on_error:
   return ;
}

static void
_efl_vg_base_efl_gfx_stack_raise(Eo *obj, Efl_VG_Base_Data *pd EINA_UNUSED)
{
   Efl_VG_Container_Data *cd;
   Eina_List *lookup, *next;
   Eo *parent;

   eo_do(obj, parent = eo_parent_get());
   if (!eo_isa(parent, EFL_VG_CONTAINER_CLASS)) goto on_error;
   cd = eo_data_scope_get(parent, EFL_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   next = eina_list_next(lookup);
   if (!next) return ;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_append_relative_list(cd->children, obj, next);

   _efl_vg_base_changed(parent);
   return ;

 on_error:
   ERR("Err");
}

static void
_efl_vg_base_efl_gfx_stack_stack_above(Eo *obj,
                                       Efl_VG_Base_Data *pd EINA_UNUSED,
                                       Efl_Gfx_Stack *above)
{
   Efl_VG_Container_Data *cd;
   Eina_List *lookup, *ref;
   Eo *parent;

   eo_do(obj, parent = eo_parent_get());
   if (!eo_isa(parent, EFL_VG_CONTAINER_CLASS)) goto on_error;
   cd = eo_data_scope_get(parent, EFL_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   ref = eina_list_data_find_list(cd->children, above);
   if (!ref) goto on_error;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_append_relative_list(cd->children, obj, ref);

   _efl_vg_base_changed(parent);
   return ;

 on_error:
   ERR("Err");
}

static void
_efl_vg_base_efl_gfx_stack_stack_below(Eo *obj,
                                       Efl_VG_Base_Data *pd EINA_UNUSED,
                                       Efl_Gfx_Stack *below)
{
   Efl_VG_Container_Data *cd;
   Eina_List *lookup, *ref;
   Eo *parent;

   eo_do(obj, parent = eo_parent_get());
   if (!eo_isa(parent, EFL_VG_CONTAINER_CLASS)) goto on_error;
   cd = eo_data_scope_get(parent, EFL_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   ref = eina_list_data_find_list(cd->children, below);
   if (!ref) goto on_error;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_prepend_relative_list(cd->children, obj, ref);

   _efl_vg_base_changed(parent);
   return ;

 on_error:
   ERR("Err");
}

static void
_efl_vg_base_efl_gfx_stack_lower(Eo *obj, Efl_VG_Base_Data *pd EINA_UNUSED)
{
   Efl_VG_Container_Data *cd;
   Eina_List *lookup, *prev;
   Eo *parent;

   eo_do(obj, parent = eo_parent_get());
   if (!eo_isa(parent, EFL_VG_CONTAINER_CLASS)) goto on_error;
   cd = eo_data_scope_get(parent, EFL_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   prev = eina_list_prev(lookup);
   if (!prev) return ;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_prepend_relative_list(cd->children, obj, prev);

   _efl_vg_base_changed(parent);
   return ;

 on_error:
   ERR("Err");
}

static Eo *
_efl_vg_base_root_parent_get(Eo *obj)
{
   Eo *parent;

   if (eo_isa(obj, EFL_VG_ROOT_NODE_CLASS))
     return obj;

   eo_do(obj, parent = eo_parent_get());

   if (!parent) return NULL;
   return _efl_vg_base_root_parent_get(parent);
}

static void
_efl_vg_base_walk_down_at(Eo *root, Eina_Array *a, Eina_Rectangle *r)
{
   Eina_Rectangle bounds;

   eo_do(root, efl_vg_bounds_get(&bounds));
   if (!eina_rectangles_intersect(&bounds, r)) return ;

   eina_array_push(a, root);

   if (eo_isa(root, EFL_VG_CONTAINER_CLASS))
     {
        Efl_VG_Container_Data *cd;
        Eina_List *l;
        Eo *child;

        cd = eo_data_scope_get(root, EFL_VG_CONTAINER_CLASS);
        EINA_LIST_FOREACH(cd->children, l, child)
          _efl_vg_base_walk_down_at(child, a, r);
     }
}

static void
_efl_vg_base_object_at(Eo *obj, Eina_Array *a, Eina_Rectangle *r)
{
   Eo *root;

   root = _efl_vg_base_root_parent_get(obj);
   if (!root) return ;

   _efl_vg_base_walk_down_at(root, a, r);
}

static Efl_Gfx_Stack *
_efl_vg_base_efl_gfx_stack_below_get(Eo *obj, Efl_VG_Base_Data *pd EINA_UNUSED)
{
   Eina_Rectangle r;
   Eina_Array a;
   Eo *current;
   Eo *below = NULL;
   Eina_Array_Iterator iterator;
   unsigned int i;

   eo_do(obj, efl_vg_bounds_get(&r));

   eina_array_step_set(&a, sizeof (Eina_Array), 8);

   _efl_vg_base_object_at(obj, &a, &r);

   EINA_ARRAY_ITER_NEXT(&a, i, current, iterator)
     if (current == obj)
       {
          i++;
          if (i < eina_array_count(&a))
            below = eina_array_data_get(&a, i);
          break;
       }

   eina_array_flush(&a);

   return below;
}

static Efl_Gfx_Stack *
_efl_vg_base_efl_gfx_stack_above_get(Eo *obj, Efl_VG_Base_Data *pd EINA_UNUSED)
{
   Eina_Rectangle r;
   Eina_Array a;
   Eo *current;
   Eo *above = NULL;
   Eina_Array_Iterator iterator;
   unsigned int i;

   eo_do(obj, efl_vg_bounds_get(&r));

   eina_array_step_set(&a, sizeof (Eina_Array), 8);

   _efl_vg_base_object_at(obj, &a, &r);

   EINA_ARRAY_ITER_NEXT(&a, i, current, iterator)
     if (current == obj)
       {
          if (i > 0)
            above = eina_array_data_get(&a, i - 1);
          break;
       }

   eina_array_flush(&a);

   return above;
}

EAPI Eina_Bool
evas_vg_node_visible_get(Eo *obj)
{
   Eina_Bool ret;

   return eo_do_ret(obj, ret, efl_gfx_visible_get());
}

EAPI void
evas_vg_node_visible_set(Eo *obj, Eina_Bool v)
{
   eo_do(obj, efl_gfx_visible_set(v));
}

EAPI void
evas_vg_node_color_get(Eo *obj, int *r, int *g, int *b, int *a)
{
   eo_do(obj, efl_gfx_color_get(r, g, b, a));
}

EAPI void
evas_vg_node_color_set(Eo *obj, int r, int g, int b, int a)
{
   eo_do(obj, efl_gfx_color_set(r, g, b, a));
}

EAPI void
evas_vg_node_geometry_get(Eo *obj, int *x, int *y, int *w, int *h)
{
   eo_do(obj,
         efl_gfx_position_get(x, y),
         efl_gfx_size_get(w, h));
}

EAPI void
evas_vg_node_geometry_set(Eo *obj, int x, int y, int w, int h)
{
   eo_do(obj,
         efl_gfx_position_set(x, y),
         efl_gfx_size_set(w, h));
}

EAPI void
evas_vg_node_stack_below(Eo *obj, Eo *below)
{
   eo_do(obj, efl_gfx_stack_below(below));
}

EAPI void
evas_vg_node_stack_above(Eo *obj, Eo *above)
{
   eo_do(obj, efl_gfx_stack_above(above));
}

EAPI void
evas_vg_node_raise(Eo *obj)
{
   eo_do(obj, efl_gfx_stack_raise());
}

EAPI void
evas_vg_node_lower(Eo *obj)
{
   eo_do(obj, efl_gfx_stack_lower());
}

#include "efl_vg_base.eo.c"

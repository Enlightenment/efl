#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"
#include "efl_vg_root_node.eo.h"

#include <string.h>
#include <math.h>

#define MY_CLASS EFL_VG_CLASS

static const Efl_VG_Interpolation interpolation_identity = {
  { 0, 0, 0, 1 },
  { 0, 0, 0, 1 },
  { 0, 0, 0 },
  { 1, 1, 1 },
  { 0, 0, 0 }
};

static void
_efl_vg_property_changed(void *data, const Eo_Event *event)
{
   Efl_VG_Data *pd = data;
   Eo *parent;

   if (!pd->flags) pd->flags = EFL_GFX_CHANGE_FLAG_ALL;

   parent = eo_parent_get(event->object);
   eo_event_callback_call(parent, event->desc, event->info);
}

static void
_efl_vg_transformation_set(Eo *obj,
                                Efl_VG_Data *pd,
                                const Eina_Matrix3 *m)
{
   if (pd->intp)
     {
        free(pd->intp);
        pd->intp = NULL;
     }

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

   pd->flags |= EFL_GFX_CHANGE_FLAG_MATRIX;
   _efl_vg_changed(obj);
}

const Eina_Matrix3 *
_efl_vg_transformation_get(Eo *obj EINA_UNUSED, Efl_VG_Data *pd)
{
   return pd->m;
}

static void
_efl_vg_origin_set(Eo *obj,
                        Efl_VG_Data *pd,
                        double x, double y)
{
   pd->x = x;
   pd->y = y;

   _efl_vg_changed(obj);
}

static void
_efl_vg_origin_get(Eo *obj EINA_UNUSED,
                        Efl_VG_Data *pd,
                        double *x, double *y)
{
   if (x) *x = pd->x;
   if (y) *y = pd->y;
}

static void
_efl_vg_efl_gfx_position_set(Eo *obj EINA_UNUSED,
                                       Efl_VG_Data *pd,
                                       int x, int y)
{
   pd->x = lrint(x);
   pd->y = lrint(y);

   _efl_vg_changed(obj);
}

static void
_efl_vg_efl_gfx_position_get(Eo *obj EINA_UNUSED,
                                       Efl_VG_Data *pd,
                                       int *x, int *y)
{
   if (x) *x = pd->x;
   if (y) *y = pd->y;
}

static void
_efl_vg_efl_gfx_visible_set(Eo *obj EINA_UNUSED,
                                      Efl_VG_Data *pd, Eina_Bool v)
{
   pd->visibility = v;

   _efl_vg_changed(obj);
}


static Eina_Bool
_efl_vg_efl_gfx_visible_get(Eo *obj EINA_UNUSED,
                                      Efl_VG_Data *pd)
{
   return pd->visibility;
}

static void
_efl_vg_efl_gfx_color_set(Eo *obj EINA_UNUSED,
                                    Efl_VG_Data *pd,
                                    int r, int g, int b, int a)
{
   if (r > 255) r = 255;
   if (r < 0) r = 0;
   if (g > 255) g = 255;
   if (g < 0) g = 0;
   if (b > 255) b = 255;
   if (b < 0) b = 0;
   if (a > 255) a = 255;
   if (a < 0) a = 0;
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

   _efl_vg_changed(obj);
}

static Eina_Bool
_efl_vg_efl_gfx_color_part_set(Eo *obj, Efl_VG_Data *pd,
                                         const char *part,
                                         int r, int g, int b, int a)
{
   if (part) return EINA_FALSE;

   _efl_vg_efl_gfx_color_set(obj, pd, r, g, b, a);
   return EINA_TRUE;
}

static void
_efl_vg_efl_gfx_color_get(Eo *obj EINA_UNUSED,
                                    Efl_VG_Data *pd,
                                    int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->r;
   if (g) *g = pd->g;
   if (b) *b = pd->b;
   if (a) *a = pd->a;
}

static Eina_Bool
_efl_vg_efl_gfx_color_part_get(Eo *obj, Efl_VG_Data *pd,
                                         const char *part,
                                         int *r, int *g, int *b, int *a)
{
   if (part) return EINA_FALSE;

   _efl_vg_efl_gfx_color_get(obj, pd, r, g, b, a);
   return EINA_TRUE;
}

static void
_efl_vg_mask_set(Eo *obj EINA_UNUSED,
                       Efl_VG_Data *pd,
                       Efl_VG *r)
{
   Efl_VG *tmp = pd->mask;

   pd->mask = eo_ref(r);
   eo_unref(tmp);

   _efl_vg_changed(obj);
}

static Efl_VG*
_efl_vg_mask_get(Eo *obj EINA_UNUSED, Efl_VG_Data *pd)
{
   return pd->mask;
}

static void
_efl_vg_efl_gfx_size_get(Eo *obj,
                                   Efl_VG_Data *pd EINA_UNUSED,
                                   int *w, int *h)
{
   Eina_Rectangle r = { 0, 0, 0, 0 };

   efl_vg_bounds_get(obj, &r);
   if (w) *w = r.w;
   if (h) *h = r.h;
}

EOLIAN static void
_efl_vg_efl_gfx_geometry_get(Eo *obj, Efl_VG_Data *pd EINA_UNUSED,
                             int *x, int *y, int *w, int *h)
{
   efl_gfx_position_get(obj, x, y);
   efl_gfx_size_get(obj, w, h);
}

// Parent should be a container otherwise dismissing the stacking operation
static Eina_Bool
_efl_vg_parent_checked_get(Eo *obj,
                                Eo **parent,
                                Efl_VG_Container_Data **cd)
{
   *cd = NULL;
   *parent = eo_parent_get(obj);

   if (eo_isa(*parent, EFL_VG_CONTAINER_CLASS))
     {
        *cd = eo_data_scope_get(*parent, EFL_VG_CONTAINER_CLASS);
        if (!*cd)
          {
             ERR("Can't get EFL_VG_CONTAINER_CLASS data.");
             goto on_error;
          }
     }
   else if (eo_isa(*parent, EVAS_VG_CLASS))
     {
        goto on_error;
     }
   else if (*parent != NULL)
     {
        ERR("Parent of unauthorized class '%s'.", eo_class_name_get(eo_class_get(*parent)));
        goto on_error;
     }

   return EINA_TRUE;

 on_error:
   *parent = NULL;
   *cd = NULL;
   return EINA_FALSE;
}

static Eo *
_efl_vg_eo_base_constructor(Eo *obj,
                                 Efl_VG_Data *pd)
{
   Efl_VG_Container_Data *cd = NULL;
   Eo *parent;

   obj = eo_constructor(eo_super(obj, MY_CLASS));

   if (!_efl_vg_parent_checked_get(obj, &parent, &cd)) {
        ERR("Failed");
        return NULL;
   }

   eo_event_callback_add(obj, EFL_GFX_CHANGED, _efl_vg_property_changed, pd);
   pd->flags = EFL_GFX_CHANGE_FLAG_ALL;
   pd->changed = EINA_TRUE;

   return obj;
}

static void
_efl_vg_eo_base_destructor(Eo *obj, Efl_VG_Data *pd)
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
   if (pd->intp)
     {
        free(pd->intp);
        pd->intp = NULL;
     }

   eo_destructor(eo_super(obj, MY_CLASS));
}

static void
_efl_vg_name_insert(Eo *obj, Efl_VG_Data *pd, Efl_VG_Container_Data *cd)
{
   Eo *set;

   if (!pd->name) return ;

   set = eina_hash_find(cd->names, pd->name);
   if (set == obj) return ;

   if (set)
     {
        eina_stringshare_del(pd->name);
        pd->name = NULL;
     }
   else
     {
        eina_hash_direct_add(cd->names, pd->name, obj);
     }
}

static void
_efl_vg_name_set(Eo *obj, Efl_VG_Data *pd, const char *name)
{
   Efl_VG_Container_Data *cd = NULL;
   Eo *parent = NULL;

   if (_efl_vg_parent_checked_get(obj, &parent, &cd))
     {
        if (pd->name) eina_hash_del(cd->names, pd->name, obj);
     }

   eina_stringshare_replace(&pd->name, name);

   if (cd) _efl_vg_name_insert(obj, pd, cd);
}

static const char *
_efl_vg_name_get(Eo *obj EINA_UNUSED, Efl_VG_Data *pd)
{
   return pd->name;
}

static void
_efl_vg_eo_base_parent_set(Eo *obj,
                                Efl_VG_Data *pd EINA_UNUSED,
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

   if (!_efl_vg_parent_checked_get(obj, &old_parent, &old_cd))
     {
        ERR("Can't check the old parent of %p.", obj);
        goto on_error;
     }

   // FIXME: this may become slow with to much object
   if (old_cd)
     {
        old_cd->children = eina_list_remove(old_cd->children, obj);

        if (pd->name) eina_hash_del(old_cd->names, pd->name, obj);
     }

   eo_parent_set(eo_super(obj, MY_CLASS), parent);
   if (cd)
     {
        cd->children = eina_list_append(cd->children, obj);

        _efl_vg_name_insert(obj, pd, cd);
     }

   _efl_vg_changed(old_parent);
   _efl_vg_changed(obj);
   _efl_vg_changed(parent);

   return ;

 on_error:
   return ;
}

static void
_efl_vg_efl_gfx_stack_raise(Eo *obj, Efl_VG_Data *pd EINA_UNUSED)
{
   Efl_VG_Container_Data *cd;
   Eina_List *lookup, *next;
   Eo *parent;

   parent = eo_parent_get(obj);
   if (!eo_isa(parent, EFL_VG_CONTAINER_CLASS)) goto on_error;
   cd = eo_data_scope_get(parent, EFL_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   next = eina_list_next(lookup);
   if (!next) return ;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_append_relative_list(cd->children, obj, next);

   _efl_vg_changed(parent);
   return ;

 on_error:
   ERR("Err");
}

static void
_efl_vg_efl_gfx_stack_stack_above(Eo *obj,
                                       Efl_VG_Data *pd EINA_UNUSED,
                                       Efl_Gfx_Stack *above)
{
   Efl_VG_Container_Data *cd;
   Eina_List *lookup, *ref;
   Eo *parent;

   parent = eo_parent_get(obj);
   if (!eo_isa(parent, EFL_VG_CONTAINER_CLASS)) goto on_error;
   cd = eo_data_scope_get(parent, EFL_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   ref = eina_list_data_find_list(cd->children, above);
   if (!ref) goto on_error;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_append_relative_list(cd->children, obj, ref);

   _efl_vg_changed(parent);
   return ;

 on_error:
   ERR("Err");
}

static void
_efl_vg_efl_gfx_stack_stack_below(Eo *obj,
                                       Efl_VG_Data *pd EINA_UNUSED,
                                       Efl_Gfx_Stack *below)
{
   Efl_VG_Container_Data *cd;
   Eina_List *lookup, *ref;
   Eo *parent;

   parent = eo_parent_get(obj);
   if (!eo_isa(parent, EFL_VG_CONTAINER_CLASS)) goto on_error;
   cd = eo_data_scope_get(parent, EFL_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   ref = eina_list_data_find_list(cd->children, below);
   if (!ref) goto on_error;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_prepend_relative_list(cd->children, obj, ref);

   _efl_vg_changed(parent);
   return ;

 on_error:
   ERR("Err");
}

static void
_efl_vg_efl_gfx_stack_lower(Eo *obj, Efl_VG_Data *pd EINA_UNUSED)
{
   Efl_VG_Container_Data *cd;
   Eina_List *lookup, *prev;
   Eo *parent;

   parent = eo_parent_get(obj);
   if (!eo_isa(parent, EFL_VG_CONTAINER_CLASS)) goto on_error;
   cd = eo_data_scope_get(parent, EFL_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   prev = eina_list_prev(lookup);
   if (!prev) return ;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_prepend_relative_list(cd->children, obj, prev);

   _efl_vg_changed(parent);
   return ;

 on_error:
   ERR("Err");
}

static Eo *
_efl_vg_root_parent_get(Eo *obj)
{
   Eo *parent;

   if (eo_isa(obj, EFL_VG_ROOT_NODE_CLASS))
     return obj;

   parent = eo_parent_get(obj);

   if (!parent) return NULL;
   return _efl_vg_root_parent_get(parent);
}

static void
_efl_vg_walk_down_at(Eo *root, Eina_Array *a, Eina_Rectangle *r)
{
   Eina_Rectangle bounds;

   efl_vg_bounds_get(root, &bounds);
   if (!eina_rectangles_intersect(&bounds, r)) return ;

   eina_array_push(a, root);

   if (eo_isa(root, EFL_VG_CONTAINER_CLASS))
     {
        Efl_VG_Container_Data *cd;
        Eina_List *l;
        Eo *child;

        cd = eo_data_scope_get(root, EFL_VG_CONTAINER_CLASS);
        EINA_LIST_FOREACH(cd->children, l, child)
          _efl_vg_walk_down_at(child, a, r);
     }
}

static void
_efl_vg_object_at(Eo *obj, Eina_Array *a, Eina_Rectangle *r)
{
   Eo *root;

   root = _efl_vg_root_parent_get(obj);
   if (!root) return ;

   _efl_vg_walk_down_at(root, a, r);
}

static Efl_Gfx_Stack *
_efl_vg_efl_gfx_stack_below_get(Eo *obj, Efl_VG_Data *pd EINA_UNUSED)
{
   Eina_Rectangle r;
   Eina_Array a;
   Eo *current;
   Eo *below = NULL;
   Eina_Array_Iterator iterator;
   unsigned int i;

   efl_vg_bounds_get(obj, &r);

   eina_array_step_set(&a, sizeof (Eina_Array), 8);

   _efl_vg_object_at(obj, &a, &r);

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
_efl_vg_efl_gfx_stack_above_get(Eo *obj, Efl_VG_Data *pd EINA_UNUSED)
{
   Eina_Rectangle r;
   Eina_Array a;
   Eo *current;
   Eo *above = NULL;
   Eina_Array_Iterator iterator;
   unsigned int i;

   efl_vg_bounds_get(obj, &r);

   eina_array_step_set(&a, sizeof (Eina_Array), 8);

   _efl_vg_object_at(obj, &a, &r);

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

static Efl_VG_Interpolation *
_efl_vg_interpolation_get(Efl_VG_Data *pd)
{
   Eina_Matrix4 m;

   if (!pd->m) return NULL;
   if (pd->intp) return pd->intp;

   pd->intp = calloc(1, sizeof (Efl_VG_Interpolation));
   if (!pd->intp) return NULL;

   eina_matrix3_matrix4_to(&m, pd->m);

   if (eina_matrix4_quaternion_to(&pd->intp->rotation,
                                  &pd->intp->perspective,
                                  &pd->intp->translation,
                                  &pd->intp->scale,
                                  &pd->intp->skew,
                                  &m))
     return pd->intp;

   free(pd->intp);
   pd->intp = NULL;

   return NULL;
}

static inline void
_efl_vg_interpolate_point(Eina_Point_3D *d,
                          const Eina_Point_3D *a, const Eina_Point_3D *b,
                          double pos_map, double from_map)
{
   d->x = a->x * from_map + b->x * pos_map;
   d->y = a->y * from_map + b->y * pos_map;
   d->z = a->z * from_map + b->z * pos_map;
}

static Eina_Bool
_efl_vg_interpolate(Eo *obj,
                         Efl_VG_Data *pd, const Efl_VG *from, const Efl_VG *to,
                         double pos_map)
{
   Efl_VG_Data *fromd, *tod;
   double from_map;
   Eina_Bool r = EINA_TRUE;

   fromd = eo_data_scope_get(from, EFL_VG_CLASS);
   tod = eo_data_scope_get(to, EFL_VG_CLASS);
   from_map = 1.0 - pos_map;

   eo_del(pd->renderer);
   pd->renderer = NULL;

   if (fromd->m || tod->m)
     {
        if (!pd->m) pd->m = malloc(sizeof (Eina_Matrix3));
        if (pd->m)
          {
             const Efl_VG_Interpolation *fi, *ti;
             Efl_VG_Interpolation result;
             Eina_Matrix4 m;

             fi = _efl_vg_interpolation_get(fromd);
             if (!fi) fi = &interpolation_identity;
             ti = _efl_vg_interpolation_get(tod);
             if (!ti) ti = &interpolation_identity;

             eina_quaternion_slerp(&result.rotation,
                                   &fi->rotation, &ti->rotation,
                                   pos_map);
             _efl_vg_interpolate_point(&result.translation,
                                       &fi->translation, &ti->translation,
                                       pos_map, from_map);
             _efl_vg_interpolate_point(&result.scale,
                                       &fi->scale, &ti->scale,
                                       pos_map, from_map);
             _efl_vg_interpolate_point(&result.skew,
                                       &fi->skew, &ti->skew,
                                       pos_map, from_map);

             result.perspective.x = fi->perspective.x * from_map + ti->perspective.x * pos_map;
             result.perspective.y = fi->perspective.y * from_map + ti->perspective.y * pos_map;
             result.perspective.z = fi->perspective.z * from_map + ti->perspective.z * pos_map;
             result.perspective.w = fi->perspective.w * from_map + ti->perspective.w * pos_map;

             eina_quaternion_matrix4_to(&m,
                                        &result.rotation,
                                        &result.perspective,
                                        &result.translation,
                                        &result.scale,
                                        &result.skew);
             eina_matrix4_matrix3_to(pd->m, &m);
          }
     }

   pd->x = fromd->x * from_map + tod->x * pos_map;
   pd->y = fromd->y * from_map + tod->y * pos_map;

   pd->r = fromd->r * from_map + tod->r * pos_map;
   pd->g = fromd->g * from_map + tod->g * pos_map;
   pd->b = fromd->b * from_map + tod->b * pos_map;
   pd->a = fromd->a * from_map + tod->a * pos_map;

   pd->visibility = pos_map >= 0.5 ? tod->visibility : fromd->visibility;

   if (fromd->mask && tod->mask && pd->mask)
     {
        r &= efl_vg_interpolate(pd->mask, fromd->mask, tod->mask, pos_map);
     }

   _efl_vg_changed(obj);

   return r;
}

static void
_efl_vg_dup(Eo *obj, Efl_VG_Data *pd, const Efl_VG *from)
{
   Efl_VG_Container_Data *cd = NULL;
   Efl_VG_Data *fromd;
   Eo *parent = NULL;

   fromd = eo_data_scope_get(from, EFL_VG_CLASS);
   if (pd->name != fromd->name)
     {
        eina_stringshare_del(pd->name);
        pd->name = eina_stringshare_ref(fromd->name);
     }

   _efl_vg_parent_checked_get(obj, &parent, &cd);
   if (cd) _efl_vg_name_insert(obj, pd, cd);

   if (pd->intp)
     {
        free(pd->intp);
        pd->intp = NULL;
     }

   if (pd->renderer)
     {
        eo_del(pd->renderer);
        pd->renderer = NULL;
     }

   if (fromd->m)
     {
        pd->m = pd->m ? pd->m : malloc(sizeof (Eina_Matrix3)) ;
        if (pd->m) memcpy(pd->m, fromd->m, sizeof (Eina_Matrix3));
     }
   else
     {
        free(pd->m);
     }

   // We may come from an already duped/initialized node, clean it first
   _efl_vg_clean_object(&pd->mask);
   if (fromd->mask)
     {
        pd->mask = eo_add(eo_class_get(fromd->mask), obj, efl_vg_dup(eo_self, pd->mask));
     }

   pd->x = fromd->x;
   pd->y = fromd->y;
   pd->r = fromd->r;
   pd->g = fromd->g;
   pd->b = fromd->b;
   pd->a = fromd->a;
   pd->visibility = fromd->visibility;

   _efl_vg_changed(obj);
}

EAPI Eina_Bool
evas_vg_node_visible_get(Eo *obj)
{
   return efl_gfx_visible_get(obj);
}

EAPI void
evas_vg_node_visible_set(Eo *obj, Eina_Bool v)
{
   efl_gfx_visible_set(obj, v);
}

EAPI void
evas_vg_node_color_get(Eo *obj, int *r, int *g, int *b, int *a)
{
   efl_gfx_color_get(obj, r, g, b, a);
}

EAPI void
evas_vg_node_color_set(Eo *obj, int r, int g, int b, int a)
{
   efl_gfx_color_set(obj, r, g, b, a);
}

EAPI void
evas_vg_node_geometry_get(Eo *obj, int *x, int *y, int *w, int *h)
{
   efl_gfx_position_get(obj, x, y);
   efl_gfx_size_get(obj, w, h);
}

EAPI void
evas_vg_node_geometry_set(Eo *obj, int x, int y, int w, int h)
{
   efl_gfx_position_set(obj, x, y);
   efl_gfx_size_set(obj, w, h);
}

EAPI void
evas_vg_node_stack_below(Eo *obj, Eo *below)
{
   efl_gfx_stack_below(obj, below);
}

EAPI void
evas_vg_node_stack_above(Eo *obj, Eo *above)
{
   efl_gfx_stack_above(obj, above);
}

EAPI void
evas_vg_node_raise(Eo *obj)
{
   efl_gfx_stack_raise(obj);
}

EAPI void
evas_vg_node_lower(Eo *obj)
{
   efl_gfx_stack_lower(obj);
}

#include "efl_vg.eo.c"

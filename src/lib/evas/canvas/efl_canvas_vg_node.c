#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#include <string.h>
#include <math.h>

#define MY_CLASS EFL_CANVAS_VG_NODE_CLASS

static const Efl_Canvas_Vg_Interpolation interpolation_identity = {
  { 0, 0, 0, 1 },
  { 0, 0, 0, 1 },
  { 0, 0, 0 },
  { 1, 1, 1 },
  { 0, 0, 0 }
};

static void
_node_change(Efl_VG *obj, Efl_Canvas_Vg_Node_Data *nd)
{
   if (!nd || nd->flags != EFL_GFX_CHANGE_FLAG_NONE) return;
   nd->flags = EFL_GFX_CHANGE_FLAG_ALL;

   Eo *p = obj;
   while ((p = efl_parent_get(p)))
     {
        if (!efl_isa(p, MY_CLASS)) break;
        Efl_Canvas_Vg_Node_Data *pnd = efl_data_scope_get(p, MY_CLASS);
        if (pnd->flags != EFL_GFX_CHANGE_FLAG_NONE) break;
        pnd->flags = EFL_GFX_CHANGE_FLAG_ALL;
     }
   if (efl_invalidated_get(nd->vg_obj)) return;
   efl_canvas_vg_object_change(nd->vd);
}

static void
_efl_canvas_vg_node_transformation_set(Eo *obj,
                                       Efl_Canvas_Vg_Node_Data *pd,
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

   /* NOTE: _node_change function is only executed
            when pd->flags is EFL_GFX_CHANGE_FLAG_NONE to prevent duplicate calls.*/
   _node_change(obj, pd);
   pd->flags |= EFL_GFX_CHANGE_FLAG_MATRIX;
}

const Eina_Matrix3 *
_efl_canvas_vg_node_transformation_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Node_Data *pd)
{
   return pd->m;
}

static void
_efl_canvas_vg_node_mask_set(Eo *obj EINA_UNUSED,
                             Efl_Canvas_Vg_Node_Data *pd EINA_UNUSED,
                             Efl_Canvas_Vg_Node *mask EINA_UNUSED,
                             int op EINA_UNUSED)
{
}

static void
_efl_canvas_vg_node_origin_set(Eo *obj,
                               Efl_Canvas_Vg_Node_Data *pd,
                               double x, double y)
{
   pd->x = x;
   pd->y = y;

   _node_change(obj, pd);
}

static void
_efl_canvas_vg_node_origin_get(const Eo *obj EINA_UNUSED,
                        Efl_Canvas_Vg_Node_Data *pd,
                        double *x, double *y)
{
   if (x) *x = pd->x;
   if (y) *y = pd->y;
}

static void
_efl_canvas_vg_node_efl_gfx_entity_position_set(Eo *obj,
                                                Efl_Canvas_Vg_Node_Data *pd,
                                                Eina_Position2D pos)
{
   pd->x = (double) pos.x;
   pd->y = (double) pos.y;

   _node_change(obj, pd);
}

static Eina_Position2D
_efl_canvas_vg_node_efl_gfx_entity_position_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Node_Data *pd)
{
   // NOTE: This casts double to int!
   return EINA_POSITION2D(pd->x, pd->y);
}

static void
_efl_canvas_vg_node_efl_gfx_entity_visible_set(Eo *obj,
                                               Efl_Canvas_Vg_Node_Data *pd,
                                               Eina_Bool v)
{
   pd->visibility = v;

   _node_change(obj, pd);
}


static Eina_Bool
_efl_canvas_vg_node_efl_gfx_entity_visible_get(const Eo *obj EINA_UNUSED,
                                      Efl_Canvas_Vg_Node_Data *pd)
{
   return pd->visibility;
}

static void
_efl_canvas_vg_node_efl_gfx_color_color_set(Eo *obj,
                                            Efl_Canvas_Vg_Node_Data *pd,
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

   _node_change(obj, pd);
}

static void
_efl_canvas_vg_node_efl_gfx_color_color_get(const Eo *obj EINA_UNUSED,
                                    Efl_Canvas_Vg_Node_Data *pd,
                                    int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->r;
   if (g) *g = pd->g;
   if (b) *b = pd->b;
   if (a) *a = pd->a;
}

static Eina_Size2D
_efl_canvas_vg_node_efl_gfx_entity_size_get(const Eo *obj, Efl_Canvas_Vg_Node_Data *pd EINA_UNUSED)
{
   Eina_Rect r;

   efl_gfx_path_bounds_get(obj, &r);
   return r.size;
}

EOLIAN static Eina_Rect
_efl_canvas_vg_node_efl_gfx_entity_geometry_get(const Eo *obj, Efl_Canvas_Vg_Node_Data *pd EINA_UNUSED)
{
   Eina_Rect r;
   r.pos = efl_gfx_entity_position_get(obj);
   r.size = efl_gfx_entity_size_get(obj);
   return r;
}

// Parent should be a container otherwise dismissing the stacking operation
static Eina_Bool
_efl_canvas_vg_node_parent_checked_get(Eo *obj,
                                       Eo **parent,
                                       Efl_Canvas_Vg_Container_Data **cd)
{
   *cd = NULL;
   *parent = efl_parent_get(obj);

   if (efl_isa(*parent, EFL_CANVAS_VG_CONTAINER_CLASS))
     *cd = efl_data_scope_get(*parent, EFL_CANVAS_VG_CONTAINER_CLASS);
   else if (efl_isa(*parent, EFL_CANVAS_VG_OBJECT_CLASS))
     *parent = NULL;
   else if (*parent)
     {
        ERR("Parent of unauthorized class '%s'.", efl_class_name_get(efl_class_get(*parent)));
        *parent = NULL;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eo *
_efl_canvas_vg_node_efl_object_constructor(Eo *obj,
                                           Efl_Canvas_Vg_Node_Data *nd)
{
   Efl_Canvas_Vg_Container_Data *cd;
   Eo *parent;

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   if (!_efl_canvas_vg_node_parent_checked_get(obj, &parent, &cd))
     {
        ERR("Failed");
        return NULL;
     }

   if (parent)
     {
        Efl_Canvas_Vg_Node_Data *parent_nd =
           efl_data_scope_get(parent, MY_CLASS);
        nd->vg_obj = parent_nd->vg_obj;
        nd->vd = parent_nd->vd;
     }

   nd->flags = EFL_GFX_CHANGE_FLAG_ALL;
   nd->changed = EINA_TRUE;
   nd->visibility = EINA_TRUE;

   return obj;
}

static void
_efl_canvas_vg_node_efl_object_invalidate(Eo *obj, Efl_Canvas_Vg_Node_Data *pd)
{
   if (pd->renderer)
     {
        efl_unref(pd->renderer);
        pd->renderer = NULL;
     }

   efl_invalidate(efl_super(obj, MY_CLASS));
}

static void
_efl_canvas_vg_node_efl_object_destructor(Eo *obj, Efl_Canvas_Vg_Node_Data *pd)
{
   if (pd->m)
     {
        free(pd->m);
        pd->m = NULL;
     }
   if (pd->intp)
     {
        free(pd->intp);
        pd->intp = NULL;
     }

   efl_destructor(efl_super(obj, MY_CLASS));
}

static void
_efl_canvas_vg_node_name_insert(Eo *obj, Efl_Canvas_Vg_Container_Data *cd)
{
   Eo *set;
   const char *name = efl_name_get(efl_super(obj, MY_CLASS));
   if (!name) return;

   set = eina_hash_find(cd->names, name);
   if (set == obj) return;

   if (set)
     {
        ERR("node name(%s) is already exist in container but child node(%p) is different...", name, obj);
        efl_name_set(efl_super(obj, MY_CLASS), NULL);
     }
   else
     {
        eina_hash_direct_add(cd->names, name, obj);
     }
}

static void
_efl_canvas_vg_node_efl_object_name_set(Eo *obj, Efl_Canvas_Vg_Node_Data *pd EINA_UNUSED, const char *name)
{
   Efl_Canvas_Vg_Container_Data *cd;
   Eo *parent;
   const char *pname = efl_name_get(obj);

   if (_efl_canvas_vg_node_parent_checked_get(obj, &parent, &cd))
     {
        if (pname) eina_hash_del(cd->names, pname, obj);
     }

   efl_name_set(efl_super(obj, MY_CLASS), name);

   if (cd) _efl_canvas_vg_node_name_insert(obj, cd);
}

static void
_efl_canvas_vg_node_efl_object_parent_set(Eo *obj,
                                          Efl_Canvas_Vg_Node_Data *nd,
                                          Eo *parent)
{
   Efl_Canvas_Vg_Container_Data *cd = NULL;
   Efl_Canvas_Vg_Container_Data *old_cd;
   Efl_VG *old_parent;

   if (parent)
     {
        if (efl_isa(parent, EFL_CANVAS_VG_CONTAINER_CLASS))
          cd = efl_data_scope_get(parent, EFL_CANVAS_VG_CONTAINER_CLASS);
        else if (efl_isa(parent, EFL_CANVAS_VG_OBJECT_CLASS))
          {
             if (nd->vg_obj != parent)
               {
                  nd->vg_obj = parent;
                  nd->vd = efl_data_scope_get(parent, EFL_CANVAS_VG_OBJECT_CLASS);
               }
          }
        else
          {
             ERR("parent(%p, class = %s) is not allowed by vg node(%p).",
                 parent, efl_class_name_get(efl_class_get(parent)), obj);
             return;
          }
     }
   else
     {
        nd->vg_obj = NULL;
        nd->vd = NULL;
     }

   if (!_efl_canvas_vg_node_parent_checked_get(obj, &old_parent, &old_cd))
     return;

   // FIXME: this may become slow with to much object
   if (old_cd)
     {
        old_cd->children = eina_list_remove(old_cd->children, obj);
        eina_hash_del(old_cd->names, efl_name_get(efl_super(obj, MY_CLASS)), obj);
        _node_change(old_parent, efl_data_scope_get(old_parent, MY_CLASS));
     }

   efl_parent_set(efl_super(obj, MY_CLASS), parent);

   if (cd)
     {
        cd->children = eina_list_append(cd->children, obj);
        _efl_canvas_vg_node_name_insert(obj, cd);

        Efl_Canvas_Vg_Node_Data *parent_nd = efl_data_scope_get(parent, MY_CLASS);
        if (nd->vg_obj != parent_nd->vg_obj)
          {
             nd->vg_obj = parent_nd->vg_obj;
             nd->vd = parent_nd->vd;
          }
     }

   if (parent) _node_change(obj, nd);
}

static void
_efl_canvas_vg_node_efl_gfx_stack_raise_to_top(Eo *obj, Efl_Canvas_Vg_Node_Data *pd EINA_UNUSED)
{
   Efl_Canvas_Vg_Container_Data *cd;
   Eina_List *lookup, *next;
   Eo *parent;

   parent = efl_parent_get(obj);
   if (!efl_isa(parent, EFL_CANVAS_VG_CONTAINER_CLASS)) goto on_error;
   cd = efl_data_scope_get(parent, EFL_CANVAS_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   next = eina_list_next(lookup);
   if (!next) return ;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_append_relative_list(cd->children, obj, next);

   _node_change(parent, efl_data_scope_get(parent, MY_CLASS));
   return;

 on_error:
   ERR("Err");
}

static void
_efl_canvas_vg_node_efl_gfx_stack_stack_above(Eo *obj,
                                              Efl_Canvas_Vg_Node_Data *pd EINA_UNUSED,
                                              Efl_Gfx_Stack *above)
{
   Efl_Canvas_Vg_Container_Data *cd;
   Eina_List *lookup, *ref;
   Eo *parent;

   parent = efl_parent_get(obj);
   if (!efl_isa(parent, EFL_CANVAS_VG_CONTAINER_CLASS)) goto on_error;
   cd = efl_data_scope_get(parent, EFL_CANVAS_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   ref = eina_list_data_find_list(cd->children, above);
   if (!ref) goto on_error;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_append_relative_list(cd->children, obj, ref);

   _node_change(parent, efl_data_scope_get(parent, MY_CLASS));
   return;

 on_error:
   ERR("Err");
}

static void
_efl_canvas_vg_node_efl_gfx_stack_stack_below(Eo *obj,
                                              Efl_Canvas_Vg_Node_Data *pd EINA_UNUSED,
                                              Efl_Gfx_Stack *below)
{
   Efl_Canvas_Vg_Container_Data *cd;
   Eina_List *lookup, *ref;
   Eo *parent;

   parent = efl_parent_get(obj);
   if (!efl_isa(parent, EFL_CANVAS_VG_CONTAINER_CLASS)) goto on_error;
   cd = efl_data_scope_get(parent, EFL_CANVAS_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   ref = eina_list_data_find_list(cd->children, below);
   if (!ref) goto on_error;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_prepend_relative_list(cd->children, obj, ref);

   _node_change(parent, efl_data_scope_get(parent, MY_CLASS));
   return;

 on_error:
   ERR("Err");
}

static void
_efl_canvas_vg_node_efl_gfx_stack_lower_to_bottom(Eo *obj, Efl_Canvas_Vg_Node_Data *pd EINA_UNUSED)
{
   Efl_Canvas_Vg_Container_Data *cd;
   Eina_List *lookup, *prev;
   Eo *parent;

   parent = efl_parent_get(obj);
   if (!efl_isa(parent, EFL_CANVAS_VG_CONTAINER_CLASS)) goto on_error;
   cd = efl_data_scope_get(parent, EFL_CANVAS_VG_CONTAINER_CLASS);

   // FIXME: this could become slow with to much object
   lookup = eina_list_data_find_list(cd->children, obj);
   if (!lookup) goto on_error;

   prev = eina_list_prev(lookup);
   if (!prev) return;

   cd->children = eina_list_remove_list(cd->children, lookup);
   cd->children = eina_list_prepend_relative_list(cd->children, obj, prev);

   _node_change(parent, efl_data_scope_get(parent, MY_CLASS));
   return;

 on_error:
   ERR("Err");
}

static const Eo *
_efl_canvas_vg_node_root_parent_get(const Eo *obj)
{
   const Eo *parent;

   parent = efl_parent_get(obj);

   if (!parent) return obj;
   return _efl_canvas_vg_node_root_parent_get(parent);
}

static void
_efl_canvas_vg_node_walk_down_at(const Eo *root, Eina_Array *a, Eina_Rectangle *r)
{
   Eina_Rect bounds;

   efl_gfx_path_bounds_get(root, &bounds);
   if (!eina_rectangles_intersect(&bounds.rect, r)) return ;

   eina_array_push(a, root);

   if (efl_isa(root, EFL_CANVAS_VG_CONTAINER_CLASS))
     {
        Efl_Canvas_Vg_Container_Data *cd;
        Eina_List *l;
        Eo *child;

        cd = efl_data_scope_get(root, EFL_CANVAS_VG_CONTAINER_CLASS);
        EINA_LIST_FOREACH(cd->children, l, child)
          _efl_canvas_vg_node_walk_down_at(child, a, r);
     }
}

static void
_efl_canvas_vg_node_object_at(const Eo *obj, Eina_Array *a, Eina_Rectangle *r)
{
   const Eo *root;

   root = _efl_canvas_vg_node_root_parent_get(obj);
   if (!root) return ;

   _efl_canvas_vg_node_walk_down_at(root, a, r);
}

static Efl_Gfx_Stack *
_efl_canvas_vg_node_efl_gfx_stack_below_get(const Eo *obj, Efl_Canvas_Vg_Node_Data *pd EINA_UNUSED)
{
   Eina_Rect r;
   Eina_Array a;
   Eo *current;
   Eo *below = NULL;
   Eina_Array_Iterator iterator;
   unsigned int i;

   efl_gfx_path_bounds_get(obj, &r);

   eina_array_step_set(&a, sizeof (Eina_Array), 8);

   _efl_canvas_vg_node_object_at(obj, &a, &r.rect);

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
_efl_canvas_vg_node_efl_gfx_stack_above_get(const Eo *obj, Efl_Canvas_Vg_Node_Data *pd EINA_UNUSED)
{
   Eina_Rect r;
   Eina_Array a;
   Eo *current;
   Eo *above = NULL;
   Eina_Array_Iterator iterator;
   unsigned int i;

   efl_gfx_path_bounds_get(obj, &r);

   eina_array_step_set(&a, sizeof (Eina_Array), 8);

   _efl_canvas_vg_node_object_at(obj, &a, &r.rect);

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

static Efl_Canvas_Vg_Interpolation *
_efl_canvas_vg_node_interpolation_get(Efl_Canvas_Vg_Node_Data *pd)
{
   Eina_Matrix4 m;

   if (!pd->m) return NULL;
   if (pd->intp) return pd->intp;

   pd->intp = calloc(1, sizeof (Efl_Canvas_Vg_Interpolation));
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
_efl_canvas_vg_node_interpolate_point(Eina_Point_3D *d,
                                      const Eina_Point_3D *a, const Eina_Point_3D *b,
                                      double pos_map, double from_map)
{
   d->x = a->x * from_map + b->x * pos_map;
   d->y = a->y * from_map + b->y * pos_map;
   d->z = a->z * from_map + b->z * pos_map;
}

/* Warning! Node itself doesn't have any path. Don't call super class(Path)'s */
static Eina_Bool
_efl_canvas_vg_node_efl_gfx_path_interpolate(Eo *obj,
                                             Efl_Canvas_Vg_Node_Data *pd,
                                             const Efl_VG *from,
                                             const Efl_VG *to,
                                             double pos_map)
{
   Efl_Canvas_Vg_Node_Data *fromd, *tod;
   double from_map;

   //Check if both objects have same type
   if (!(efl_isa(from, MY_CLASS) && efl_isa(to, MY_CLASS)))
     return EINA_FALSE;

   fromd = efl_data_scope_get(from, MY_CLASS);
   tod = efl_data_scope_get(to, MY_CLASS);
   from_map = 1.0 - pos_map;

   efl_unref(pd->renderer);
   pd->renderer = NULL;

   //Interpolates Node Transform Matrix
   if (fromd->m || tod->m)
     {
        if (!pd->m) pd->m = malloc(sizeof (Eina_Matrix3));
        if (pd->m)
          {
             const Efl_Canvas_Vg_Interpolation *fi, *ti;
             Efl_Canvas_Vg_Interpolation result;
             Eina_Matrix4 m;

             fi = _efl_canvas_vg_node_interpolation_get(fromd);
             if (!fi) fi = &interpolation_identity;

             ti = _efl_canvas_vg_node_interpolation_get(tod);
             if (!ti) ti = &interpolation_identity;

             eina_quaternion_slerp(&result.rotation,
                                   &fi->rotation, &ti->rotation,
                                   pos_map);
             _efl_canvas_vg_node_interpolate_point(&result.translation,
                                       &fi->translation, &ti->translation,
                                       pos_map, from_map);
             _efl_canvas_vg_node_interpolate_point(&result.scale,
                                       &fi->scale, &ti->scale,
                                       pos_map, from_map);
             _efl_canvas_vg_node_interpolate_point(&result.skew,
                                       &fi->skew, &ti->skew,
                                       pos_map, from_map);

             result.perspective.x =
                fi->perspective.x * from_map + ti->perspective.x * pos_map;
             result.perspective.y =
                fi->perspective.y * from_map + ti->perspective.y * pos_map;
             result.perspective.z =
                fi->perspective.z * from_map + ti->perspective.z * pos_map;
             result.perspective.w =
                fi->perspective.w * from_map + ti->perspective.w * pos_map;

             eina_quaternion_matrix4_to(&m,
                                        &result.rotation,
                                        &result.perspective,
                                        &result.translation,
                                        &result.scale,
                                        &result.skew);

             eina_matrix4_matrix3_to(pd->m, &m);
          }
     }

   //Position
   pd->x = fromd->x * from_map + tod->x * pos_map;
   pd->y = fromd->y * from_map + tod->y * pos_map;

   //Color
   pd->r = fromd->r * from_map + tod->r * pos_map;
   pd->g = fromd->g * from_map + tod->g * pos_map;
   pd->b = fromd->b * from_map + tod->b * pos_map;
   pd->a = fromd->a * from_map + tod->a * pos_map;

   pd->visibility = pos_map >= 0.5 ? tod->visibility : fromd->visibility;

   _node_change(obj, pd);

   return EINA_TRUE;
}

void
efl_canvas_vg_node_vg_obj_set(Efl_VG *node, Efl_VG *vg_obj, Efl_Canvas_Vg_Object_Data *vd)
{
   Efl_Canvas_Vg_Node_Data *nd = efl_data_scope_get(node, MY_CLASS);
   if (nd->vg_obj == vg_obj) return;
   nd->vg_obj = vg_obj;
   nd->vd = vd;

   //root node is always container.
   efl_canvas_vg_container_vg_obj_update(node, nd);
}

void
efl_canvas_vg_node_change(Eo *obj)
{
   if (!obj) return;
   _node_change(obj, efl_data_scope_get(obj, EFL_CANVAS_VG_NODE_CLASS));
}

EOLIAN static Efl_VG *
_efl_canvas_vg_node_efl_duplicate_duplicate(const Eo *obj, Efl_Canvas_Vg_Node_Data *pd)
{
   Efl_VG *node;
   Efl_Canvas_Vg_Node_Data *nd;

   node = efl_add_ref(efl_class_get(obj), NULL);
   nd = efl_data_scope_get(node, MY_CLASS);

   //Hmm...?
   efl_name_set(efl_super(node, MY_CLASS), efl_name_get(efl_super(obj, MY_CLASS)));

   if (pd->m)
     {
        nd->m = malloc(sizeof(Eina_Matrix3));
        if (nd->m) memcpy(nd->m, pd->m, sizeof(Eina_Matrix3));
     }

   nd->x = pd->x;
   nd->y = pd->y;
   nd->r = pd->r;
   nd->g = pd->g;
   nd->b = pd->b;
   nd->a = pd->a;
   nd->visibility = pd->visibility;

   return node;
}

EAPI Eina_Bool
evas_vg_node_visible_get(Eo *obj)
{
   return efl_gfx_entity_visible_get(obj);
}

EAPI void
evas_vg_node_visible_set(Eo *obj, Eina_Bool v)
{
   efl_gfx_entity_visible_set(obj, v);
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
   Eina_Rect r;
   r.pos = efl_gfx_entity_position_get(obj);
   r.size = efl_gfx_entity_size_get(obj);
   if (x) *x = r.x;
   if (y) *y = r.y;
   if (w) *w = r.w;
   if (h) *h = r.h;
}

EAPI void
evas_vg_node_geometry_set(Eo *obj, int x, int y, int w, int h)
{
   efl_gfx_entity_position_set(obj, EINA_POSITION2D(x, y));
   efl_gfx_entity_size_set(obj, EINA_SIZE2D(w,  h));
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
   efl_gfx_stack_raise_to_top(obj);
}

EAPI void
evas_vg_node_lower(Eo *obj)
{
   efl_gfx_stack_lower_to_bottom(obj);
}

#include "efl_canvas_vg_node.eo.c"
#include "efl_canvas_vg_node_eo.legacy.c"

#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EFL_VG_CONTAINER_CLASS

static void
_efl_vg_container_render_pre(Eo *obj EINA_UNUSED,
                             Eina_Matrix3 *parent,
                             Ector_Surface *s,
                             void *data,
                             Efl_VG_Data *nd)
{
   Efl_VG_Container_Data *pd = data;
   Eina_List *l;
   Eo *child;

   if (!nd->changed) return ;
   nd->changed = EINA_FALSE;

   EFL_VG_COMPUTE_MATRIX(current, parent, nd);

   EINA_LIST_FOREACH(pd->children, l, child)
     _evas_vg_render_pre(child, s, current);
}

static Eo *
_efl_vg_container_eo_base_constructor(Eo *obj,
                                      Efl_VG_Container_Data *pd)
{
   Efl_VG_Data *nd;

   pd->names = eina_hash_stringshared_new(NULL);

   obj = eo_constructor(eo_super(obj, MY_CLASS));

   nd = eo_data_scope_get(obj, EFL_VG_CLASS);
   nd->render_pre = _efl_vg_container_render_pre;
   nd->data = pd;

   return obj;
}

static void
_efl_vg_container_eo_base_destructor(Eo *obj,
                                     Efl_VG_Container_Data *pd EINA_UNUSED)
{
   eo_destructor(eo_super(obj, MY_CLASS));

   eina_hash_free(pd->names);
   pd->names = NULL;
}

static void
_efl_vg_container_efl_vg_bounds_get(Eo *obj EINA_UNUSED,
                                        Efl_VG_Container_Data *pd,
                                        Eina_Rectangle *r)
{
   Eina_Rectangle s;
   Eina_Bool first = EINA_TRUE;
   Eina_List *l;
   Eo *child;

   EINA_RECTANGLE_SET(&s, -1, -1, 0, 0);

   EINA_LIST_FOREACH(pd->children, l, child)
     {
        if (first)
          {
             efl_vg_bounds_get(child, r);
             first = EINA_FALSE;
          }
        else
          {
             efl_vg_bounds_get(child, &s);
             eina_rectangle_union(r, &s);
          }
     }
}

static Efl_VG *
_efl_vg_container_child_get(Eo *obj EINA_UNUSED, Efl_VG_Container_Data *pd, const char *name)
{
   const char *tmp = eina_stringshare_add(name);
   Efl_VG *r;

   r = eina_hash_find(pd->names, tmp);
   eina_stringshare_del(tmp);

   return r;
}

static Eina_Iterator *
_efl_vg_container_children_get(Eo *obj EINA_UNUSED, Efl_VG_Container_Data *pd)
{
   return eina_list_iterator_new(pd->children);
}

static Eina_Bool
_efl_vg_container_efl_vg_interpolate(Eo *obj,
                                          Efl_VG_Container_Data *pd,
                                          const Efl_VG *from, const Efl_VG *to,
                                          double pos_map)
{
   Eina_Iterator *from_it, *to_it;
   Eina_List *l;
   Eina_Bool r, res = EINA_TRUE;
   Eo *from_child, *to_child, *child;

   //1. check if both the object are containers
   if (!(eo_isa(from, EFL_VG_CONTAINER_CLASS) &&
         eo_isa(to, EFL_VG_CONTAINER_CLASS)))
     return EINA_FALSE;

   r = efl_vg_interpolate(eo_super(obj, EFL_VG_CONTAINER_CLASS), from, to, pos_map);

   if (!r) return EINA_FALSE;

   from_it = efl_vg_container_children_get((Efl_VG *)from);
   to_it = efl_vg_container_children_get((Efl_VG *)to);
   EINA_LIST_FOREACH (pd->children, l, child)
     {
        res &= eina_iterator_next(from_it, (void **)&from_child);
        res &= eina_iterator_next(to_it, (void **)&to_child);
        if (!res && (eo_class_get(from_child) != eo_class_get(to_child) ||
            (eo_class_get(child) != eo_class_get(from_child))))
          {
             r = EINA_FALSE;
             break;
          }
        r &= efl_vg_interpolate(child, from_child, to_child, pos_map);
        if (!r)
          break;
     }
   eina_iterator_free(from_it);
   eina_iterator_free(to_it);

   return r;
}

static void
_efl_vg_container_efl_vg_dup(Eo *obj,
                                  Efl_VG_Container_Data *pd,
                                  const Efl_VG *from)
{
   Efl_VG_Container_Data *fromd;
   Eina_List *l;
   Eo *child;

   efl_vg_dup(eo_super(obj, EFL_VG_CONTAINER_CLASS), from);

   fromd = eo_data_scope_get(from, EFL_VG_CONTAINER_CLASS);

   EINA_LIST_FREE(pd->children, child)
     eo_unref(child);

   EINA_LIST_FOREACH(fromd->children, l, child)
     {
        // By setting parent, we automatically reference
        // this new object as a child of obj. Magic at work !
        (void) eo_add_ref(eo_class_get(child), obj, efl_vg_dup(eo_self, child));
     }
}

EAPI Efl_VG*
evas_vg_container_add(Efl_VG *parent)
{
   return eo_add(EFL_VG_CONTAINER_CLASS, parent);
}

#include "efl_vg_container.eo.c"

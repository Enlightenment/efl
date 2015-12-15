#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EFL_VG_CONTAINER_CLASS

static void
_efl_vg_container_render_pre(Eo *obj EINA_UNUSED,
                             Eina_Matrix3 *parent,
                             Ector_Surface *s,
                             void *data,
                             Efl_VG_Base_Data *nd)
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
   Efl_VG_Base_Data *nd;

   pd->names = eina_hash_stringshared_new(NULL);

   obj = eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());

   nd = eo_data_scope_get(obj, EFL_VG_BASE_CLASS);
   nd->render_pre = _efl_vg_container_render_pre;
   nd->data = pd;

   return obj;
}

static void
_efl_vg_container_eo_base_destructor(Eo *obj,
                                     Efl_VG_Container_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());

   eina_hash_free(pd->names);
   pd->names = NULL;
}

static void
_efl_vg_container_efl_vg_base_bounds_get(Eo *obj EINA_UNUSED,
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
             eo_do(child, efl_vg_bounds_get(r));
             first = EINA_FALSE;
          }
        else
          {
             eo_do(child, efl_vg_bounds_get(&s));
             eina_rectangle_union(r, &s);
          }
     }
}

static Efl_VG_Base *
_efl_vg_container_child_get(Eo *obj EINA_UNUSED, Efl_VG_Container_Data *pd, const char *name)
{
   const char *tmp = eina_stringshare_add(name);
   Efl_VG_Base *r;

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
_efl_vg_container_efl_vg_base_interpolate(Eo *obj,
                                          Efl_VG_Container_Data *pd,
                                          const Efl_VG_Base *from, const Efl_VG_Base *to,
                                          double pos_map)
{
   Efl_VG_Container_Data *fd;
   Efl_VG_Container_Data *td;
   Eina_Iterator *it;
   Eina_Hash_Tuple *tuple;
   Eina_Bool r;

   eo_do_super(obj, EFL_VG_CONTAINER_CLASS, r = efl_vg_interpolate(from, to, pos_map));

   if (!r) return EINA_FALSE;

   fd = eo_data_scope_get(from, EFL_VG_CONTAINER_CLASS);
   td = eo_data_scope_get(to, EFL_VG_CONTAINER_CLASS);

   it = eina_hash_iterator_tuple_new(pd->names);
   EINA_ITERATOR_FOREACH(it, tuple)
     {
        Eo *fromc, *toc;
        Eo *cc = tuple->data;

        fromc = eina_hash_find(fd->names, tuple->key);
        toc = eina_hash_find(td->names, tuple->key);

        if (!toc || !fromc) continue ;
        if (eo_class_get(toc) != eo_class_get(fromc)) continue ;

        eo_do(cc, r &= efl_vg_interpolate(fromc, toc, pos_map));
     }
   eina_iterator_free(it);

   return r;
}

static void
_efl_vg_container_efl_vg_base_dup(Eo *obj,
                                  Efl_VG_Container_Data *pd,
                                  const Efl_VG_Base *from)
{
   Efl_VG_Container_Data *fromd;
   Eina_List *l;
   Eo *child;

   eo_do_super(obj, EFL_VG_CONTAINER_CLASS, efl_vg_dup(from));

   fromd = eo_data_scope_get(from, EFL_VG_CONTAINER_CLASS);

   EINA_LIST_FREE(pd->children, child)
     eo_unref(child);

   EINA_LIST_FOREACH(fromd->children, l, child)
     {
        // By setting parent, we automatically reference
        // this new object as a child of obj. Magic at work !
        (void) eo_add_ref(eo_class_get(child), obj, efl_vg_dup(child));
     }
}

EAPI Efl_VG*
evas_vg_container_add(Efl_VG *parent)
{
   return eo_add(EFL_VG_CONTAINER_CLASS, parent);
}

#include "efl_vg_container.eo.c"

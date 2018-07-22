#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EFL_CANVAS_VG_CONTAINER_CLASS

static void
_invalidate_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Canvas_Vg_Container_Data *pd;
   Eina_List *l;
   Efl_VG* child;

   pd = efl_data_scope_get(event->object, MY_CLASS);

   /* Clean up all the references by copying.
      If the container is copied, it copies its children as well.
      Since evas_vg_node is duplicated using efl_add_ref(),
      we definitely call efl_unref() after that. */
   EINA_LIST_FOREACH(pd->children, l, child)
      efl_unref(child);
}

static void
_efl_canvas_vg_container_render_pre(Eo *obj EINA_UNUSED,
                             Eina_Matrix3 *parent,
                             Ector_Surface *s,
                             void *data,
                             Efl_Canvas_Vg_Node_Data *nd)
{
   Efl_Canvas_Vg_Container_Data *pd = data;
   Eina_List *l;
   Eo *child;
   Efl_Canvas_Vg_Node_Data *child_nd;
   Efl_Gfx_Change_Flag flag;

   if (nd->flags == EFL_GFX_CHANGE_FLAG_NONE) return ;

   flag = nd->flags;
   nd->flags = EFL_GFX_CHANGE_FLAG_NONE;

   EFL_CANVAS_VG_COMPUTE_MATRIX(current, parent, nd);

   EINA_LIST_FOREACH(pd->children, l, child)
     {
        if (flag & EFL_GFX_CHANGE_FLAG_MATRIX)
          {
             child_nd = efl_data_scope_get(child, EFL_CANVAS_VG_NODE_CLASS);
             child_nd->flags |= EFL_GFX_CHANGE_FLAG_MATRIX;
          }
        _evas_vg_render_pre(child, s, current);
     }
}

static Eo *
_efl_canvas_vg_container_efl_object_constructor(Eo *obj,
                                      Efl_Canvas_Vg_Container_Data *pd)
{
   Efl_Canvas_Vg_Node_Data *nd;

   pd->names = eina_hash_stringshared_new(NULL);

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   nd = efl_data_scope_get(obj, EFL_CANVAS_VG_NODE_CLASS);
   nd->render_pre = _efl_canvas_vg_container_render_pre;
   nd->data = pd;
   nd->flags = EFL_GFX_CHANGE_FLAG_ALL;

   return obj;
}

static void
_efl_canvas_vg_container_efl_object_destructor(Eo *obj,
                                     Efl_Canvas_Vg_Container_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
   eina_hash_free(pd->names);
}

static void
_efl_canvas_vg_container_efl_gfx_path_bounds_get(const Eo *obj EINA_UNUSED,
                                    Efl_Canvas_Vg_Container_Data *pd,
                                    Eina_Rect *r)
{
   Eina_Rect s;
   Eina_Bool first = EINA_TRUE;
   Eina_List *l;
   Eo *child;

   EINA_RECT_SET(s, -1, -1, 0, 0);

   EINA_LIST_FOREACH(pd->children, l, child)
     {
        if (first)
          {
             efl_gfx_path_bounds_get(child, r);
             first = EINA_FALSE;
          }
        else
          {
             efl_gfx_path_bounds_get(child, &s);
             eina_rectangle_union(&r->rect, &s.rect);
          }
     }
}

static Efl_VG *
_efl_canvas_vg_container_child_get(Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Container_Data *pd, const char *name)
{
   const char *tmp = eina_stringshare_add(name);
   Efl_VG *r;

   r = eina_hash_find(pd->names, tmp);
   eina_stringshare_del(tmp);

   return r;
}

static Eina_Iterator *
_efl_canvas_vg_container_children_get(Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Container_Data *pd)
{
   return eina_list_iterator_new(pd->children);
}

static Eina_Bool
_efl_canvas_vg_container_efl_gfx_path_interpolate(Eo *obj, Efl_Canvas_Vg_Container_Data *pd, const Efl_VG *from, const Efl_VG *to, double pos_map)
{
   Eina_Iterator *from_it, *to_it;
   Eina_List *l;
   Eina_Bool r, res = EINA_TRUE;
   Eo *from_child, *to_child, *child;

   //Check if both objects have same type
   if (!(efl_isa(from, MY_CLASS) && efl_isa(to, MY_CLASS)))
     return EINA_FALSE;

   //Interpolates this nodes
   r = efl_gfx_path_interpolate(efl_super(obj, MY_CLASS), from, to, pos_map);
   if (!r) return EINA_FALSE;

   from_it = efl_canvas_vg_container_children_get((Efl_VG *)from);
   to_it = efl_canvas_vg_container_children_get((Efl_VG *)to);

   //Interpolates children
   EINA_LIST_FOREACH (pd->children, l, child)
     {
        res &= eina_iterator_next(from_it, (void **)&from_child);
        res &= eina_iterator_next(to_it, (void **)&to_child);
        if (!res && (efl_class_get(from_child) != efl_class_get(to_child) ||
            (efl_class_get(child) != efl_class_get(from_child))))
          {
             r = EINA_FALSE;
             break;
          }
        r = efl_gfx_path_interpolate(child, from_child, to_child, pos_map);
        if (!r) break;
     }

   eina_iterator_free(from_it);
   eina_iterator_free(to_it);

   return r;
}

EOLIAN static Efl_VG *
_efl_canvas_vg_container_efl_duplicate_duplicate(const Eo *obj,
                                             Efl_Canvas_Vg_Container_Data *pd)
{
   Eina_List *l;
   Efl_VG *child;
   Efl_VG *container;

   container = efl_duplicate(efl_super(obj, MY_CLASS));
   efl_event_callback_add(container, EFL_EVENT_INVALIDATE, _invalidate_cb, NULL);
   efl_parent_set(container, efl_parent_get(obj));

   //Copy Children
   EINA_LIST_FOREACH(pd->children, l, child)
     {
        Efl_VG *eo = efl_duplicate(child);
        efl_parent_set(eo, container);
     }

   return container;
}

EAPI Efl_VG*
evas_vg_container_add(Efl_VG *parent)
{
   /* Warn it because the usage has been changed.
      We can remove this message after v1.21. */
   if (!parent) CRI("Efl_VG Container doesn't allow null parent!");
   return efl_add(MY_CLASS, parent);
}

#include "efl_canvas_vg_container.eo.c"

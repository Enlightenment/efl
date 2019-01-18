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

static Ector_Buffer *
_prepare_mask(Evas_Object_Protected_Data *obj,     //vector object
              Efl_Canvas_Vg_Node* mask_obj,
              Ector_Surface *surface,
              Eina_Matrix3 *ptransform,
              Ector_Buffer *mask,
              int mask_op)
{
   Efl_Canvas_Vg_Container_Data *pd = efl_data_scope_get(mask_obj, MY_CLASS);
   Efl_Canvas_Vg_Node_Data *nd =
         efl_data_scope_get(mask_obj, EFL_CANVAS_VG_NODE_CLASS);
   if (nd->flags == EFL_GFX_CHANGE_FLAG_NONE) return pd->mask.buffer;

   //1. Mask Size
   Eina_Rect mbound;
   mbound.x = 0;
   mbound.y = 0;
   mbound.w = obj->cur->geometry.w;
   mbound.h = obj->cur->geometry.h;

//   efl_gfx_path_bounds_get(mask, &mbound);

   //2. Reusable ector buffer?
   if (!pd->mask.buffer || (pd->mask.bound.w != mbound.w) ||
         (pd->mask.bound.h != mbound.h))
     {
        if (pd->mask.pixels) free(pd->mask.pixels);
        if (pd->mask.buffer) efl_unref(pd->mask.buffer);
        pd->mask.pixels = calloc(sizeof(uint32_t), mbound.w * mbound.h);
        pd->mask.buffer = ENFN->ector_buffer_new(ENC, obj->layer->evas->evas,
                                                 mbound.w, mbound.h,
                                                 EFL_GFX_COLORSPACE_ARGB8888,
                                                 ECTOR_BUFFER_FLAG_DRAWABLE |
                                                 ECTOR_BUFFER_FLAG_CPU_READABLE |
                                                 ECTOR_BUFFER_FLAG_CPU_WRITABLE);
        ector_buffer_pixels_set(pd->mask.buffer, pd->mask.pixels,
                                mbound.w, mbound.h, 0,
                                EFL_GFX_COLORSPACE_ARGB8888, EINA_TRUE);
        pd->mask.bound.w = mbound.w;
        pd->mask.bound.h = mbound.h;
        pd->mask.vg_pd = obj;
     }
   else
     {
        if (pd->mask.pixels)
          memset(pd->mask.pixels, 0x0, sizeof(uint32_t) * mbound.w * mbound.h);
     }

   pd->mask.bound.x = mbound.x;
   pd->mask.bound.y = mbound.y;

   if (!pd->mask.buffer) ERR("Mask Buffer is invalid");

   pd->mask.dirty = EINA_TRUE;

   //3. Prepare Drawing shapes...
   _evas_vg_render_pre(obj, mask_obj, surface, ptransform, mask, mask_op);

   return pd->mask.buffer;
}

static void
_efl_canvas_vg_container_render_pre(Evas_Object_Protected_Data *vg_pd,
                                    Efl_VG *obj EINA_UNUSED,
                                    Efl_Canvas_Vg_Node_Data *nd,
                                    Ector_Surface *surface,
                                    Eina_Matrix3 *ptransform,
                                    Ector_Buffer *mask,
                                    int mask_op,
                                    void *data)
{
   Efl_Canvas_Vg_Container_Data *pd = data;
   Eina_List *l;
   Efl_VG *child;
   Efl_Gfx_Change_Flag flag;

   if (nd->flags == EFL_GFX_CHANGE_FLAG_NONE) return;

   flag = nd->flags;
   nd->flags = EFL_GFX_CHANGE_FLAG_NONE;

   EFL_CANVAS_VG_COMPUTE_MATRIX(ctransform, ptransform, nd);

   //Container may have mask source.
   if (pd->mask_src)
     {
        mask = _prepare_mask(vg_pd, pd->mask_src, surface, ptransform, mask,
                             mask_op);
        mask_op = pd->mask.option;
     }

   EINA_LIST_FOREACH(pd->children, l, child)
     {
        //Don't need to update mask nodes.
        if (efl_isa(child, MY_CLASS))
          {
             Efl_Canvas_Vg_Container_Data *child_cd =
                efl_data_scope_get(child, MY_CLASS);
             if (child_cd->mask.target) continue;
          }

        //Skip Gradients. they will be updated by Shape.
        if (efl_isa(child, EFL_CANVAS_VG_GRADIENT_CLASS))
          continue;

        Efl_Canvas_Vg_Node_Data *child_nd =
           efl_data_scope_get(child, EFL_CANVAS_VG_NODE_CLASS);

        if (flag & EFL_GFX_CHANGE_FLAG_MATRIX)
          child_nd->flags |= EFL_GFX_CHANGE_FLAG_MATRIX;

        _evas_vg_render_pre(vg_pd, child, surface, ctransform, mask, mask_op);
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
                                               Efl_Canvas_Vg_Container_Data *pd)
{
   //Destroy mask surface
   if (pd->mask.buffer) efl_unref(pd->mask.buffer);
   if (pd->mask.pixels) free(pd->mask.pixels);

   efl_unref(pd->mask_src);
   eina_list_free(pd->mask.target);
   eina_hash_free(pd->names);

   efl_destructor(efl_super(obj, MY_CLASS));
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

   //Interpolates Mask
   Efl_Canvas_Vg_Container_Data *fromd = efl_data_scope_get(from, MY_CLASS);
   Efl_Canvas_Vg_Container_Data *tod = efl_data_scope_get(to, MY_CLASS);

   if (fromd->mask_src && tod->mask_src && pd->mask_src)
     {
        if (!efl_gfx_path_interpolate(pd->mask_src,
                                      fromd->mask_src, tod->mask_src, pos_map))
          return EINA_FALSE;
     }

   eina_iterator_free(from_it);
   eina_iterator_free(to_it);

   return r;
}

static void
_efl_canvas_vg_container_efl_canvas_vg_node_mask_set(Eo *obj,
                                                     Efl_Canvas_Vg_Container_Data *pd,
                                                     Efl_Canvas_Vg_Node *mask,
                                                     int op)
{
   if (pd->mask_src == mask) return;

   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(mask, MY_CLASS));

   if (pd->mask_src)
     {
        Efl_Canvas_Vg_Container_Data *pd2 =
              efl_data_scope_get(pd->mask_src, MY_CLASS);
        pd2->mask.target = eina_list_remove(pd2->mask.target, obj);
     }

   if (mask)
     {
        Efl_Canvas_Vg_Container_Data *pd2 = efl_data_scope_get(mask, MY_CLASS);
        pd2->mask.target = eina_list_append(pd2->mask.target, obj);
     }

   pd->mask.option = op;
   efl_replace(&pd->mask_src, mask);
   efl_canvas_vg_node_change(obj);
}

static void
_efl_canvas_vg_container_efl_object_parent_set(Eo *obj,
                                               Efl_Canvas_Vg_Container_Data *cd EINA_UNUSED,
                                               Eo *parent)
{
   efl_parent_set(efl_super(obj, MY_CLASS), parent);

   Efl_Canvas_Vg_Node_Data *nd = efl_data_scope_get(obj, EFL_CANVAS_VG_NODE_CLASS);
   efl_canvas_vg_container_vg_obj_update(obj, nd);
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

   //Copy Mask
   if (pd->mask_src)
     {
        Eo * mask_src = efl_duplicate(pd->mask_src);
        efl_parent_set(mask_src, container);
        efl_canvas_vg_node_mask_set(container, mask_src, pd->mask.option);
     }

   //Copy Children
   EINA_LIST_FOREACH(pd->children, l, child)
     {
        Efl_VG *eo = efl_duplicate(child);
        efl_parent_set(eo, container);
     }

   return container;
}

void
efl_canvas_vg_container_vg_obj_update(Efl_VG *obj, Efl_Canvas_Vg_Node_Data *nd)
{
   if (!obj) return;

   Efl_Canvas_Vg_Container_Data *cd = efl_data_scope_get(obj, MY_CLASS);
   if (!cd) return;

   Eina_List *l;
   Efl_VG* child;

   EINA_LIST_FOREACH(cd->children, l, child)
     {
        Efl_Canvas_Vg_Node_Data *child_nd =
           efl_data_scope_get(child, EFL_CANVAS_VG_NODE_CLASS);

        if (child_nd->vg_obj == nd->vg_obj) continue;

        child_nd->vg_obj = nd->vg_obj;
        child_nd->vd = nd->vd;

        if (efl_isa(child, MY_CLASS))
          efl_canvas_vg_container_vg_obj_update(child, child_nd);
     }
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

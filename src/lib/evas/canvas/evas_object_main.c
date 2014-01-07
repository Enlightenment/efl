#include "evas_common_private.h"
#include "evas_private.h"

EVAS_MEMPOOL(_mp_sh);

#include "Eo.h"

#define MY_CLASS EVAS_OBJ_CLASS

#define MY_CLASS_NAME "Evas_Object"

static void
_show(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj);
static void
_hide(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj);

static Eina_Inlist *
get_layer_objects(Evas_Layer *l)
{
   if ((!l) || (!l->objects)) return NULL;
   return (EINA_INLIST_GET(l->objects));
}

/* evas internal stuff */
static const Evas_Object_Proxy_Data default_proxy = {
  NULL, NULL, NULL, 0, 0, NULL, 0, 0, 0, 0
};
static const Evas_Object_Map_Data default_map = {
  { NULL, NULL, 0, 0 }, { NULL, NULL, 0, 0 }, NULL, 0, 0, NULL, NULL
};
static const Evas_Object_Protected_State default_state = {
  NULL, { 0, 0, 0, 0 },
  { { 0, 0, 0, 0,  0, 0, 0, 0, EINA_FALSE, EINA_FALSE } },
  { 255, 255, 255, 255 },
  1.0, EVAS_RENDER_BLEND, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE
};

Eina_Cow *evas_object_proxy_cow = NULL;
Eina_Cow *evas_object_map_cow = NULL;
Eina_Cow *evas_object_state_cow = NULL;

Eina_Cow *evas_object_3d_cow = NULL;

static Eina_Bool
_init_cow(void)
{
   if (evas_object_map_cow && evas_object_proxy_cow && evas_object_state_cow && evas_object_3d_cow) return EINA_TRUE;

   evas_object_proxy_cow = eina_cow_add("Evas Object Proxy", sizeof (Evas_Object_Proxy_Data), 8, &default_proxy, EINA_TRUE);
   evas_object_map_cow = eina_cow_add("Evas Object Map", sizeof (Evas_Object_Map_Data), 8, &default_map, EINA_TRUE);
   evas_object_state_cow = eina_cow_add("Evas Object State", sizeof (Evas_Object_Protected_State), 64, &default_state, EINA_FALSE);

   evas_object_3d_cow = eina_cow_add("Evas Object 3D", sizeof (Evas_Object_3D_Data), 8, &default_proxy, EINA_TRUE);

   if (!(evas_object_map_cow && evas_object_proxy_cow && evas_object_state_cow && evas_object_3d_cow))
     {
        eina_cow_del(evas_object_proxy_cow);
        eina_cow_del(evas_object_map_cow);
        eina_cow_del(evas_object_state_cow);
        evas_object_proxy_cow = NULL;
        evas_object_map_cow = NULL;
        evas_object_state_cow = NULL;

        eina_cow_del(evas_object_3d_cow);
        evas_object_3d_cow = NULL;

        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static void
_evas_object_eo_base_constructor(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   eo_do_super(eo_obj, MY_CLASS, eo_constructor());
   eo_do(eo_obj, evas_obj_type_set(MY_CLASS_NAME));
   eo_manual_free_set(eo_obj, EINA_TRUE);

   if (!obj || !_init_cow())
     {
        eo_error_set(eo_obj);
        return;
     }

   obj->is_frame = EINA_FALSE;
   obj->object = eo_obj;
   obj->proxy = eina_cow_alloc(evas_object_proxy_cow);
   obj->map = eina_cow_alloc(evas_object_map_cow);
   obj->cur = eina_cow_alloc(evas_object_state_cow);
   obj->prev = eina_cow_alloc(evas_object_state_cow);
   obj->data_3d = eina_cow_alloc(evas_object_3d_cow);
}

void
evas_object_change_reset(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!obj) return;
   obj->changed = EINA_FALSE;
   obj->changed_move = EINA_FALSE;
   obj->changed_color = EINA_FALSE;
   obj->changed_pchange = EINA_FALSE;
   obj->changed_src_visible = EINA_FALSE;
}

void
evas_object_cur_prev(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!obj) return;
   if (!obj->map->prev.valid_map && obj->map->prev.map)
     {
        EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
          {
             if (map_write->prev.map != map_write->cur.map)
               evas_map_free(map_write->prev.map);
             if (map_write->cache_map == map_write->prev.map)
               map_write->cache_map = NULL;
             map_write->prev.map = NULL;
          }
        EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
     }

   if (obj->map->cur.map != obj->map->prev.map)
     {
        EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
          {
             if (map_write->cache_map) evas_map_free(map_write->cache_map);
             map_write->cache_map = map_write->prev.map;
          }
        EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
     }
   if (memcmp(&obj->map->prev, &obj->map->cur, sizeof (obj->map->cur)))
     {
        EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
          map_write->prev = map_write->cur;
        EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
     }
   eina_cow_memcpy(evas_object_state_cow, (const Eina_Cow_Data **) &obj->prev, obj->cur);
}

void
evas_object_free(Evas_Object *eo_obj, int clean_layer)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!obj) return;
   obj->clean_layer = clean_layer;

   int was_smart_child = 0;

   _evas_object_image_free(eo_obj);
   evas_object_map_set(eo_obj, NULL);
   if (obj->map->prev.map) evas_map_free(obj->map->prev.map);
   if (obj->map->cache_map) evas_map_free(obj->map->cache_map);
   if (obj->map->surface)
     {
        if (obj->layer)
          {
             obj->layer->evas->engine.func->image_map_surface_free
               (obj->layer->evas->engine.data.output,
                   obj->map->surface);
          }
        EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
          map_write->surface = NULL;
        EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
     }
   evas_object_grabs_cleanup(eo_obj, obj);
   evas_object_intercept_cleanup(eo_obj);
   if (obj->smart.parent) was_smart_child = 1;
   evas_object_smart_cleanup(eo_obj);
   if (obj->func->free)
     {
        obj->func->free(eo_obj, obj, obj->private_data);
     }
   if (!was_smart_child) evas_object_release(eo_obj, obj, obj->clean_layer);
   if (obj->clip.clipees)
     obj->clip.clipees = eina_list_free(obj->clip.clipees);
   obj->clip.cache_clipees_answer = eina_list_free(obj->clip.cache_clipees_answer);
   evas_object_clip_changes_clean(eo_obj);
   evas_object_event_callback_all_del(eo_obj);
   evas_object_event_callback_cleanup(eo_obj);
   if (obj->map->spans)
     {
        EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
          {
             free(map_write->spans);
             map_write->spans = NULL;
          }
        EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
     }
   if (obj->size_hints)
     {
       EVAS_MEMPOOL_FREE(_mp_sh, obj->size_hints);
     }
   eina_cow_free(evas_object_proxy_cow, (const Eina_Cow_Data**) &obj->proxy);
   eina_cow_free(evas_object_map_cow, (const Eina_Cow_Data**) &obj->map);
   eina_cow_free(evas_object_state_cow, (const Eina_Cow_Data**) &obj->cur);
   eina_cow_free(evas_object_state_cow, (const Eina_Cow_Data**) &obj->prev);
   eina_cow_free(evas_object_3d_cow, (const Eina_Cow_Data**) &obj->data_3d);
   eo_data_unref(eo_obj, obj->private_data);
   obj->private_data = NULL;

   /* Try to manual free, and if it fails, unset it so the next unref will
    * actually free the object. */
   if (!eo_manual_free(eo_obj))
      eo_manual_free_set(eo_obj, EINA_FALSE);
}

void
evas_object_change(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Eina_List *l;
   Evas_Object_Protected_Data *obj2;
   Evas_Object *eo_obj2;
   Eina_Bool movch = EINA_FALSE;
   Evas_3D_Texture *texture;

   if (!obj->layer) return;
   if (obj->layer->evas->nochange) return;
   obj->layer->evas->changed = EINA_TRUE;

   if (obj->changed_move)
     {
        movch = EINA_TRUE;
        obj->changed_move = EINA_FALSE;
     }

   if (obj->changed) return;

   evas_render_object_recalc(eo_obj);
   /* set changed flag on all objects this one clips too */
   if (!((movch) && (obj->is_static_clip)))
     {
        EINA_LIST_FOREACH(obj->clip.clipees, l, obj2)
          {
             evas_object_change(obj2->object, obj2);
          }
     }
   EINA_LIST_FOREACH(obj->proxy->proxies, l, eo_obj2)
     {
        obj2 = eo_data_scope_get(eo_obj2, MY_CLASS);

        if (!obj2) continue;
        evas_object_change(eo_obj2, obj2);
     }
   EINA_LIST_FOREACH(obj->proxy->proxy_textures, l, texture)
     {
        evas_3d_object_change(&texture->base, EVAS_3D_STATE_TEXTURE_DATA, NULL);
     }
   if (obj->smart.parent)
     {
        Evas_Object_Protected_Data *smart_parent = eo_data_scope_get(obj->smart.parent, MY_CLASS);
        if (!smart_parent) return;
        evas_object_change(obj->smart.parent, smart_parent);
     }
}

void
evas_object_content_change(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if ((obj->map) && (obj->map->surface))
     {
        EINA_COW_WRITE_BEGIN(evas_object_map_cow,
                             obj->map, Evas_Object_Map_Data, map_write)
          {
             obj->layer->evas->engine.func->image_map_surface_free
               (obj->layer->evas->engine.data.output, map_write->surface);
             map_write->surface = NULL;
          }
        EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
     }
}

void
evas_object_render_pre_visible_change(Eina_Array *rects, Evas_Object *eo_obj, int is_v, int was_v)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, MY_CLASS);

   if (!obj) return;
   if (obj->is_smart) return;
   if (is_v == was_v) return;
   if (is_v)
     {
        evas_add_rect(rects,
                      obj->cur->cache.clip.x,
                      obj->cur->cache.clip.y,
                      obj->cur->cache.clip.w,
                      obj->cur->cache.clip.h);
     }
   else
     {
        evas_add_rect(rects,
                      obj->prev->cache.clip.x,
                      obj->prev->cache.clip.y,
                      obj->prev->cache.clip.w,
                      obj->prev->cache.clip.h);
     }
}

void
evas_object_render_pre_clipper_change(Eina_Array *rects, Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, MY_CLASS);

   if (!obj) return;
   if (obj->is_smart) return;
   if (obj->cur->clipper == obj->prev->clipper) return;
   if ((obj->cur->clipper) && (obj->prev->clipper))
     {
        /* get difference rects between clippers */
        evas_rects_return_difference_rects(rects,
                                           obj->cur->clipper->cur->cache.clip.x,
                                           obj->cur->clipper->cur->cache.clip.y,
                                           obj->cur->clipper->cur->cache.clip.w,
                                           obj->cur->clipper->cur->cache.clip.h,
                                           obj->prev->clipper->prev->cache.clip.x,
                                           obj->prev->clipper->prev->cache.clip.y,
                                           obj->prev->clipper->prev->cache.clip.w,
                                           obj->prev->clipper->prev->cache.clip.h);
     }
   else if (obj->cur->clipper)
     {
        evas_rects_return_difference_rects(rects,
                                           obj->cur->geometry.x,
                                           obj->cur->geometry.y,
                                           obj->cur->geometry.w,
                                           obj->cur->geometry.h,
////	rl = evas_rects_return_difference_rects(obj->cur->cache.geometry.x,
////						obj->cur->cache.geometry.y,
////						obj->cur->cache.geometry.w,
////						obj->cur->cache.geometry.h,
                                           obj->cur->clipper->cur->cache.clip.x,
                                           obj->cur->clipper->cur->cache.clip.y,
                                           obj->cur->clipper->cur->cache.clip.w,
                                           obj->cur->clipper->cur->cache.clip.h);
     }
   else if (obj->prev->clipper)
     {
     evas_rects_return_difference_rects(rects,
                                        obj->prev->geometry.x,
                                        obj->prev->geometry.y,
                                        obj->prev->geometry.w,
                                        obj->prev->geometry.h,
////	rl = evas_rects_return_difference_rects(obj->prev->cache.geometry.x,
////						obj->prev->cache.geometry.y,
////						obj->prev->cache.geometry.w,
////						obj->prev->cache.geometry.h,
                                        obj->prev->clipper->prev->cache.clip.x,
                                        obj->prev->clipper->prev->cache.clip.y,
                                        obj->prev->clipper->prev->cache.clip.w,
                                        obj->prev->clipper->prev->cache.clip.h);
     }
}

void
evas_object_render_pre_prev_cur_add(Eina_Array *rects, Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (!obj) return;

   evas_add_rect(rects,
                 obj->cur->cache.clip.x,
                 obj->cur->cache.clip.y,
                 obj->cur->cache.clip.w,
                 obj->cur->cache.clip.h);
   evas_add_rect(rects,
                 obj->prev->cache.clip.x,
                 obj->prev->cache.clip.y,
                 obj->prev->cache.clip.w,
                 obj->prev->cache.clip.h);
/*
        evas_add_rect(rects,
                      obj->cur->geometry.x,
                      obj->cur->geometry.y,
                      obj->cur->geometry.w,
                      obj->cur->geometry.h);
////	    obj->cur->cache.geometry.x,
////	    obj->cur->cache.geometry.y,
////	    obj->cur->cache.geometry.w,
////	    obj->cur->cache.geometry.h);
        evas_add_rect(rects,
                      obj->prev->geometry.x,
                      obj->prev->geometry.y,
                      obj->prev->geometry.w,
                      obj->prev->geometry.h);
////	    obj->prev->cache.geometry.x,
////	    obj->prev->cache.geometry.y,
////	    obj->prev->cache.geometry.w,
////	    obj->prev->cache.geometry.h);
*/
}

void
evas_object_clip_changes_clean(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!obj) return;

   Eina_Rectangle *r;

   EINA_LIST_FREE(obj->clip.changes, r)
     eina_rectangle_free(r);
}

void
evas_object_render_pre_effect_updates(Eina_Array *rects, Evas_Object *eo_obj, int is_v, int was_v EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, MY_CLASS);
   Eina_Rectangle *r;
   Eina_List *l;
   unsigned int i;
   Eina_Array_Iterator it;
   int x, y, w, h;

   if (!obj) return;
   if (!obj->layer) return;

   if (obj->is_smart) goto end;
   /* FIXME: was_v isn't used... why? */
   if (!obj->clip.clipees)
     {
        Evas_Public_Data *e;
        e = obj->layer->evas;
        EINA_ARRAY_ITER_NEXT(rects, i, r, it)
          {
             /* get updates and clip to current clip */
             x = r->x;
             y = r->y;
             w = r->w;
             h = r->h;
             RECTS_CLIP_TO_RECT(x, y, w, h,
                                obj->cur->cache.clip.x,
                                obj->cur->cache.clip.y,
                                obj->cur->cache.clip.w,
                                obj->cur->cache.clip.h);
             if ((w > 0) && (h > 0))
               obj->layer->evas->engine.func->output_redraws_rect_add(e->engine.data.output,
                                                                      x + e->framespace.x,
                                                                      y + e->framespace.y,
                                                                      w, h);
             /* get updates and clip to previous clip */
             x = r->x;
             y = r->y;
             w = r->w;
             h = r->h;
             RECTS_CLIP_TO_RECT(x, y, w, h,
                                obj->prev->cache.clip.x,
                                obj->prev->cache.clip.y,
                                obj->prev->cache.clip.w,
                                obj->prev->cache.clip.h);
             if ((w > 0) && (h > 0))
               obj->layer->evas->engine.func->output_redraws_rect_add(e->engine.data.output,
                                                                      x + e->framespace.x,
                                                                      y + e->framespace.y,
                                                                      w, h);
          }
        /* if the object is actually visible, take any parent clip changes */
        if (is_v)
          {
             Evas_Object_Protected_Data *clipper;

             clipper = obj->cur->clipper;
             while (clipper)
               {
                  EINA_LIST_FOREACH(clipper->clip.changes, l, r)
                    {
                       /* get updates and clip to current clip */
                       x = r->x; y = r->y; w = r->w; h = r->h;
                       RECTS_CLIP_TO_RECT(x, y, w, h,
                                          obj->cur->cache.clip.x,
                                          obj->cur->cache.clip.y,
                                          obj->cur->cache.clip.w,
                                          obj->cur->cache.clip.h);
                       if ((w > 0) && (h > 0))
                         obj->layer->evas->engine.func->output_redraws_rect_add(e->engine.data.output,
                                                                                x + e->framespace.x,
                                                                                y + e->framespace.y,
                                                                                w, h);
                       /* get updates and clip to previous clip */
                       x = r->x; y = r->y; w = r->w; h = r->h;
                       RECTS_CLIP_TO_RECT(x, y, w, h,
                                          obj->prev->cache.clip.x,
                                          obj->prev->cache.clip.y,
                                          obj->prev->cache.clip.w,
                                          obj->prev->cache.clip.h);
                       if ((w > 0) && (h > 0))
                         obj->layer->evas->engine.func->output_redraws_rect_add(e->engine.data.output,
                                                                                x + e->framespace.x,
                                                                                y + e->framespace.y,
                                                                                w, h);
                    }
                  clipper = clipper->cur->clipper;
               }
          }
     }
   else
     {
        evas_object_clip_changes_clean(eo_obj);
        EINA_ARRAY_ITER_NEXT(rects, i, r, it)
           obj->clip.changes = eina_list_append(obj->clip.changes, r);
        eina_array_clean(rects);
     }

 end:
   EINA_ARRAY_ITER_NEXT(rects, i, r, it)
     eina_rectangle_free(r);
   eina_array_clean(rects);
}

int
evas_object_was_in_output_rect(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, int x, int y, int w, int h)
{
   if (obj->is_smart && !obj->map->prev.map && !obj->map->prev.usemap) return 0;
   /* assumes coords have been recalced */
   if ((RECTS_INTERSECT(x, y, w, h,
                        obj->prev->cache.clip.x,
                        obj->prev->cache.clip.y,
                        obj->prev->cache.clip.w,
                        obj->prev->cache.clip.h)))
     return 1;
   return 0;
}

int
evas_object_was_opaque(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   if (obj->is_smart) return 0;
   if (obj->prev->cache.clip.a == 255)
     {
        if (obj->func->was_opaque)
          return obj->func->was_opaque(eo_obj, obj, obj->private_data);
        return 1;
     }
   return 0;
}

int
evas_object_is_inside(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord x, Evas_Coord y)
{
   if (obj->is_smart) return 0;
   if (obj->func->is_inside)
     return obj->func->is_inside(eo_obj, obj, obj->private_data, x, y);
   return 0;
}

int
evas_object_was_inside(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord x, Evas_Coord y)
{
   if (obj->is_smart) return 0;
   if (obj->func->was_inside)
     return obj->func->was_inside(eo_obj, obj, obj->private_data, x, y);
   return 0;
}
/* routines apps will call */

EAPI void
evas_object_ref(Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   eo_ref(eo_obj);

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!obj) return;
   obj->ref++;
}

EAPI void
evas_object_unref(Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!obj) return;

   if (obj->ref == 0) return;
   obj->ref--;
   eo_unref(eo_obj);
   if ((obj->del_ref) && (obj->ref == 0)) evas_object_del(eo_obj);

}

EAPI int
evas_object_ref_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!obj) return 0;
   return obj->ref;
}

EAPI void
evas_object_del(Evas_Object *eo_obj)
{
   if (!eo_obj) return;
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, MY_CLASS);

   if (!obj) return;
   if (obj->delete_me || obj->eo_del_called) return;
   if (obj->ref > 0)
     {
        obj->del_ref = EINA_TRUE;
        return;
     }

   obj->eo_del_called = EINA_TRUE;

   eo_do(eo_obj, eo_parent_set(NULL));
//   eo_del(eo_obj);
}

EOLIAN static void
_evas_object_eo_base_destructor(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object *proxy;
   Eina_List *l, *l2;

   evas_object_hide(eo_obj);
   if (obj->focused)
     {
        obj->focused = EINA_FALSE;
        if (obj->layer)
          obj->layer->evas->focused = NULL;
        _evas_object_event_new();
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_FOCUS_OUT, NULL, _evas_event_counter);
        if (obj->layer)
          _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
     }
   _evas_object_event_new();
   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_DEL, NULL, _evas_event_counter);
   if (obj->layer)
     _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
   if ((obj->mouse_grabbed > 0) && (obj->layer))
      obj->layer->evas->pointer.mouse_grabbed -= obj->mouse_grabbed;
   if (((obj->mouse_in) || (obj->mouse_grabbed > 0)) && (obj->layer))
      obj->layer->evas->pointer.object.in = eina_list_remove(obj->layer->evas->pointer.object.in, eo_obj);
   obj->mouse_grabbed = 0;
   obj->mouse_in = 0;
   if (obj->name) evas_object_name_set(eo_obj, NULL);
   if (!obj->layer)
     {
        eo_manual_free_set(eo_obj, EINA_FALSE);
        obj->clean_layer = 1;
        goto end;
     }
   evas_object_grabs_cleanup(eo_obj, obj);
   /* "while" should be used for null check of obj->clip.clipees,
      because evas_objct_clip_unset can set null to obj->clip.clipees */
   while (obj->clip.clipees)
     {
        Evas_Object_Protected_Data *tmp;
        tmp = eina_list_data_get(obj->clip.clipees);
        evas_object_clip_unset(tmp->object);
     }
   EINA_LIST_FOREACH_SAFE(obj->proxy->proxies, l, l2, proxy)
     {
        if (eo_isa(proxy, EVAS_OBJ_IMAGE_CLASS))
          evas_object_image_source_unset(proxy);
        else if (eo_isa(proxy, EVAS_OBJ_TEXT_CLASS))
          eo_do(proxy, evas_obj_text_filter_source_set(NULL, eo_obj));
     }

   while (obj->proxy->proxy_textures)
     evas_3d_texture_source_set(obj->proxy->proxy_textures->data, NULL);

   if (obj->cur->clipper) evas_object_clip_unset(eo_obj);
   evas_object_map_set(eo_obj, NULL);
   if (obj->is_smart) evas_object_smart_del(eo_obj);
   _evas_object_event_new();
   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_FREE, NULL, _evas_event_counter);
   if (obj->layer)
     _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
   evas_object_smart_cleanup(eo_obj);
   obj->delete_me = 1;
   evas_object_change(eo_obj, obj);
end:
   evas_object_event_callback_all_del(eo_obj);
   evas_object_event_callback_cleanup(eo_obj);

   eo_do_super(eo_obj, MY_CLASS, eo_destructor());
}

EAPI void
evas_object_geometry_set(Evas_Object *eo_obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj,
         evas_obj_position_set(x, y),
         evas_obj_size_set(w, h));
}

EOLIAN static void
_evas_object_position_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord x, Evas_Coord y)
{

   Eina_Bool is, was = EINA_FALSE;
   Eina_Bool pass = EINA_FALSE, freeze = EINA_FALSE;
   Eina_Bool source_invisible = EINA_FALSE;

   if (obj->delete_me) return;
   if (!obj->layer) return;

   if (evas_object_intercept_call_move(eo_obj, obj, x, y)) return;

   if (obj->doing.in_move > 0)
     {
        WRN("evas_object_move() called on object %p when in the middle of moving the same object", obj);
        return;
     }

   if ((obj->cur->geometry.x == x) && (obj->cur->geometry.y == y)) return;

   if (!(obj->layer->evas->is_frozen))
     {
        pass = evas_event_passes_through(eo_obj, obj);
        freeze = evas_event_freezes_through(eo_obj, obj);
        source_invisible = evas_object_is_source_invisible(eo_obj, obj);
        if ((!pass) && (!freeze) && (!source_invisible))
          was = evas_object_is_in_output_rect(eo_obj, obj,
                                              obj->layer->evas->pointer.x,
                                              obj->layer->evas->pointer.y, 1, 1);
     }
   obj->doing.in_move++;

   if (obj->is_smart)
     {
        eo_do(eo_obj, evas_obj_smart_move(x, y));
     }

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->geometry.x = x;
        state_write->geometry.y = y;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_update_bounding_box(eo_obj, obj);

////   obj->cur->cache.geometry.validity = 0;
   obj->changed_move = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   obj->doing.in_move--;
   if (!(obj->layer->evas->is_frozen))
     {
        evas_object_recalc_clippees(obj);
        if (!pass)
          {
             if (!obj->is_smart)
               {
                  is = evas_object_is_in_output_rect(eo_obj, obj,
                                                     obj->layer->evas->pointer.x,
                                                     obj->layer->evas->pointer.y, 1, 1);
                  if ((is ^ was) && obj->cur->visible)
                    evas_event_feed_mouse_move(obj->layer->evas->evas,
                                               obj->layer->evas->pointer.x,
                                               obj->layer->evas->pointer.y,
                                               obj->layer->evas->last_timestamp,
                                               NULL);
               }
          }
     }
   evas_object_inform_call_move(eo_obj, obj);
}

EOLIAN static void
_evas_object_size_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord w, Evas_Coord h)
{
   Eina_Bool is, was = EINA_FALSE;
   Eina_Bool pass = EINA_FALSE, freeze = EINA_FALSE;
   Eina_Bool source_invisible = EINA_FALSE;

   if (obj->delete_me) return;
   if (!obj->layer) return;
   if (w < 0) w = 0; if (h < 0) h = 0;

   if (evas_object_intercept_call_resize(eo_obj, obj, w, h)) return;

   if (obj->doing.in_resize > 0)
     {
        WRN("evas_object_resize() called on object %p when in the middle of resizing the same object", obj);
        return;
     }

   if ((obj->cur->geometry.w == w) && (obj->cur->geometry.h == h)) return;

   if (!(obj->layer->evas->is_frozen))
     {
        pass = evas_event_passes_through(eo_obj, obj);
        freeze = evas_event_freezes_through(eo_obj, obj);
        source_invisible = evas_object_is_source_invisible(eo_obj, obj);
        if ((!pass) && (!freeze) && (!source_invisible))
          was = evas_object_is_in_output_rect(eo_obj, obj,
                                              obj->layer->evas->pointer.x,
                                              obj->layer->evas->pointer.y, 1, 1);
     }
   obj->doing.in_resize++;

   if (obj->is_smart)
     {
        eo_do(eo_obj, evas_obj_smart_resize(w, h));
     }

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->geometry.w = w;
        state_write->geometry.h = h;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_update_bounding_box(eo_obj, obj);

////   obj->cur->cache.geometry.validity = 0;
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   obj->doing.in_resize--;
   /* NB: evas_object_recalc_clippees was here previously ( < 08/07/2009) */
   if (!(obj->layer->evas->is_frozen))
     {
        /* NB: If this creates glitches on screen then move to above position */
        evas_object_recalc_clippees(obj);

        //   if (obj->func->coords_recalc) obj->func->coords_recalc(eo_obj);
        if (!pass)
          {
             if (!obj->is_smart)
               {
                  is = evas_object_is_in_output_rect(eo_obj, obj,
                                                     obj->layer->evas->pointer.x,
                                                     obj->layer->evas->pointer.y, 1, 1);
                  if ((is ^ was) && (obj->cur->visible))
                    evas_event_feed_mouse_move(obj->layer->evas->evas,
                                               obj->layer->evas->pointer.x,
                                               obj->layer->evas->pointer.y,
                                               obj->layer->evas->last_timestamp,
                                               NULL);
               }
          }
     }
   evas_object_inform_call_resize(eo_obj);
}

EAPI void
evas_object_geometry_get(const Evas_Object *eo_obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   if (x) *x = 0; if (y) *y = 0; if (w) *w = 0; if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   eo_do((Eo *)eo_obj, evas_obj_position_get(x, y), evas_obj_size_get(w, h));
}

EOLIAN static void
_evas_object_position_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Coord *x, Evas_Coord *y)
{
   if ((obj->delete_me) || (!obj->layer))
     {
        if (x) *x = 0; if (y) *y = 0;
        return;
     }

   if (x) *x = obj->cur->geometry.x;
   if (y) *y = obj->cur->geometry.y;
}

EOLIAN static void
_evas_object_size_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Coord *w, Evas_Coord *h)
{
   if (obj->delete_me)
     {
        if (w) *w = 0; if (h) *h = 0;
        return;
     }

   if (w) *w = obj->cur->geometry.w;
   if (h) *h = obj->cur->geometry.h;
}

static void
_evas_object_size_hint_alloc(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->size_hints) return;

   EVAS_MEMPOOL_INIT(_mp_sh, "evas_size_hints", Evas_Size_Hints, 32, );
   obj->size_hints = EVAS_MEMPOOL_ALLOC(_mp_sh, Evas_Size_Hints);
   if (!obj->size_hints) return;
   EVAS_MEMPOOL_PREP(_mp_sh, obj->size_hints, Evas_Size_Hints);
   obj->size_hints->max.w = -1;
   obj->size_hints->max.h = -1;
   obj->size_hints->align.x = 0.5;
   obj->size_hints->align.y = 0.5;
   obj->size_hints->dispmode = EVAS_DISPLAY_MODE_NONE;
}

EOLIAN static Evas_Display_Mode
_evas_object_size_hint_display_mode_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (!obj) return EVAS_DISPLAY_MODE_NONE;
   if ((!obj->size_hints) || obj->delete_me)
     return EVAS_DISPLAY_MODE_NONE;
   return obj->size_hints->dispmode;
}

EOLIAN static void
_evas_object_size_hint_display_mode_set(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Display_Mode dispmode)
{
   if (!obj) return;
   if (obj->delete_me) return;
   _evas_object_size_hint_alloc(eo_obj, obj);
   if (obj->size_hints->dispmode == dispmode) return;
   obj->size_hints->dispmode = dispmode;

   evas_object_inform_call_changed_size_hints(eo_obj);
}

EOLIAN static void
_evas_object_size_hint_min_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Coord *w, Evas_Coord *h)
{
   if ((!obj->size_hints) || obj->delete_me)
     {
        if (w) *w = 0; if (h) *h = 0;
        return;
     }
   if (w) *w = obj->size_hints->min.w;
   if (h) *h = obj->size_hints->min.h;
}

EOLIAN static void
_evas_object_size_hint_min_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord w, Evas_Coord h)
{
   if (obj->delete_me)
     return;
   _evas_object_size_hint_alloc(eo_obj, obj);
   if ((obj->size_hints->min.w == w) && (obj->size_hints->min.h == h)) return;
   obj->size_hints->min.w = w;
   obj->size_hints->min.h = h;

   evas_object_inform_call_changed_size_hints(eo_obj);
}

EOLIAN static void
_evas_object_size_hint_max_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Coord *w, Evas_Coord *h)
{
   if ((!obj->size_hints) || obj->delete_me)
     {
        if (w) *w = -1; if (h) *h = -1;
        return;
     }
   if (w) *w = obj->size_hints->max.w;
   if (h) *h = obj->size_hints->max.h;
}

EOLIAN static void
_evas_object_size_hint_max_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord w, Evas_Coord h)
{
   if (obj->delete_me)
     return;
   _evas_object_size_hint_alloc(eo_obj, obj);
   if ((obj->size_hints->max.w == w) && (obj->size_hints->max.h == h)) return;
   obj->size_hints->max.w = w;
   obj->size_hints->max.h = h;

   evas_object_inform_call_changed_size_hints(eo_obj);
}

EOLIAN static void
_evas_object_size_hint_request_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Coord *w, Evas_Coord *h)
{
   if ((!obj->size_hints) || obj->delete_me)
     {
        if (w) *w = 0; if (h) *h = 0;
        return;
     }
   if (w) *w = obj->size_hints->request.w;
   if (h) *h = obj->size_hints->request.h;
}

EOLIAN static void
_evas_object_size_hint_request_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord w, Evas_Coord h)
{
   if (obj->delete_me)
     return;
   _evas_object_size_hint_alloc(eo_obj, obj);
   if ((obj->size_hints->request.w == w) && (obj->size_hints->request.h == h)) return;
   obj->size_hints->request.w = w;
   obj->size_hints->request.h = h;

   evas_object_inform_call_changed_size_hints(eo_obj);
}

EOLIAN static void
_evas_object_size_hint_aspect_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Aspect_Control *aspect, Evas_Coord *w, Evas_Coord *h)
{
   if ((!obj->size_hints) || obj->delete_me)
     {
        if (aspect) *aspect = EVAS_ASPECT_CONTROL_NONE;
        if (w) *w = 0; if (h) *h = 0;
        return;
     }
   if (aspect) *aspect = obj->size_hints->aspect.mode;
   if (w) *w = obj->size_hints->aspect.size.w;
   if (h) *h = obj->size_hints->aspect.size.h;
}

EOLIAN static void
_evas_object_size_hint_aspect_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Aspect_Control aspect, Evas_Coord w, Evas_Coord h)
{
   if (obj->delete_me)
     return;
   _evas_object_size_hint_alloc(eo_obj, obj);
   if ((obj->size_hints->aspect.mode == aspect) && (obj->size_hints->aspect.size.w == w) && (obj->size_hints->aspect.size.h == h)) return;
   obj->size_hints->aspect.mode = aspect;
   obj->size_hints->aspect.size.w = w;
   obj->size_hints->aspect.size.h = h;

   evas_object_inform_call_changed_size_hints(eo_obj);
}

EOLIAN static void
_evas_object_size_hint_align_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, double *x, double *y)
{
   if ((!obj->size_hints) || obj->delete_me)
     {
        if (x) *x = 0.5; if (y) *y = 0.5;
        return;
     }
   if (x) *x = obj->size_hints->align.x;
   if (y) *y = obj->size_hints->align.y;
}

EOLIAN static void
_evas_object_size_hint_align_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, double x, double y)
{
   if (obj->delete_me)
     return;
   _evas_object_size_hint_alloc(eo_obj, obj);
   if ((obj->size_hints->align.x == x) && (obj->size_hints->align.y == y)) return;
   obj->size_hints->align.x = x;
   obj->size_hints->align.y = y;

   evas_object_inform_call_changed_size_hints(eo_obj);
}

EOLIAN static void
_evas_object_size_hint_weight_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, double *x, double *y)
{
   if ((!obj->size_hints) || obj->delete_me)
     {
        if (x) *x = 0.0; if (y) *y = 0.0;
        return;
     }
   if (x) *x = obj->size_hints->weight.x;
   if (y) *y = obj->size_hints->weight.y;
}

EOLIAN static void
_evas_object_size_hint_weight_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, double x, double y)
{
   if (obj->delete_me)
     return;
   _evas_object_size_hint_alloc(eo_obj, obj);
   if ((obj->size_hints->weight.x == x) && (obj->size_hints->weight.y == y)) return;
   obj->size_hints->weight.x = x;
   obj->size_hints->weight.y = y;

   evas_object_inform_call_changed_size_hints(eo_obj);
}

EOLIAN static void
_evas_object_size_hint_padding_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b)
{
   if ((!obj->size_hints) || obj->delete_me)
     {
        if (l) *l = 0; if (r) *r = 0;
        if (t) *t = 0; if (b) *b = 0;
        return;
     }
   if (l) *l = obj->size_hints->padding.l;
   if (r) *r = obj->size_hints->padding.r;
   if (t) *t = obj->size_hints->padding.t;
   if (b) *b = obj->size_hints->padding.b;
}

EOLIAN static void
_evas_object_size_hint_padding_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord l, Evas_Coord r, Evas_Coord t, Evas_Coord b)
{
   if (obj->delete_me)
     return;
   _evas_object_size_hint_alloc(eo_obj, obj);
   if ((obj->size_hints->padding.l == l) && (obj->size_hints->padding.r == r) && (obj->size_hints->padding.t == t) && (obj->size_hints->padding.b == b)) return;
   obj->size_hints->padding.l = l;
   obj->size_hints->padding.r = r;
   obj->size_hints->padding.t = t;
   obj->size_hints->padding.b = b;

   evas_object_inform_call_changed_size_hints(eo_obj);
}

EAPI void
evas_object_show(Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_visibility_set(EINA_TRUE));
}

EAPI void
evas_object_hide(Evas_Object *eo_obj)
{
   if (!eo_obj) return;
   eo_do(eo_obj, evas_obj_visibility_set(EINA_FALSE));
}

static void
_evas_object_visibility_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool visible)
{
   if (visible) _show(eo_obj, obj);
   else _hide(eo_obj, obj);
}

static void
_show(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   if (!obj->layer) return;
   if (obj->delete_me) return;
   if (evas_object_intercept_call_show(eo_obj, obj)) return;
   if (obj->is_smart)
     {
        eo_do(eo_obj, evas_obj_smart_show());
     }
   if (obj->cur->visible)
     {
        return;
     }
   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->visible = 1;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   if (!(obj->layer->evas->is_frozen))
     {
        evas_object_clip_across_clippees_check(eo_obj, obj);
        evas_object_recalc_clippees(obj);
        if ((!evas_event_passes_through(eo_obj, obj)) &&
            (!evas_event_freezes_through(eo_obj, obj)) &&
            (!evas_object_is_source_invisible(eo_obj, obj)))
          {
             if (!obj->is_smart)
               {
                  if (evas_object_is_in_output_rect(eo_obj, obj,
                                                    obj->layer->evas->pointer.x,
                                                    obj->layer->evas->pointer.y, 1, 1))
                    evas_event_feed_mouse_move(obj->layer->evas->evas,
                                               obj->layer->evas->pointer.x,
                                               obj->layer->evas->pointer.y,
                                               obj->layer->evas->last_timestamp,
                                               NULL);
               }
          }
     }
   evas_object_inform_call_show(eo_obj);
}

static void
_hide(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!obj->layer) return;
   if (obj->delete_me) return;
   if (evas_object_intercept_call_hide(eo_obj, obj)) return;
   if (obj->is_smart)
     {
        eo_do(eo_obj, evas_obj_smart_hide());
     }
   if (!obj->cur->visible)
     {
        return;
     }

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->visible = 0;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   if (!(obj->layer->evas->is_frozen))
     {
        evas_object_clip_across_clippees_check(eo_obj, obj);
        evas_object_recalc_clippees(obj);
        if ((!evas_event_passes_through(eo_obj, obj)) &&
            (!evas_event_freezes_through(eo_obj, obj)) &&
            (!evas_object_is_source_invisible(eo_obj, obj)))
          {
             if ((!obj->is_smart) ||
                 ((obj->map->cur.map) && (obj->map->cur.map->count == 4) && (obj->map->cur.usemap)))
               {
                  if (!obj->mouse_grabbed)
                    {
                       if (evas_object_is_in_output_rect(eo_obj, obj,
                                                         obj->layer->evas->pointer.x,
                                                         obj->layer->evas->pointer.y, 1, 1))
                          evas_event_feed_mouse_move(obj->layer->evas->evas,
                                                     obj->layer->evas->pointer.x,
                                                     obj->layer->evas->pointer.y,
                                                     obj->layer->evas->last_timestamp,
                                                     NULL);
                    }
/* this is at odds to handling events when an obj is moved out of the mouse
 * ore resized out or clipped out. if mouse is grabbed - regardless of
 * visibility, mouse move events should keep happening and mouse up.
 * for better or worse it's at least consistent.
                  if (obj->delete_me) return;
                  if (obj->mouse_grabbed > 0)
                    obj->layer->evas->pointer.mouse_grabbed -= obj->mouse_grabbed;
                  if ((obj->mouse_in) || (obj->mouse_grabbed > 0))
                    obj->layer->evas->pointer.object.in = eina_list_remove(obj->layer->evas->pointer.object.in, eo_obj);
                  obj->mouse_grabbed = 0;
                  if (obj->layer->evas->is_frozen)
                    {
                       obj->mouse_in = 0;
                       return;
                    }
                  if (obj->mouse_in)
                    {
                       Evas_Event_Mouse_Out ev;

                       _evas_object_event_new();

                       obj->mouse_in = 0;
                       ev.buttons = obj->layer->evas->pointer.button;
                       ev.output.x = obj->layer->evas->pointer.x;
                       ev.output.y = obj->layer->evas->pointer.y;
                       ev.canvas.x = obj->layer->evas->pointer.x;
                       ev.canvas.y = obj->layer->evas->pointer.y;
                       ev.data = NULL;
                       ev.modifiers = &(obj->layer->evas->modifiers);
                       ev.locks = &(obj->layer->evas->locks);
                       ev.timestamp = obj->layer->evas->last_timestamp;
                       ev.event_flags = EVAS_EVENT_FLAG_NONE;
                       evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, &ev);
                       _evas_post_event_callback_call(obj->layer->evas, obj->layer->evas);
                    }
 */
               }
          }
     }
   else
     {
/*
        if (obj->mouse_grabbed > 0)
          obj->layer->evas->pointer.mouse_grabbed -= obj->mouse_grabbed;
        if ((obj->mouse_in) || (obj->mouse_grabbed > 0))
          obj->layer->evas->pointer.object.in = eina_list_remove(obj->layer->evas->pointer.object.in, eo_obj);
        obj->mouse_grabbed = 0;
        obj->mouse_in = 0;
 */
     }
   evas_object_inform_call_hide(eo_obj);
}

static Eina_Bool
_evas_object_visibility_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->delete_me) return EINA_FALSE;
   return obj->cur->visible;
}

EOLIAN static void
_evas_object_color_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, int r, int g, int b, int a)
{
   if (obj->delete_me) return;
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

   if (evas_object_intercept_call_color_set(eo_obj, obj, r, g, b, a)) return;
   if (obj->is_smart)
     {
        eo_do(eo_obj, evas_obj_smart_color_set(r, g, b, a));
     }
   if ((obj->cur->color.r == r) &&
       (obj->cur->color.g == g) &&
       (obj->cur->color.b == b) &&
       (obj->cur->color.a == a)) return;

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->color.r = r;
        state_write->color.g = g;
        state_write->color.b = b;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_clip_dirty(eo_obj, obj);
   if ((obj->cur->color.a == 0) && (a == 0) && (obj->cur->render_op == EVAS_RENDER_BLEND)) return;

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->color.a = a;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   obj->changed_color = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_object_color_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, int *r, int *g, int *b, int *a)
{
   if (obj->delete_me)
     {
        if (r) *r = 0; if (g) *g = 0; if (b) *b = 0; if (a) *a = 0;
        return;
     }
   if (r) *r = obj->cur->color.r;
   if (g) *g = obj->cur->color.g;
   if (b) *b = obj->cur->color.b;
   if (a) *a = obj->cur->color.a;
}

EOLIAN static void
_evas_object_anti_alias_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool anti_alias)
{
   if (obj->delete_me) return;
   anti_alias = !!anti_alias;
   if (obj->cur->anti_alias == anti_alias)return;

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->anti_alias = anti_alias;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_change(eo_obj, obj);
}

EOLIAN static Eina_Bool
_evas_object_anti_alias_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->delete_me) return EINA_FALSE;
   return obj->cur->anti_alias;
}

EOLIAN static void
_evas_object_scale_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, double scale)
{
   if (obj->delete_me) return;
   if (obj->cur->scale == scale) return;

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->scale = scale;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_change(eo_obj, obj);
   if (obj->func->scale_update) obj->func->scale_update(eo_obj, obj, obj->private_data);
}

EOLIAN static double
_evas_object_scale_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->delete_me) return 1.0;
   return obj->cur->scale;
}

EOLIAN static void
_evas_object_render_op_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Render_Op render_op)
{
   if (obj->delete_me) return;
   if (obj->cur->render_op == render_op) return;

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->render_op = render_op;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_change(eo_obj, obj);
}

EOLIAN static Evas_Render_Op
_evas_object_render_op_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->delete_me) return EVAS_RENDER_BLEND;
   return obj->cur->render_op;
}

EOLIAN static void
_evas_object_eo_base_dbg_info_get(Eo *eo_obj, Evas_Object_Protected_Data *obj EINA_UNUSED, Eo_Dbg_Info *root)
{
   eo_do_super(eo_obj, MY_CLASS, eo_dbg_info_get(root));
   Eo_Dbg_Info *group = EO_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);
   Eo_Dbg_Info *node;
   const char *name;
   double dblw, dblh;
   double dblx, dbly;
   double scale;
   unsigned int m;
   int r, g, b, a;
   int w, h;
   int requestw, requesth;
   int minw, minh;
   int maxw, maxh;
   int x, y;
   short layer;
   Eina_Bool focus;
   Eina_Bool visible;
   Eina_Bool pass_event;
   Eina_Bool propagate_event;
   Eina_Bool repeat_event;
   Eina_Bool clipees_has;

   eo_do(eo_obj,
         visible = evas_obj_visibility_get(),
         layer = evas_obj_layer_get(),
         name = evas_obj_name_get(),
         evas_obj_position_get(&x, &y),
         evas_obj_size_get(&w, &h),
         scale = evas_obj_scale_get(),
         evas_obj_size_hint_min_get(&minw, &minh),
         evas_obj_size_hint_max_get(&maxw, &maxh),
         evas_obj_size_hint_request_get(&requestw, &requesth),
         evas_obj_size_hint_align_get(&dblx, &dbly),
         evas_obj_size_hint_weight_get(&dblw, &dblh),
         evas_obj_color_get(&r, &g, &b, &a),
         focus = evas_obj_focus_get(),
         m = evas_obj_pointer_mode_get(),
         pass_event = evas_obj_pass_events_get(),
         repeat_event = evas_obj_repeat_events_get(),
         propagate_event = evas_obj_propagate_events_get(),
         clipees_has = evas_obj_clipees_has());

   EO_DBG_INFO_APPEND(group, "Visibility", EINA_VALUE_TYPE_CHAR, visible);

   if (name)
      EO_DBG_INFO_APPEND(group, "Name", EINA_VALUE_TYPE_STRING, name);

   EO_DBG_INFO_APPEND(group, "Layer", EINA_VALUE_TYPE_INT, layer);

   node = EO_DBG_INFO_LIST_APPEND(group, "Position");
   EO_DBG_INFO_APPEND(node, "x", EINA_VALUE_TYPE_INT, x);
   EO_DBG_INFO_APPEND(node, "y", EINA_VALUE_TYPE_INT, y);

   node = EO_DBG_INFO_LIST_APPEND(group, "Size");
   EO_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_INT, w);
   EO_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_INT, h);

   EO_DBG_INFO_APPEND(group, "Scale", EINA_VALUE_TYPE_DOUBLE, scale);

   node = EO_DBG_INFO_LIST_APPEND(group, "Min size");
   EO_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_INT, minw);
   EO_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_INT, minh);

   node = EO_DBG_INFO_LIST_APPEND(group, "Max size");
   EO_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_INT, maxw);
   EO_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_INT, maxh);

   node = EO_DBG_INFO_LIST_APPEND(group, "Request size");
   EO_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_INT, requestw);
   EO_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_INT, requesth);

   node = EO_DBG_INFO_LIST_APPEND(group, "Align");
   EO_DBG_INFO_APPEND(node, "x", EINA_VALUE_TYPE_DOUBLE, dblx);
   EO_DBG_INFO_APPEND(node, "y", EINA_VALUE_TYPE_DOUBLE, dbly);

   node = EO_DBG_INFO_LIST_APPEND(group, "Weight");
   EO_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_DOUBLE, dblw);
   EO_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_DOUBLE, dblh);

   node = EO_DBG_INFO_LIST_APPEND(group, "Color");
   EO_DBG_INFO_APPEND(node, "r", EINA_VALUE_TYPE_INT, r);
   EO_DBG_INFO_APPEND(node, "g", EINA_VALUE_TYPE_INT, g);
   EO_DBG_INFO_APPEND(node, "b", EINA_VALUE_TYPE_INT, b);
   EO_DBG_INFO_APPEND(node, "a", EINA_VALUE_TYPE_INT, a);

   EO_DBG_INFO_APPEND(group, "Has focus", EINA_VALUE_TYPE_CHAR, focus);

     {
        const char *text = NULL;
        switch (m)
          {
           case EVAS_OBJECT_POINTER_MODE_AUTOGRAB:
              text = "EVAS_OBJECT_POINTER_MODE_AUTOGRAB";
              break;
           case EVAS_OBJECT_POINTER_MODE_NOGRAB:
              text = "EVAS_OBJECT_POINTER_MODE_NOGRAB";
              break;
           case EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN:
              text = "EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN";
              break;
           default:
              text = NULL;
              break;
          }

        if (text)
           EO_DBG_INFO_APPEND(group, "Pointer Mode", EINA_VALUE_TYPE_STRING, text);
     }

   EO_DBG_INFO_APPEND(group, "Pass Events", EINA_VALUE_TYPE_CHAR, pass_event);
   EO_DBG_INFO_APPEND(group, "Repeat Events", EINA_VALUE_TYPE_CHAR, repeat_event);
   EO_DBG_INFO_APPEND(group, "Propagate Events", EINA_VALUE_TYPE_CHAR, propagate_event);
   EO_DBG_INFO_APPEND(group, "Has clipees", EINA_VALUE_TYPE_CHAR, clipees_has);

   Evas_Object *clipper = NULL;
   eo_do(eo_obj, clipper = evas_obj_clip_get());
   EO_DBG_INFO_APPEND(group, "Clipper", EINA_VALUE_TYPE_UINT64, (uintptr_t) clipper);

   const Evas_Map *map = evas_object_map_get(eo_obj);
   if (map)
     {  /* Save map coords count info if object has map */
        node = EO_DBG_INFO_LIST_APPEND(group, "Evas Map");
        int points_count = evas_map_count_get(map);
        Eo_Dbg_Info *points = EO_DBG_INFO_LIST_APPEND(node, "Points");
        Eo_Dbg_Info *pointsuv = EO_DBG_INFO_LIST_APPEND(node, "Image UV");
        for (int i = 0 ; i < points_count; i++)
          {
               {
                  Evas_Coord px, py, pz;
                  evas_map_point_coord_get(map, i, &px, &py, &pz);
                  Eo_Dbg_Info *point = EO_DBG_INFO_LIST_APPEND(points, "Points");
                  EO_DBG_INFO_APPEND(point, "x", EINA_VALUE_TYPE_INT, px);
                  EO_DBG_INFO_APPEND(point, "y", EINA_VALUE_TYPE_INT, py);
                  EO_DBG_INFO_APPEND(point, "z", EINA_VALUE_TYPE_INT, pz);
               }

               {
                  double pu, pv;
                  evas_map_point_image_uv_get(map, i, &pu, &pv);
                  Eo_Dbg_Info *point = EO_DBG_INFO_LIST_APPEND(pointsuv, "Image UV");
                  EO_DBG_INFO_APPEND(point, "u", EINA_VALUE_TYPE_DOUBLE, pu);
                  EO_DBG_INFO_APPEND(point, "v", EINA_VALUE_TYPE_DOUBLE, pv);
               }
          }
     }
}

static Evas *
_evas_object_evas_common_interface_evas_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if ((obj->delete_me) || (!obj->layer)) return NULL;
   return obj->layer->evas->evas;
}

EOLIAN Evas_Object*
_evas_object_top_at_xy_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Evas_Layer *lay;
   int xx, yy;

   xx = x;
   yy = y;
////   xx = evas_coord_world_x_to_screen(eo_e, x);
////   yy = evas_coord_world_y_to_screen(eo_e, y);
   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
        Evas_Object *eo_obj;
        Evas_Object_Protected_Data *obj;

        EINA_INLIST_REVERSE_FOREACH(get_layer_objects(lay), obj)
          {
             eo_obj = obj->object;
             if (obj->delete_me) continue;
             if ((!include_pass_events_objects) &&
                 (evas_event_passes_through(eo_obj, obj))) continue;
             if (evas_object_is_source_invisible(eo_obj, obj)) continue;
             if ((!include_hidden_objects) && (!obj->cur->visible)) continue;
             evas_object_clip_recalc(obj);
             if ((evas_object_is_in_output_rect(eo_obj, obj, xx, yy, 1, 1)) &&
                 (!obj->clip.clipees)) return eo_obj;
          }
     }
   return NULL;
}

EAPI Evas_Object *
evas_object_top_at_pointer_get(const Evas *eo_e)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Object *ret = NULL;
   if (!e) return NULL;
   eo_do((Eo *)eo_e, ret = evas_canvas_object_top_at_xy_get(e->pointer.x, e->pointer.y, EINA_TRUE,
                                    EINA_TRUE));
   return ret;
}

EOLIAN Evas_Object*
_evas_object_top_in_rectangle_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Evas_Layer *lay;
   int xx, yy, ww, hh;

   xx = x;
   yy = y;
   ww = w;
   hh = h;
////   xx = evas_coord_world_x_to_screen(eo_e, x);
////   yy = evas_coord_world_y_to_screen(eo_e, y);
////   ww = evas_coord_world_x_to_screen(eo_e, w);
////   hh = evas_coord_world_y_to_screen(eo_e, h);
   if (ww < 1) ww = 1;
   if (hh < 1) hh = 1;
   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
        Evas_Object *eo_obj;
        Evas_Object_Protected_Data *obj;

        EINA_INLIST_REVERSE_FOREACH(get_layer_objects(lay), obj)
          {
             eo_obj = obj->object;
             if (obj->delete_me) continue;
             if ((!include_pass_events_objects) &&
                 (evas_event_passes_through(eo_obj, obj))) continue;
             if (evas_object_is_source_invisible(eo_obj, obj)) continue;
             if ((!include_hidden_objects) && (!obj->cur->visible)) continue;
             evas_object_clip_recalc(obj);
             if ((evas_object_is_in_output_rect(eo_obj, obj, xx, yy, ww, hh)) &&
                 (!obj->clip.clipees)) return eo_obj;
          }
     }
   return NULL;
}

EOLIAN Eina_List*
_evas_objects_at_xy_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Eina_List *in = NULL;
   Evas_Layer *lay;
   int xx, yy;

   xx = x;
   yy = y;
////   xx = evas_coord_world_x_to_screen(eo_e, x);
////   yy = evas_coord_world_y_to_screen(eo_e, y);
   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
        Evas_Object *eo_obj;
        Evas_Object_Protected_Data *obj;

        EINA_INLIST_REVERSE_FOREACH(get_layer_objects(lay), obj)
          {
             eo_obj = obj->object;
             // FIXME - Daniel: we don't know yet how to handle the next line
             if (obj->delete_me) continue;
             if ((!include_pass_events_objects) &&
                   (evas_event_passes_through(eo_obj, obj))) continue;
             if (evas_object_is_source_invisible(eo_obj, obj)) continue;
             if ((!include_hidden_objects) && (!obj->cur->visible)) continue;
             evas_object_clip_recalc(obj);
             if ((evas_object_is_in_output_rect(eo_obj, obj, xx, yy, 1, 1)) &&
                 (!obj->clip.clipees))
               in = eina_list_prepend(in, eo_obj);
          }
     }
   return in;
}

/**
 * Retrieves the objects in the given rectangle region
 * @param   eo_e The given evas object.
 * @param   x The horizontal coordinate.
 * @param   y The vertical coordinate.
 * @param   w The width size.
 * @param   h The height size.
 * @param   include_pass_events_objects Boolean Flag to include or not pass events objects
 * @param   include_hidden_objects Boolean Flag to include or not hidden objects
 * @return  The list of evas object in the rectangle region.
 *
 */
EOLIAN Eina_List*
_evas_objects_in_rectangle_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Eina_List *in = NULL;
   Evas_Layer *lay;
   int xx, yy, ww, hh;

   xx = x;
   yy = y;
   ww = w;
   hh = h;
////   xx = evas_coord_world_x_to_screen(eo_e, x);
////   yy = evas_coord_world_y_to_screen(eo_e, y);
////   ww = evas_coord_world_x_to_screen(eo_e, w);
////   hh = evas_coord_world_y_to_screen(eo_e, h);
   if (ww < 1) ww = 1;
   if (hh < 1) hh = 1;
   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
        Evas_Object *eo_obj;
        Evas_Object_Protected_Data *obj;

        EINA_INLIST_REVERSE_FOREACH(get_layer_objects(lay), obj)
          {
             eo_obj = obj->object;
             // FIXME - Daniel: we don't know yet how to handle the next line
             if (obj->delete_me) continue;
             if ((!include_pass_events_objects) &&
                 (evas_event_passes_through(eo_obj, obj))) continue;
             if (evas_object_is_source_invisible(eo_obj, obj)) continue;
             if ((!include_hidden_objects) && (!obj->cur->visible)) continue;
             evas_object_clip_recalc(obj);
             if ((evas_object_is_in_output_rect(eo_obj, obj, xx, yy, ww, hh)) &&
                 (!obj->clip.clipees))
               in = eina_list_prepend(in, eo_obj);
          }
     }
   return in;
}

EOLIAN static const char *
_evas_object_type_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->delete_me) return "";
   return obj->type;
}

EOLIAN static void
_evas_object_type_set(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, const char *type)
{
   obj->type = type; // Store it as the top type of this class
}

EOLIAN static void
_evas_object_precise_is_inside_set(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Eina_Bool precise)
{
   obj->precise_is_inside = precise;
}

EOLIAN static Eina_Bool
_evas_object_precise_is_inside_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->precise_is_inside;
}

EOLIAN static void
_evas_object_static_clip_set(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Eina_Bool is_static_clip)
{
   obj->is_static_clip = is_static_clip;
}

EOLIAN static Eina_Bool
_evas_object_static_clip_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->is_static_clip;
}

static void
_is_frame_flag_set(Evas_Object_Protected_Data *obj, Eina_Bool is_frame)
{
   const Eina_Inlist *l;
   Evas_Object_Protected_Data *child;

   obj->is_frame = is_frame;

   l = evas_object_smart_members_get_direct(obj->object);

   EINA_INLIST_FOREACH(l, child)
     _is_frame_flag_set(child, is_frame);
}

EOLIAN static void
_evas_object_is_frame_object_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool is_frame)
{
   Evas_Coord x, y;

   evas_object_geometry_get(eo_obj, &x, &y, NULL, NULL);

   _is_frame_flag_set(obj, is_frame);

   evas_object_move(eo_obj, x, y);
}

EOLIAN static Eina_Bool
_evas_object_is_frame_object_get(Eo *eo_obj  EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->is_frame;
}

EOLIAN static Evas_Object *
_evas_object_smart_parent_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (!obj) return NULL;
   return obj->smart.parent;
}

EOLIAN static void *
_evas_object_smart_data_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj EINA_UNUSED)
{
   return NULL;
}

EOLIAN static Eina_Bool
_evas_object_smart_type_check(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj EINA_UNUSED, const char *type EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_evas_object_smart_type_check_ptr(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj EINA_UNUSED, const char *type EINA_UNUSED)
{
   return EINA_FALSE;
}

#include "canvas/evas_object.eo.c"


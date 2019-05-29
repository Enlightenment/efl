#ifndef EVAS_INLINE_H
#define EVAS_INLINE_H

#include "evas_private.h"

/* Paranoid safety checks.
 * This can avoid lots of SEGV with dangling pointers to deleted objects.
 * Two variants: valid or alive (extra check on delete_me).
 */
#define EVAS_OBJECT_DATA_VALID(o) ((o) && (o)->layer && (o)->layer->evas)
#define EVAS_OBJECT_DATA_ALIVE(o) (EVAS_OBJECT_DATA_VALID(o) && !(o)->delete_me)
#define EVAS_OBJECT_DATA_VALID_CHECK(o, ...) do { \
   if (EINA_UNLIKELY(!EVAS_OBJECT_DATA_VALID(o))) return __VA_ARGS__; } while (0)
#define EVAS_OBJECT_DATA_ALIVE_CHECK(o, ...) do { \
   if (EINA_UNLIKELY(!EVAS_OBJECT_DATA_ALIVE(o))) return __VA_ARGS__; } while (0)
#define EVAS_OBJECT_DATA_GET(eo_o) \
   efl_data_scope_get((eo_o), EFL_CANVAS_OBJECT_CLASS)
#define EVAS_OBJECT_DATA_SAFE_GET(eo_o) \
   efl_data_scope_safe_get((eo_o), EFL_CANVAS_OBJECT_CLASS)

static inline Eina_Bool
_evas_render_has_map(Evas_Object_Protected_Data *obj)
{
   return (obj->map->cur.map && obj->map->cur.usemap);
}

static inline Eina_Bool
_evas_render_can_map(Evas_Object_Protected_Data *obj)
{
   if (!obj->func->can_map) return EINA_FALSE;
   return obj->func->can_map(obj->object);
}

static inline void
_evas_object_gfx_mapping_update(Evas_Object_Protected_Data *obj)
{
   if (!obj->gfx_mapping_has) return;
   _efl_gfx_mapping_update(obj->object);
}

static inline int
_evas_object_event_new(void)
{
   return (++_evas_event_counter);
}

static inline Eina_Bool
_evas_object_callback_has_by_type(Evas_Object_Protected_Data *obj, Evas_Callback_Type type)
{
   return (obj->callback_mask & (((uint64_t)1) << type)) != 0;
}

static inline int
evas_object_was_visible(Evas_Object_Protected_Data *obj)
{
   if (EINA_UNLIKELY(!obj->prev)) return 0;
   if ((obj->prev->visible) && (!obj->no_render) &&
       ((obj->prev->cache.clip.visible) || obj->is_smart) &&
       ((obj->prev->cache.clip.a > 0 && obj->prev->render_op == EVAS_RENDER_BLEND)
       || obj->prev->render_op != EVAS_RENDER_BLEND))
     {
        if (obj->func->was_visible)
          return obj->func->was_visible(obj->object);
        return 1;
     }
   return 0;
}

static inline void
evas_add_rect(Eina_Array *rects, int x, int y, int w, int h)
{
   Eina_Rectangle *r;

   NEW_RECT(r, x, y, w, h);
   if (r) eina_array_push(rects, r);
}

static inline Cutout_Rect*
evas_common_draw_context_cutouts_add(Cutout_Rects* rects,
                                     int x, int y, int w, int h)
{
   Cutout_Rect* rect;

   if (rects->max < (rects->active + 1))
     {
        rects->max += 512;
        rects->rects = (Cutout_Rect *)realloc(rects->rects, sizeof(Cutout_Rect) * rects->max);
     }

   rect = rects->rects + rects->active;
   rect->x = x;
   rect->y = y;
   rect->w = w;
   rect->h = h;
   rects->active++;

   return rect;
}

static inline int
evas_object_is_opaque(Evas_Object_Protected_Data *obj)
{
   if (obj->is_smart || obj->no_render) return 0;
   if (obj->cur->render_op == EVAS_RENDER_COPY)
     return 1;
   /* If clipped: Assume alpha */
   if (obj->cur->cache.clip.a == 255)
     {
        /* If has mask image: Always assume non opaque */
        if ((obj->cur->clipper && obj->cur->clipper->mask->is_mask) ||
            (obj->clip.mask))
          return 0;
        if (obj->func->is_opaque)
          return obj->func->is_opaque(obj->object, obj, obj->private_data);
        return 1;
     }
   return 0;
}

static inline int
evas_object_is_on_plane(Evas_Object_Protected_Data *obj)
{
   if (obj->func->is_on_plane)
     return obj->func->is_on_plane(obj->object, obj, obj->private_data);
   return 0;
}

static inline int
evas_object_plane_changed(Evas_Object_Protected_Data *obj)
{
   if (obj->func->plane_changed)
     return obj->func->plane_changed(obj->object, obj, obj->private_data);
   return 0;
}

static inline int
evas_event_freezes_through(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->freeze_events) return 1;
   if (obj->parent_cache.freeze_events_valid)
     return obj->parent_cache.freeze_events;
   if (!obj->smart.parent) return 0;
   Evas_Object_Protected_Data *smart_parent_pd = efl_data_scope_get(obj->smart.parent, EFL_CANVAS_OBJECT_CLASS);
   obj->parent_cache.freeze_events =
      evas_event_freezes_through(obj->smart.parent, smart_parent_pd);
   obj->parent_cache.freeze_events_valid = EINA_TRUE;
   return obj->parent_cache.freeze_events;
}

static inline int
evas_event_passes_through(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->pass_events || obj->no_render) return 1;
   if (obj->parent_cache.pass_events_valid)
     return obj->parent_cache.pass_events;
   if (!obj->smart.parent) return 0;
   Evas_Object_Protected_Data *smart_parent_pd = efl_data_scope_get(obj->smart.parent, EFL_CANVAS_OBJECT_CLASS);
   obj->parent_cache.pass_events =
      evas_event_passes_through(obj->smart.parent, smart_parent_pd);
   obj->parent_cache.pass_events_valid = EINA_TRUE;
   return obj->parent_cache.pass_events;
}

static inline int
evas_object_is_source_invisible(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->parent_cache.src_invisible_valid)
     return obj->parent_cache.src_invisible;
   if ((obj->proxy->proxies || obj->proxy->proxy_textures) && obj->proxy->src_invisible) return 1;
   if (!obj->smart.parent) return 0;
   if (obj->mask->is_mask) return 0;
   Evas_Object_Protected_Data *smart_parent_pd =
      efl_data_scope_get(obj->smart.parent, EFL_CANVAS_OBJECT_CLASS);
   obj->parent_cache.src_invisible =
      evas_object_is_source_invisible(obj->smart.parent, smart_parent_pd);
   obj->parent_cache.src_invisible_valid = EINA_TRUE;
   return obj->parent_cache.src_invisible;
}

static inline int
evas_object_is_visible(Evas_Object_Protected_Data *obj)
{
   if (EINA_UNLIKELY(!obj->cur)) return 0;
   if ((obj->cur->visible) && (!obj->no_render) &&
       ((obj->cur->cache.clip.visible) || (obj->is_smart)) &&
       ((obj->cur->cache.clip.a > 0 && obj->cur->render_op == EVAS_RENDER_BLEND)
       || obj->cur->render_op != EVAS_RENDER_BLEND))
     {
        if (obj->func->is_visible)
          return obj->func->is_visible(obj->object);
        return 1;
     }
   return 0;
}

static inline int
evas_object_clippers_is_visible(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->cur->visible)
     {
        if (obj->cur->clipper)
          {
             return evas_object_clippers_is_visible(obj->cur->clipper->object,
                                                    obj->cur->clipper);
          }
        return 1;
     }
   return 0;
}

static inline int
evas_object_is_proxy_visible(Evas_Object_Protected_Data *obj)
{
   if ((obj->cur->visible) &&
       //FIXME: Check the cached clipper visible properly.
       (obj->is_smart || !obj->cur->clipper || obj->cur->clipper->cur->visible) &&
       ((obj->cur->cache.clip.a > 0 && obj->cur->render_op == EVAS_RENDER_BLEND)
        || obj->cur->render_op != EVAS_RENDER_BLEND))
     {
        if (obj->func->is_visible)
          return obj->func->is_visible(obj->object);
        return 1;
     }
   return 0;
}

static inline int
evas_object_is_in_output_rect(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, int x, int y, int w, int h)
{
   /* assumes coords have been recalced */
   if ((RECTS_INTERSECT(x, y, w, h,
                        obj->cur->cache.clip.x,
                        obj->cur->cache.clip.y,
                        obj->cur->cache.clip.w,
                        obj->cur->cache.clip.h)))
     return 1;
   return 0;
}

static inline int
evas_object_is_active(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   if (evas_object_is_visible(obj) || evas_object_was_visible(obj))
     {
        Evas_Public_Data *e = obj->layer->evas;
        int fx, fy;
        fx = e->framespace.x;
        fy = e->framespace.y;
        if (obj->is_smart && !(obj->map->cur.map && obj->map->cur.usemap))
          return 1;

        if (evas_object_is_in_output_rect(eo_obj, obj, -fx, -fy,
                                          e->output.w, e->output.h) ||
            evas_object_was_in_output_rect(eo_obj, obj, -fx, -fy,
                                           e->output.w, e->output.h))
          return 1;
     }
   /* FIXME: forcing object with proxies to stay active,
      need to be smarter and only do that when really needed. */
   if (obj->proxy->proxies && obj->changed)
     return 1;
   if (obj->mask->is_mask && obj->clip.clipees)
     return 1;
   return 0;
}

static inline void
evas_object_coords_recalc(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
////   if (obj->cur->cache.geometry.validity == obj->layer->evas->output_validity)
////     return;
////   obj->cur->cache.geometry.x =
////     evas_coord_world_x_to_screen(obj->layer->evas, obj->cur->geometry.x);
////   obj->cur->cache.geometry.y =
////     evas_coord_world_y_to_screen(obj->layer->evas, obj->cur->geometry.y);
////   obj->cur->cache.geometry.w =
////     evas_coord_world_x_to_screen(obj->layer->evas, obj->cur->geometry.w) -
////     evas_coord_world_x_to_screen(obj->layer->evas, 0);
////   obj->cur->cache.geometry.h =
////     evas_coord_world_y_to_screen(obj->layer->evas, obj->cur->geometry.h) -
////     evas_coord_world_y_to_screen(obj->layer->evas, 0);
   if (obj->func->coords_recalc) obj->func->coords_recalc(eo_obj, obj, obj->private_data);
////   obj->cur->cache.geometry.validity = obj->layer->evas->output_validity;
}

static inline void
evas_object_clip_recalc(Evas_Object_Protected_Data *obj)
{
   Evas_Object_Protected_Data *clipper = NULL;

   EVAS_OBJECT_DATA_ALIVE_CHECK(obj);

   clipper = obj->cur->clipper;

   if (EINA_LIKELY(((!obj->cur->cache.clip.dirty) &&
                    !(!clipper || clipper->cur->cache.clip.dirty)))) return;

   if (EINA_UNLIKELY(obj->layer->evas->is_frozen)) return;

   evas_object_clip_recalc_do(obj, clipper);
}

static inline void
evas_object_clip_dirty(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->cur->cache.clip.dirty) return;
   evas_object_clip_dirty_do(obj);
}

static inline void
evas_object_async_block(Evas_Object_Protected_Data *obj)
{
   if (EVAS_OBJECT_DATA_VALID(obj))
     {
        eina_lock_take(&(obj->layer->evas->lock_objects));
        eina_lock_release(&(obj->layer->evas->lock_objects));
     }
}

static inline void
evas_canvas_async_block(Evas_Public_Data *e)
{
   if (e)
     {
        eina_lock_take(&(e->lock_objects));
        eina_lock_release(&(e->lock_objects));
     }
}

static inline void
evas_common_draw_context_cache_update(RGBA_Draw_Context *dc)
{
   dc->cache.used++;
   if (dc->cache.used >= 4096)
     {
        evas_common_draw_context_cutouts_real_free(dc->cache.rects);
        dc->cache.rects = NULL;
        dc->cache.used = 0;
     }
}

static inline Eina_Bool
_evas_eina_rectangle_inside(const Eina_Rectangle *big, const Eina_Rectangle *small)
{
   Eina_Rectangle inter = *big;

   if (!eina_rectangle_intersection(&inter, small))
     return EINA_FALSE;
   if ((inter.w == small->w) && (inter.h == small->h))
     return EINA_TRUE;
   return EINA_FALSE;
}

// Temporary until we have multi output support
static inline void *
_evas_default_output_get(Evas_Public_Data *e)
{
   Efl_Canvas_Output *output;

   output = eina_list_data_get(e->outputs);
   return output ? output->output : NULL;
}

static inline void *
_evas_engine_context(Evas_Public_Data *e)
{
   return e->backend;
}

static inline void
_evas_wrap_del(Evas_Object **eo, Evas_Object_Protected_Data *pd)
{
   if (!*eo) return ;
   if (pd && pd->legacy.ctor) evas_object_del(*eo);
   else efl_del(*eo);
}

#define _EVAS_COLOR_CLAMP(x, y) do { \
   if (x > y) { x = y; bad = 1; } \
   if (x < 0) { x = 0; bad = 1; } } while (0)

#define EVAS_COLOR_SANITIZE(r, g, b, a) \
   ({ int bad = 0; \
   _EVAS_COLOR_CLAMP(a, 255); \
   _EVAS_COLOR_CLAMP(r, a); \
   _EVAS_COLOR_CLAMP(g, a); \
   _EVAS_COLOR_CLAMP(b, a); \
   bad; })

#endif

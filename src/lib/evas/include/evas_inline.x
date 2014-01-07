#ifndef EVAS_INLINE_H
#define EVAS_INLINE_H

static inline Eina_Bool
_evas_render_has_map(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   return (((obj->map->cur.map) && (obj->map->cur.usemap)) &&
           !((obj->func->can_map) && (obj->func->can_map(eo_obj))));
   //   return ((obj->map->cur.map) && (obj->map->cur.usemap));
}

static inline void
_evas_object_event_new(void)
{
   _evas_event_counter++;
}

static inline int
evas_object_was_visible(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   if ((obj->prev->visible) &&
       ((obj->prev->cache.clip.visible) || obj->is_smart) &&
       ((obj->prev->cache.clip.a > 0 && obj->prev->render_op == EVAS_RENDER_BLEND)
       || obj->prev->render_op != EVAS_RENDER_BLEND))
     {
        if (obj->func->was_visible)
          return obj->func->was_visible(eo_obj);
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
        rects->max += 128;
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
evas_object_is_opaque(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   if (obj->is_smart) return 0;
   /* If a mask: Assume alpha */
   if (obj->cur->cache.clip.a == 255)
     {
        if (obj->func->is_opaque)
          return obj->func->is_opaque(eo_obj, obj, obj->private_data);
        return 1;
     }
   if (obj->cur->render_op == EVAS_RENDER_COPY)
     return 1;
   return 0;
}

static inline int
evas_event_freezes_through(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->freeze_events) return 1;
   if (obj->parent_cache.freeze_events_valid)
     return obj->parent_cache.freeze_events;
   if (!obj->smart.parent) return 0;
   Evas_Object_Protected_Data *smart_parent_pd = eo_data_scope_get(obj->smart.parent, EVAS_OBJ_CLASS);
   obj->parent_cache.freeze_events =
      evas_event_freezes_through(obj->smart.parent, smart_parent_pd);
   obj->parent_cache.freeze_events_valid = EINA_TRUE;
   return obj->parent_cache.freeze_events;
}

static inline int
evas_event_passes_through(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->pass_events) return 1;
   if (obj->parent_cache.pass_events_valid)
     return obj->parent_cache.pass_events;
   if (!obj->smart.parent) return 0;
   Evas_Object_Protected_Data *smart_parent_pd = eo_data_scope_get(obj->smart.parent, EVAS_OBJ_CLASS);
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
#ifdef EVAS_3D
   if ((obj->proxy->proxies || obj->proxy->proxy_textures) && obj->proxy->src_invisible) return 1;
#else
   if (obj->proxy->proxies && obj->proxy->src_invisible) return 1;
#endif
   if (!obj->smart.parent) return 0;
   Evas_Object_Protected_Data *smart_parent_pd =
      eo_data_scope_get(obj->smart.parent, EVAS_OBJ_CLASS);
   obj->parent_cache.src_invisible =
      evas_object_is_source_invisible(obj->smart.parent, smart_parent_pd);
   obj->parent_cache.src_invisible_valid = EINA_TRUE;
   return obj->parent_cache.src_invisible;
}

static inline int
evas_object_is_visible(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{                        /* post 1.0 -> enable? */
   if ((obj->cur->visible)/* && (obj->cur->color.a > 0)*/ &&
       ((obj->cur->cache.clip.visible) || (obj->is_smart)) &&
       ((obj->cur->cache.clip.a > 0 && obj->cur->render_op == EVAS_RENDER_BLEND)
       || obj->cur->render_op != EVAS_RENDER_BLEND))
     {
        if (obj->func->is_visible)
          return obj->func->is_visible(eo_obj);
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
evas_object_is_proxy_visible(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   if ((obj->cur->visible) &&
       //FIXME: Check the cached clipper visible properly.
       (obj->is_smart || !obj->cur->clipper || obj->cur->clipper->cur->visible) &&
       ((obj->cur->cache.clip.a > 0 && obj->cur->render_op == EVAS_RENDER_BLEND)
        || obj->cur->render_op != EVAS_RENDER_BLEND))
     {
        if (obj->func->is_visible)
          return obj->func->is_visible(eo_obj);
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
   if (evas_object_is_visible(eo_obj, obj) || evas_object_was_visible(eo_obj, obj))
     {
        Evas_Public_Data *e = obj->layer->evas;
        int fx, fy;
        fx = e->framespace.x;
        fy = e->framespace.y;
        if (obj->is_smart)
          {
             int mapsmt = 0;
             if (obj->map->cur.map && obj->map->cur.usemap) mapsmt = 1;
             if (!mapsmt) return 1;
             if (evas_object_is_in_output_rect(eo_obj, obj, -fx, -fy,
                                               e->output.w, e->output.h) ||
                 evas_object_was_in_output_rect(eo_obj, obj, -fx, -fy,
                                                e->output.w, e->output.h))
               return 1;
          }
        else
          {
             if (evas_object_is_in_output_rect(eo_obj, obj, -fx, -fy,
                                               e->output.w, e->output.h) ||
                 evas_object_was_in_output_rect(eo_obj, obj, -fx, -fy,
                                                e->output.w, e->output.h))
               return 1;
          }
     }
   /* FIXME: forcing object with proxies to stay active,
      need to be smarter and only do that when really needed. */
   if (obj->proxy->proxies && obj->changed)
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
   int cx, cy, cw, ch, cr, cg, cb, ca;
   int nx, ny, nw, nh, nr, ng, nb, na;
   Eina_Bool cvis, nvis;
   Evas_Object *eo_obj;

   eo_obj = obj->object;
   clipper = obj->cur->clipper;

   if ((!obj->cur->cache.clip.dirty) &&
       !(!obj->cur->clipper || clipper->cur->cache.clip.dirty)) return;

   if (obj->layer->evas->is_frozen) return;

   evas_object_coords_recalc(eo_obj, obj);

   if ((obj->map->cur.map) && (obj->map->cur.usemap))
     {
        cx = obj->map->cur.map->normal_geometry.x;
        cy = obj->map->cur.map->normal_geometry.y;
        cw = obj->map->cur.map->normal_geometry.w;
        ch = obj->map->cur.map->normal_geometry.h;
     }
   else
     {
        cx = obj->cur->geometry.x;
        cy = obj->cur->geometry.y;
        cw = obj->cur->geometry.w;
        ch = obj->cur->geometry.h;
     }

   if (obj->cur->color.a == 0 && obj->cur->render_op == EVAS_RENDER_BLEND)
      cvis = EINA_FALSE;
   else cvis = obj->cur->visible;

   cr = obj->cur->color.r; cg = obj->cur->color.g;
   cb = obj->cur->color.b; ca = obj->cur->color.a;

   if (clipper)
     {
        // this causes problems... hmmm ?????
        if (clipper->cur->cache.clip.dirty)
          evas_object_clip_recalc(clipper);

        // I don't know why this test was here in the first place. As I have
        // no issue showing up due to this, I keep it and move color out of it.
        // breaks cliping of mapped images!!!
        if (clipper->map->cur.map_parent == obj->map->cur.map_parent)
          {
             nx = clipper->cur->cache.clip.x;
             ny = clipper->cur->cache.clip.y;
             nw = clipper->cur->cache.clip.w;
             nh = clipper->cur->cache.clip.h;
             RECTS_CLIP_TO_RECT(cx, cy, cw, ch, nx, ny, nw, nh);
          }

        nvis = clipper->cur->cache.clip.visible;
        nr = clipper->cur->cache.clip.r;
        ng = clipper->cur->cache.clip.g;
        nb = clipper->cur->cache.clip.b;
        na = clipper->cur->cache.clip.a;
        cvis = (cvis & nvis);
        cr = (cr * (nr + 1)) >> 8;
        cg = (cg * (ng + 1)) >> 8;
        cb = (cb * (nb + 1)) >> 8;
        ca = (ca * (na + 1)) >> 8;
     }
   if ((ca == 0 && obj->cur->render_op == EVAS_RENDER_BLEND) ||
       (cw <= 0) || (ch <= 0)) cvis = EINA_FALSE;

   if (obj->cur->cache.clip.x == cx &&
       obj->cur->cache.clip.y == cy &&
       obj->cur->cache.clip.w == cw &&
       obj->cur->cache.clip.h == ch &&
       obj->cur->cache.clip.visible == cvis &&
       obj->cur->cache.clip.r == cr &&
       obj->cur->cache.clip.g == cg &&
       obj->cur->cache.clip.b == cb &&
       obj->cur->cache.clip.a == ca &&
       obj->cur->cache.clip.dirty == EINA_FALSE)
     return ;

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->cache.clip.x = cx;
        state_write->cache.clip.y = cy;
        state_write->cache.clip.w = cw;
        state_write->cache.clip.h = ch;
        state_write->cache.clip.visible = cvis;
        state_write->cache.clip.r = cr;
        state_write->cache.clip.g = cg;
        state_write->cache.clip.b = cb;
        state_write->cache.clip.a = ca;
        state_write->cache.clip.dirty = EINA_FALSE;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);
}

#endif

#ifndef EVAS_INLINE_H
#define EVAS_INLINE_H

static inline int
evas_object_was_visible(Evas_Object *obj)
{
   if (obj->smart.smart) return 0;
   if ((obj->prev.visible) &&
       (obj->prev.cache.clip.visible) &&
       (obj->prev.cache.clip.a > 0))
     {
	if (obj->func->was_visible)
	  return obj->func->was_visible(obj);
	return 1;
     }
   return 0;
}

static inline void
evas_add_rect(Evas_Rectangles *rects, int x, int y, int w, int h)
{
   if ((rects->count + 1) > rects->total)
     {
	Evas_Rectangle *_add_rect;
	unsigned int _tmp_total;

	_tmp_total = rects->total + 32;
	_add_rect = (Evas_Rectangle *)realloc(rects->array, sizeof(Evas_Rectangle) * _tmp_total);
	if (!_add_rect) return ;

	rects->total = _tmp_total;
	rects->array = _add_rect;
     }
   rects->array[rects->count].x = x;
   rects->array[rects->count].y = y;
   rects->array[rects->count].w = w;
   rects->array[rects->count].h = h;
   rects->count += 1;
}

static inline Cutout_Rect*
evas_common_draw_context_cutouts_add(Cutout_Rects* rects,
                                     int x, int y, int w, int h)
{
   Cutout_Rect* rect;

   if (rects->max < (rects->active + 1))
     {
	rects->max += 32;
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
evas_object_is_opaque(Evas_Object *obj)
{
   if (obj->smart.smart) return 0;
   if (obj->cur.cache.clip.a == 255)
     {
	if (obj->func->is_opaque)
	  return obj->func->is_opaque(obj);
	return 1;
     }
   return 0;
}

static inline int
evas_event_passes_through(Evas_Object *obj)
{
   if (obj->layer->evas->events_frozen > 0) return 1;
   if (obj->pass_events) return 1;
   if (obj->parent_cache_valid) return obj->parent_pass_events;
   if (obj->smart.parent)
     {
	int par_pass;

	par_pass = evas_event_passes_through(obj->smart.parent);
	obj->parent_cache_valid = 1;
	obj->parent_pass_events = par_pass;
	return par_pass;
     }
   return 0;
}

static inline int
evas_object_is_visible(Evas_Object *obj)
{
   if (obj->smart.smart) return 0;
   if ((obj->cur.visible) &&
       (obj->cur.cache.clip.visible) &&
       (obj->cur.cache.clip.a > 0))
     {
	if (obj->func->is_visible)
	  return obj->func->is_visible(obj);
	return 1;
     }
   return 0;
}

static inline int
evas_object_clippers_is_visible(Evas_Object *obj)
{
   if (obj->cur.visible)
     {
	if (obj->cur.clipper)
	  return evas_object_clippers_is_visible(obj->cur.clipper);
	return 1;
     }
   return 0;
}

static inline int
evas_object_is_in_output_rect(Evas_Object *obj, int x, int y, int w, int h)
{
   if (obj->smart.smart) return 0;
   /* assumes coords have been recalced */
   if ((RECTS_INTERSECT(x, y, w, h,
			obj->cur.cache.clip.x,
			obj->cur.cache.clip.y,
			obj->cur.cache.clip.w,
			obj->cur.cache.clip.h)))
     return 1;
   return 0;
}

static inline int
evas_object_is_active(Evas_Object *obj)
{
   if (obj->smart.smart) return 0;
   if ((evas_object_is_visible(obj) || evas_object_was_visible(obj)) &&
       (evas_object_is_in_output_rect(obj, 0, 0, obj->layer->evas->output.w,
				      obj->layer->evas->output.h) ||
	evas_object_was_in_output_rect(obj, 0, 0, obj->layer->evas->output.w,
				       obj->layer->evas->output.h)))
     return 1;
   return 0;
}

static inline void
evas_object_coords_recalc(Evas_Object *obj)
{
   if (obj->smart.smart) return;
////   if (obj->cur.cache.geometry.validity == obj->layer->evas->output_validity)
////     return;
////   obj->cur.cache.geometry.x =
////     evas_coord_world_x_to_screen(obj->layer->evas, obj->cur.geometry.x);
////   obj->cur.cache.geometry.y =
////     evas_coord_world_y_to_screen(obj->layer->evas, obj->cur.geometry.y);
////   obj->cur.cache.geometry.w =
////     evas_coord_world_x_to_screen(obj->layer->evas, obj->cur.geometry.w) -
////     evas_coord_world_x_to_screen(obj->layer->evas, 0);
////   obj->cur.cache.geometry.h =
////     evas_coord_world_y_to_screen(obj->layer->evas, obj->cur.geometry.h) -
////     evas_coord_world_y_to_screen(obj->layer->evas, 0);
   if (obj->func->coords_recalc) obj->func->coords_recalc(obj);
////   obj->cur.cache.geometry.validity = obj->layer->evas->output_validity;
}

static inline void
evas_object_clip_recalc(Evas_Object *obj)
{
   int cx, cy, cw, ch, cvis, cr, cg, cb, ca;
   int nx, ny, nw, nh, nvis, nr, ng, nb, na;

   if (obj->layer->evas->events_frozen > 0) return;
//   if (!obj->cur.clipper->cur.cache.clip.dirty) return;
   evas_object_coords_recalc(obj);
   cx = obj->cur.geometry.x; cy = obj->cur.geometry.y;
   cw = obj->cur.geometry.w; ch = obj->cur.geometry.h;
////   cx = obj->cur.cache.geometry.x; cy = obj->cur.cache.geometry.y;
////   cw = obj->cur.cache.geometry.w; ch = obj->cur.cache.geometry.h;
   if (obj->cur.color.a == 0) cvis = 0;
   else cvis = obj->cur.visible;
   cr = obj->cur.color.r; cg = obj->cur.color.g;
   cb = obj->cur.color.b; ca = obj->cur.color.a;
   if (obj->cur.clipper)
     {
// this causes problems... hmmm
//	if (obj->cur.clipper->cur.cache.clip.dirty)
	  evas_object_clip_recalc(obj->cur.clipper);
	nx = obj->cur.clipper->cur.cache.clip.x;
	ny = obj->cur.clipper->cur.cache.clip.y;
	nw = obj->cur.clipper->cur.cache.clip.w;
	nh = obj->cur.clipper->cur.cache.clip.h;
	RECTS_CLIP_TO_RECT(cx, cy, cw, ch, nx, ny, nw, nh);

	nvis = obj->cur.clipper->cur.cache.clip.visible;
	nr = obj->cur.clipper->cur.cache.clip.r;
	ng = obj->cur.clipper->cur.cache.clip.g;
	nb = obj->cur.clipper->cur.cache.clip.b;
	na = obj->cur.clipper->cur.cache.clip.a;
	cvis = cvis * nvis;
	cr = (cr * (nr + 1)) >> 8;
	cg = (cg * (ng + 1)) >> 8;
	cb = (cb * (nb + 1)) >> 8;
	ca = (ca * (na + 1)) >> 8;
     }
   if ((ca == 0) || (cw <= 0) || (ch <= 0)) cvis = 0;
   obj->cur.cache.clip.x = cx;
   obj->cur.cache.clip.y = cy;
   obj->cur.cache.clip.w = cw;
   obj->cur.cache.clip.h = ch;
   obj->cur.cache.clip.visible = cvis;
   obj->cur.cache.clip.r = cr;
   obj->cur.cache.clip.g = cg;
   obj->cur.cache.clip.b = cb;
   obj->cur.cache.clip.a = ca;
   obj->cur.cache.clip.dirty = 0;
}

#endif

#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

void
evas_object_clip_recalc(Evas_Object *obj)
{
   int cx, cy, cw, ch, cvis, cr, cg, cb, ca;
   int nx, ny, nw, nh, nvis, nr, ng, nb, na;

   evas_object_coords_recalc(obj);
   cx = obj->cur.cache.geometry.x; cy = obj->cur.cache.geometry.y;
   cw = obj->cur.cache.geometry.w; ch = obj->cur.cache.geometry.h;
   if (obj->cur.color.a == 0) cvis = 0;
   else cvis = obj->cur.visible;
   cr = obj->cur.color.r; cg = obj->cur.color.g;
   cb = obj->cur.color.b; ca = obj->cur.color.a;
   if (obj->cur.clipper)
     {
	evas_object_clip_recalc(obj->cur.clipper);
	nx = obj->cur.clipper->cur.cache.clip.x;
	ny = obj->cur.clipper->cur.cache.clip.y;
	nw = obj->cur.clipper->cur.cache.clip.w;
	nh = obj->cur.clipper->cur.cache.clip.h;
	nvis = obj->cur.clipper->cur.cache.clip.visible;
	nr = obj->cur.clipper->cur.cache.clip.r;
	ng = obj->cur.clipper->cur.cache.clip.g;
	nb = obj->cur.clipper->cur.cache.clip.b;
	na = obj->cur.clipper->cur.cache.clip.a;
	RECTS_CLIP_TO_RECT(cx, cy, cw, ch, nx, ny, nw, nh);
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
}

void
evas_object_recalc_clippees(Evas_Object *obj)
{
   Evas_List *l;
   
   evas_object_clip_recalc(obj);   
   for (l = obj->clip.clipees; l; l = l->next)
     {
	evas_object_recalc_clippees(l->data);
     }
}

int
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

int
evas_object_clippers_was_visible(Evas_Object *obj)
{
   if (obj->prev.visible)
     {
	if (obj->prev.clipper)
	  return evas_object_clippers_is_visible(obj->prev.clipper);
	return 1;
     }
   return 0;
}

/* public functions */

/**
 * Clip one object to another.
 * @param obj The object to be clipped
 * @param clip The object to clip @p obj by
 * 
 * This function will clip the object @p obj to the area occupied by the
 * object @p clipper.
 * 
 * Example:
 * @code
 * extern Evas *evas;
 * extern Evas_Object *obj;
 * Evas_Object *clipper;
 * 
 * clipper = evas_object_rectangle_add(evas);
 * evas_object_color_ser(clipper, 255, 255, 255, 255);
 * evas_object_move(clipper, 10, 10);
 * evas_object_resize(clipper, 20, 50);
 * evas_object_clip_set(obj, clipper);
 * evas_object_show(clipper);
 * @endcode
 */
void
evas_object_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!clip) 
     {
	evas_object_clip_unset(obj);
	return;
     }
   MAGIC_CHECK(clip, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->cur.clipper == clip) return;
   if (obj->smart.smart)
     {
       if (obj->smart.smart->smart_class->clip_set)
	  obj->smart.smart->smart_class->clip_set(obj, clip);
     }
   if (obj->cur.clipper)
     {
	/* unclip */
	obj->cur.clipper->clip.clipees = evas_list_remove(obj->cur.clipper->clip.clipees, obj);
	evas_object_change(obj);
	obj->cur.clipper = NULL;	
     }
   /* clip me */
   obj->cur.clipper = clip;
   clip->clip.clipees = evas_list_append(clip->clip.clipees, obj);
   evas_object_change(obj);
   evas_object_recalc_clippees(obj);
   if (!obj->smart.smart)
     {
	if (evas_object_is_in_output_rect(obj, 
					  obj->layer->evas->pointer.x, 
					  obj->layer->evas->pointer.y, 1, 1))
	  evas_event_feed_mouse_move(obj->layer->evas, 
				     obj->layer->evas->pointer.x, 
				     obj->layer->evas->pointer.y);
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Object *
evas_object_clip_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   return obj->cur.clipper;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_clip_unset(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!obj->cur.clipper) return;
   /* unclip */
   if (obj->smart.smart)
     {
       if (obj->smart.smart->smart_class->clip_unset)
	  obj->smart.smart->smart_class->clip_unset(obj);
     }
   obj->cur.clipper->clip.clipees = evas_list_remove(obj->cur.clipper->clip.clipees, obj);
   obj->cur.clipper = NULL;
   evas_object_change(obj);
   evas_object_recalc_clippees(obj);
   if (!obj->smart.smart)
     {
	if (evas_object_is_in_output_rect(obj, 
					  obj->layer->evas->pointer.x, 
					  obj->layer->evas->pointer.y, 1, 1))
	  evas_event_feed_mouse_move(obj->layer->evas, 
				     obj->layer->evas->pointer.x, 
				     obj->layer->evas->pointer.y);
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
const Evas_List *
evas_object_clipees_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   return obj->clip.clipees;
}

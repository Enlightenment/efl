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
 * object @p clipper. This means the object @p obj will only be visible within
 * the area occupied by the clipping object (@p clip). The color of the object
 * being clipped will be multiplied by the color of the clipping object, so
 * the resulting color for the clipped object is
 * RESULT = (OBJ * CLIP) / (255 * 255) per color element (red, green, blue and
 * alpha). Clipping is recursive, so clip objects may be clipped by other
 * objects, and their color will in tern be multiplied. You may NOT set up
 * circular clipping lists (i.e. object 1 clips object 2 which clips object 1).
 * The behavior of Evas is undefined in this case. Objects which do not clip
 * others are visible as normal, those that clip 1 or more objects become
 * invisible themselves, only affecting what they clip. If an object ceases to
 * have other objects being clipped by it, it will become visible again. The
 * visibility of an object affects the objects that are clipped by it, so if
 * the object clipping others is not shown, the objects clipped will not be
 * shown either. If the object was being clipped by another object when this
 * function is called, it is implicitly removed from the clipper it is being
 * clipped to, and now is made to clip its new clipper.
 * 
 * At the moment the only objects that can validly be used to clip other
 * objects are rectangle objects. All other object types are invalid and the
 * result of using them is undefined.
 * 
 * The clip object @p clip must be a valid object, but may also be NULL in
 * which case the effect of this function is the same as calling
 * evas_object_clip_unset() on the @p obj object.
 * 
 * Example:
 * @code
 * extern Evas *evas;
 * extern Evas_Object *obj;
 * Evas_Object *clipper;
 * 
 * clipper = evas_object_rectangle_add(evas);
 * evas_object_color_set(clipper, 255, 255, 255, 255);
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
 * Get the object clipping this one (if any).
 * @param obj The object to get the clipper from
 *
 * This function returns the the object clipping @p obj. If @p obj not being
 * clipped, NULL is returned. The object @p obj must be a valid object.
 * 
 * See also evas_object_clip_set(), evas_object_clip_unset() and
 * evas_object_clipees_get().
 * 
 * Example:
 * @code
 * extern Evas_Object *obj;
 * Evas_Object *clipper;
 * 
 * clipper = evas_object_clip_get(obj);
 * if (clipper) evas_object_show(clipper);
 * @endcode
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
 * Disable clipping for an object.
 *
 * @param obj The object to cease clipping on
 * 
 * This function disables clipping for the object @p obj, if it was already
 * clipped. If it wasn't, this has no effect. The object @p obj must be a
 * valid object.
 * 
 * See also evas_object_clip_set(), evas_object_clipees_get() and 
 * evas_object_clip_get().
 * 
 * Example:
 * @code
 * extern Evas_Object *obj;
 * Evas_Object *clipper;
 * 
 * clipper = evas_object_clip_get(obj);
 * if (clipper)
 *   {
 *     evas_object_clip_unset(obj);
 *     evas_object_hide(obj);
 *   }
 * @endcode
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
 * Return a list of objects currently clipped by a specific object.
 * 
 * @param obj The object to get a list of clippees from
 * 
 * This returns the inernal list handle that contains all objects clipped by
 * the object @p obj. If none are clipped, it returns NULL. This list is only
 * valid until the clip list is changed and should be fetched again with another
 * call to evas_object_clipees_get() if any objects being clipped by this object
 * are unclipped, clipped by a new object, are deleted or the clipper is
 * deleted.  These operations will invalidate the list returned so it should
 * not be used anymore after that point. Any use of the list after this may have
 * undefined results, not limited just to strange behavior but possible
 * segfaults and other strange memory errors. The object @p obj must be a valid
 * object.
 *
 * See also evas_object_clip_set(), evas_object_clip_unset() and 
 * evas_object_clip_get().
 * 
 * Example:
 * @code
 * extern Evas_Object *obj;
 * Evas_Object *clipper;
 * 
 * clipper = evas_object_clip_get(obj);
 * if (clipper)
 *   {
 *     Evas_List *clippees, *l;
 * 
 *     clippees = evas_object_clipees_get(clipper);
 *     printf("Clipper clips %i objects\n", evas_list_count(clippees));
 *     for (l = clippees; l; l = l->next)
 *       {
 *         Evas_Object *obj_tmp;
 * 
 *         obj_tmp = l->data;
 *         evas_object_show(obj_tmp);
 *       }
 *   }
 * @endcode
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

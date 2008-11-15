#include "evas_common.h"
#include "evas_private.h"

/* private magic number for line objects */
static const char o_type[] = "line";

/* private struct for line object internal data */
typedef struct _Evas_Object_Line      Evas_Object_Line;

struct _Evas_Object_Line
{
   DATA32            magic;
   struct {
      struct {
	 int         x1, y1, x2, y2;
	 struct {
	    Evas_Coord w, h;
	 } object;
      } cache;
      Evas_Coord         x1, y1, x2, y2;
   } cur, prev;

   void             *engine_data;

   char              changed : 1;
};

/* private methods for line objects */
static void evas_object_line_init(Evas_Object *obj);
static void *evas_object_line_new(void);
static void evas_object_line_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_line_free(Evas_Object *obj);
static void evas_object_line_render_pre(Evas_Object *obj);
static void evas_object_line_render_post(Evas_Object *obj);

static unsigned int evas_object_line_id_get(Evas_Object *obj);
static unsigned int evas_object_line_visual_id_get(Evas_Object *obj);
static void *evas_object_line_engine_data_get(Evas_Object *obj);

static int evas_object_line_is_opaque(Evas_Object *obj);
static int evas_object_line_was_opaque(Evas_Object *obj);
static int evas_object_line_is_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static int evas_object_line_was_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void evas_object_line_coords_recalc(Evas_Object *obj);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_line_free,
     evas_object_line_render,
     evas_object_line_render_pre,
     evas_object_line_render_post,
     evas_object_line_id_get,
     evas_object_line_visual_id_get,
     evas_object_line_engine_data_get,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_line_is_opaque,
     evas_object_line_was_opaque,
     evas_object_line_is_inside,
     evas_object_line_was_inside,
     evas_object_line_coords_recalc,
     NULL
};

/* the actual api call to add a rect */
/* it has no other api calls as all properties are standard */

/**
 * @defgroup Evas_Line_Group Line Object Functions
 *
 * Functions used to deal with evas line objects.
 */

/**
 * @addtogroup Evas_Line_Group
 * @{
 * @ingroup Evas_Object_Specific
 */

/**
 * Adds a new evas line object to the given evas.
 * @param   e The given evas.
 * @return  The new evas line object.
 */
EAPI Evas_Object *
evas_object_line_add(Evas *e)
{
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new();
   evas_object_line_init(obj);
   evas_object_inject(obj, e);
   return obj;
}

/**
 * Sets the coordinates of the end points of the given evas line object.
 * @param   obj The given evas line object.
 * @param   x1  The X coordinate of the first point.
 * @param   y1  The Y coordinate of the first point.
 * @param   x2  The X coordinate of the second point.
 * @param   y2  The Y coordinate of the second point.
 */
EAPI void
evas_object_line_xy_set(Evas_Object *obj, Evas_Coord x1, Evas_Coord y1, Evas_Coord x2, Evas_Coord y2)
{
   Evas_Object_Line *o;
   Evas_Coord min_x, max_x, min_y, max_y;
   int is, was = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Line *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Line, MAGIC_OBJ_LINE);
   return;
   MAGIC_CHECK_END();
   if ((x1 == o->cur.x1) && (y1 == o->cur.y1) &&
       (x2 == o->cur.x2) && (y2 == o->cur.y2)) return;
   if (obj->layer->evas->events_frozen <= 0)
     {
	if (!evas_event_passes_through(obj))
	  was = evas_object_is_in_output_rect(obj,
					      obj->layer->evas->pointer.x,
					      obj->layer->evas->pointer.y, 1, 1);
     }
   if (x1 < x2)
     {
	min_x = x1;
	max_x = x2;
     }
   else
     {
	min_x = x2;
	max_x = x1;
     }
   if (y1 < y2)
     {
	min_y = y1;
	max_y = y2;
     }
   else
     {
	min_y = y2;
	max_y = y1;
     }
   obj->cur.geometry.x = min_x;
   obj->cur.geometry.y = min_y;
   obj->cur.geometry.w = max_x - min_x + 2.0;
   obj->cur.geometry.h = max_y - min_y + 2.0;
////   obj->cur.cache.geometry.validity = 0;
   o->cur.x1 = x1 - min_x;
   o->cur.y1 = y1 - min_y;
   o->cur.x2 = x2 - min_x;
   o->cur.y2 = y2 - min_y;
   o->changed = 1;
   evas_object_change(obj);
   evas_object_coords_recalc(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
	is = evas_object_is_in_output_rect(obj,
					   obj->layer->evas->pointer.x,
					   obj->layer->evas->pointer.y, 1, 1);
	if (!evas_event_passes_through(obj))
	  {
	     if ((is ^ was) && obj->cur.visible)
	       evas_event_feed_mouse_move(obj->layer->evas,
					  obj->layer->evas->pointer.x,
					  obj->layer->evas->pointer.y,
					  obj->layer->evas->last_timestamp,
					  NULL);
	  }
     }
   evas_object_inform_call_move(obj);
   evas_object_inform_call_resize(obj);
}

/**
 * Retrieves the coordinates of the end points of the given evas line object.
 * @param obj The given line object.
 * @param x1  Pointer to an integer in which to store the X coordinate of the
 *            first end point.
 * @param y1  Pointer to an integer in which to store the Y coordinate of the
 *            first end point.
 * @param x2  Pointer to an integer in which to store the X coordinate of the
 *            second end point.
 * @param y2  Pointer to an integer in which to store the Y coordinate of the
 *            second end point.
 */
EAPI void
evas_object_line_xy_get(const Evas_Object *obj, Evas_Coord *x1, Evas_Coord *y1, Evas_Coord *x2, Evas_Coord *y2)
{
   Evas_Object_Line *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (x1) *x1 = 0.0;
   if (y1) *y1 = 0.0;
   if (x2) *x2 = 0.0;
   if (y2) *y2 = 0.0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Line *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Line, MAGIC_OBJ_LINE);
   if (x1) *x1 = 0.0;
   if (y1) *y1 = 0.0;
   if (x2) *x2 = 0.0;
   if (y2) *y2 = 0.0;
   return;
   MAGIC_CHECK_END();
   if (x1) *x1 = obj->cur.geometry.x + o->cur.x1;
   if (y1) *y1 = obj->cur.geometry.y + o->cur.y1;
   if (x2) *x2 = obj->cur.geometry.x + o->cur.x2;
   if (y2) *y2 = obj->cur.geometry.y + o->cur.y2;
}

/**
 * @}
 */

/* all nice and private */
static void
evas_object_line_init(Evas_Object *obj)
{
   /* alloc image ob, setup methods and default values */
   obj->object_data = evas_object_line_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0;
   obj->cur.geometry.y = 0;
   obj->cur.geometry.w = 0;
   obj->cur.geometry.h = 0;
   obj->cur.layer = 0;
   obj->cur.anti_alias = 1;
   obj->cur.render_op = EVAS_RENDER_BLEND;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->type = o_type;
}

static void *
evas_object_line_new(void)
{
   Evas_Object_Line *o;

   /* alloc obj private data */
   o = calloc(1, sizeof(Evas_Object_Line));
   o->magic = MAGIC_OBJ_LINE;
   o->cur.x1 = 0.0;
   o->cur.y1 = 0.0;
   o->cur.x2 = 31.0;
   o->cur.y2 = 31.0;
   o->prev = o->cur;
   return o;
}

static void
evas_object_line_free(Evas_Object *obj)
{
   Evas_Object_Line *o;

   /* frees private object data. very simple here */
   o = (Evas_Object_Line *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Line, MAGIC_OBJ_LINE);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   o->magic = 0;
   free(o);
}

static void
evas_object_line_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Line *o;

   /* render object to surface with context, and offxet by x,y */
   o = (Evas_Object_Line *)(obj->object_data);
   obj->layer->evas->engine.func->context_color_set(output,
						    context,
						    obj->cur.cache.clip.r,
						    obj->cur.cache.clip.g,
						    obj->cur.cache.clip.b,
						    obj->cur.cache.clip.a);
   obj->layer->evas->engine.func->context_multiplier_unset(output,
							   context);
   obj->layer->evas->engine.func->context_anti_alias_set(output, context,
							 obj->cur.anti_alias);
   obj->layer->evas->engine.func->context_render_op_set(output, context,
							obj->cur.render_op);
   obj->layer->evas->engine.func->line_draw(output,
					    context,
					    surface,
					    o->cur.cache.x1 + x,
					    o->cur.cache.y1 + y,
					    o->cur.cache.x2 + x,
					    o->cur.cache.y2 + y);
}

static void
evas_object_line_render_pre(Evas_Object *obj)
{
   Evas_Rectangles rects = { 0, 0, NULL };
   Evas_Object_Line *o;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw lines */
   o = (Evas_Object_Line *)(obj->object_data);
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
	if (obj->cur.cache.clip.dirty)
	  evas_object_clip_recalc(obj->cur.clipper);
	obj->cur.clipper->func->render_pre(obj->cur.clipper);
     }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);
   if (is_v != was_v)
     {
	evas_object_render_pre_visible_change(&rects, obj, is_v, was_v);
	goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&rects, obj);
   /* if we restacked (layer or just within a layer) */
   if (obj->restack)
     {
	evas_object_render_pre_prev_cur_add(&rects, obj);
	goto done;
     }
   /* if it changed anti_alias */
   if (obj->cur.anti_alias != obj->prev.anti_alias)
     {
	evas_object_render_pre_prev_cur_add(&rects, obj);
	goto done;
     }
   /* if it changed render op */
   if (obj->cur.render_op != obj->prev.render_op)
     {
	evas_object_render_pre_prev_cur_add(&rects, obj);
	goto done;
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     {
	evas_object_render_pre_prev_cur_add(&rects, obj);
	goto done;
     }
   /* if it changed geometry - and obviously not visibility or color */
   /* caluclate differences since we have a constant color fill */
   /* we really only need to update the differences */
   if ((obj->cur.geometry.x != obj->prev.geometry.x) ||
       (obj->cur.geometry.y != obj->prev.geometry.y) ||
       (obj->cur.geometry.w != obj->prev.geometry.w) ||
       (obj->cur.geometry.h != obj->prev.geometry.h) ||
       ((o->changed) &&
	((o->cur.x1 != o->prev.x1) ||
	 (o->cur.y1 != o->prev.y1) ||
	 (o->cur.x2 != o->prev.x2) ||
	 (o->cur.y2 != o->prev.y2)))
       )
     {
	evas_object_render_pre_prev_cur_add(&rects, obj);
	goto done;
     }
   done:
   evas_object_render_pre_effect_updates(&rects, obj, is_v, was_v);
}

static void
evas_object_line_render_post(Evas_Object *obj)
{
   Evas_Object_Line *o;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Line *)(obj->object_data);
   /* remove those pesky changes */
   while (obj->clip.changes)
     {
	Evas_Rectangle *r;

	r = (Evas_Rectangle *)obj->clip.changes->data;
	obj->clip.changes = eina_list_remove(obj->clip.changes, r);
	free(r);
     }
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
   o->prev = o->cur;
   o->changed = 0;
}

static unsigned int evas_object_line_id_get(Evas_Object *obj)
{
   Evas_Object_Line *o;

   o = (Evas_Object_Line *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_LINE;
}

static unsigned int evas_object_line_visual_id_get(Evas_Object *obj)
{
   Evas_Object_Line *o;

   o = (Evas_Object_Line *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_SHAPE;
}

static void *evas_object_line_engine_data_get(Evas_Object *obj)
{
   Evas_Object_Line *o;

   o = (Evas_Object_Line *)(obj->object_data);
   if (!o) return NULL;
   return o->engine_data;
}

static int
evas_object_line_is_opaque(Evas_Object *obj)
{
   Evas_Object_Line *o;

   /* this returns 1 if the internal object data implies that the object is */
   /* currently fully opaque over the entire line it occupies */
   o = (Evas_Object_Line *)(obj->object_data);
   return 0;
}

static int
evas_object_line_was_opaque(Evas_Object *obj)
{
   Evas_Object_Line *o;

   /* this returns 1 if the internal object data implies that the object was */
   /* previously fully opaque over the entire line it occupies */
   o = (Evas_Object_Line *)(obj->object_data);
   return 0;
}

static int
evas_object_line_is_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Line *o;

   /* this returns 1 if the canvas co-ordinates are inside the object based */
   /* on object private data. not much use for rects, but for polys, images */
   /* and other complex objects it might be */
   o = (Evas_Object_Line *)(obj->object_data);
   return 1;
   x = 0;
   y = 0;
}

static int
evas_object_line_was_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Line *o;

   /* this returns 1 if the canvas co-ordinates were inside the object based */
   /* on object private data. not much use for rects, but for polys, images */
   /* and other complex objects it might be */
   o = (Evas_Object_Line *)(obj->object_data);
   return 1;
   x = 0;
   y = 0;
}

static void
evas_object_line_coords_recalc(Evas_Object *obj)
{
   Evas_Object_Line *o;

   o = (Evas_Object_Line *)(obj->object_data);
   o->cur.cache.x1 = obj->cur.geometry.x + o->cur.x1;
   o->cur.cache.y1 = obj->cur.geometry.y + o->cur.y1;
   o->cur.cache.x2 = obj->cur.geometry.x + o->cur.x2;
   o->cur.cache.y2 = obj->cur.geometry.y + o->cur.y2;
   o->cur.cache.object.w = obj->cur.geometry.w;
   o->cur.cache.object.h = obj->cur.geometry.h;
}

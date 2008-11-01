#include "evas_common.h"
#include "evas_private.h"

/* private magic number for rectangle objects */
static const char o_type[] = "rectangle";

/* private struct for rectangle object internal data */
typedef struct _Evas_Object_Rectangle      Evas_Object_Rectangle;

struct _Evas_Object_Rectangle
{
   DATA32            magic;
   void             *engine_data;
};

/* private methods for rectangle objects */
static void evas_object_rectangle_init(Evas_Object *obj);
static void *evas_object_rectangle_new(void);
static void evas_object_rectangle_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_rectangle_free(Evas_Object *obj);
static void evas_object_rectangle_render_pre(Evas_Object *obj);
static void evas_object_rectangle_render_post(Evas_Object *obj);

static unsigned int evas_object_rectangle_id_get(Evas_Object *obj);
static unsigned int evas_object_rectangle_visual_id_get(Evas_Object *obj);
static void *evas_object_rectangle_engine_data_get(Evas_Object *obj);

static int evas_object_rectangle_is_opaque(Evas_Object *obj);
static int evas_object_rectangle_was_opaque(Evas_Object *obj);

#if 0 /* usless calls for a rect object. much more useful for images etc. */
static void evas_object_rectangle_store(Evas_Object *obj);
static void evas_object_rectangle_unstore(Evas_Object *obj);
static int evas_object_rectangle_is_visible(Evas_Object *obj);
static int evas_object_rectangle_was_visible(Evas_Object *obj);
static int evas_object_rectangle_is_inside(Evas_Object *obj, double x, double y);
static int evas_object_rectangle_was_inside(Evas_Object *obj, double x, double y);
#endif

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_rectangle_free,
     evas_object_rectangle_render,
     evas_object_rectangle_render_pre,
     evas_object_rectangle_render_post,
     evas_object_rectangle_id_get,
     evas_object_rectangle_visual_id_get,
     evas_object_rectangle_engine_data_get,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_rectangle_is_opaque,
     evas_object_rectangle_was_opaque,
     NULL,
     NULL,
     NULL
};

/* the actual api call to add a rect */
/* it has no other api calls as all properties are standard */

/**
 * @defgroup Evas_Object_Rectangle Rectangle Object Functions
 *
 * Functions that operate on evas rectangle objects.
 */

/**
 * @addtogroup Evas_Object_Rectangle
 * @{
 * @ingroup Evas_Object_Specific
 */

/**
 * Adds a rectangle to the given evas.
 * @param   e The given evas.
 * @return  The new rectangle object.
 */
EAPI Evas_Object *
evas_object_rectangle_add(Evas *e)
{
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new();
   evas_object_rectangle_init(obj);
   evas_object_inject(obj, e);
   return obj;
}

/**
 * @}
 */

/* all nice and private */
static void
evas_object_rectangle_init(Evas_Object *obj)
{
   /* alloc image ob, setup methods and default values */
   obj->object_data = evas_object_rectangle_new();
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
   obj->cur.render_op = EVAS_RENDER_BLEND;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->type = o_type;
}

static void *
evas_object_rectangle_new(void)
{
   Evas_Object_Rectangle *o;

   /* alloc obj private data */
   o = calloc(1, sizeof(Evas_Object_Rectangle));
   o->magic = MAGIC_OBJ_RECTANGLE;
   return o;
}

static void
evas_object_rectangle_free(Evas_Object *obj)
{
   Evas_Object_Rectangle *o;

   /* frees private object data. very simple here */
   o = (Evas_Object_Rectangle *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Rectangle, MAGIC_OBJ_RECTANGLE);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   o->magic = 0;
   free(o);
}

static void
evas_object_rectangle_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Rectangle *o;

   /* render object to surface with context, and offxet by x,y */
   o = (Evas_Object_Rectangle *)(obj->object_data);
   obj->layer->evas->engine.func->context_color_set(output,
						    context,
						    obj->cur.cache.clip.r,
						    obj->cur.cache.clip.g,
						    obj->cur.cache.clip.b,
						    obj->cur.cache.clip.a);
   obj->layer->evas->engine.func->context_multiplier_unset(output,
							   context);
   obj->layer->evas->engine.func->context_render_op_set(output, context,
							obj->cur.render_op);
   obj->layer->evas->engine.func->rectangle_draw(output,
						 context,
						 surface,
						 obj->cur.geometry.x + x,
						 obj->cur.geometry.y + y,
						 obj->cur.geometry.w,
						 obj->cur.geometry.h);
////						 obj->cur.cache.geometry.x + x,
////						 obj->cur.cache.geometry.y + y,
////						 obj->cur.cache.geometry.w,
////						 obj->cur.cache.geometry.h);
}

static void
evas_object_rectangle_render_pre(Evas_Object *obj)
{
   Evas_Rectangles rects = { 0, 0, NULL };
   Evas_Object_Rectangle *o;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw rectangles */
   o = (Evas_Object_Rectangle *)(obj->object_data);
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
   /* if we restacked (layer or just within a layer) and don't clip anyone */
   if ((obj->restack) && (!obj->clip.clipees))
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
       (obj->cur.geometry.h != obj->prev.geometry.h))
     {
	evas_rects_return_difference_rects(&rects,
					   obj->cur.geometry.x,
					   obj->cur.geometry.y,
					   obj->cur.geometry.w,
					   obj->cur.geometry.h,
					   obj->prev.geometry.x,
					   obj->prev.geometry.y,
					   obj->prev.geometry.w,
					   obj->prev.geometry.h);
////	rl = evas_rects_return_difference_rects(obj->cur.cache.geometry.x,
////						obj->cur.cache.geometry.y,
////						obj->cur.cache.geometry.w,
////						obj->cur.cache.geometry.h,
////						obj->prev.cache.geometry.x,
////						obj->prev.cache.geometry.y,
////						obj->prev.cache.geometry.w,
////						obj->prev.cache.geometry.h);
	goto done;
     }
   /* it obviously didn't change - add a NO obscure - this "unupdates"  this */
   /* area so if there were updates for it they get wiped. don't do it if we */
   /* arent fully opaque and we are visible */
 /*   
   if (evas_object_is_visible(obj) &&
       evas_object_is_opaque(obj) &&
       (!obj->clip.clipees))
     obj->layer->evas->engine.func->output_redraws_rect_del(obj->layer->evas->engine.data.output,
							    obj->cur.cache.clip.x,
							    obj->cur.cache.clip.y,
							    obj->cur.cache.clip.w,
							    obj->cur.cache.clip.h);
  */
   done:
   evas_object_render_pre_effect_updates(&rects, obj, is_v, was_v);
}

static void
evas_object_rectangle_render_post(Evas_Object *obj)
{
   Evas_Object_Rectangle *o;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Rectangle *)(obj->object_data);
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
}

static int
evas_object_rectangle_is_opaque(Evas_Object *obj)
{
   Evas_Object_Rectangle *o;

   /* this returns 1 if the internal object data implies that the object is */
   /* currently fully opaque over the entire rectangle it occupies */
   o = (Evas_Object_Rectangle *)(obj->object_data);
   if (obj->cur.render_op == EVAS_RENDER_COPY)
	return 1;
   if (obj->cur.render_op != EVAS_RENDER_BLEND)
	return 0;
   return 1;
}

static int
evas_object_rectangle_was_opaque(Evas_Object *obj)
{
   Evas_Object_Rectangle *o;

   /* this returns 1 if the internal object data implies that the object was */
   /* previously fully opaque over the entire rectangle it occupies */
   o = (Evas_Object_Rectangle *)(obj->object_data);
   if (obj->prev.render_op == EVAS_RENDER_COPY)
	return 1;
   if (obj->prev.render_op != EVAS_RENDER_BLEND)
	return 0;
   return 1;
}

static unsigned int evas_object_rectangle_id_get(Evas_Object *obj)
{
   Evas_Object_Rectangle *o;

   o = (Evas_Object_Rectangle *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_RECTANGLE;
}

static unsigned int evas_object_rectangle_visual_id_get(Evas_Object *obj)
{
   Evas_Object_Rectangle *o;

   o = (Evas_Object_Rectangle *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_SHAPE;
}

static void *evas_object_rectangle_engine_data_get(Evas_Object *obj)
{
   Evas_Object_Rectangle *o;

   o = (Evas_Object_Rectangle *)(obj->object_data);
   if (!o) return NULL;
   return o->engine_data;
}


#if 0 /* usless calls for a rect object. much more useful for images etc. */
static void
evas_object_rectangle_store(Evas_Object *obj)
{
   /* store... nothing for rectangle objects... it's a bit silly */
   /* but for others that may have expensive caluclations to do to */
   /* generate the object data, hint that they might want to be pre-calced */
   /* once and stored */
}

static void
evas_object_rectangle_unstore(Evas_Object *obj)
{
   /* store... nothing for rectangle objects... it's a bit silly */
}

static int
evas_object_rectangle_is_visible(Evas_Object *obj)
{
   Evas_Object_Rectangle *o;

   /* this returns 1 if the internal object data would imply that it is */
   /* visible (ie drawing it draws something. this is not to do with events */
   o = (Evas_Object_Rectangle *)(obj->object_data);
   return 1;
}

static int
evas_object_rectangle_was_visible(Evas_Object *obj)
{
   Evas_Object_Rectangle *o;

   /* this returns 1 if the internal object data would imply that it was */
   /* visible (ie drawing it draws something. this is not to do with events */
   o = (Evas_Object_Rectangle *)(obj->object_data);
   return 1;
}

static int
evas_object_rectangle_is_inside(Evas_Object *obj, double x, double y)
{
   Evas_Object_Rectangle *o;

   /* this returns 1 if the canvas co-ordinates are inside the object based */
   /* on object private data. not much use for rects, but for polys, images */
   /* and other complex objects it might be */
   o = (Evas_Object_Rectangle *)(obj->object_data);
   return 1;
}

static int
evas_object_rectangle_was_inside(Evas_Object *obj, double x, double y)
{
   Evas_Object_Rectangle *o;

   /* this returns 1 if the canvas co-ordinates were inside the object based */
   /* on object private data. not much use for rects, but for polys, images */
   /* and other complex objects it might be */
   o = (Evas_Object_Rectangle *)(obj->object_data);
   return 1;
}
#endif

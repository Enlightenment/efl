#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

/* private magic number for gradient objects */
static const char o_type[] = "gradient";

/* private struct for gradient object internal data */
typedef struct _Evas_Object_Gradient      Evas_Object_Gradient;

struct _Evas_Object_Gradient
{
   DATA32            magic;
   
   struct {
      Evas_Angle     angle;
   } cur, prev;
   char              changed : 1;
   char              gradient_changed : 1;
   char              gradient_opaque : 1;
   
   void             *engine_data;
};

/* private methods for gradient objects */
static void evas_object_gradient_init(Evas_Object *obj);
static void *evas_object_gradient_new(void);
static void evas_object_gradient_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_gradient_free(Evas_Object *obj);
static void evas_object_gradient_render_pre(Evas_Object *obj);
static void evas_object_gradient_render_post(Evas_Object *obj);

static int evas_object_gradient_is_opaque(Evas_Object *obj);
static int evas_object_gradient_was_opaque(Evas_Object *obj);

static Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_gradient_free,
     evas_object_gradient_render,
     evas_object_gradient_render_pre,
     evas_object_gradient_render_post,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_gradient_is_opaque,
     evas_object_gradient_was_opaque,
     NULL,
     NULL,
     NULL
};

/* the actual api call to add a rect */
/* it has no other api calls as all properties are standard */

/**
 * @defgroup Evas_Object_Gradient_Group Evas Gradient Object Functions
 *
 * Functions that work on evas gradient objects.
 *
 * The following example shows how
 */

/**
 * Adds a gradient object to the given evas.
 * @param   e The given evas.
 * @return  A new evas gradient object if successful.  Otherwise, @c NULL.
 * @ingroup Evas_Object_Gradient_Group
 */
Evas_Object *
evas_object_gradient_add(Evas *e)
{
   Evas_Object *obj;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new();
   evas_object_gradient_init(obj);
   evas_object_inject(obj, e);
   return obj;
}

/**
 * Adds a colour to the given evas gradient object.
 *
 * The @p distance parameter determines the proportion of the gradient
 * object that is to be set to this colour.  For instance, if red is
 * added with @p distance set to 2, and green is added with @p
 * distance set to 1, two-thirds will be red or reddish and one-third
 * will be green or greenish.
 *
 * Colours are added from the top downwards.
 *
 * @param   obj      The given evas gradient object.
 * @param   r        Red component of the given colour.
 * @param   g        Green component of the given colour.
 * @param   b        Blue component of the given colour.
 * @param   a        Alpha component of the given colour.
 * @param   distance Proportion of the gradient object that is this colour.
 * @ingroup Evas_Object_Gradient_Group
 */
void
evas_object_gradient_color_add(Evas_Object *obj, int r, int g, int b, int a, int distance)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   o->engine_data = obj->layer->evas->engine.func->gradient_color_add(obj->layer->evas->engine.data.output, 
								      obj->layer->evas->engine.data.context,
								      o->engine_data,
								      r, g, b, a, 
								      distance);
   o->gradient_changed = 1;
   if (a != 255) o->gradient_opaque = 0;
   evas_object_change(obj);
}

/**
 * Deletes all colours set for the given evas gradient object.
 * @param obj The given evas gradient object.
 */
void
evas_object_gradient_colors_clear(Evas_Object *obj)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   o->engine_data = obj->layer->evas->engine.func->gradient_colors_clear(obj->layer->evas->engine.data.output, 
									 obj->layer->evas->engine.data.context,
									 o->engine_data);
   o->gradient_changed = 1;
   o->gradient_opaque = 1;
   evas_object_change(obj);
}

/**
 * Sets the angle at which the given evas gradient object sits clockwise
 * from vertical.
 * @param   obj   The given evas gradient object.
 * @param   angle Angle in degrees.  Can be negative.
 * @ingroup Evas_Object_Gradient
 */
void
evas_object_gradient_angle_set(Evas_Object *obj, Evas_Angle angle)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if (angle == o->cur.angle) return;
   o->cur.angle = angle;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the angle at which the given evas gradient object sits
 * clockwise from the vertical.
 * @param   obj The given evas gradient object.
 * @return  The current angle if successful. @c 0.0 otherwise.
 * @ingroup Evas_Object_Gradient
 */
Evas_Angle
evas_object_gradient_angle_get(Evas_Object *obj)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0.0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return 0.0;
   MAGIC_CHECK_END();
   return o->cur.angle;
   evas_object_change(obj);
}





/* all nice and private */
static void
evas_object_gradient_init(Evas_Object *obj)
{
   /* alloc image ob, setup methods and default values */
   obj->object_data = evas_object_gradient_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0.0;
   obj->cur.geometry.y = 0.0;
   obj->cur.geometry.w = 32.0;
   obj->cur.geometry.h = 32.0;
   obj->cur.layer = 0;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->type = o_type;
}

static void *
evas_object_gradient_new(void)
{
   Evas_Object_Gradient *o;
   
   /* alloc obj private data */
   o = calloc(1, sizeof(Evas_Object_Gradient));
   o->magic = MAGIC_OBJ_GRADIENT;
   o->cur.angle = 0.0;
   o->prev = o->cur;
   o->gradient_opaque = 1;
   return o;
}

static void
evas_object_gradient_free(Evas_Object *obj)
{
   Evas_Object_Gradient *o;

   /* frees private object data. very simple here */
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   o->engine_data = obj->layer->evas->engine.func->gradient_colors_clear(obj->layer->evas->engine.data.output, 
									 obj->layer->evas->engine.data.context,
									 o->engine_data);
   o->magic = 0;
   free(o);
}

static void
evas_object_gradient_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Gradient *o;

   /* render object to surface with context, and offxet by x,y */
   o = (Evas_Object_Gradient *)(obj->object_data);
   obj->layer->evas->engine.func->context_color_set(output,
						    context,
						    255, 255, 255, 255);
   if ((obj->cur.cache.clip.r == 255) &&
       (obj->cur.cache.clip.g == 255) &&
       (obj->cur.cache.clip.b == 255) &&
       (obj->cur.cache.clip.a == 255))
     obj->layer->evas->engine.func->context_multiplier_unset(output,
							     context);
   else
     obj->layer->evas->engine.func->context_multiplier_set(output,
							   context,
							   obj->cur.cache.clip.r,
							   obj->cur.cache.clip.g,
							   obj->cur.cache.clip.b,
							   obj->cur.cache.clip.a);
   if (o->engine_data)
     obj->layer->evas->engine.func->gradient_draw(output,
						  context,
						  surface,
						  o->engine_data,
						  obj->cur.cache.geometry.x + x,
						  obj->cur.cache.geometry.y + y,
						  obj->cur.cache.geometry.w,
						  obj->cur.cache.geometry.h,
						  o->cur.angle);
}

static void
evas_object_gradient_render_pre(Evas_Object *obj)
{
   Evas_List *updates = NULL;
   Evas_Object_Gradient *o;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add thr appropriate redraw rectangles */
   o = (Evas_Object_Gradient *)(obj->object_data);
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
	evas_object_clip_recalc(obj->cur.clipper);
	obj->cur.clipper->func->render_pre(obj->cur.clipper);
     }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);
   if (is_v != was_v)
     {
	updates = evas_object_render_pre_visible_change(updates, obj, is_v, was_v);
	goto done;
     }
   /* its not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   updates = evas_object_render_pre_clipper_change(updates, obj);
   /* if we restacked (layer or just within a layer) and dont clip anyone */
   if (obj->restack)
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
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
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   /* angle changed */
   if ((o->changed) && (o->cur.angle != o->prev.angle))
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   /* angle changed */
   if (o->gradient_changed)
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   /* it obviously didn't change - add a NO obscure - this "unupdates"  this */
   /* area so if there were updates for it they get wiped. don't do it if we */
   /* arent fully opaque and we are visible */
   if (evas_object_is_visible(obj) &&
       evas_object_is_opaque(obj))
     obj->layer->evas->engine.func->output_redraws_rect_del(obj->layer->evas->engine.data.output,
							    obj->cur.cache.clip.x, 
							    obj->cur.cache.clip.y, 
							    obj->cur.cache.clip.w, 
							    obj->cur.cache.clip.h);
   done:
   evas_object_render_pre_effect_updates(updates, obj, is_v, was_v);
}

static void
evas_object_gradient_render_post(Evas_Object *obj)
{
   Evas_Object_Gradient *o;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Gradient *)(obj->object_data);
   /* remove those pesky changes */
   while (obj->clip.changes)
     {
	Evas_Rectangle *r;
	
	r = (Evas_Rectangle *)obj->clip.changes->data;
	obj->clip.changes = evas_list_remove(obj->clip.changes, r);
	free(r);
     }
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
   o->prev = o->cur;
   o->changed = 0;
   o->gradient_changed = 0;
}

static int
evas_object_gradient_is_opaque(Evas_Object *obj)
{
   Evas_Object_Gradient *o;

   /* this returns 1 if the internal object data implies that the object is */
   /* currently fulyl opque over the entire gradient it occupies */
   o = (Evas_Object_Gradient *)(obj->object_data);
   return o->gradient_opaque;
}

static int
evas_object_gradient_was_opaque(Evas_Object *obj)
{
   Evas_Object_Gradient *o;
   
   /* this returns 1 if the internal object data implies that the object was */
   /* currently fulyl opque over the entire gradient it occupies */
   o = (Evas_Object_Gradient *)(obj->object_data);
   return o->gradient_opaque;
}

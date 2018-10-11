#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EFL_CANVAS_RECTANGLE_CLASS

/* private magic number for rectangle objects */
static const char o_type[] = "rectangle";

const char *o_rect_type = o_type;

/* private struct for rectangle object internal data */
typedef struct _Efl_Canvas_Rectangle_Data Efl_Canvas_Rectangle_Data;

struct _Efl_Canvas_Rectangle_Data
{
   void             *engine_data;
};

/* private methods for rectangle objects */
static void evas_object_rectangle_init(Evas_Object *eo_obj);
static void evas_object_rectangle_render(Evas_Object *eo_obj,
                                         Evas_Object_Protected_Data *obj,
                                         void *type_private_data,
                                         void *engine, void *output, void *context, void *surface,
                                         int x, int y, Eina_Bool do_async);
static void evas_object_rectangle_render_pre(Evas_Object *eo_obj,
					     Evas_Object_Protected_Data *obj,
					     void *type_private_data);
static void evas_object_rectangle_render_post(Evas_Object *eo_obj,
					      Evas_Object_Protected_Data *obj,
					      void *type_private_data);

static void *evas_object_rectangle_engine_data_get(Evas_Object *eo_obj);

static int evas_object_rectangle_is_opaque(Evas_Object *eo_obj,
					   Evas_Object_Protected_Data *obj,
					   void *type_private_data);
static int evas_object_rectangle_was_opaque(Evas_Object *eo_obj,
					    Evas_Object_Protected_Data *obj,
					    void *type_private_data);


#if 0 /* usless calls for a rect object. much more useful for images etc. */
static void evas_object_rectangle_store(Evas_Object *eo_obj);
static void evas_object_rectangle_unstore(Evas_Object *eo_obj);
static int evas_object_rectangle_is_visible(Evas_Object *eo_obj);
static int evas_object_rectangle_was_visible(Evas_Object *eo_obj);
static int evas_object_rectangle_is_inside(Evas_Object *eo_obj, double x, double y);
static int evas_object_rectangle_was_inside(Evas_Object *eo_obj, double x, double y);
#endif

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   NULL,
     evas_object_rectangle_render,
     evas_object_rectangle_render_pre,
     evas_object_rectangle_render_post,
     evas_object_rectangle_engine_data_get,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     evas_object_rectangle_is_opaque,
     evas_object_rectangle_was_opaque,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL, // render_prepare
};

/* the actual api call to add a rect */
/* it has no other api calls as all properties are standard */

EAPI Evas_Object *
evas_object_rectangle_add(Evas *e)
{
   e = evas_find(e);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(e, EVAS_CANVAS_CLASS), NULL);
   return efl_add(EFL_CANVAS_RECTANGLE_CLASS, e, efl_canvas_object_legacy_ctor(efl_added));
}

EOLIAN static Eo *
_efl_canvas_rectangle_efl_object_constructor(Eo *eo_obj, Efl_Canvas_Rectangle_Data *class_data EINA_UNUSED)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   evas_object_rectangle_init(eo_obj);

   return eo_obj;
}

/* all nice and private */
static void
evas_object_rectangle_init(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->private_data = efl_data_ref(eo_obj, MY_CLASS);
   obj->type = o_type;
}

static void
evas_object_rectangle_render(Evas_Object *eo_obj EINA_UNUSED,
                             Evas_Object_Protected_Data *obj,
                             void *type_private_data EINA_UNUSED,
                             void *engine, void *output, void *context, void *surface, int x, int y, Eina_Bool do_async)
{
   /* render object to surface with context, and offxet by x,y */
   obj->layer->evas->engine.func->context_color_set(engine,
                                                    context,
                                                    obj->cur->cache.clip.r,
                                                    obj->cur->cache.clip.g,
                                                    obj->cur->cache.clip.b,
                                                    obj->cur->cache.clip.a);
   obj->layer->evas->engine.func->context_anti_alias_set(engine, context,
                                                         obj->cur->anti_alias);
   obj->layer->evas->engine.func->context_multiplier_unset(engine, context);
   obj->layer->evas->engine.func->context_render_op_set(engine, context,
                                                        obj->cur->render_op);
   obj->layer->evas->engine.func->rectangle_draw(engine,
                                                 output,
                                                 context,
                                                 surface,
                                                 obj->cur->geometry.x + x,
                                                 obj->cur->geometry.y + y,
                                                 obj->cur->geometry.w,
                                                 obj->cur->geometry.h,
                                                 do_async);
}

static void
evas_object_rectangle_render_pre(Evas_Object *eo_obj,
				 Evas_Object_Protected_Data *obj,
				 void *type_private_data EINA_UNUSED)
{
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = EINA_TRUE;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw rectangles */
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur->clipper)
     {
        if (obj->cur->cache.clip.dirty)
          evas_object_clip_recalc(obj->cur->clipper);
        obj->cur->clipper->func->render_pre(obj->cur->clipper->object,
					    obj->cur->clipper,
					    obj->cur->clipper->private_data);
     }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(eo_obj, obj);
   was_v = evas_object_was_visible(eo_obj,obj);
   if (!(is_v | was_v)) goto done;
   if (is_v != was_v)
     {
	evas_object_render_pre_visible_change(&obj->layer->evas->clip_changes, eo_obj, is_v, was_v);
	goto done;
     }
   if (obj->changed_map || obj->changed_src_visible)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
        goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&obj->layer->evas->clip_changes, eo_obj);
   /* if we restacked (layer or just within a layer) and don't clip anyone */
   if ((obj->restack) && (!obj->clip.clipees))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
	goto done;
     }
   /* if it changed render op */
   if (obj->cur->render_op != obj->prev->render_op)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
	goto done;
     }
   /* if it changed color */
   if ((obj->cur->color.r != obj->prev->color.r) ||
       (obj->cur->color.g != obj->prev->color.g) ||
       (obj->cur->color.b != obj->prev->color.b) ||
       (obj->cur->color.a != obj->prev->color.a))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
	goto done;
     }
   /* it obviously didn't change - add a NO obscure - this "unupdates"  this */
   /* area so if there were updates for it they get wiped. don't do it if we */
   /* arent fully opaque and we are visible */
   if (evas_object_is_visible(eo_obj, obj) &&
       evas_object_is_opaque(eo_obj, obj) &&
       (!obj->clip.clipees))
     {
        Evas_Coord x, y, w, h;
        
        x = obj->cur->cache.clip.x;
        y = obj->cur->cache.clip.y;
        w = obj->cur->cache.clip.w;
        h = obj->cur->cache.clip.h;
        if (obj->cur->clipper)
          {
             RECTS_CLIP_TO_RECT(x, y, w, h,
                                obj->cur->clipper->cur->cache.clip.x,
                                obj->cur->clipper->cur->cache.clip.y,
                                obj->cur->clipper->cur->cache.clip.w,
                                obj->cur->clipper->cur->cache.clip.h);
          }
        evas_render_update_del(obj->layer->evas,
                               x + obj->layer->evas->framespace.x,
                               y + obj->layer->evas->framespace.y,
                               w, h);
     }
   /* if it changed geometry - and obviously not visibility or color */
   /* calculate differences since we have a constant color fill */
   /* we really only need to update the differences */
   if ((obj->cur->geometry.x != obj->prev->geometry.x) ||
       (obj->cur->geometry.y != obj->prev->geometry.y) ||
       (obj->cur->geometry.w != obj->prev->geometry.w) ||
       (obj->cur->geometry.h != obj->prev->geometry.h))
     {
	evas_rects_return_difference_rects(&obj->layer->evas->clip_changes,
					   obj->cur->geometry.x,
					   obj->cur->geometry.y,
					   obj->cur->geometry.w,
					   obj->cur->geometry.h,
					   obj->prev->geometry.x,
					   obj->prev->geometry.y,
					   obj->prev->geometry.w,
					   obj->prev->geometry.h);
////	rl = evas_rects_return_difference_rects(obj->cur->cache.geometry.x,
////						obj->cur->cache.geometry.y,
////						obj->cur->cache.geometry.w,
////						obj->cur->cache.geometry.h,
////						obj->prev->cache.geometry.x,
////						obj->prev->cache.geometry.y,
////						obj->prev->cache.geometry.w,
////						obj->prev->cache.geometry.h);
	goto done;
     }
   done:
   evas_object_render_pre_effect_updates(&obj->layer->evas->clip_changes, eo_obj, is_v, was_v);
}

static void
evas_object_rectangle_render_post(Evas_Object *eo_obj EINA_UNUSED,
                                  Evas_Object_Protected_Data *obj,
                                  void *type_private_data EINA_UNUSED)
{

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   /* remove those pesky changes */
   evas_object_clip_changes_clean(obj);
   /* move cur to prev safely for object data */
   evas_object_cur_prev(obj);
}

static int
evas_object_rectangle_is_opaque(Evas_Object *eo_obj EINA_UNUSED,
				Evas_Object_Protected_Data *obj,
				void *type_private_data EINA_UNUSED)
{
   /* this returns 1 if the internal object data implies that the object is */
   /* currently fully opaque over the entire rectangle it occupies */
   if ((obj->map->cur.map) && (obj->map->cur.usemap)) return 0;
   if (obj->cur->render_op == EVAS_RENDER_COPY)
	return 1;
   if (obj->cur->render_op != EVAS_RENDER_BLEND)
	return 0;
   return (obj->cur->cache.clip.a == 255) ? 1 : 0;
}

static int
evas_object_rectangle_was_opaque(Evas_Object *eo_obj EINA_UNUSED,
				 Evas_Object_Protected_Data *obj,
				 void *type_private_data EINA_UNUSED)
{
   /* this returns 1 if the internal object data implies that the object was */
   /* previously fully opaque over the entire rectangle it occupies */
   if (obj->prev->render_op == EVAS_RENDER_COPY)
	return 1;
   if (obj->prev->render_op != EVAS_RENDER_BLEND)
	return 0;
   return (obj->prev->cache.clip.a == 255) ? 1 : 0;
}

static void *evas_object_rectangle_engine_data_get(Evas_Object *eo_obj)
{
   Efl_Canvas_Rectangle_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   return o->engine_data;
}

#if 0 /* usless calls for a rect object. much more useful for images etc. */
static void
evas_object_rectangle_store(Evas_Object *eo_obj)
{
   /* store... nothing for rectangle objects... it's a bit silly */
   /* but for others that may have expensive caluclations to do to */
   /* generate the object data, hint that they might want to be pre-calced */
   /* once and stored */
}

static void
evas_object_rectangle_unstore(Evas_Object *eo_obj)
{
   /* store... nothing for rectangle objects... it's a bit silly */
}

static int
evas_object_rectangle_is_visible(Evas_Object *eo_obj)
{
   /* this returns 1 if the internal object data would imply that it is */
   /* visible (ie drawing it draws something. this is not to do with events */
   return 1;
}

static int
evas_object_rectangle_was_visible(Evas_Object *eo_obj)
{
   /* this returns 1 if the internal object data would imply that it was */
   /* visible (ie drawing it draws something. this is not to do with events */
   return 1;
}

static int
evas_object_rectangle_is_inside(Evas_Object *eo_obj, double x, double y)
{
   /* this returns 1 if the canvas co-ordinates are inside the object based */
   /* on object private data. not much use for rects, but for polys, images */
   /* and other complex objects it might be */
   return 1;
}

static int
evas_object_rectangle_was_inside(Evas_Object *eo_obj, double x, double y)
{
   /* this returns 1 if the canvas co-ordinates were inside the object based */
   /* on object private data. not much use for rects, but for polys, images */
   /* and other complex objects it might be */
   return 1;
}
#endif

#include "canvas/efl_canvas_rectangle.eo.c"

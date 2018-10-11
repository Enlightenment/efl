#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EVAS_LINE_CLASS

/* private magic number for line objects */
static const char o_type[] = "line";

/* private struct for line object internal data */
typedef struct _Evas_Line_Data      Evas_Line_Data;

struct _Evas_Line_Data
{
   struct {
      struct {
         Evas_Coord    x1, y1, x2, y2;
         struct {
            Evas_Coord w, h;
         } object;
      } cache;
      Evas_Coord     x1, y1, x2, y2;
   } cur, prev;

   void             *engine_data;
   Eina_Bool         changed : 1;
};

/* private methods for line objects */
static void evas_object_line_init(Evas_Object *eo_obj);
static void evas_object_line_render(Evas_Object *eo_obj,
                                    Evas_Object_Protected_Data *obj,
                                    void *type_private_data,
                                    void *engine, void *output, void *context, void *surface,
                                    int x, int y, Eina_Bool do_async);
static void evas_object_line_render_pre(Evas_Object *eo_obj,
					Evas_Object_Protected_Data *obj,
					void *type_private_data);
static void evas_object_line_render_post(Evas_Object *eo_obj,
					 Evas_Object_Protected_Data *obj,
					 void *type_private_data);

static void *evas_object_line_engine_data_get(Evas_Object *eo_obj);

static int evas_object_line_is_opaque(Evas_Object *eo_obj,
				      Evas_Object_Protected_Data *obj,
				      void *type_private_data);
static int evas_object_line_was_opaque(Evas_Object *eo_obj,
				       Evas_Object_Protected_Data *obj,
				       void *type_private_data);
static int evas_object_line_is_inside(Evas_Object *eo_obj,
				      Evas_Object_Protected_Data *obj,
				      void *type_private_data,
				      Evas_Coord x, Evas_Coord y);
static int evas_object_line_was_inside(Evas_Object *eo_obj,
				       Evas_Object_Protected_Data *obj,
				       void *type_private_data,
				       Evas_Coord x, Evas_Coord y);
static void evas_object_line_coords_recalc(Evas_Object *eo_obj,
					   Evas_Object_Protected_Data *obj,
					   void *type_private_data);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   NULL,
   evas_object_line_render,
   evas_object_line_render_pre,
   evas_object_line_render_post,
   evas_object_line_engine_data_get,
   /* these are optional. NULL = nothing */
   NULL,
   NULL,
   evas_object_line_is_opaque,
   evas_object_line_was_opaque,
   evas_object_line_is_inside,
   evas_object_line_was_inside,
   evas_object_line_coords_recalc,
   NULL,
   NULL,
   NULL,
   NULL // render_prepare
};

/* the actual api call to add a rect */
/* it has no other api calls as all properties are standard */

EAPI Evas_Object *
evas_object_line_add(Evas *e)
{
   e = evas_find(e);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(e, EVAS_CANVAS_CLASS), NULL);
   return efl_add(EVAS_LINE_CLASS, e, efl_canvas_object_legacy_ctor(efl_added));
}

EOLIAN static void
_evas_line_xy_set(Eo *eo_obj, Evas_Line_Data *_pd, Evas_Coord x1, Evas_Coord y1, Evas_Coord x2, Evas_Coord y2)
{

   Evas_Line_Data *o = _pd;
   Evas_Coord min_x, max_x, min_y, max_y;
   Eina_List *was = NULL;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if ((x1 == (obj->cur->geometry.x + o->cur.x1)) &&
       (y1 == (obj->cur->geometry.y + o->cur.y1)) &&
       (x2 == (obj->cur->geometry.x + o->cur.x2)) &&
       (y2 == (obj->cur->geometry.y + o->cur.y2))) return;
   evas_object_async_block(obj);

   if (!(obj->layer->evas->is_frozen))
     {
        if (!evas_event_passes_through(eo_obj, obj) &&
            !evas_event_freezes_through(eo_obj, obj) &&
            !evas_object_is_source_invisible(eo_obj, obj))
          was = _evas_pointer_list_in_rect_get(obj->layer->evas, eo_obj, obj,
                                               1, 1);
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

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
       state_write->geometry.x = min_x;
       state_write->geometry.y = min_y;
       state_write->geometry.w = max_x - min_x + 2;
       state_write->geometry.h = max_y - min_y + 2;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

////   obj->cur->cache.geometry.validity = 0;
   o->cur.x1 = x1 - min_x;
   o->cur.y1 = y1 - min_y;
   o->cur.x2 = x2 - min_x;
   o->cur.y2 = y2 - min_y;
   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_coords_recalc(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   if (!(obj->layer->evas->is_frozen) &&
       !evas_event_passes_through(eo_obj, obj) &&
       !evas_event_freezes_through(eo_obj, obj) &&
       !evas_object_is_source_invisible(eo_obj, obj) &&
       obj->cur->visible)
     _evas_canvas_event_pointer_in_list_mouse_move_feed(obj->layer->evas, was, eo_obj, obj, 1, 1, EINA_TRUE, NULL);
   eina_list_free(was);
   evas_object_inform_call_move(eo_obj, obj);
   evas_object_inform_call_resize(eo_obj, obj);
}

EOLIAN static void
_evas_line_xy_get(const Eo *eo_obj, Evas_Line_Data *_pd, Evas_Coord *x1, Evas_Coord *y1, Evas_Coord *x2, Evas_Coord *y2)
{
   const Evas_Line_Data *o = _pd;


   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   if (x1) *x1 = obj->cur->geometry.x + o->cur.x1;
   if (y1) *y1 = obj->cur->geometry.y + o->cur.y1;
   if (x2) *x2 = obj->cur->geometry.x + o->cur.x2;
   if (y2) *y2 = obj->cur->geometry.y + o->cur.y2;
}

/* all nice and private */
static void
evas_object_line_init(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->private_data = efl_data_ref(eo_obj, MY_CLASS);
   obj->type = o_type;
}

EOLIAN static Eo *
_evas_line_efl_object_constructor(Eo *eo_obj, Evas_Line_Data *class_data EINA_UNUSED)
{
   Evas_Line_Data *o;

   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   evas_object_line_init(eo_obj);

   o = class_data;
   /* alloc obj private data */
   o->cur.x1 = 0;
   o->cur.y1 = 0;
   o->cur.x2 = 31;
   o->cur.y2 = 31;
   o->prev = o->cur;

   return eo_obj;
}

static void
evas_object_line_render(Evas_Object *eo_obj EINA_UNUSED,
                        Evas_Object_Protected_Data *obj,
                        void *type_private_data,
                        void *engine, void *output, void *context, void *surface,
                        int x, int y, Eina_Bool do_async)
{
   Evas_Line_Data *o = type_private_data;

   /* render object to surface with context, and offxet by x,y */

   obj->layer->evas->engine.func->context_color_set(engine,
                                                    context,
                                                    obj->cur->cache.clip.r,
                                                    obj->cur->cache.clip.g,
                                                    obj->cur->cache.clip.b,
                                                    obj->cur->cache.clip.a);
   obj->layer->evas->engine.func->context_multiplier_unset(engine, context);
   obj->layer->evas->engine.func->context_anti_alias_set(engine, context,
                                                         obj->cur->anti_alias);
   obj->layer->evas->engine.func->context_render_op_set(engine, context,
                                                        obj->cur->render_op);
   obj->layer->evas->engine.func->line_draw(engine, output,
                                            context,
                                            surface,
                                            o->cur.cache.x1 + x,
                                            o->cur.cache.y1 + y,
                                            o->cur.cache.x2 + x,
                                            o->cur.cache.y2 + y,
                                            do_async);
}

static void
evas_object_line_render_pre(Evas_Object *eo_obj,
                            Evas_Object_Protected_Data *obj,
                            void *type_private_data)
{
   Evas_Line_Data *o = type_private_data;
   int is_v, was_v;
   Eina_Bool changed_color = EINA_FALSE;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = EINA_TRUE;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw lines */
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
   was_v = evas_object_was_visible(eo_obj, obj);
   if (is_v != was_v)
     {
	evas_object_render_pre_visible_change(&obj->layer->evas->clip_changes, eo_obj, is_v, was_v);
	goto done;
     }
   if (obj->changed_map || obj->changed_src_visible)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj,
                                            obj);
        goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&obj->layer->evas->clip_changes, eo_obj);

   if ((obj->cur->color.r != obj->prev->color.r) ||
       (obj->cur->color.g != obj->prev->color.g) ||
       (obj->cur->color.b != obj->prev->color.b) ||
       (obj->cur->color.a != obj->prev->color.a))
     changed_color = EINA_TRUE;

   /* if we restacked (layer or just within a layer) */
   /* or if it changed anti_alias */
   /* or if ii changed render op */
   /* or if it changed color */
   if ((obj->restack) ||
       (obj->cur->anti_alias != obj->prev->anti_alias) ||
       (obj->cur->render_op != obj->prev->render_op) ||
       (changed_color)
      )
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        goto done;
     }

   /* if it changed geometry - and obviously not visibility or color */
   /* calculate differences since we have a constant color fill */
   /* we really only need to update the differences */
   if ((obj->cur->geometry.x != obj->prev->geometry.x) ||
       (obj->cur->geometry.y != obj->prev->geometry.y) ||
       (obj->cur->geometry.w != obj->prev->geometry.w) ||
       (obj->cur->geometry.h != obj->prev->geometry.h) ||
       ((o->changed) &&
        ((o->cur.x1 != o->prev.x1) ||
         (o->cur.y1 != o->prev.y1) ||
         (o->cur.x2 != o->prev.x2) ||
         (o->cur.y2 != o->prev.y2)))
      )
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
	goto done;
     }
   done:
   evas_object_render_pre_effect_updates(&obj->layer->evas->clip_changes, eo_obj, is_v, was_v);
}

static void
evas_object_line_render_post(Evas_Object *eo_obj EINA_UNUSED,
                             Evas_Object_Protected_Data *obj,
                             void *type_private_data)
{
   Evas_Line_Data *o = type_private_data;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   /* remove those pesky changes */
   evas_object_clip_changes_clean(obj);
   /* move cur to prev safely for object data */
   evas_object_cur_prev(obj);
   o->prev = o->cur;
}

static void *evas_object_line_engine_data_get(Evas_Object *eo_obj)
{
   Evas_Line_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   return o->engine_data;
}

static int
evas_object_line_is_opaque(Evas_Object *eo_obj EINA_UNUSED,
                           Evas_Object_Protected_Data *obj EINA_UNUSED,
                           void *type_private_data EINA_UNUSED)
{
   /* this returns 1 if the internal object data implies that the object is */
   /* currently fully opaque over the entire line it occupies */
   return 0;
}

static int
evas_object_line_was_opaque(Evas_Object *eo_obj EINA_UNUSED,
                            Evas_Object_Protected_Data *obj EINA_UNUSED,
                            void *type_private_data EINA_UNUSED)
{
   /* this returns 1 if the internal object data implies that the object was */
   /* previously fully opaque over the entire line it occupies */
   return 0;
}

static int
evas_object_line_is_inside(Evas_Object *eo_obj EINA_UNUSED,
                           Evas_Object_Protected_Data *obj EINA_UNUSED,
                           void *type_private_data EINA_UNUSED,
                           Evas_Coord x EINA_UNUSED, Evas_Coord y EINA_UNUSED)
{
   /* this returns 1 if the canvas co-ordinates are inside the object based */
   /* on object private data. not much use for rects, but for polys, images */
   /* and other complex objects it might be */
   return 1;
}

static int
evas_object_line_was_inside(Evas_Object *eo_obj EINA_UNUSED,
                            Evas_Object_Protected_Data *obj EINA_UNUSED,
                            void *type_private_data EINA_UNUSED,
                            Evas_Coord x EINA_UNUSED, Evas_Coord y EINA_UNUSED)
{
   /* this returns 1 if the canvas co-ordinates were inside the object based */
   /* on object private data. not much use for rects, but for polys, images */
   /* and other complex objects it might be */
   return 1;
}

static void
evas_object_line_coords_recalc(Evas_Object *eo_obj EINA_UNUSED,
                               Evas_Object_Protected_Data *obj,
                               void *type_private_data)
{
   Evas_Line_Data *o = type_private_data;

   o->cur.cache.x1 = obj->cur->geometry.x + o->cur.x1;
   o->cur.cache.y1 = obj->cur->geometry.y + o->cur.y1;
   o->cur.cache.x2 = obj->cur->geometry.x + o->cur.x2;
   o->cur.cache.y2 = obj->cur->geometry.y + o->cur.y2;
   o->cur.cache.object.w = obj->cur->geometry.w;
   o->cur.cache.object.h = obj->cur->geometry.h;
}

#include "canvas/evas_line.eo.c"

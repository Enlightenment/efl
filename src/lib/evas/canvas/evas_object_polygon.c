#include "evas_common_private.h"
#include "evas_private.h"

#include "Eo.h"

#define MY_CLASS EVAS_OBJ_POLYGON_CLASS

/* private magic number for polygon objects */
static const char o_type[] = "polygon";

/* private struct for line object internal data */
typedef struct _Evas_Polygon_Data      Evas_Polygon_Data;
typedef struct _Evas_Polygon_Point       Evas_Polygon_Point;

struct _Evas_Polygon_Data
{
   Eina_List           *points;
   void                *engine_data;
   struct {
      int x, y;
   } offset;
   Evas_Coord_Rectangle geometry;
   char                 changed : 1;
};

struct _Evas_Polygon_Point
{
   Evas_Coord x, y;
};

/* private methods for polygon objects */
static void evas_object_polygon_init(Evas_Object *eo_obj);
static void evas_object_polygon_render(Evas_Object *eo_obj,
                                       Evas_Object_Protected_Data *obj,
                                       void *type_private_data,
                                       void *output, void *context, void *surface,
                                       int x, int y, Eina_Bool do_async);
static void evas_object_polygon_free(Evas_Object *eo_obj,
                                     Evas_Object_Protected_Data *obj,
                                     void *type_private_data);
static void evas_object_polygon_render_pre(Evas_Object *eo_obj,
                                           Evas_Object_Protected_Data *obj,
                                           void *type_private_data);
static void evas_object_polygon_render_post(Evas_Object *eo_obj,
                                            Evas_Object_Protected_Data *obj,
                                            void *type_private_data);

static unsigned int evas_object_polygon_id_get(Evas_Object *eo_obj);
static unsigned int evas_object_polygon_visual_id_get(Evas_Object *eo_obj);
static void *evas_object_polygon_engine_data_get(Evas_Object *eo_obj);

static int evas_object_polygon_is_opaque(Evas_Object *eo_obj,
                                         Evas_Object_Protected_Data *obj,
                                         void *type_private_data);
static int evas_object_polygon_was_opaque(Evas_Object *eo_obj,
                                          Evas_Object_Protected_Data *obj,
                                          void *type_private_data);
static int evas_object_polygon_is_inside(Evas_Object *eo_obj,
                                         Evas_Object_Protected_Data *obj,
                                         void *type_private_data,
                                         Evas_Coord x, Evas_Coord y);
static int evas_object_polygon_was_inside(Evas_Object *eo_obj,
                                          Evas_Object_Protected_Data *obj,
                                          void *type_private_data,
                                          Evas_Coord x, Evas_Coord y);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_polygon_free,
     evas_object_polygon_render,
     evas_object_polygon_render_pre,
     evas_object_polygon_render_post,
     evas_object_polygon_id_get,
     evas_object_polygon_visual_id_get,
     evas_object_polygon_engine_data_get,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_polygon_is_opaque,
     evas_object_polygon_was_opaque,
     evas_object_polygon_is_inside,
     evas_object_polygon_was_inside,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL
};

/* the actual api call to add a rect */
/* it has no other api calls as all properties are standard */

EAPI Evas_Object *
evas_object_polygon_add(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *eo_obj = eo_add(EVAS_OBJ_POLYGON_CLASS, e);
   eo_unref(eo_obj);
   return eo_obj;
}

EOLIAN static void
_evas_polygon_eo_base_constructor(Eo *eo_obj, Evas_Polygon_Data *class_data EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj;
   Eo *parent;

   eo_do_super(eo_obj, MY_CLASS, eo_constructor());

   obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_polygon_init(eo_obj);
   eo_do(eo_obj, eo_parent_get(&parent));
   evas_object_inject(eo_obj, obj, evas_object_evas_get(parent));
}

EOLIAN static void
_evas_polygon_point_add(Eo *eo_obj, Evas_Polygon_Data *_pd, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Polygon_Data *o = _pd;
   Evas_Polygon_Point *p;
   Evas_Coord min_x, max_x, min_y, max_y;
   int is, was = 0;


   if (!obj->layer->evas->is_frozen)
     {
        if (!evas_event_passes_through(eo_obj, obj) &&
            !evas_event_freezes_through(eo_obj, obj) &&
            !evas_object_is_source_invisible(eo_obj, obj))
          was = evas_object_is_in_output_rect(eo_obj, obj,
                                              obj->layer->evas->pointer.x,
                                              obj->layer->evas->pointer.y,
                                              1, 1);
     }
   if (!o->points)
     {
        o->offset.x = obj->cur->geometry.x;
        o->offset.y = obj->cur->geometry.y;
     }
   else
     {
        /* Update all points and take offset into account. */
        Eina_List *over;

        EINA_LIST_FOREACH(o->points, over, p)
          {
             p->x += o->offset.x;
             p->y += o->offset.y;
          }
     }

   p = malloc(sizeof(Evas_Polygon_Point));
   if (!p) return;
   p->x = x + o->offset.x;
   p->y = y + o->offset.y;

   if (!o->points)
     {
        EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
          {
             state_write->geometry.x = p->x;
             state_write->geometry.y = p->y;
             state_write->geometry.w = 2;
             state_write->geometry.h = 2;
          }
        EINA_COW_STATE_WRITE_END(obj, state_write, cur);
     }
   else
     {
        if (p->x < obj->cur->geometry.x) min_x = p->x;
        else min_x = obj->cur->geometry.x;
        if (p->x > (obj->cur->geometry.x + obj->cur->geometry.w - 2))
          max_x = p->x;
        else max_x = obj->cur->geometry.x + obj->cur->geometry.w - 2;
        if (p->y < obj->cur->geometry.y) min_y = p->y;
        else min_y = obj->cur->geometry.y;
        if (p->y > (obj->cur->geometry.y + obj->cur->geometry.h - 2))
          max_y = p->y;
        else max_y = obj->cur->geometry.y + obj->cur->geometry.h - 2;

        EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
          {
             state_write->geometry.x = min_x;
             state_write->geometry.y = min_y;
             state_write->geometry.w = max_x - min_x + 2;
             state_write->geometry.h = max_y - min_y + 2;
          }
        EINA_COW_STATE_WRITE_END(obj, state_write, cur);
     }
   o->points = eina_list_append(o->points, p);

   o->geometry = obj->cur->geometry;
   o->offset.x = 0;
   o->offset.y = 0;

   ////   obj->cur->cache.geometry.validity = 0;
   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_coords_recalc(eo_obj, obj);
   if (!obj->layer->evas->is_frozen)
     {
        is = evas_object_is_in_output_rect(eo_obj, obj,
                                           obj->layer->evas->pointer.x,
                                           obj->layer->evas->pointer.y, 1, 1);
        if (!evas_event_passes_through(eo_obj, obj) &&
            !evas_event_freezes_through(eo_obj, obj) &&
            !evas_object_is_source_invisible(eo_obj, obj))
          {
             if ((is ^ was) && obj->cur->visible)
               evas_event_feed_mouse_move(obj->layer->evas->evas,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y,
                                          obj->layer->evas->last_timestamp,
                                          NULL);
          }
     }
   evas_object_inform_call_move(eo_obj, obj);
   evas_object_inform_call_resize(eo_obj);
}

EOLIAN static void
_evas_polygon_points_clear(Eo *eo_obj, Evas_Polygon_Data *_pd)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Polygon_Data *o = _pd;
   void *list_data;
   int is, was;

   was = evas_object_is_in_output_rect(eo_obj, obj,
                                       obj->layer->evas->pointer.x,
                                       obj->layer->evas->pointer.y, 1, 1);
   EINA_LIST_FREE(o->points, list_data)
     {
        free(list_data);
     }

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->geometry.x = 0;
        state_write->geometry.y = 0;
        state_write->geometry.w = 0;
        state_write->geometry.h = 0;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   ////   obj->cur->cache.geometry.validity = 0;
   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_coords_recalc(eo_obj, obj);
   is = evas_object_is_in_output_rect(eo_obj, obj,
                                      obj->layer->evas->pointer.x,
                                      obj->layer->evas->pointer.y, 1, 1);
   if ((is || was) && obj->cur->visible)
     evas_event_feed_mouse_move(obj->layer->evas->evas,
                                obj->layer->evas->pointer.x,
                                obj->layer->evas->pointer.y,
                                obj->layer->evas->last_timestamp,
                                NULL);
   evas_object_inform_call_move(eo_obj, obj);
   evas_object_inform_call_resize(eo_obj);
}

/* all nice and private */
static void
evas_object_polygon_init(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->private_data = eo_data_ref(eo_obj, MY_CLASS);
   obj->type = o_type;
}

EOLIAN static void
_evas_polygon_eo_base_destructor(Eo *eo_obj, Evas_Polygon_Data *_pd EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);

   evas_object_polygon_free(eo_obj, obj, obj->private_data);
   eo_do_super(eo_obj, MY_CLASS, eo_destructor());
}

static void
evas_object_polygon_free(Evas_Object *eo_obj EINA_UNUSED,
                         Evas_Object_Protected_Data *obj,
                         void *type_private_data)
{
   Evas_Polygon_Data *o = type_private_data;
   void *list_data;
   /* free obj */
   EINA_LIST_FREE(o->points, list_data)
     {
        free(list_data);
     }
   o->engine_data = obj->layer->evas->engine.func->polygon_points_clear(obj->layer->evas->engine.data.output,
                                                                        obj->layer->evas->engine.data.context,
                                                                        o->engine_data);
}

static void
evas_object_polygon_render(Evas_Object *eo_obj EINA_UNUSED,
			   Evas_Object_Protected_Data *obj,
			   void *type_private_data,
			   void *output, void *context, void *surface, int x, int y, Eina_Bool do_async)
{
   Evas_Polygon_Data *o = type_private_data;
   Eina_List *l;
   Evas_Polygon_Point *p;

   /* render object to surface with context, and offxet by x,y */
   obj->layer->evas->engine.func->context_color_set(output,
                                                    context,
                                                    obj->cur->cache.clip.r,
                                                    obj->cur->cache.clip.g,
                                                    obj->cur->cache.clip.b,
                                                    obj->cur->cache.clip.a);
   obj->layer->evas->engine.func->context_multiplier_unset(output,
                                                           context);
   obj->layer->evas->engine.func->context_render_op_set(output, context,
                                                        obj->cur->render_op);
   if (o->changed)
     {
        o->engine_data = obj->layer->evas->engine.func->polygon_points_clear(obj->layer->evas->engine.data.output,
                                                                             obj->layer->evas->engine.data.context,
                                                                             o->engine_data);
        EINA_LIST_FOREACH(o->points, l, p)
          {
             //px = evas_coord_world_x_to_screen(obj->layer->evas, p->x);
             //py = evas_coord_world_y_to_screen(obj->layer->evas, p->y);
             o->engine_data = obj->layer->evas->engine.func->polygon_point_add(obj->layer->evas->engine.data.output,
                                                                               obj->layer->evas->engine.data.context,
                                                                               o->engine_data,
                                                                               p->x, p->y);
          }
     }

   if (o->engine_data)
     obj->layer->evas->engine.func->polygon_draw(output,
                                                 context,
                                                 surface,
                                                 o->engine_data,
                                                 o->offset.x + x, o->offset.y + y,
                                                 do_async);
}

static void
evas_object_polygon_render_pre(Evas_Object *eo_obj,
			       Evas_Object_Protected_Data *obj,
			       void *type_private_data)
{
   Evas_Polygon_Data *o = type_private_data;
   int is_v, was_v;

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
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
        goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&obj->layer->evas->clip_changes, eo_obj);
   /* if we restacked (layer or just within a layer) */
   if (obj->restack)
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
   /* if it changed geometry - and obviously not visibility or color */
   /* calculate differences since we have a constant color fill */
   /* we really only need to update the differences */
   if ((obj->cur->geometry.x != obj->prev->geometry.x) ||
       (obj->cur->geometry.y != obj->prev->geometry.y) ||
       (obj->cur->geometry.w != obj->prev->geometry.w) ||
       (obj->cur->geometry.h != obj->prev->geometry.h) ||
       (o->changed))
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
        goto done;
     }
   done:
   if ((obj->cur->geometry.x != obj->prev->geometry.x) ||
       (obj->cur->geometry.y != obj->prev->geometry.y))
     {
        if (!o->changed)
          {
             o->offset.x = obj->cur->geometry.x - obj->prev->geometry.x;
             o->offset.y = obj->cur->geometry.y - obj->prev->geometry.y;
          }
        else
          {
             o->offset.x = obj->cur->geometry.x - o->geometry.x;
             o->offset.y = obj->cur->geometry.y - o->geometry.y;
          }
     }
   evas_object_render_pre_effect_updates(&obj->layer->evas->clip_changes, eo_obj, is_v, was_v);
}

static void
evas_object_polygon_render_post(Evas_Object *eo_obj,
				Evas_Object_Protected_Data *obj EINA_UNUSED,
				void *type_private_data EINA_UNUSED)
{
   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   /* remove those pesky changes */
   evas_object_clip_changes_clean(eo_obj);
   /* move cur to prev safely for object data */
   evas_object_cur_prev(eo_obj);
}

static unsigned int evas_object_polygon_id_get(Evas_Object *eo_obj)
{
   Evas_Polygon_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_POLYGON;
}

static unsigned int evas_object_polygon_visual_id_get(Evas_Object *eo_obj)
{
   Evas_Polygon_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_SHAPE;
}

static void *evas_object_polygon_engine_data_get(Evas_Object *eo_obj)
{
   Evas_Polygon_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   return o->engine_data;
}

static int
evas_object_polygon_is_opaque(Evas_Object *eo_obj EINA_UNUSED,
			      Evas_Object_Protected_Data *obj EINA_UNUSED,
			      void *type_private_data EINA_UNUSED)
{
   /* this returns 1 if the internal object data implies that the object is */
   /* currently fully opaque over the entire line it occupies */
   return 0;
}

static int
evas_object_polygon_was_opaque(Evas_Object *eo_obj EINA_UNUSED,
			       Evas_Object_Protected_Data *obj EINA_UNUSED,
			       void *type_private_data EINA_UNUSED)
{
   /* this returns 1 if the internal object data implies that the object was */
   /* previously fully opaque over the entire line it occupies */
   return 0;
}

/* We count the number of edges a "ray" 90 degs upwards from our point
 * intersects with. If it's even, we are outside of the polygon, if it's odd,
 * we are inside of it. */
static int
evas_object_polygon_is_inside(Evas_Object *eo_obj EINA_UNUSED,
			      Evas_Object_Protected_Data *obj EINA_UNUSED,
			      void *type_private_data,
			      Evas_Coord x, Evas_Coord y)
{
   Evas_Polygon_Data *o = type_private_data;
   int num_edges = 0; /* Number of edges we crossed */
   Eina_List *itr;
   Evas_Polygon_Point *p;

   if (!o->points) return 0;

   /* Adjust X and Y according to current geometry */
   x -= o->offset.x;
   y -= o->offset.y;

   if (eina_list_count(o->points) == 1)
     {
        p = eina_list_data_get(o->points);
        return ((p->x == x) && (p->y == y));
     }

   EINA_LIST_FOREACH(o->points, itr, p)
     {
        Evas_Coord line_y;
        Eina_List *next = eina_list_next(itr);
        Evas_Polygon_Point *p_next;
        /* Get the next, or if there's no next, take the first */
        if (next)
          {
             p_next = eina_list_data_get(next);
          }
        else
          {
             p_next = eina_list_data_get(o->points);
          }

        /* Make sure that we are directly below the edge,
         * and that p->x != p_next->x */
        if (((p->x < p_next->x) && (p->x <= x) && (x < p_next->x)) ||
              ((p->x > p_next->x) && (p_next->x < x) && (x <= p->x)))
          {
             line_y = ((double) (p->y - p_next->y) /
                   (double) (p->x - p_next->x)) *
                (x - p_next->x) + p_next->y;
             /* We crossed that edge if the line is directly above us */
             if (line_y < y)
                num_edges++;
          }
     }

   /* Return true if num_edges is odd */
   return ((num_edges % 2) == 1);
}

static int
evas_object_polygon_was_inside(Evas_Object *eo_obj EINA_UNUSED,
                               Evas_Object_Protected_Data *obj EINA_UNUSED,
                               void *type_private_data EINA_UNUSED,
                               Evas_Coord x EINA_UNUSED, Evas_Coord y EINA_UNUSED)
{
   /* this returns 1 if the canvas co-ordinates were inside the object based */
   /* on object private data. not much use for rects, but for polys, images */
   /* and other complex objects it might be */
   return 1;
}

#include "canvas/evas_polygon.eo.c"

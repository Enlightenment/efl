#include "evas_common.h"
#include "evas_private.h"

#include <stdbool.h>

/* Switch this to not always use the map.
 * Tested using map, which is a little slower */
#define ALWAYS_MAP   1

/* Magic number for Proxy objects */
static const char o_type[] = "proxy";

/* Internal object data */
typedef struct _Evas_Object_Proxy
{
   DATA32   magic;

   Evas_Object *source;

   void *engine_data;

   Evas_Map *defmap;
   Eina_Bool mapupdate;
} Evas_Object_Proxy;


/* Helpers */
static void evas_object_proxy_init(Evas_Object *obj);
static Evas_Object_Proxy *evas_object_proxy_new(void);
static void _proxy_unset(Evas_Object *proxy);
static void _proxy_set(Evas_Object *proxy, Evas_Object *src);
static void _proxy_subrender(Evas *e, Evas_Object *source);

/* Engine Functions */
static void _proxy_free(Evas_Object *obj);
static void _proxy_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void _proxy_render_pre(Evas_Object *obj);
static void _proxy_render_post(Evas_Object *obj);
static unsigned int _proxy_id_get(Evas_Object *obj);
static unsigned int _proxy_visual_id_get(Evas_Object *obj);
static void *_proxy_engine_data_get(Evas_Object *obj);

static int _proxy_is_opaque(Evas_Object *obj);
static int _proxy_was_opaque(Evas_Object *obj);
static int _proxy_can_map(Evas_Object *obj);

/*
   void _proxy_store (Evas_Object *obj);
   void _proxy_unstore (Evas_Object *obj);

   int  _proxy_is_visible (Evas_Object *obj);
   int  _proxy_was_visible (Evas_Object *obj);

   int  _proxy_is_inside (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
   int  _proxy_was_inside (Evas_Object *obj, Evas_Coord x, Evas_Coord y);

   void _proxy_coords_recalc (Evas_Object *obj);
   void _proxy_scale_update (Evas_Object *obj);

   int _proxy_has_opaque_rect (Evas_Object *obj);
   int _proxy_get_opaque_rect (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
*/



static const Evas_Object_Func object_func =
{
  /* Required */
  .free = _proxy_free,
  .render = _proxy_render,
  .render_pre = _proxy_render_pre,
  .render_post = _proxy_render_post,

  .type_id_get = _proxy_id_get,
  .visual_id_get = _proxy_visual_id_get,
  .engine_data_get = _proxy_engine_data_get,

  .is_opaque = _proxy_is_opaque,
  .was_opaque = _proxy_was_opaque,

  /* Optional
  .store =

  .is_visible =
  .was_visible =

  .is_inside =
  .was_inside =

  .coords_recalc =
  .scale_update =

  .has_opaque_rect =
  .get_opaque_rect =
  */

  .can_map = _proxy_can_map,
};


EAPI Evas_Object *
evas_object_proxy_add(Evas *e)
{
   Evas_Object *obj;
   Evas_Object_Proxy *o;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
      return NULL;
   MAGIC_CHECK_END();

   obj = evas_object_new(e);
   evas_object_proxy_init(obj);
   evas_object_inject(obj, e);

   o = obj->object_data;

   /* Do stuff here */

   return obj;
}

EAPI Eina_Bool
evas_object_proxy_source_set(Evas_Object *obj, Evas_Object *src)
{
   Evas_Object_Proxy *o,*so;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return false;
   MAGIC_CHECK_END();
   o = obj->object_data;
   MAGIC_CHECK(o, Evas_Object_Proxy, MAGIC_OBJ_PROXY);
   return false;
   MAGIC_CHECK_END();

   if (o->source == src) return true;

   if (src)
     {
        MAGIC_CHECK(src, Evas_Object, MAGIC_OBJ);
        return false;
        MAGIC_CHECK_END();
        so = src->object_data;
        /* Stop the loop _now_ */
        /* FIXME: Should I check for smarts that contain proxies too? */
        if (so->magic == MAGIC_OBJ_PROXY)
           return false;
     }

   if (o->source)
     {
        _proxy_unset(obj);
     }

   if (src)
     {
        _proxy_set(obj, src);
     }

   return true;
}

EAPI Evas_Object *
evas_object_proxy_source_get(Evas_Object *obj)
{
   Evas_Object_Proxy *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = obj->object_data;
   MAGIC_CHECK(o, Evas_Object_Proxy, MAGIC_OBJ_PROXY);
   return NULL;
   MAGIC_CHECK_END();

   return o->source;
}

EAPI Eina_Bool
evas_object_proxy_source_unset(Evas_Object *o)
{
   return evas_object_proxy_source_set(o, NULL);
}




/* Internal helpers */
static void
evas_object_proxy_init(Evas_Object *obj)
{
   obj->object_data = evas_object_proxy_new();
   obj->cur.color.r =obj->cur.color.g =obj->cur.color.b =obj->cur.color.a = 255;
   obj->cur.geometry.x = obj->cur.geometry.y = 0;
   obj->cur.geometry.w = obj->cur.geometry.h = 0;
   obj->cur.layer = 0;
   obj->cur.anti_alias = 0;
   obj->cur.render_op = EVAS_RENDER_BLEND;
   obj->prev = obj->cur;

   obj->func = &object_func;
   obj->type = o_type;
}

static Evas_Object_Proxy *
evas_object_proxy_new(void)
{
   Evas_Object_Proxy *o;

   o = calloc(1,sizeof(Evas_Object_Proxy));
   o->magic = MAGIC_OBJ_PROXY;
   o->source = NULL;
   return o;
}


static void
_proxy_unset(Evas_Object *proxy)
{
   Evas_Object_Proxy *o;

   o = proxy->object_data;
   if (!o->source) return;

   o->source->proxy.proxies = eina_list_remove(o->source->proxy.proxies, proxy);

   o->source = NULL;
}


static void
_proxy_set(Evas_Object *proxy, Evas_Object *src)
{
   Evas_Object_Proxy *o;

   o = proxy->object_data;

   o->source = src;

   src->proxy.proxies = eina_list_append(src->proxy.proxies, proxy);
   src->proxy.redraw = EINA_TRUE;
   o->mapupdate = EINA_TRUE;
}



static void
_proxy_free(Evas_Object *obj)
{
   Evas_Object_Proxy *o;

   o = obj->object_data;
   MAGIC_CHECK(o, Evas_Object_Proxy, MAGIC_OBJ_PROXY);
      return;
   MAGIC_CHECK_END();

   if (o->source) _proxy_unset(obj);
   if (o->defmap) evas_map_free(o->defmap);
   o->magic = 0;
   free(o);
}

static void
_proxy_map_update(Evas_Object *obj)
{
   Evas_Object_Proxy *o;
   int x,y, w, h;
   int sw,sh;
   Evas_Map *m;

   o = obj->object_data;

   if (!o->source) return;
   o->mapupdate = EINA_FALSE;


   x = obj->cur.geometry.x;
   y = obj->cur.geometry.y;
   w = obj->cur.geometry.w;
   h = obj->cur.geometry.h;
   sw = o->source->proxy.w;
   sh = o->source->proxy.h;

   if (!o->defmap)
      o->defmap = evas_map_new(4);
   m = o->defmap;

   evas_map_point_coord_set   (m, 0, x, y, 0);
   evas_map_point_image_uv_set(m, 0, 0, 0);
   evas_map_point_color_set   (m, 0, 255, 255, 255, 255);

   evas_map_point_coord_set   (m, 1, x + w, y, 0);
   evas_map_point_image_uv_set(m, 1, sw, 0);
   evas_map_point_color_set   (m, 1, 255, 255, 255, 255);

   evas_map_point_coord_set   (m, 2, x + w, y + h, 0);
   evas_map_point_image_uv_set(m, 2, sw, sh);
   evas_map_point_color_set   (m, 2, 255, 255, 255, 255);

   evas_map_point_coord_set   (m, 3, x, y + h, 0);
   evas_map_point_image_uv_set(m, 3, 0, sh);
   evas_map_point_color_set   (m, 3, 255, 255, 255, 255);

}


static void
_proxy_render(Evas_Object *obj, void *output, void *context,
              void *surface, int x, int y)
{
   Evas_Object_Proxy *o;
   void *pixels;
   int w,h;

   o = obj->object_data;

   if (!o->source) return;

//   ENFN->context_multiplier_unset(output, context);
  // ENFN->context_render_op_set(output, context, obj->cur.render_op);

   if (o->source->proxy.surface && o->source->proxy.redraw == EINA_FALSE)
     {
         pixels = o->source->proxy.surface;
     }
   /* Making this faster would be nice... */
   else if (strcmp(evas_object_type_get(o->source),"image") == 0)
     {
        pixels = o->source->func->engine_data_get(o->source);
        evas_object_image_size_get(o->source, &o->source->proxy.w,
                                   &o->source->proxy.h);
     }
   else
     {
         _proxy_subrender(obj->layer->evas, o->source);
         pixels = o->source->proxy.surface;
     }

   if (o->mapupdate) _proxy_map_update(obj);

   if (!pixels)
     {
        return;
     }

   w = obj->cur.geometry.w;
   h = obj->cur.geometry.h;

   /* If we have a map: Use that */
   if (ALWAYS_MAP ||
       ((obj->cur.map) && (obj->cur.map->count == 4) && (obj->cur.usemap)))
     {
        const Evas_Map_Point *p, *p_end;
        RGBA_Map_Point pts[4], *pt;
        Evas_Map *map;

        if (!obj->cur.usemap || !obj->cur.map)
           map = o->defmap;
        else
           map = obj->cur.map;

        p = map->points;
        p_end = p + 4;
        pt = pts;

        pts[0].px = map->persp.px << FP;
        pts[0].py = map->persp.py << FP;
        pts[0].foc = map->persp.foc << FP;
        pts[0].z0 = map->persp.z0 << FP;

        pts[0].px = map->persp.px << FP;
        pts[0].py = map->persp.py << FP;
        pts[0].foc = map->persp.foc << FP;
        pts[0].z0 = map->persp.z0 << FP;
        // draw geom +x +y
        for (; p < p_end; p++, pt++)
          {
             pt->x = (p->x + x) << FP;
             pt->y = (p->y + y) << FP;
             pt->z = (p->z)     << FP;
             pt->x3 = p->px << FP;
             pt->y3 = p->py << FP;
             pt->u = p->u * FP1;
             pt->v = p->v * FP1;
             pt->col = ARGB_JOIN(p->a, p->r, p->g, p->b);
          }
        obj->layer->evas->engine.func->image_map4_draw
           (output, context, surface, pixels, pts, map->smooth, 0);
     }
   else
     {
        obj->layer->evas->engine.func->image_draw(output, context,
                                             surface, pixels,
                                             0, 0,
                                             w, h,
                                             obj->cur.geometry.x + /*ix +*/ x,
                                             obj->cur.geometry.y + /*iy +*/ y,
                                             w, h,/* was iw,ih */
                                             1);
     }
}

/**
 * Render the subobject
 */
static void
_proxy_subrender(Evas *e, Evas_Object *source)
{
   void *ctx;
   Evas_Object *obj2;
   int w,h;

   if (!source) return;

   w = source->cur.geometry.w;
   h = source->cur.geometry.h;

   source->proxy.redraw = EINA_FALSE;

   /* We need to redraw surface then */
   if (source->proxy.surface && (source->proxy.w != w || source->proxy.h != h))
     {
        e->engine.func->image_map_surface_free(e->engine.data.output,
                                               source->proxy.surface);
        source->proxy.surface = NULL;
     }

   /* FIXME: Hardcoded alpha 'on' */
   /* FIXME (cont): Should see if the object has alpha */
   if (!source->proxy.surface)
     {
        source->proxy.surface = e->engine.func->image_map_surface_new(
           e->engine.data.output, w, h, 1);
        source->proxy.w = w;
        source->proxy.h = w;
     }

   ctx = e->engine.func->context_new(e->engine.data.output);
   e->engine.func->context_color_set(e->engine.data.output, ctx, 0, 0, 0, 0);
   e->engine.func->context_render_op_set(e->engine.data.output, ctx, EVAS_RENDER_COPY);
   e->engine.func->rectangle_draw(e->engine.data.output, ctx,
                                  source->proxy.surface, 0, 0, w, h);
   e->engine.func->context_free(e->engine.data.output, ctx);

   ctx = e->engine.func->context_new(e->engine.data.output);
   if (source->smart.smart)
     {
        EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(source), obj2){
           obj2->func->render(obj2, e->engine.data.output, ctx,
                              source->proxy.surface,
                              -source->cur.geometry.x,
                              -source->cur.geometry.y);
        }
     }
   else
     {
        source->func->render(source, e->engine.data.output, ctx,
                                    source->proxy.surface,
                                    -source->cur.geometry.x,
                                    -source->cur.geometry.y);
     }
   e->engine.func->context_free(e->engine.data.output, ctx);

}

static void
_proxy_render_pre(Evas_Object *obj)
{
   Evas_Object_Proxy *o;
   Evas *e;
   int was_v, is_v;

   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;

   e = obj->layer->evas;
   o = obj->object_data;

   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);

   if (is_v != was_v)
     {
        evas_object_render_pre_visible_change(&e->clip_changes, obj, is_v, was_v);
     }
   if (((obj->cur.geometry.x != obj->prev.geometry.x) ||
	(obj->cur.geometry.y != obj->prev.geometry.y) ||
	(obj->cur.geometry.w != obj->prev.geometry.w) ||
	(obj->cur.geometry.h != obj->prev.geometry.h))
       )
     {
	evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
        o->mapupdate = EINA_TRUE;
     }

   if (o->source && o->source->proxy.redraw)
     {
        evas_add_rect(&e->clip_changes,
                      obj->cur.geometry.x, obj->cur.geometry.y,
                      obj->cur.geometry.w, obj->cur.geometry.h);
        o->mapupdate = EINA_TRUE;
     }

   evas_object_render_pre_effect_updates(&e->clip_changes, obj, is_v, was_v);
}
static void
_proxy_render_post(Evas_Object *obj)
{
   Evas_Object_Proxy *o;

   o = obj->object_data;

   evas_object_clip_changes_clean(obj);
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
   obj->changed = 0;
}
static unsigned int
_proxy_id_get(Evas_Object *obj)
{
   return obj->object_data ? MAGIC_OBJ_PROXY : 0;
}
static unsigned int
_proxy_visual_id_get(Evas_Object *obj)
{
   return obj->object_data ? MAGIC_OBJ_PROXY : 0;
}

static void *
_proxy_engine_data_get(Evas_Object *obj)
{
   return ((Evas_Object_Proxy *)obj->object_data)->engine_data;
}


static int
_proxy_is_opaque(Evas_Object *obj)
{
   Evas_Object_Proxy *o = obj->object_data;

   /* No source: Sure, it's opaque */
   if (!o->source) return 1;
   return 0;
}

static int
_proxy_was_opaque(Evas_Object *obj)
{
   Evas_Object_Proxy *o = obj->object_data;

   /* No source: Sure, it's opaque */
   if (!o->source) return 1;
   return 0;
}

static int
_proxy_can_map (Evas_Object *obj)
{
   /* Of course it can: that's the bloody point */
   return 1;
}









/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/

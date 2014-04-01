#include "evas_common_private.h"
#include "evas_private.h"
#include <math.h>
#include <assert.h>
#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

#ifdef EVAS_RENDER_DEBUG_TIMING
#include <sys/time.h>
#endif

// symbols of funcs from evas_render2 which is a next-gen replacement of
// the current evas render code. see evas_render2.c for more.
Eina_Bool _evas_render2_begin(Eo *eo_e, Eina_Bool make_updates, Eina_Bool do_draw, Eina_Bool do_async);
void _evas_render2_idle_flush(Eo *eo_e);
void _evas_render2_dump(Eo *eo_e);
void _evas_render2_wait(Eo *eo_e);


/* debug rendering
 * NOTE: Define REND_DBG 1 in evas_private.h to enable debugging. Don't define
 * it here since the flag is used on other places too. */

/* #define STDOUT_DBG 1 */

#ifdef REND_DBG
static FILE *dbf = NULL;

static void
rend_dbg(const char *txt)
{
   if (!dbf)
     {
#ifdef STDOUT_DBG
        dbf = stdout;
#else
        dbf = fopen("EVAS-RENDER-DEBUG.log", "w");
#endif
        if (!dbf) return;
     }
   fputs(txt, dbf);
   fflush(dbf);
}
#define RD(args...) \
   { \
      char __tmpbuf[4096]; \
      \
      snprintf(__tmpbuf, sizeof(__tmpbuf), ##args); \
      rend_dbg(__tmpbuf); \
   }
#define RDI(xxxx) \
   { \
      char __tmpbuf[4096]; int __tmpi; \
      for (__tmpi = 0; __tmpi < xxxx; __tmpi++) \
        __tmpbuf[__tmpi] = ' '; \
      __tmpbuf[__tmpi] = 0; \
      rend_dbg(__tmpbuf); \
   }
#else
#define RD(args...)
#define RDI(x)
#endif

#define OBJ_ARRAY_PUSH(array, obj)  \
do                                  \
{                                   \
   eina_array_push(array, obj);     \
   eo_data_ref(obj->object, NULL);  \
} while (0)

#define OBJS_ARRAY_CLEAN(array)                       \
{                                                     \
   Evas_Object_Protected_Data *item;                  \
   Eina_Array_Iterator iterator;                      \
   unsigned int idx;                                  \
   EINA_ARRAY_ITER_NEXT(array, idx, item, iterator)   \
      eo_data_unref(item->object, item);              \
   eina_array_clean(array);                           \
}

#define OBJS_ARRAY_FLUSH(array)                       \
{                                                     \
   Evas_Object_Protected_Data *item;                  \
   Eina_Array_Iterator iterator;                      \
   unsigned int idx;                                  \
   EINA_ARRAY_ITER_NEXT(array, idx, item, iterator)   \
      eo_data_unref(item->object, item);              \
   eina_array_flush(array);                           \
}

typedef struct _Render_Updates Render_Updates;
struct _Render_Updates
{
   void *surface;
   Eina_Rectangle *area;
};

static Eina_Bool
evas_render_updates_internal(Evas *eo_e, unsigned char make_updates, unsigned char do_draw, Evas_Render_Done_Cb done_func, void *done_data, Eina_Bool do_async);

static Eina_List *_rendering_evases = NULL;

#ifdef EVAS_RENDER_DEBUG_TIMING
static double
_time_get()
{
   struct timeval tv;

   gettimeofday(&tv, NULL);

   return (tv.tv_sec + tv.tv_usec / 1000000.0) * 1000.0;
}

struct accumulator {
   double total, min, max;
   int samples;
   const char *what;
};

static struct accumulator async_accumulator = {
   .total = 0,
   .min = 1000000,
   .max = 0,
   .samples = 0,
   .what = "async render"
};
static struct accumulator sync_accumulator = {
   .total = 0,
   .min = 1000000,
   .max = 0,
   .samples = 0,
   .what = "sync render"
};

static void
_accumulate_time(double before, struct accumulator *acc)
{
   double diff = _time_get() - before;

   acc->total += diff;
   if (diff > acc->max) acc->max = diff;
   if (diff < acc->min) acc->min = diff;

   acc->samples++;
   if (acc->samples % 100 == 0)
     {
        fprintf(stderr, "*** %s: avg %fms min %fms max %fms\n",
                acc->what, acc->total / 100.0, acc->min, acc->max);
        acc->total = 0.0;
        acc->max = 0.0;
        acc->min = 1000000;
     }
}
#endif

EOLIAN void
_evas_damage_rectangle_add(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, int x, int y, int w, int h)
{
   Eina_Rectangle *r;

   NEW_RECT(r, x, y, w, h);
   if (!r) return;
   e->damages = eina_list_append(e->damages, r);
   e->changed = EINA_TRUE;
}

EOLIAN void
_evas_obscured_rectangle_add(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, int x, int y, int w, int h)
{
   Eina_Rectangle *r;

   NEW_RECT(r, x, y, w, h);
   if (!r) return;
   e->obscures = eina_list_append(e->obscures, r);
}

EOLIAN void
_evas_obscured_clear(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   Eina_Rectangle *r;

   EINA_LIST_FREE(e->obscures, r)
     {
        eina_rectangle_free(r);
     }
}

static Eina_Bool
_evas_clip_changes_free(const void *container EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   eina_rectangle_free(data);
   return EINA_TRUE;
}

static Eina_Bool
_evas_render_had_map(Evas_Object_Protected_Data *obj)
{
   return ((obj->map->prev.map) && (obj->map->prev.usemap));
   //   return ((!obj->map->cur.map) && (obj->prev->usemap));
}

static Eina_Bool
_evas_render_is_relevant(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   return ((evas_object_is_visible(eo_obj, obj) && (!obj->cur->have_clipees)) ||
           (evas_object_was_visible(eo_obj, obj) && (!obj->prev->have_clipees)));
}

static Eina_Bool
_evas_render_can_render(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   return (evas_object_is_visible(eo_obj, obj) && (!obj->cur->have_clipees));
}

static void
_evas_render_prev_cur_clip_cache_add(Evas_Public_Data *e, Evas_Object_Protected_Data *obj)
{
   e->engine.func->output_redraws_rect_add(e->engine.data.output,
                                           obj->prev->cache.clip.x + e->framespace.x,
                                           obj->prev->cache.clip.y + e->framespace.y,
                                           obj->prev->cache.clip.w,
                                           obj->prev->cache.clip.h);
   e->engine.func->output_redraws_rect_add(e->engine.data.output,
                                           obj->cur->cache.clip.x + e->framespace.x,
                                           obj->cur->cache.clip.y + e->framespace.y,
                                           obj->cur->cache.clip.w,
                                           obj->cur->cache.clip.h);
}

static void
_evas_render_cur_clip_cache_del(Evas_Public_Data *e, Evas_Object_Protected_Data *obj)
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
   e->engine.func->output_redraws_rect_del(e->engine.data.output,
                                           x + e->framespace.x,
                                           y + e->framespace.y, w, h);
}

static void
_evas_proxy_redraw_set(Evas_Public_Data *e, Evas_Object_Protected_Data *obj,
                       Eina_Bool render)
{
   Evas_Object *eo_proxy;
   Evas_Object_Protected_Data *proxy;
   Eina_List *l;

   EINA_LIST_FOREACH(obj->proxy->proxies, l, eo_proxy)
     {
        proxy = eo_data_scope_get(eo_proxy, EVAS_OBJ_CLASS);

        /* Flag need redraw on proxy too */
        EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, proxy->proxy,
                             Evas_Object_Proxy_Data, proxy_write)
           proxy_write->redraw = EINA_TRUE;
        EINA_COW_WRITE_END(evas_object_proxy_cow, proxy->proxy, proxy_write);

        if (render)
          {
             proxy->func->render_pre(eo_proxy, proxy, proxy->private_data);
             _evas_render_prev_cur_clip_cache_add(e, proxy);
          }

        //Update the proxies recursively.
        _evas_proxy_redraw_set(e, proxy, render);
     }
}

static void
_evas_render_phase1_direct(Evas_Public_Data *e,
                           Eina_Array *active_objects,
                           Eina_Array *restack_objects EINA_UNUSED,
                           Eina_Array *delete_objects EINA_UNUSED,
                           Eina_Array *render_objects)
{
   unsigned int i;
   Evas_Object *eo_obj;
   Eina_Bool changed;

   RD("  [--- PHASE 1 DIRECT\n");
   for (i = 0; i < active_objects->count; i++)
     {
        Evas_Object_Protected_Data *obj =
           eina_array_data_get(active_objects, i);

        if (obj->changed) evas_object_clip_recalc(obj);
        if (!obj->proxy->proxies) continue;

        if (obj->smart.smart)
          changed = evas_object_smart_changed_get(obj->object);
        else changed = obj->changed;

        if (changed) _evas_proxy_redraw_set(e, obj, EINA_FALSE);
     }
   for (i = 0; i < render_objects->count; i++)
     {
        Evas_Object_Protected_Data *obj =
           eina_array_data_get(render_objects, i);
        eo_obj = obj->object;

        RD("    OBJ [%p", obj);
        if (obj->name)
          {
             RD(":%s", obj->name);
          }
        RD("] changed %i\n", obj->changed);

        if (obj->changed)
          {
             evas_object_clip_recalc(obj);
             obj->func->render_pre(eo_obj, obj, obj->private_data);
             if (obj->proxy->redraw)
               _evas_render_prev_cur_clip_cache_add(e, obj);
             if (obj->proxy->proxies)
               {
                  if (!obj->smart.smart || evas_object_smart_changed_get(eo_obj))
                    {
                       EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, obj->proxy,
                                            Evas_Object_Proxy_Data, proxy_write)
                          proxy_write->redraw = EINA_TRUE;
                       EINA_COW_WRITE_END(evas_object_proxy_cow, obj->proxy,
                                          proxy_write);
                       _evas_proxy_redraw_set(e, obj, EINA_TRUE);
                    }
               }

             RD("      pre-render-done smart:%p|%p  [%p, %i] | [%p, %i] has_map:%i had_map:%i\n",
                obj->smart.smart,
                evas_object_smart_members_get_direct(eo_obj),
                obj->map->cur.map, obj->map->cur.usemap,
                obj->map->prev.map, obj->map->prev.usemap,
                _evas_render_has_map(eo_obj, obj),
                _evas_render_had_map(obj));
             if ((obj->is_smart) &&
                 ((_evas_render_has_map(eo_obj, obj) ||
                 (obj->changed_src_visible))))
               {
                  RD("      has map + smart\n");
                  _evas_render_prev_cur_clip_cache_add(e, obj);
               }
          }
        else
          {
             if (obj->is_smart)
               {
                  //                  obj->func->render_pre(eo_obj);
               }
             else if (evas_object_is_visible(eo_obj, obj) &&
                      ((obj->rect_del) ||
                      (evas_object_is_opaque(eo_obj, obj))) &&
                      (!evas_object_is_source_invisible(eo_obj, obj)))
               {
                  RD("    rect del\n");
                  _evas_render_cur_clip_cache_del(e, obj);
               }
          }
     }
   RD("  ---]\n");
}

static Eina_Bool
_evas_render_phase1_object_process(Evas_Public_Data *e, Evas_Object *eo_obj,
                                   Eina_Array *active_objects,
                                   Eina_Array *restack_objects,
                                   Eina_Array *delete_objects,
                                   Eina_Array *render_objects,
                                   int restack,
                                   int *redraw_all,
                                   Eina_Bool mapped_parent,
                                   Eina_Bool src_changed
#ifdef REND_DBG
                                   , int level
#endif
                                  )
{
   Eina_Bool clean_them = EINA_FALSE;
   int is_active;
   Eina_Bool map, hmap;

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   //Need pre render for the children of mapped object.
   //But only when they have changed.
   if (mapped_parent && (!obj->changed)) return EINA_FALSE;

   obj->rect_del = EINA_FALSE;
   obj->render_pre = EINA_FALSE;

   if (obj->delete_me == 2)
      OBJ_ARRAY_PUSH(delete_objects, obj);
   else if (obj->delete_me != 0) obj->delete_me++;
   /* If the object will be removed, we should not cache anything during this run. */
   if (obj->delete_me != 0) clean_them = EINA_TRUE;

   /* build active object list */
   evas_object_clip_recalc(obj);

   if (src_changed) is_active = EINA_TRUE;
   else
     {
       is_active = evas_object_is_active(eo_obj, obj);
       src_changed = is_active;
     }
   obj->is_active = is_active;

   RDI(level);
   RD("    [--- PROCESS [%p", obj);
   if (obj->name)
     {
        RD(":%s", obj->name);
     }
   RD("] '%s' active = %i, del = %i | %i %i %ix%i\n", obj->type, is_active, obj->delete_me, obj->cur->geometry.x, obj->cur->geometry.y, obj->cur->geometry.w, obj->cur->geometry.h);

   if ((!mapped_parent) && ((is_active) || (obj->delete_me != 0)))
      OBJ_ARRAY_PUSH(active_objects, obj);

#ifdef REND_DBG
   if (!is_active)
     {
        RDI(level);
        RD("     [%p", obj);
        if (obj->name) RD(":%s", obj->name);
        RD("] vis: %i, cache.clip.vis: %i cache.clip.a: %i [%p]\n", obj->cur->visible, obj->cur->cache.clip.visible, obj->cur->cache.clip.a, obj->func->is_visible);
     }
#endif

   map = _evas_render_has_map(eo_obj, obj);
   hmap = _evas_render_had_map(obj);

   if ((restack) && (!map))
     {
        if (!obj->changed)
          {
             OBJ_ARRAY_PUSH(&e->pending_objects, obj);
             obj->changed = EINA_TRUE;
          }
        obj->restack = EINA_TRUE;
        clean_them = EINA_TRUE;
     }

   if (map)
     {
        RDI(level);
        RD("      obj mapped\n");
        if (obj->changed)
          {
             if (map != hmap) *redraw_all = 1;

             if ((is_active) && (!obj->clip.clipees) &&
                 ((evas_object_is_visible(eo_obj, obj) && (!obj->cur->have_clipees)) ||
                  (evas_object_was_visible(eo_obj, obj) && (!obj->prev->have_clipees))))
               {
                  OBJ_ARRAY_PUSH(render_objects, obj);
                  _evas_render_prev_cur_clip_cache_add(e, obj);
                  obj->render_pre = EINA_TRUE;

                  if (obj->is_smart)
                    {
                       Evas_Object_Protected_Data *obj2;

                       EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(eo_obj), obj2)
                         {
                            _evas_render_phase1_object_process(e, obj2->object,
                                                               active_objects,
                                                               restack_objects,
                                                               delete_objects,
                                                               render_objects,
                                                               obj->restack,
                                                               redraw_all,
                                                               EINA_TRUE,
                                                               src_changed
#ifdef REND_DBG
                                                               , level + 1
#endif
                                                              );
                         }
                    }
               }
          }
        return clean_them;
     }
   else if (hmap)
     {
        RDI(level);
        RD("      had map - restack objs\n");
        //        OBJ_ARRAY_PUSH(restack_objects, obj);
        _evas_render_prev_cur_clip_cache_add(e, obj);
        if (obj->changed)
          {
             if (!map)
               {
                  if ((obj->map->cur.map) && (obj->map->cur.usemap)) map = EINA_TRUE;
               }
             if (map != hmap)
               {
                  *redraw_all = 1;
               }
          }
     }

   /* handle normal rendering. this object knows how to handle maps */
   if (obj->changed)
     {
        if (obj->is_smart)
          {
             RDI(level);
             RD("      changed + smart - render ok\n");
             OBJ_ARRAY_PUSH(render_objects, obj);

             if (!is_active && obj->proxy->proxies) src_changed = EINA_TRUE;

             obj->render_pre = EINA_TRUE;
             Evas_Object_Protected_Data *obj2;
             EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(eo_obj),
                                 obj2)
               {
                  _evas_render_phase1_object_process(e, obj2->object,
                                                     active_objects,
                                                     restack_objects,
                                                     delete_objects,
                                                     render_objects,
                                                     obj->restack,
                                                     redraw_all,
                                                     mapped_parent,
                                                     src_changed
#ifdef REND_DBG
                                                     , level + 1
#endif
                                                    );
               }
          }
        else
          {
             if ((!obj->clip.clipees) && _evas_render_is_relevant(eo_obj))
               {
                  if (is_active)
                    {
                       RDI(level);
                       RD("      relevant + active\n");
                       if (obj->restack)
                          OBJ_ARRAY_PUSH(restack_objects, obj);
                       else
                         {
                            OBJ_ARRAY_PUSH(render_objects, obj);
                            obj->render_pre = EINA_TRUE;
                         }
                    }
                  else
                    {
                       /* It goes to be hidden. Prev caching should be replaced
                          by the current (hidden) state. */
                       if (evas_object_is_visible(eo_obj, obj) !=
                           evas_object_was_visible(eo_obj, obj))
                         evas_object_cur_prev(eo_obj);

                       RDI(level);
                       RD("      skip - not smart, not active or clippees or not relevant\n");
                    }
               }
             else
               {
                  RDI(level);
                  RD("      skip - not smart, not active or clippees or not relevant\n");
               }
          }
     }
   else
     {
        RD("      not changed... [%i] -> (%i %i %p %i) [%i]\n",
           evas_object_is_visible(eo_obj, obj),
           obj->cur->visible, obj->cur->cache.clip.visible, obj->smart.smart,
           obj->cur->cache.clip.a, evas_object_was_visible(eo_obj, obj));

        if ((!obj->clip.clipees) && (obj->delete_me == 0) &&
            (_evas_render_can_render(eo_obj, obj) ||
             (evas_object_was_visible(eo_obj, obj) && (!obj->prev->have_clipees))))
          {
             if (obj->is_smart)
               {
                  RDI(level);
                  RD("      smart + visible/was visible + not clip\n");
                  OBJ_ARRAY_PUSH(render_objects, obj);
                  obj->render_pre = EINA_TRUE;
                  Evas_Object_Protected_Data *obj2;
                  EINA_INLIST_FOREACH
                     (evas_object_smart_members_get_direct(eo_obj), obj2)
                       {
                          _evas_render_phase1_object_process(e, obj2->object,
                                                             active_objects,
                                                             restack_objects,
                                                             delete_objects,
                                                             render_objects,
                                                             restack,
                                                             redraw_all,
                                                             mapped_parent,
                                                             src_changed
#ifdef REND_DBG
                                                             , level + 1
#endif
                                                            );
                       }
               }
             else
               {
                  if (evas_object_is_opaque(eo_obj, obj) &&
                      evas_object_is_visible(eo_obj, obj))
                    {
                       RDI(level);
                       RD("      opaque + visible\n");
                       OBJ_ARRAY_PUSH(render_objects, obj);
                       obj->rect_del = EINA_TRUE;
                    }
                  else if (evas_object_is_visible(eo_obj, obj))
                    {
                       RDI(level);
                       RD("      visible\n");
                       OBJ_ARRAY_PUSH(render_objects, obj);
                       obj->render_pre = EINA_TRUE;
                    }
                  else
                    {
                       RDI(level);
                       RD("      skip\n");
                    }
               }
          }
 /*       else if (obj->smart.smart)
          {
             RDI(level);
             RD("      smart + mot visible/was visible\n");
             OBJ_ARRAY_PUSH(render_objects, obj);
             obj->render_pre = 1;
             EINA_INLIST_FOREACH (evas_object_smart_members_get_direct(eo_obj),
                                  obj2)
               {
                  _evas_render_phase1_object_process(e, obj2,
                                                     active_objects,
                                                     restack_objects,
                                                     delete_objects,
                                                     render_objects,
                                                     restack,
                                                     redraw_all
#ifdef REND_DBG
                                                     , level + 1
#endif
                                                    );
               }
          }
*/
     }
   if (!is_active) obj->restack = EINA_FALSE;
   RDI(level);
   RD("    ---]\n");
   return clean_them;
}

static Eina_Bool
_evas_render_phase1_process(Evas_Public_Data *e,
                            Eina_Array *active_objects,
                            Eina_Array *restack_objects,
                            Eina_Array *delete_objects,
                            Eina_Array *render_objects,
                            int *redraw_all)
{
   Evas_Layer *lay;
   Eina_Bool clean_them = EINA_FALSE;

   RD("  [--- PHASE 1\n");
   EINA_INLIST_FOREACH(e->layers, lay)
     {
        Evas_Object_Protected_Data *obj;

        EINA_INLIST_FOREACH(lay->objects, obj)
          {
             clean_them |= _evas_render_phase1_object_process
                (e, obj->object, active_objects, restack_objects, delete_objects,
                 render_objects, 0, redraw_all, EINA_FALSE, EINA_FALSE
#ifdef REND_DBG
                 , 1
#endif
                );
          }
     }
   RD("  ---]\n");
   return clean_them;
}

static void
_evas_render_check_pending_objects(Eina_Array *pending_objects, Evas *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   unsigned int i;

   for (i = 0; i < pending_objects->count; ++i)
     {
        Evas_Object *eo_obj;
        int is_active;
        Eina_Bool ok = EINA_FALSE;

        Evas_Object_Protected_Data *obj = eina_array_data_get(pending_objects, i);
        eo_obj = obj->object;

        if (!obj->layer) goto clean_stuff;

       //If the children are in active objects, They should be cleaned up.
       if (obj->changed_map && _evas_render_has_map(eo_obj, obj))
         goto clean_stuff;

        evas_object_clip_recalc(obj);
        is_active = evas_object_is_active(eo_obj, obj);

        if ((!is_active) && (!obj->is_active) && (!obj->render_pre) &&
            (!obj->rect_del))
          {
             ok = EINA_TRUE;
             goto clean_stuff;
          }

        if (obj->is_active == is_active)
          {
             if (obj->changed)
               {
                  if (obj->is_smart)
                    {
                       if (obj->render_pre || obj->rect_del) ok = EINA_TRUE;
                    }
                  else
                    if ((is_active) && (obj->restack) && (!obj->clip.clipees) &&
                        (_evas_render_can_render(eo_obj, obj) ||
                         (evas_object_was_visible(eo_obj, obj) && (!obj->prev->have_clipees))))
                      {
                         if (!(obj->render_pre || obj->rect_del))
                           ok = EINA_TRUE;
                      }
                    else
                      if (is_active && (!obj->clip.clipees) &&
                          (_evas_render_can_render(eo_obj, obj) ||
                           (evas_object_was_visible(eo_obj, obj) && (!obj->prev->have_clipees))))
                        {
                           if (obj->render_pre || obj->rect_del) ok = EINA_TRUE;
                        }
               }
             else
               {
                  if ((!obj->clip.clipees) && (obj->delete_me == 0) &&
                      (!obj->cur->have_clipees || (evas_object_was_visible(eo_obj, obj) && (!obj->prev->have_clipees)))
                      && evas_object_is_opaque(eo_obj, obj) && evas_object_is_visible(eo_obj, obj))
                    {
                       if (obj->rect_del || obj->is_smart) ok = EINA_TRUE;
                    }
               }
          }

clean_stuff:
        if (!ok)
          {
             OBJS_ARRAY_CLEAN(&e->active_objects);
             OBJS_ARRAY_CLEAN(&e->render_objects);
             OBJS_ARRAY_CLEAN(&e->restack_objects);
             OBJS_ARRAY_CLEAN(&e->delete_objects);
             e->invalidate = EINA_TRUE;
             return;
          }
     }
}

Eina_Bool
pending_change(void *data, void *gdata EINA_UNUSED)
{
   Evas_Object *eo_obj;

   Evas_Object_Protected_Data *obj = data;
   eo_obj = obj->object;
   if (obj->delete_me) return EINA_FALSE;
   if (obj->pre_render_done)
     {
        RD("  OBJ [%p", obj);
        if (obj->name) 
          {
             RD(":%s", obj->name);
          }
        RD("] pending change %i -> 0, pre %i\n", obj->changed, obj->pre_render_done);
        obj->func->render_post(eo_obj, obj, obj->private_data);
        obj->pre_render_done = EINA_FALSE;
        evas_object_change_reset(eo_obj);
     }
   else if (!_evas_render_can_render(eo_obj, obj) &&
            (!obj->is_active) && (!obj->render_pre) &&
            (!obj->rect_del))
     {
        evas_object_change_reset(eo_obj);
     }
   if (!obj->changed) eo_data_unref(eo_obj, obj);
   return obj->changed ? EINA_TRUE : EINA_FALSE;
}

static Eina_Bool
_evas_render_can_use_overlay(Evas_Public_Data *e, Evas_Object *eo_obj)
{
   Eina_Rectangle *r;
   Evas_Object *eo_tmp;
   Eina_List *alphas = NULL;
   Eina_List *opaques = NULL;
   Evas_Object *video_parent = NULL;
   Eina_Rectangle zone;
   Evas_Coord xc1, yc1, xc2, yc2;
   unsigned int i;
   Eina_Bool nooverlay;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Protected_Data *tmp = NULL;
   Evas_Coord imgw, imgh;
   unsigned int caps;
   Eina_Bool surface_below, stacking_check, object_above = EINA_FALSE;
   Eina_Bool ignore_window;

   video_parent = _evas_object_image_video_parent_get(eo_obj);

   /* Check if any one is the stack make this object mapped */
   eo_tmp = eo_obj;
   while (tmp && !_evas_render_has_map(eo_tmp, tmp))
     {
        tmp = eo_data_scope_get(eo_tmp, EVAS_OBJ_CLASS);
        eo_tmp = tmp->smart.parent;
     }

   if (tmp && _evas_render_has_map(eo_tmp, tmp)) return EINA_FALSE; /* we are mapped, we can't be an overlay */

   if (!evas_object_is_visible(eo_obj, obj)) return EINA_FALSE; /* no need to update the overlay if it's not visible */

   /* If any recoloring of the surface is needed, n overlay to */
   if ((obj->cur->cache.clip.r != 255) ||
       (obj->cur->cache.clip.g != 255) ||
       (obj->cur->cache.clip.b != 255) ||
       (obj->cur->cache.clip.a != 255))
     return EINA_FALSE;

   caps = evas_object_image_video_surface_caps_get(eo_obj);

   /* check if surface is above the canvas */
   surface_below = !!(caps & EVAS_VIDEO_SURFACE_BELOW);
   if (!surface_below)
     {
        /* above canvas, must support resize and clipping */

        /* check if video surface supports resize */
        evas_object_image_size_get(eo_obj, &imgw, &imgh);
        if ((obj->cur->geometry.w != imgw) ||
            (obj->cur->geometry.h != imgh))
          {
             if (!(caps & EVAS_VIDEO_SURFACE_RESIZE))
                return EINA_FALSE;
          }
        /* check if video surface supports clipping */
        evas_object_image_size_get(eo_obj, &imgw, &imgh);
        if ((obj->cur->cache.clip.x != obj->cur->geometry.x) ||
            (obj->cur->cache.clip.y != obj->cur->geometry.y) ||
            (obj->cur->cache.clip.w != obj->cur->geometry.w) ||
            (obj->cur->cache.clip.h != obj->cur->geometry.h))
          {
             if (!(caps & EVAS_VIDEO_SURFACE_CLIP))
                return EINA_FALSE;
          }
     }

   /* check for window/surface/canvas limits */
   ignore_window = !!(caps & EVAS_VIDEO_SURFACE_IGNORE_WINDOW);
   if (!ignore_window)
     {
        Evas_Coord x1, x2, y1, y2;
        Evas_Coord fx, fy, fw, fh;

        fx = e->framespace.x;
        fy = e->framespace.y;
        fw = e->framespace.w;
        fh = e->framespace.h;

        x1 = obj->cur->geometry.x + fx;
        y1 = obj->cur->geometry.y + fy;
        x2 = obj->cur->geometry.x + obj->cur->geometry.w + fx;
        y2 = obj->cur->geometry.y + obj->cur->geometry.h + fy;

        if ((x1 < fx) || (y1 < fy) ||
            (x2 > e->output.w - (fw - fx)) ||
            (y2 > e->output.h - (fh - fy)))
          return EINA_FALSE;
     }

   /* check if there are other objects above the video object? */
   stacking_check = !!(caps & EVAS_VIDEO_SURFACE_STACKING_CHECK);
   if (!stacking_check)
     return EINA_TRUE;

   /* Check presence of transparent object on top of the video object */
   EINA_RECTANGLE_SET(&zone,
                      obj->cur->cache.clip.x,
                      obj->cur->cache.clip.y,
                      obj->cur->cache.clip.w,
                      obj->cur->cache.clip.h);

   for (i = e->active_objects.count - 1; i > 0; i--)
     {
        Eina_Rectangle self;
        Eina_Rectangle *match;
        Evas_Object *eo_current;
        Eina_List *l;
        int xm1, ym1, xm2, ym2;

        Evas_Object_Protected_Data *current = eina_array_data_get(&e->active_objects, i);
        eo_current = current->object;

        /* Did we find the video object in the stack ? */
        if (eo_current == video_parent || eo_current == eo_obj)
          break;

        EINA_RECTANGLE_SET(&self,
                           current->cur->cache.clip.x,
                           current->cur->cache.clip.y,
                           current->cur->cache.clip.w,
                           current->cur->cache.clip.h);

        /* This doesn't cover the area of the video object, so don't bother with that object */
        if (!eina_rectangles_intersect(&zone, &self))
          continue;

        xc1 = current->cur->cache.clip.x;
        yc1 = current->cur->cache.clip.y;
        xc2 = current->cur->cache.clip.x + current->cur->cache.clip.w;
        yc2 = current->cur->cache.clip.y + current->cur->cache.clip.h;

        if (evas_object_is_visible(eo_current, current) &&
            (!current->clip.clipees) &&
            (current->cur->visible) &&
            (!current->delete_me) &&
            (current->cur->cache.clip.visible) &&
            (!eo_isa(eo_current, EVAS_OBJ_SMART_CLASS)))
          {
             Eina_Bool included = EINA_FALSE;

             if (!surface_below)
               {
                  object_above = EINA_TRUE;
                  break;
               }

             if (evas_object_is_opaque(eo_current, current) ||
                 ((current->func->has_opaque_rect) &&
                  (current->func->has_opaque_rect(eo_current, current, current->private_data))))
               {
                  /* The object is opaque */

                  /* Check if the opaque object is inside another opaque object */
                  EINA_LIST_FOREACH(opaques, l, match)
                    {
                       xm1 = match->x;
                       ym1 = match->y;
                       xm2 = match->x + match->w;
                       ym2 = match->y + match->h;

                       /* Both object are included */
                       if (xc1 >= xm1 && yc1 >= ym1 && xc2 <= xm2 && yc2 <= ym2)
                         {
                            included = EINA_TRUE;
                            break;
                         }
                    }

                  /* Not included yet */
                  if (!included)
                    {
                       Eina_List *ln;
                       Evas_Coord xn2, yn2;

                       r = eina_rectangle_new(current->cur->cache.clip.x, current->cur->cache.clip.y,
                                              current->cur->cache.clip.w, current->cur->cache.clip.h);

                       opaques = eina_list_append(opaques, r);

                       xn2 = r->x + r->w;
                       yn2 = r->y + r->h;

                       /* Remove all the transparent object that are covered by the new opaque object */
                       EINA_LIST_FOREACH_SAFE(alphas, l, ln, match)
                         {
                            xm1 = match->x;
                            ym1 = match->y;
                            xm2 = match->x + match->w;
                            ym2 = match->y + match->h;

                            if (xm1 >= r->x && ym1 >= r->y && xm2 <= xn2 && ym2 <= yn2)
                              {
                                 /* The new rectangle is over some transparent object,
                                    so remove the transparent object */
                                 alphas = eina_list_remove_list(alphas, l);
                              }
                         }
                    }
               }
             else
               {
                  /* The object has some transparency */

                  /* Check if the transparent object is inside any other transparent object */
                  EINA_LIST_FOREACH(alphas, l, match)
                    {
                       xm1 = match->x;
                       ym1 = match->y;
                       xm2 = match->x + match->w;
                       ym2 = match->y + match->h;

                       /* Both object are included */
                       if (xc1 >= xm1 && yc1 >= ym1 && xc2 <= xm2 && yc2 <= ym2)
                         {
                            included = EINA_TRUE;
                            break;
                         }
                    }

                  /* If not check if it is inside any opaque one */
                  if (!included)
                    {
                       EINA_LIST_FOREACH(opaques, l, match)
                         {
                            xm1 = match->x;
                            ym1 = match->y;
                            xm2 = match->x + match->w;
                            ym2 = match->y + match->h;

                            /* Both object are included */
                            if (xc1 >= xm1 && yc1 >= ym1 && xc2 <= xm2 && yc2 <= ym2)
                              {
                                 included = EINA_TRUE;
                                 break;
                              }
                         }
                    }

                  /* No inclusion at all, so add it */
                  if (!included)
                    {
                       r = eina_rectangle_new(current->cur->cache.clip.x, current->cur->cache.clip.y,
                                              current->cur->cache.clip.w, current->cur->cache.clip.h);

                       alphas = eina_list_append(alphas, r);
                    }
               }
          }
     }

   /* If there is any pending transparent object, then no overlay */
   nooverlay = !!eina_list_count(alphas);

   EINA_LIST_FREE(alphas, r)
     eina_rectangle_free(r);
   EINA_LIST_FREE(opaques, r)
     eina_rectangle_free(r);

   if (nooverlay || object_above)
     return EINA_FALSE;

   return EINA_TRUE;
}

static void
_evas_render_mapped_context_clip_set(Evas_Public_Data *e, Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, void *ctx, Evas_Proxy_Render_Data *proxy_render_data, int off_x, int off_y)
{
   int x, y, w, h;
   Eina_Bool proxy_src_clip = EINA_TRUE;

   if (proxy_render_data) proxy_src_clip = proxy_render_data->source_clip;

   if (proxy_src_clip)
     {
        x = obj->cur->cache.clip.x;
        y = obj->cur->cache.clip.y;
        w = obj->cur->cache.clip.w;
        h = obj->cur->cache.clip.h;

        RECTS_CLIP_TO_RECT(x, y, w, h,
                           obj->cur->clipper->cur->cache.clip.x,
                           obj->cur->clipper->cur->cache.clip.y,
                           obj->cur->clipper->cur->cache.clip.w,
                           obj->cur->clipper->cur->cache.clip.h);

        e->engine.func->context_clip_set(e->engine.data.output,
                                         ctx, x + off_x, y + off_y, w, h);
     }
   else
     {
        //FIXME: Consider to clip by the proxy clipper.
        if (proxy_render_data->eo_src != eo_obj)
          {
             x = obj->cur->clipper->cur->geometry.x + off_x;
             y = obj->cur->clipper->cur->geometry.y + off_y;
             w = obj->cur->clipper->cur->geometry.w;
             h = obj->cur->clipper->cur->geometry.h;
             e->engine.func->context_clip_set(e->engine.data.output, ctx, x, y,
                                              w, h);
          }
     }
}

Eina_Bool
evas_render_mapped(Evas_Public_Data *e, Evas_Object *eo_obj,
                   Evas_Object_Protected_Data *obj, void *context,
                   void *surface, int off_x, int off_y, int mapped, int ecx,
                   int ecy, int ecw, int ech,
                   Evas_Proxy_Render_Data *proxy_render_data
#ifdef REND_DBG
                   , int level
#endif
                   , Eina_Bool do_async)
{
   void *ctx;
   Evas_Object_Protected_Data *obj2;
   Eina_Bool clean_them = EINA_FALSE;
   Eina_Bool proxy_src_clip = EINA_TRUE;

   //Don't Render if the source is invisible.
   if (!proxy_render_data)
     {
        if ((evas_object_is_source_invisible(eo_obj, obj)))
          return clean_them;
     }
   else
     proxy_src_clip = proxy_render_data->source_clip;

   evas_object_clip_recalc(obj);

   RDI(level);
   RD("      { evas_render_mapped(%p, %p", e, obj);
   if (obj->name) 
     {
        RD(":%s", obj->name);
     }
   RD(",   %p, %p,   %i, %i,   %i,   %i)\n", context, surface, off_x, off_y, mapped, level);

   if (mapped)
     {
        if (proxy_src_clip)
          {
             if ((!evas_object_is_visible(eo_obj, obj)) || (obj->clip.clipees)
                 || (obj->cur->have_clipees))
               {
                  RDI(level);
                  RD("      }\n");
                  return clean_them;
               }
          }
        else
          {
             if (!evas_object_is_proxy_visible(eo_obj, obj) ||
                 (obj->clip.clipees) || (obj->cur->have_clipees))
               {
                  RDI(level);
                  RD("      }\n");
                  return clean_them;
               }
          }
     }
   else if (!(((evas_object_is_active(eo_obj, obj) && (!obj->clip.clipees) &&
                (_evas_render_can_render(eo_obj, obj))))
             ))
     {
        RDI(level);
        RD("      }\n");
        return clean_them;
     }

   // set render_pre - for child objs that may not have gotten it.
   obj->pre_render_done = EINA_TRUE;
   RD("          Hasmap: %p (%d) %p %d -> %d\n",obj->func->can_map,
      obj->func->can_map ? obj->func->can_map(eo_obj): -1,
      obj->map->cur.map, obj->map->cur.usemap,
      _evas_render_has_map(eo_obj, obj));
   if (_evas_render_has_map(eo_obj, obj))
     {
        int sw, sh;
        Eina_Bool changed = EINA_FALSE, rendered = EINA_FALSE;

        clean_them = EINA_TRUE;

        sw = obj->cur->geometry.w;
        sh = obj->cur->geometry.h;
        RDI(level);
        RD("        mapped obj: %ix%i\n", sw, sh);
        if ((sw <= 0) || (sh <= 0))
          {
             RDI(level);
             RD("      }\n");
             return clean_them;
          }

        changed = evas_object_map_update(eo_obj, off_x, off_y, sw, sh, sw, sh);

        if (obj->map->surface)
          {
             if ((obj->map->surface_w != sw) ||
                 (obj->map->surface_h != sh))
               {
                  RDI(level);
                  RD("        new surf: %ix%i\n", sw, sh);
		  EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
		    {
		      obj->layer->evas->engine.func->image_map_surface_free
			(e->engine.data.output, map_write->surface);
		      map_write->surface = NULL;
		    }
		  EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
               }
          }
        if (!obj->map->surface)
          {
             EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
               {
                  map_write->surface_w = sw;
                  map_write->surface_h = sh;

                  map_write->surface =
                    obj->layer->evas->engine.func->image_map_surface_new
                    (e->engine.data.output, map_write->surface_w,
                     map_write->surface_h,
                     map_write->cur.map->alpha);
               }
             EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);

             RDI(level);
             RD("        fisrt surf: %ix%i\n", sw, sh);
             changed = EINA_TRUE;
          }

        if (!changed) changed = evas_object_smart_changed_get(eo_obj);

        /* mark the old map as invalid, so later we don't reuse it as a
         * cache. */
        if (changed && obj->map->prev.map)
          {
             EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
               map_write->prev.valid_map = EINA_FALSE;
             EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
          }

        // clear surface before re-render
        if ((changed) && (obj->map->surface))
          {
             int off_x2, off_y2;

             RDI(level);
             RD("        children redraw\n");
             // FIXME: calculate "changes" within map surface and only clear
             // and re-render those
             if (obj->map->cur.map->alpha)
               {
                  ctx = e->engine.func->context_new(e->engine.data.output);
                  e->engine.func->context_color_set
                     (e->engine.data.output, ctx, 0, 0, 0, 0);
                  e->engine.func->context_render_op_set
                     (e->engine.data.output, ctx, EVAS_RENDER_COPY);
                  e->engine.func->rectangle_draw(e->engine.data.output,
                                                 ctx,
                                                 obj->map->surface,
                                                 0, 0,
                                                 obj->map->surface_w,
                                                 obj->map->surface_h,
                                                 EINA_FALSE);
                  e->engine.func->context_free(e->engine.data.output, ctx);
               }
             ctx = e->engine.func->context_new(e->engine.data.output);
             off_x2 = -obj->cur->geometry.x;
             off_y2 = -obj->cur->geometry.y;
             if (obj->is_smart)
               {
                  EINA_INLIST_FOREACH
                     (evas_object_smart_members_get_direct(eo_obj), obj2)
                       {
                          clean_them |= evas_render_mapped(e, obj2->object,
                                                           obj2, ctx,
                                                           obj->map->surface,
                                                           off_x2, off_y2, 1,
                                                           ecx, ecy, ecw, ech,
                                                           proxy_render_data
#ifdef REND_DBG
                                                           , level + 1
#endif
                                                           , do_async);
                          /* We aren't sure this object will be rendered by
                             normal(not proxy) drawing after, we reset this
                             only in case of normal drawing. For optmizing,
                             push this object in an array then reset them 
                             in the end of the rendering.*/
                          if (!proxy_render_data)
                            evas_object_change_reset(obj2->object);
                       }
               }
             else
               {
                  int x = 0, y = 0, w = 0, h = 0;

                  w = obj->map->surface_w;
                  h = obj->map->surface_h;
                  RECTS_CLIP_TO_RECT(x, y, w, h,
                                     obj->cur->geometry.x + off_x2,
                                     obj->cur->geometry.y + off_y2,
                                     obj->cur->geometry.w,
                                     obj->cur->geometry.h);

                  e->engine.func->context_clip_set(e->engine.data.output,
                                                   ctx, x, y, w, h);
                  obj->func->render(eo_obj, obj, obj->private_data,
				    e->engine.data.output, ctx,
                                    obj->map->surface, off_x2, off_y2,
                                    EINA_FALSE);
               }
             e->engine.func->context_free(e->engine.data.output, ctx);
             rendered = EINA_TRUE;
          }

        RDI(level);
        RD("        draw map\n");

        if (rendered)
          {
             EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
               {
                  map_write->surface = e->engine.func->image_dirty_region
                    (e->engine.data.output, map_write->surface,
                     0, 0, map_write->surface_w, map_write->surface_h);

                  map_write->cur.valid_map = EINA_TRUE;
               }
             EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
          }
        e->engine.func->context_clip_unset(e->engine.data.output,
                                           context);
        if (obj->map->surface)
          {
             if (obj->cur->clipper)
               {
                  evas_object_clip_recalc(obj);

                  if (obj->is_smart)
                    {
                       Evas_Object *tobj;

                       EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
                         {
                            state_write->cache.clip.dirty = EINA_TRUE;
                         }
                       EINA_COW_STATE_WRITE_END(obj, state_write, cur);

                       EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map,
                                            Evas_Object_Map_Data, map_write)
                         {
                            tobj = map_write->cur.map_parent;
                            map_write->cur.map_parent =
                               obj->cur->clipper->map->cur.map_parent;
                            map_write->cur.map_parent = tobj;
                         }
                       EINA_COW_WRITE_END(evas_object_map_cow, obj->map,
                                          map_write);
                    }
                  _evas_render_mapped_context_clip_set(e, eo_obj, obj, context,
                                                       proxy_render_data, off_x,
                                                       off_y);
               }
          }
//        if (surface == e->engine.data.output)
          e->engine.func->context_clip_clip(e->engine.data.output,
                                            context,
                                            ecx, ecy, ecw, ech);
        if (obj->cur->cache.clip.visible || !proxy_src_clip)
          {
             obj->layer->evas->engine.func->context_multiplier_unset
               (e->engine.data.output, context);
             obj->layer->evas->engine.func->context_render_op_set
               (e->engine.data.output, context, obj->cur->render_op);
             evas_draw_image_map_async_check
               (obj, e->engine.data.output, context, surface,
                obj->map->surface, obj->map->spans,
                obj->map->cur.map->smooth, 0, do_async);
          }
        // FIXME: needs to cache these maps and
        // keep them only rendering updates
        //        obj->layer->evas->engine.func->image_map_surface_free
        //          (e->engine.data.output, obj->map->surface);
        //        obj->map->surface = NULL;
     }
   else
     {
#if 0
        if (0 && obj->cur->cached_surface)
          fprintf(stderr, "We should cache '%s' [%i, %i, %i, %i]\n",
                  evas_object_type_get(eo_obj),
                  obj->cur->bounding_box.x, obj->cur->bounding_box.x,
                  obj->cur->bounding_box.w, obj->cur->bounding_box.h);
#endif
        if (mapped)
          {
             RDI(level);
             RD("        draw child of mapped obj\n");
             ctx = e->engine.func->context_new(e->engine.data.output);
             if (obj->is_smart)
               {
                  EINA_INLIST_FOREACH
                     (evas_object_smart_members_get_direct(eo_obj), obj2)
                       {
                          clean_them |= evas_render_mapped(e, obj2->object,
                                                           obj2, ctx, surface,
                                                           off_x, off_y, 1,
                                                           ecx, ecy, ecw, ech,
                                                           proxy_render_data
#ifdef REND_DBG
                                                           , level + 1
#endif
                                                           , do_async);
                          /* We aren't sure this object will be rendered by
                             normal(not proxy) drawing after, we reset this
                             only in case of normal drawing. For optmizing,
                             push this object in an array then reset them 
                             in the end of the rendering.*/
                          if (!proxy_render_data)
                            evas_object_change_reset(obj2->object);
                       }
               }
             else
               {
                  RDI(level);

                  if (obj->cur->clipper)
                    {
                       if (_evas_render_has_map(eo_obj, obj))
                         evas_object_clip_recalc(obj);
                       _evas_render_mapped_context_clip_set(e, eo_obj, obj, ctx,
                                                            proxy_render_data,
                                                           off_x, off_y);
                    }
                  obj->func->render(eo_obj, obj, obj->private_data,
				    e->engine.data.output, ctx,
                                    surface, off_x, off_y, EINA_FALSE);
               }
             e->engine.func->context_free(e->engine.data.output, ctx);
          }
        else
          {
             if (obj->cur->clipper)
               {
                  int x, y, w, h;

                  if (_evas_render_has_map(eo_obj, obj))
                    evas_object_clip_recalc(obj);
                  x = obj->cur->cache.clip.x;
                  y = obj->cur->cache.clip.y;
                  w = obj->cur->cache.clip.w;
                  h = obj->cur->cache.clip.h;
                  RECTS_CLIP_TO_RECT(x, y, w, h,
                                     obj->cur->clipper->cur->cache.clip.x,
                                     obj->cur->clipper->cur->cache.clip.y,
                                     obj->cur->clipper->cur->cache.clip.w,
                                     obj->cur->clipper->cur->cache.clip.h);
                  e->engine.func->context_clip_set(e->engine.data.output,
                                                   context,
                                                   x + off_x, y + off_y, w, h);
                  e->engine.func->context_clip_clip(e->engine.data.output,
                                                    context,
                                                    ecx, ecy, ecw, ech);
               }

             RDI(level);
             RD("        draw normal obj\n");
             obj->func->render(eo_obj, obj, obj->private_data,
			       e->engine.data.output, context, surface,
                               off_x, off_y, do_async);
          }
        if (obj->changed_map) clean_them = EINA_TRUE;
     }
   RDI(level);
   RD("      }\n");

   return clean_them;
}

static void
_evas_render_cutout_add(Evas_Public_Data *e, Evas_Object_Protected_Data *obj, int off_x, int off_y)
{
   if (evas_object_is_source_invisible(obj->object, obj)) return;
   if (evas_object_is_opaque(obj->object, obj))
     {
        Evas_Coord cox, coy, cow, coh;

        cox = obj->cur->cache.clip.x;
        coy = obj->cur->cache.clip.y;
        cow = obj->cur->cache.clip.w;
        coh = obj->cur->cache.clip.h;
        if ((obj->map->cur.map) && (obj->map->cur.usemap))
          {
             Evas_Object *eo_oo;
             Evas_Object_Protected_Data *oo;

             eo_oo = obj->object;
             oo = eo_data_scope_get(eo_oo, EVAS_OBJ_CLASS);
             while (oo->cur->clipper)
               {
                  if ((oo->cur->clipper->map->cur.map_parent
                       != oo->map->cur.map_parent) &&
                      (!((oo->map->cur.map) && (oo->map->cur.usemap))))
                    break;
                  RECTS_CLIP_TO_RECT(cox, coy, cow, coh,
                                     oo->cur->geometry.x,
                                     oo->cur->geometry.y,
                                     oo->cur->geometry.w,
                                     oo->cur->geometry.h);
                  eo_oo = oo->cur->clipper->object;
                  oo = oo->cur->clipper;
               }
          }
        e->engine.func->context_cutout_add
          (e->engine.data.output, e->engine.data.context,
              cox + off_x, coy + off_y, cow, coh);
     }
   else
     {
        if (obj->func->get_opaque_rect)
          {
             Evas_Coord obx, oby, obw, obh;

             obj->func->get_opaque_rect(obj->object, obj, obj->private_data, &obx, &oby, &obw, &obh);
             if ((obw > 0) && (obh > 0))
               {
                  obx += off_x;
                  oby += off_y;
                  RECTS_CLIP_TO_RECT(obx, oby, obw, obh,
                                     obj->cur->cache.clip.x + off_x,
                                     obj->cur->cache.clip.y + off_y,
                                     obj->cur->cache.clip.w,
                                     obj->cur->cache.clip.h);
                  e->engine.func->context_cutout_add
                    (e->engine.data.output, e->engine.data.context,
                        obx, oby, obw, obh);
               }
          }
     }
}

void
evas_render_rendering_wait(Evas_Public_Data *evas)
{
   while (evas->rendering) evas_async_events_process_blocking();
}

/*
 * Syncs ALL async rendering canvases. Must be called in the main thread.
 */
void
evas_all_sync(void)
{
   Evas_Public_Data *evas;

   if (!_rendering_evases) return;

   evas = eina_list_data_get(eina_list_last(_rendering_evases));
   evas_render_rendering_wait(evas);

   assert(_rendering_evases == NULL);
}

static Eina_Bool
_drop_scie_ref(const void *container EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   evas_common_rgba_image_scalecache_item_unref(data);
   return EINA_TRUE;
}

static Eina_Bool
_drop_image_cache_ref(const void *container EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get() && evas_cache2_image_cached(data))
     evas_cache2_image_close((Image_Entry *)data);
   else
#endif
     evas_cache_image_drop((Image_Entry *)data);

   return EINA_TRUE;
}

static void
_cb_always_call(Evas *eo_e, Evas_Callback_Type type, void *event_info)
{
   int freeze_num = 0, i;

   eo_do(eo_e, eo_event_freeze_get(&freeze_num));
   for (i = 0; i < freeze_num; i++) eo_do(eo_e, eo_event_thaw());
   evas_event_callback_call(eo_e, type, event_info);
   for (i = 0; i < freeze_num; i++) eo_do(eo_e, eo_event_freeze());
}

static Eina_Bool
evas_render_updates_internal(Evas *eo_e,
                             unsigned char make_updates,
                             unsigned char do_draw,
                             Evas_Render_Done_Cb done_func,
                             void *done_data,
                             Eina_Bool do_async)
{
   Evas_Object *eo_obj;
   Evas_Object_Protected_Data *obj;
   Evas_Public_Data *e;
   Eina_List *ll;
   void *surface;
   Eina_Bool clean_them = EINA_FALSE;
   Eina_Bool alpha;
   Eina_Rectangle *r;
   int ux, uy, uw, uh;
   int cx, cy, cw, ch;
   unsigned int i, j;
   int redraw_all = 0;
   Eina_Bool haveup = 0;
   Evas_Render_Mode render_mode = EVAS_RENDER_MODE_UNDEF;
#ifdef EVAS_RENDER_DEBUG_TIMING
   double start_time = _time_get();
#endif
   Eina_Rectangle clip_rect;

   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   e = eo_data_scope_get(eo_e, EVAS_CLASS);
   if (!e->changed) return EINA_FALSE;

   if (e->rendering)
     {
        if (do_async)
          return EINA_FALSE;
        else
          {
              WRN("Mixing render sync as already doing async "
                  "render! Syncing! e=%p [%s]", e,
                  e->engine.module->definition->name);
              evas_render_rendering_wait(e);
          }
     }

#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get())
      evas_cserve2_dispatch();
#endif
   evas_call_smarts_calculate(eo_e);

   RD("[--- RENDER EVAS (size: %ix%i)\n", e->viewport.w, e->viewport.h);

   _cb_always_call(eo_e, EVAS_CALLBACK_RENDER_PRE, NULL);

   /* Check if the modified object mean recalculating every thing */
   if (!e->invalidate)
     _evas_render_check_pending_objects(&e->pending_objects, eo_e, e);

   /* phase 1. add extra updates for changed objects */
   if (e->invalidate || e->render_objects.count <= 0)
     clean_them = _evas_render_phase1_process(e,
                                              &e->active_objects,
                                              &e->restack_objects,
                                              &e->delete_objects,
                                              &e->render_objects,
                                              &redraw_all);

   if (!strncmp(e->engine.module->definition->name, "wayland", 7))
     {
        evas_event_freeze(eo_e);
        /* check for master clip */
        if (!e->framespace.clip)
          {
             e->framespace.clip = evas_object_rectangle_add(eo_e);
             evas_object_color_set(e->framespace.clip, 255, 255, 255, 255);
             evas_object_move(e->framespace.clip, 0, 0);
             evas_object_resize(e->framespace.clip, 
                                e->viewport.w - e->framespace.w, 
                                e->viewport.h - e->framespace.h);
             evas_object_pass_events_set(e->framespace.clip, EINA_TRUE);
             evas_object_layer_set(e->framespace.clip, EVAS_LAYER_MIN);
             evas_object_show(e->framespace.clip);
          }

        /* setup master clip rectangle for comparison to objects */
        EINA_RECTANGLE_SET(&clip_rect, e->framespace.x, e->framespace.y, 
                           e->viewport.w - e->framespace.w, 
                           e->viewport.h - e->framespace.h);

        for (i = 0; i < e->render_objects.count; ++i)
          {
             Eina_Rectangle obj_rect;

             obj = eina_array_data_get(&e->render_objects, i);
             if (obj->delete_me) continue;
             if (obj->is_frame) continue;
             if (obj->object == e->framespace.clip) continue;

             /* setup object rectangle for comparison to clip rectangle */
             EINA_RECTANGLE_SET(&obj_rect, 
                                obj->cur->geometry.x, obj->cur->geometry.y, 
                                obj->cur->geometry.w, obj->cur->geometry.h);

             /* check if this object intersects with the master clip */
             if (!eina_rectangles_intersect(&clip_rect, &obj_rect))
               continue;

             if (!evas_object_clip_get(obj->object))
               {
                  /* clip this object to the master clip */
                  evas_object_clip_set(obj->object, e->framespace.clip);
               }
          }
        if (!evas_object_clipees_get(e->framespace.clip))
          evas_object_hide(e->framespace.clip);
        evas_event_thaw(eo_e);
     }

   /* phase 1.5. check if the video should be inlined or stay in their overlay */
   alpha = e->engine.func->canvas_alpha_get(e->engine.data.output,
                                            e->engine.data.context);

   EINA_LIST_FOREACH(e->video_objects, ll, eo_obj)
     {
        /* we need the surface to be transparent to display the underlying overlay */
       if (alpha && _evas_render_can_use_overlay(e, eo_obj))
          _evas_object_image_video_overlay_show(eo_obj);
        else
          _evas_object_image_video_overlay_hide(eo_obj);
     }
   /* phase 1.8. pre render for proxy */
   _evas_render_phase1_direct(e, &e->active_objects, &e->restack_objects,
                              &e->delete_objects, &e->render_objects);

   /* phase 2. force updates for restacks */
   for (i = 0; i < e->restack_objects.count; ++i)
     {
        obj = eina_array_data_get(&e->restack_objects, i);
        obj->func->render_pre(obj->object, obj, obj->private_data);
        _evas_render_prev_cur_clip_cache_add(e, obj);
     }
   OBJS_ARRAY_CLEAN(&e->restack_objects);

   /* phase 3. add exposes */
   EINA_LIST_FREE(e->damages, r)
     {
        e->engine.func->output_redraws_rect_add(e->engine.data.output,
                                                r->x, r->y, r->w, r->h);
        eina_rectangle_free(r);
     }

   /* phase 4. framespace, output & viewport changes */
   if (e->viewport.changed)
     {
        e->engine.func->output_redraws_rect_add(e->engine.data.output,
                                                0, 0,
                                                e->output.w, e->output.h);
     }
   if (e->output.changed)
     {
        e->engine.func->output_resize(e->engine.data.output,
                                      e->output.w, e->output.h);
        e->engine.func->output_redraws_rect_add(e->engine.data.output,
                                                0, 0,
                                                e->output.w, e->output.h);
     }
   if ((e->output.w != e->viewport.w) || (e->output.h != e->viewport.h))
     {
        ERR("viewport size != output size!");
     }

   if (e->framespace.changed)
     {
        /* NB: If the framespace changes, we need to add a redraw rectangle
         * which covers the Whole viewport. This is because 'framespace' is 
         * defined as "the space IN the viewport which is Occupied by the 
         * window frame" */
        e->engine.func->output_redraws_rect_add(e->engine.data.output,
                                                e->viewport.x, e->viewport.y,
                                                e->viewport.w, e->viewport.h);
     }

   if (redraw_all)
     {
        e->engine.func->output_redraws_rect_add(e->engine.data.output, 0, 0,
                                                e->output.w, e->output.h);
     }

   /* phase 5. add obscures */
   EINA_LIST_FOREACH(e->obscures, ll, r)
     {
        e->engine.func->output_redraws_rect_del(e->engine.data.output,
                                                r->x, r->y, r->w, r->h);
     }
   /* build obscure objects list of active objects that obscure */
   for (i = 0; i < e->active_objects.count; ++i)
     {
        obj = eina_array_data_get(&e->active_objects, i);
        eo_obj = obj->object;
        if (UNLIKELY((evas_object_is_opaque(eo_obj, obj) ||
                      ((obj->func->has_opaque_rect) &&
                       (obj->func->has_opaque_rect(eo_obj, obj, obj->private_data)))) &&
                     evas_object_is_visible(eo_obj, obj) &&
                     (!obj->clip.clipees) &&
                     (obj->cur->visible) &&
                     (!obj->delete_me) &&
                     (obj->cur->cache.clip.visible) &&
                     (!obj->is_smart)))
          /*	  obscuring_objects = eina_list_append(obscuring_objects, obj); */
          OBJ_ARRAY_PUSH(&e->obscuring_objects, obj);
     }

   /* save this list */
   /*    obscuring_objects_orig = obscuring_objects; */
   /*    obscuring_objects = NULL; */
   /* phase 6. go thru each update rect and render objects in it*/
   if (do_draw)
     {
        unsigned int offset = 0;
        int fx = e->framespace.x;
        int fy = e->framespace.y;

        while ((surface =
                e->engine.func->output_redraws_next_update_get
                (e->engine.data.output,
                 &ux, &uy, &uw, &uh,
                 &cx, &cy, &cw, &ch)))
          {
             int off_x, off_y;
             Render_Updates *ru;

             RD("  [--- UPDATE %i %i %ix%i\n", ux, uy, uw, uh);
             if (do_async)
               {
                  ru = malloc(sizeof(*ru));
                  ru->surface = surface;
                  NEW_RECT(ru->area, ux, uy, uw, uh);
                  e->render.updates = eina_list_append(e->render.updates, ru);
                  evas_cache_image_ref(surface);
               }
             else if (make_updates)
               {
                  Eina_Rectangle *rect;

                  NEW_RECT(rect, ux, uy, uw, uh);
                  if (rect)
                     e->render.updates = eina_list_append(e->render.updates,
                                                          rect);
               }
             haveup = EINA_TRUE;
             off_x = cx - ux;
             off_y = cy - uy;
             /* build obscuring objects list (in order from bottom to top) */
             if (alpha)
               {
                  e->engine.func->context_clip_set(e->engine.data.output,
                                                   e->engine.data.context,
                                                   ux + off_x, uy + off_y, uw, uh);
               }
             for (i = 0; i < e->obscuring_objects.count; ++i)
               {
                  obj = (Evas_Object_Protected_Data *)eina_array_data_get
                     (&e->obscuring_objects, i);
                  if (evas_object_is_in_output_rect(obj->object, obj, ux - fx, uy - fy, uw, uh))
                    {
                       OBJ_ARRAY_PUSH(&e->temporary_objects, obj);

                       /* reset the background of the area if needed (using cutout and engine alpha flag to help) */
                       if (alpha)
                         _evas_render_cutout_add(e, obj, off_x + fx, off_y + fy);
                    }
               }
             if (alpha)
               {
                  e->engine.func->context_color_set(e->engine.data.output,
                                                    e->engine.data.context,
                                                    0, 0, 0, 0);
                  e->engine.func->context_multiplier_unset
                     (e->engine.data.output, e->engine.data.context);
                  e->engine.func->context_render_op_set(e->engine.data.output,
                                                        e->engine.data.context,
                                                        EVAS_RENDER_COPY);
                  e->engine.func->rectangle_draw(e->engine.data.output,
                                                 e->engine.data.context,
                                                 surface,
                                                 cx, cy, cw, ch, do_async);
                  e->engine.func->context_cutout_clear(e->engine.data.output,
                                                       e->engine.data.context);
                  e->engine.func->context_clip_unset(e->engine.data.output,
                                                     e->engine.data.context);
               }

             /* render all object that intersect with rect */
             for (i = 0; i < e->active_objects.count; ++i)
               {
                  obj = eina_array_data_get(&e->active_objects, i);
                  eo_obj = obj->object;

                  /* if it's in our outpout rect and it doesn't clip anything */
                  RD("    OBJ: [%p", obj);
                  if (obj->name) 
                    {
                       RD(":%s", obj->name);
                    }
                  RD("] '%s' %i %i %ix%i\n", obj->type, obj->cur->geometry.x, obj->cur->geometry.y, obj->cur->geometry.w, obj->cur->geometry.h);
                  if ((evas_object_is_in_output_rect(eo_obj, obj, ux - fx, uy - fy, uw, uh) ||
                       (obj->is_smart)) &&
                      (!obj->clip.clipees) &&
                      (obj->cur->visible) &&
                      (!obj->delete_me) &&
                      (obj->cur->cache.clip.visible) &&
//		      (!obj->is_smart) &&
                      ((obj->cur->color.a > 0 || obj->cur->render_op != EVAS_RENDER_BLEND)))
                    {
                       int x, y, w, h;

                       RD("      DRAW (vis: %i, a: %i, clipees: %p\n", obj->cur->visible, obj->cur->color.a, obj->clip.clipees);
                       if ((e->temporary_objects.count > offset) &&
                           (eina_array_data_get(&e->temporary_objects, offset) == obj))
                         offset++;
                       x = cx; y = cy; w = cw; h = ch;
                       if (((w > 0) && (h > 0)) || (obj->is_smart))
                         {
                            if (!obj->is_smart)
                              {
                                 RECTS_CLIP_TO_RECT(x, y, w, h,
                                                    obj->cur->cache.clip.x + off_x + fx,
                                                    obj->cur->cache.clip.y + off_y + fy,
                                                    obj->cur->cache.clip.w,
                                                    obj->cur->cache.clip.h);
                              }

                            e->engine.func->context_clip_set(e->engine.data.output,
                                                             e->engine.data.context,
                                                             x, y, w, h);
#if 1 /* FIXME: this can slow things down... figure out optimum... coverage */
                            for (j = offset; j < e->temporary_objects.count; ++j)
                              {
                                 Evas_Object_Protected_Data *obj2;

                                 obj2 = (Evas_Object_Protected_Data *)eina_array_data_get
                                   (&e->temporary_objects, j);
                                 _evas_render_cutout_add(e, obj2, off_x + fx, off_y + fy);
                              }
#endif
                            clean_them |= evas_render_mapped(e, eo_obj, obj, e->engine.data.context,
                                                             surface, off_x + fx,
                                                             off_y + fy, 0,
                                                             cx, cy, cw, ch,
                                                             NULL
#ifdef REND_DBG
                                                             , 1
#endif
                                                             , do_async);
                            e->engine.func->context_cutout_clear(e->engine.data.output,
                                                                 e->engine.data.context);
                         }
                    }
               }

             if (!do_async) render_mode = EVAS_RENDER_MODE_SYNC;
             else render_mode = EVAS_RENDER_MODE_ASYNC_INIT;

             e->engine.func->output_redraws_next_update_push(e->engine.data.output,
                                                             surface,
                                                             ux, uy, uw, uh,
                                                             render_mode);

             /* free obscuring objects list */
             OBJS_ARRAY_CLEAN(&e->temporary_objects);
             RD("  ---]\n");
          }

        if (do_async)
          {
             eo_ref(eo_e);
             e->rendering = EINA_TRUE;
             _rendering_evases = eina_list_append(_rendering_evases, e);

             evas_thread_queue_flush((Evas_Thread_Command_Cb)done_func, done_data);
          }
        else if (haveup)
          {
             EINA_LIST_FOREACH(e->video_objects, ll, eo_obj)
               {
                  _evas_object_image_video_overlay_do(eo_obj);
               }
             _cb_always_call(eo_e, EVAS_CALLBACK_RENDER_FLUSH_PRE, NULL);
             e->engine.func->output_flush(e->engine.data.output,
                                          EVAS_RENDER_MODE_SYNC);
             _cb_always_call(eo_e, EVAS_CALLBACK_RENDER_FLUSH_POST, NULL);
          }
     }

   if (!do_async)
     {
        /* clear redraws */
        e->engine.func->output_redraws_clear(e->engine.data.output);
     }

   /* and do a post render pass */
   for (i = 0; i < e->active_objects.count; ++i)
     {
        obj = eina_array_data_get(&e->active_objects, i);
        eo_obj = obj->object;
        obj->pre_render_done = EINA_FALSE;
        RD("    OBJ [%p", obj);
        if (obj->name) 
          {
             RD(":%s", obj->name);
          }
        RD("] post... %i %i\n", obj->changed, do_draw);
        if ((clean_them) || (obj->changed && do_draw))
          {
             RD("    OBJ [%p", obj);
             if (obj->name) 
               {
                  RD(":%s", obj->name);
               }
             RD("] post... func\n");
             obj->func->render_post(eo_obj, obj, obj->private_data);
             obj->restack = EINA_FALSE;
             evas_object_change_reset(eo_obj);
          }
        /* moved to other pre-process phase 1
           if (obj->delete_me == 2)
           {
           delete_objects = eina_list_append(delete_objects, obj);
           }
           else if (obj->delete_me != 0) obj->delete_me++;
         */
     }
   /* free our obscuring object list */
   OBJS_ARRAY_CLEAN(&e->obscuring_objects);

   /* If some object are still marked as changed, do not remove
      them from the pending list. */
   eina_array_remove(&e->pending_objects, pending_change, NULL);

   /* Reinsert parent of changed object in the pending changed state */
   for (i = 0; i < e->pending_objects.count; ++i)
     {
        obj = eina_array_data_get(&e->pending_objects, i);
        eo_obj = obj->object;
        if (obj->smart.parent)
          {
             Evas_Object_Protected_Data *smart_parent;

             smart_parent = eo_data_scope_get(obj->smart.parent,
                                              EVAS_OBJ_CLASS);
             evas_object_change(obj->smart.parent, smart_parent);
          }
     }

   for (i = 0; i < e->render_objects.count; ++i)
     {
        obj = eina_array_data_get(&e->render_objects, i);
        eo_obj = obj->object;
        obj->pre_render_done = EINA_FALSE;
        if ((obj->changed) && (do_draw))
          {
             obj->func->render_post(eo_obj, obj, obj->private_data);
             obj->restack = EINA_FALSE;
             evas_object_change_reset(eo_obj);
          }
     }

   if (!strncmp(e->engine.module->definition->name, "wayland", 7))
     {
        /* unclip objects from master clip */
        for (i = 0; i < e->render_objects.count; ++i)
          {
             obj = eina_array_data_get(&e->render_objects, i);
             if (obj->is_frame) continue;
             if (obj->object == e->framespace.clip) continue;

             if (evas_object_clip_get(obj->object) == e->framespace.clip)
               {
                  /* unclip this object from the master clip */
                  evas_object_clip_unset(obj->object);
               }
          }

        /* delete master clip */
        evas_object_del(e->framespace.clip);
        e->framespace.clip = NULL;
     }

   e->changed = EINA_FALSE;
   e->viewport.changed = EINA_FALSE;
   e->output.changed = EINA_FALSE;
   e->framespace.changed = EINA_FALSE;
   e->invalidate = EINA_FALSE;

   // always clean... lots of mem waste!
   /* If their are some object to restack or some object to delete,
    * it's useless to keep the render object list around. */
   if (clean_them)
     {
        OBJS_ARRAY_CLEAN(&e->active_objects);
        OBJS_ARRAY_CLEAN(&e->render_objects);
        OBJS_ARRAY_CLEAN(&e->restack_objects);
        OBJS_ARRAY_CLEAN(&e->temporary_objects);
        eina_array_foreach(&e->clip_changes, _evas_clip_changes_free, NULL);
        eina_array_clean(&e->clip_changes);
/* we should flush here and have a mempool system for this        
        eina_array_flush(&e->active_objects);
        eina_array_flush(&e->render_objects);
        eina_array_flush(&e->restack_objects);
        eina_array_flush(&e->delete_objects);
        eina_array_flush(&e->obscuring_objects);
        eina_array_flush(&e->temporary_objects);
        eina_array_flush(&e->clip_changes);
 */
        e->invalidate = EINA_TRUE;
     }

   /* delete all objects flagged for deletion now */
   for (i = 0; i < e->delete_objects.count; ++i)
     {
        obj = eina_array_data_get(&e->delete_objects, i);
        evas_object_free(obj->object, 1);
     }
   eina_array_clean(&e->delete_objects);
   /* if we deleted no objects this frame or we deleted a lot (> 1024) then
    * try and reset the deleted objects array to empty (no mem used) for
    * efficiency */
   if ((e->delete_objects.count == 0) || (e->delete_objects.count > 1024))
     eina_array_flush(&e->delete_objects);
     
   evas_module_clean();

   if (!do_async)
     {
        Evas_Event_Render_Post post;

        post.updated_area = e->render.updates;
        _cb_always_call(eo_e, EVAS_CALLBACK_RENDER_POST, e->render.updates ? &post : NULL);
     }

   RD("---]\n");

#ifdef EVAS_RENDER_DEBUG_TIMING
   _accumulate_time(start_time, do_async ? &async_accumulator : &sync_accumulator);
#endif

   return EINA_TRUE;
}

static Eina_Bool
_drop_glyph_ref(const void *container EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   evas_common_font_glyphs_unref(data);
   return EINA_TRUE;
}

static Eina_Bool
_drop_texts_ref(const void *container EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   evas_common_font_fonts_unref(data);

   return EINA_TRUE;
}

static void
evas_render_wakeup(Evas *eo_e)
{
   Evas_Event_Render_Post post;
   Render_Updates *ru;
   Eina_Bool haveup = EINA_FALSE;
   Eina_List *ret_updates = NULL;
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);

   EINA_LIST_FREE(e->render.updates, ru)
     {
        /* punch rect out */
        e->engine.func->output_redraws_next_update_push
          (e->engine.data.output, ru->surface,
           ru->area->x, ru->area->y, ru->area->w, ru->area->h,
           EVAS_RENDER_MODE_ASYNC_END);

        ret_updates = eina_list_append(ret_updates, ru->area);
        evas_cache_image_drop(ru->surface);
        free(ru);
        haveup = EINA_TRUE;
     }

   /* flush redraws */
   if (haveup)
     {
        Eina_List *ll;
        Evas_Object *eo_obj;
        EINA_LIST_FOREACH(e->video_objects, ll, eo_obj)
          {
             _evas_object_image_video_overlay_do(eo_obj);
          }
        _cb_always_call(eo_e, EVAS_CALLBACK_RENDER_FLUSH_PRE, NULL);
        e->engine.func->output_flush(e->engine.data.output,
                                     EVAS_RENDER_MODE_ASYNC_END);
        _cb_always_call(eo_e, EVAS_CALLBACK_RENDER_FLUSH_POST, NULL);
     }

   /* clear redraws */
   e->engine.func->output_redraws_clear(e->engine.data.output);

   /* unref queues */
   eina_array_foreach(&e->scie_unref_queue, _drop_scie_ref, NULL);
   eina_array_clean(&e->scie_unref_queue);
   evas_common_rgba_image_scalecache_prune();

   eina_array_foreach(&e->image_unref_queue, _drop_image_cache_ref, NULL);
   eina_array_clean(&e->image_unref_queue);

   eina_array_foreach(&e->glyph_unref_queue, _drop_glyph_ref, NULL);
   eina_array_clean(&e->glyph_unref_queue);

   eina_array_foreach(&e->texts_unref_queue, _drop_texts_ref, NULL);
   eina_array_clean(&e->texts_unref_queue);

   /* post rendering */
   _rendering_evases = eina_list_remove(_rendering_evases, e);
   e->rendering = EINA_FALSE;

   post.updated_area = ret_updates;
   _cb_always_call(eo_e, EVAS_CALLBACK_RENDER_POST, &post);

   evas_render_updates_free(ret_updates);

   eo_unref(eo_e);
}

static void
evas_render_async_wakeup(void *target, Evas_Callback_Type type EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Public_Data *e = target;
   evas_render_wakeup(e->evas);
}

static void
evas_render_pipe_wakeup(void *data)
{
   evas_async_events_put(data, 0, NULL, evas_render_async_wakeup);
}

EAPI void
evas_render_updates_free(Eina_List *updates)
{
   Eina_Rectangle *r;

   EINA_LIST_FREE(updates, r)
      eina_rectangle_free(r);
}

EOLIAN Eina_Bool
_evas_render_async(Eo *eo_e, Evas_Public_Data *e)
{
   static int render_2 = -1;

   if (render_2 == -1)
     {
        if (getenv("EVAS_RENDER2")) render_2 = 1;
        else render_2 = 0;
     }
   if (render_2)
     return _evas_render2_begin(eo_e, EINA_TRUE, EINA_TRUE, EINA_TRUE);
   else
     return evas_render_updates_internal(eo_e, 1, 1, evas_render_pipe_wakeup,
                                         e, EINA_TRUE);
}

static Eina_List *
evas_render_updates_internal_wait(Evas *eo_e,
                                  unsigned char make_updates,
                                  unsigned char do_draw)
{
   Eina_List *ret = NULL;
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   static int render_2 = -1;

   if (render_2 == -1)
     {
        if (getenv("EVAS_RENDER2")) render_2 = 1;
        else render_2 = 0;
     }
   if (render_2)
     {
        if (!_evas_render2_begin(eo_e, make_updates, do_draw, EINA_FALSE))
          return NULL;
     }
   else
     {
        if (!evas_render_updates_internal(eo_e, make_updates, do_draw, NULL,
                                          NULL, EINA_FALSE))
          return NULL;
     }

   ret = e->render.updates;
   e->render.updates = NULL;

   return ret;
}
EOLIAN Eina_List*
_evas_render_updates(Eo *eo_e, Evas_Public_Data *e)
{
   if (!e->changed) return NULL;
   return evas_render_updates_internal_wait(eo_e, 1, 1);
}

EOLIAN void
_evas_render(Eo *eo_e, Evas_Public_Data *e)
{
   if (!e->changed) return;
   evas_render_updates_internal_wait(eo_e, 0, 1);
}

EOLIAN void
_evas_norender(Eo *eo_e, Evas_Public_Data *_pd EINA_UNUSED)
{
   //   if (!e->changed) return;
   evas_render_updates_internal_wait(eo_e, 0, 0);
}

EOLIAN void
_evas_render_idle_flush(Eo *eo_e, Evas_Public_Data *e)
{
   static int render_2 = -1;

   if (render_2 == -1)
     {
        if (getenv("EVAS_RENDER2")) render_2 = 1;
        else render_2 = 0;
     }
   if (render_2)
     {
        _evas_render2_idle_flush(eo_e);
     }
   else
     {

        evas_render_rendering_wait(e);
        
        evas_fonts_zero_pressure(eo_e);
        
        if ((e->engine.func) && (e->engine.func->output_idle_flush) &&
            (e->engine.data.output))
          e->engine.func->output_idle_flush(e->engine.data.output);
        
        OBJS_ARRAY_FLUSH(&e->active_objects);
        OBJS_ARRAY_FLUSH(&e->render_objects);
        OBJS_ARRAY_FLUSH(&e->restack_objects);
        OBJS_ARRAY_FLUSH(&e->delete_objects);
        OBJS_ARRAY_FLUSH(&e->obscuring_objects);
        OBJS_ARRAY_FLUSH(&e->temporary_objects);
        eina_array_foreach(&e->clip_changes, _evas_clip_changes_free, NULL);
        eina_array_clean(&e->clip_changes);
        
        e->invalidate = EINA_TRUE;
     }
}

EOLIAN void
_evas_sync(Eo *eo_e, Evas_Public_Data *e)
{
   static int render_2 = -1;

   if (render_2 == -1)
     {
        if (getenv("EVAS_RENDER2")) render_2 = 1;
        else render_2 = 0;
     }
   if (render_2)
     _evas_render2_wait(eo_e);
   else
     evas_render_rendering_wait(e);
}

void
_evas_render_dump_map_surfaces(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   if ((obj->map->cur.map) && obj->map->surface)
     {
        obj->layer->evas->engine.func->image_map_surface_free
           (obj->layer->evas->engine.data.output, obj->map->surface);
        EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
          map_write->surface = NULL;
        EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
     }

   if (obj->is_smart)
     {
        Evas_Object_Protected_Data *obj2;

        EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(eo_obj), obj2)
           _evas_render_dump_map_surfaces(obj2->object);
     }
}

EOLIAN void
_evas_render_dump(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   static int render_2 = -1;

   if (render_2 == -1)
     {
        if (getenv("EVAS_RENDER2")) render_2 = 1;
        else render_2 = 0;
     }
   if (render_2)
     {
        _evas_render2_dump(eo_e);
     }
   else
     {
        Evas_Layer *lay;
        
        evas_all_sync();
        evas_cache_async_freeze();
        
        EINA_INLIST_FOREACH(e->layers, lay)
          {
             Evas_Object_Protected_Data *obj;
             
             EINA_INLIST_FOREACH(lay->objects, obj)
               {
                  if (obj->proxy)
                    {
                       EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, obj->proxy, Evas_Object_Proxy_Data, proxy_write)
                         {
                            if (proxy_write->surface)
                              {
                                 e->engine.func->image_map_surface_free(e->engine.data.output,
                                                                        proxy_write->surface);
                                 proxy_write->surface = NULL;
                              }
                         }
                       EINA_COW_WRITE_END(evas_object_proxy_cow, obj->proxy, proxy_write);
                    }
                  if ((obj->type) && (!strcmp(obj->type, "image")))
                    evas_object_inform_call_image_unloaded(obj->object);
                  _evas_render_dump_map_surfaces(obj->object);
               }
          }
        if ((e->engine.func) && (e->engine.func->output_dump) &&
            (e->engine.data.output))
          e->engine.func->output_dump(e->engine.data.output);

#define GC_ALL(Cow) \
   while (eina_cow_gc(Cow))

        GC_ALL(evas_object_proxy_cow);
        GC_ALL(evas_object_map_cow);
        GC_ALL(evas_object_image_pixels_cow);
        GC_ALL(evas_object_image_load_opts_cow);
        GC_ALL(evas_object_image_state_cow);
        
        evas_fonts_zero_pressure(eo_e);
   
        if ((e->engine.func) && (e->engine.func->output_idle_flush) &&
            (e->engine.data.output))
          e->engine.func->output_idle_flush(e->engine.data.output);
   
        OBJS_ARRAY_FLUSH(&e->active_objects);
        OBJS_ARRAY_FLUSH(&e->render_objects);
        OBJS_ARRAY_FLUSH(&e->restack_objects);
        OBJS_ARRAY_FLUSH(&e->delete_objects);
        OBJS_ARRAY_FLUSH(&e->obscuring_objects);
        OBJS_ARRAY_FLUSH(&e->temporary_objects);
        eina_array_foreach(&e->clip_changes, _evas_clip_changes_free, NULL);
        eina_array_clean(&e->clip_changes);
        
        e->invalidate = EINA_TRUE;
        
        evas_cache_async_thaw();
     }
}

void
evas_render_invalidate(Evas *eo_e)
{
   Evas_Public_Data *e;

   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e = eo_data_scope_get(eo_e, EVAS_CLASS);

   OBJS_ARRAY_CLEAN(&e->active_objects);
   OBJS_ARRAY_CLEAN(&e->render_objects);

   OBJS_ARRAY_FLUSH(&e->restack_objects);
   OBJS_ARRAY_FLUSH(&e->delete_objects);

   e->invalidate = EINA_TRUE;
}

void
evas_render_object_recalc(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   if ((!obj->changed) && (obj->delete_me < 2))
     {
       Evas_Public_Data *e;

       e = obj->layer->evas;
       if ((!e) || (e->cleanup)) return;
       OBJ_ARRAY_PUSH(&e->pending_objects, obj);
       obj->changed = EINA_TRUE;
     }
}

void
evas_unref_queue_image_put(Evas_Public_Data *pd, void *image)
{
   eina_array_push(&pd->image_unref_queue, image);
   evas_common_rgba_image_scalecache_items_ref(image, &pd->scie_unref_queue);
}

void
evas_unref_queue_glyph_put(Evas_Public_Data *pd, void *glyph)
{
   eina_array_push(&pd->glyph_unref_queue, glyph);
}

void
evas_unref_queue_texts_put(Evas_Public_Data *pd, void *texts)
{
   eina_array_push(&pd->texts_unref_queue, texts);
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/


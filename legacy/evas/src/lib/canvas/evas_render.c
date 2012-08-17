#include "evas_common.h"
#include "evas_private.h"
#include <math.h>
#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

// debug rendering
/* #define REND_DGB 1 */
/* #define STDOUT_DBG 1 */

#ifdef REND_DGB
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

static Eina_List *
evas_render_updates_internal(Evas *e, unsigned char make_updates, unsigned char do_draw);

EAPI void
evas_damage_rectangle_add(Evas *e, int x, int y, int w, int h)
{
   Eina_Rectangle *r;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   NEW_RECT(r, x, y, w, h);
   if (!r) return;
   e->damages = eina_list_append(e->damages, r);
   e->changed = EINA_TRUE;
}

EAPI void
evas_obscured_rectangle_add(Evas *e, int x, int y, int w, int h)
{
   Eina_Rectangle *r;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   NEW_RECT(r, x, y, w, h);
   if (!r) return;
   e->obscures = eina_list_append(e->obscures, r);
}

EAPI void
evas_obscured_clear(Evas *e)
{
   Eina_Rectangle *r;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   EINA_LIST_FREE(e->obscures, r)
     {
        eina_rectangle_free(r);
     }
}

static Eina_Bool
_evas_render_had_map(Evas_Object *obj)
{
   return ((obj->prev.map) && (obj->prev.usemap));
   //   return ((!obj->cur.map) && (obj->prev.usemap));
}

static Eina_Bool
_evas_render_is_relevant(Evas_Object *obj)
{
   return ((evas_object_is_visible(obj) && (!obj->cur.have_clipees)) ||
           (evas_object_was_visible(obj) && (!obj->prev.have_clipees)));
}

static Eina_Bool
_evas_render_can_render(Evas_Object *obj)
{
   return (evas_object_is_visible(obj) && (!obj->cur.have_clipees));
}

static void
_evas_render_prev_cur_clip_cache_add(Evas *e, Evas_Object *obj)
{
   e->engine.func->output_redraws_rect_add(e->engine.data.output,
                                           obj->prev.cache.clip.x,
                                           obj->prev.cache.clip.y,
                                           obj->prev.cache.clip.w,
                                           obj->prev.cache.clip.h);
   e->engine.func->output_redraws_rect_add(e->engine.data.output,
                                           obj->cur.cache.clip.x,
                                           obj->cur.cache.clip.y,
                                           obj->cur.cache.clip.w,
                                           obj->cur.cache.clip.h);
}

static void
_evas_render_cur_clip_cache_del(Evas *e, Evas_Object *obj)
{
   Evas_Coord x, y, w, h;

   x = obj->cur.cache.clip.x;
   y = obj->cur.cache.clip.y;
   w = obj->cur.cache.clip.w;
   h = obj->cur.cache.clip.h;
   if (obj->cur.clipper)
     {
        RECTS_CLIP_TO_RECT(x, y, w, h,
                           obj->cur.clipper->cur.cache.clip.x,
                           obj->cur.clipper->cur.cache.clip.y,
                           obj->cur.clipper->cur.cache.clip.w,
                           obj->cur.clipper->cur.cache.clip.h);
     }
   e->engine.func->output_redraws_rect_del(e->engine.data.output,
                                           x, y, w, h);
}

static void
_evas_render_phase1_direct(Evas *e,
                           Eina_Array *active_objects,
                           Eina_Array *restack_objects __UNUSED__,
                           Eina_Array *delete_objects __UNUSED__,
                           Eina_Array *render_objects)
{
   unsigned int i;
   Eina_List *l;
   Evas_Object *proxy;

   RD("  [--- PHASE 1 DIRECT\n");
   for (i = 0; i < active_objects->count; i++)
     {
        Evas_Object *obj;

        obj = eina_array_data_get(active_objects, i);
        if (obj->changed)
          {
             /* Flag need redraw on proxy too */
             evas_object_clip_recalc(obj);
	     EINA_LIST_FOREACH(obj->proxy.proxies, l, proxy)
               proxy->proxy.redraw = EINA_TRUE;
          }
     }
   for (i = 0; i < render_objects->count; i++)
     {
        Evas_Object *obj;

        obj = eina_array_data_get(render_objects, i);
        RD("    OBJ [%p] changed %i\n", obj, obj->changed);
        if (obj->changed)
          {
             /* Flag need redraw on proxy too */
             evas_object_clip_recalc(obj);
             obj->func->render_pre(obj);
             if (obj->proxy.redraw)
               _evas_render_prev_cur_clip_cache_add(e, obj);
             if (obj->proxy.proxies)
               {
                  obj->proxy.redraw = EINA_TRUE;
                  EINA_LIST_FOREACH(obj->proxy.proxies, l, proxy)
                    {
                       proxy->func->render_pre(proxy);
                       _evas_render_prev_cur_clip_cache_add(e, proxy);
                    }
               }

             RD("      pre-render-done smart:%p|%p  [%p, %i] | [%p, %i] has_map:%i had_map:%i\n",
                obj->smart.smart,
                evas_object_smart_members_get_direct(obj),
                obj->cur.map, obj->cur.usemap,
                obj->prev.map, obj->prev.usemap,
                _evas_render_has_map(obj),
                _evas_render_had_map(obj));
             if ((obj->smart.smart) &&
                 (_evas_render_has_map(obj)))
               {
                  RD("      has map + smart\n");
                  _evas_render_prev_cur_clip_cache_add(e, obj);
               }
          }
        else
          {
             if (obj->smart.smart)
               {
                  //                  obj->func->render_pre(obj);
               }
             else if ((obj->rect_del) ||
                      (evas_object_is_opaque(obj) && evas_object_is_visible(obj)))
               {
                  RD("    rect del\n");
                  _evas_render_cur_clip_cache_del(e, obj);
               }
          }
     }
   RD("  ---]\n");
}

static Eina_Bool
_evas_render_phase1_object_process(Evas *e, Evas_Object *obj,
                                   Eina_Array *active_objects,
                                   Eina_Array *restack_objects,
                                   Eina_Array *delete_objects,
                                   Eina_Array *render_objects,
                                   int restack,
                                   int *redraw_all,
                                   Eina_Bool mapped_parent
#ifdef REND_DGB
                                   , int level
#endif
                                  )
{
   Eina_Bool clean_them = EINA_FALSE;
   Evas_Object *obj2;
   int is_active;
   Eina_Bool map, hmap;

   //Need pre render for the children of mapped object.
   //But only when they have changed.
   if (mapped_parent && (!obj->changed)) return EINA_FALSE;

   obj->rect_del = EINA_FALSE;
   obj->render_pre = EINA_FALSE;

   if (obj->delete_me == 2)
     eina_array_push(delete_objects, obj);
   else if (obj->delete_me != 0) obj->delete_me++;
   /* If the object will be removed, we should not cache anything during this run. */
   if (obj->delete_me != 0) clean_them = EINA_TRUE;

   /* build active object list */
   evas_object_clip_recalc(obj);
   is_active = evas_object_is_active(obj);
   obj->is_active = is_active;

   RDI(level);
   RD("    [--- PROCESS [%p] '%s' active = %i, del = %i | %i %i %ix%i\n", obj, obj->type, is_active, obj->delete_me, obj->cur.geometry.x, obj->cur.geometry.y, obj->cur.geometry.w, obj->cur.geometry.h);

   if ((!mapped_parent) && ((is_active) || (obj->delete_me != 0)))
     eina_array_push(active_objects, obj);

#ifdef REND_DGB
   if (!is_active)
     {
        RDI(level);
        RD("     [%p] vis: %i, cache.clip.vis: %i cache.clip.a: %i [%p]\n", obj, obj->cur.visible, obj->cur.cache.clip.visible, obj->cur.cache.clip.a, obj->func->is_visible);
     }
#endif

   map = _evas_render_has_map(obj);
   hmap = _evas_render_had_map(obj);

   if ((restack) && (!map))
     {
        if (!obj->changed)
          {
             eina_array_push(&e->pending_objects, obj);
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
                 ((evas_object_is_visible(obj) && (!obj->cur.have_clipees)) ||
                  (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
               {
                  eina_array_push(render_objects, obj);
                  _evas_render_prev_cur_clip_cache_add(e, obj);
                  obj->render_pre = EINA_TRUE;

                  if (obj->smart.smart)
                    {
                       EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj), obj2)
                         {
                            _evas_render_phase1_object_process(e, obj2,
                                                               active_objects,
                                                               restack_objects,
                                                               delete_objects,
                                                               render_objects,
                                                               obj->restack,
                                                               redraw_all,
                                                               EINA_TRUE
#ifdef REND_DGB
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
        //        eina_array_push(restack_objects, obj);
        _evas_render_prev_cur_clip_cache_add(e, obj);
        if (obj->changed)
          {
             if (!map)
               {
                  if ((obj->cur.map) && (obj->cur.usemap)) map = EINA_TRUE;
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
        if (obj->smart.smart)
          {
             RDI(level);
             RD("      changed + smart - render ok\n");
             eina_array_push(render_objects, obj);
             obj->render_pre = EINA_TRUE;
             EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj),
                                 obj2)
               {
                  _evas_render_phase1_object_process(e, obj2,
                                                     active_objects,
                                                     restack_objects,
                                                     delete_objects,
                                                     render_objects,
                                                     obj->restack,
                                                     redraw_all,
                                                     mapped_parent
#ifdef REND_DGB
                                                     , level + 1
#endif
                                                    );
               }
          }
        else
          {
             if ((is_active) && (!obj->clip.clipees) &&
                 _evas_render_is_relevant(obj))
               {
                  RDI(level);
                  RD("      relevant + active\n");
                  if (obj->restack)
                    eina_array_push(restack_objects, obj);
                  else
                    {
                       eina_array_push(render_objects, obj);
                       obj->render_pre = EINA_TRUE;
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
           evas_object_is_visible(obj),
           obj->cur.visible, obj->cur.cache.clip.visible, obj->smart.smart,
           obj->cur.cache.clip.a, evas_object_was_visible(obj));

        if ((!obj->clip.clipees) && (obj->delete_me == 0) &&
            (_evas_render_can_render(obj) ||
             (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
          {
             if (obj->smart.smart)
               {
                  RDI(level);
                  RD("      smart + visible/was visible + not clip\n");
                  eina_array_push(render_objects, obj);
                  obj->render_pre = EINA_TRUE;
                  EINA_INLIST_FOREACH
                     (evas_object_smart_members_get_direct(obj), obj2)
                       {
                          _evas_render_phase1_object_process(e, obj2,
                                                             active_objects,
                                                             restack_objects,
                                                             delete_objects,
                                                             render_objects,
                                                             restack,
                                                             redraw_all,
                                                             mapped_parent
#ifdef REND_DGB
                                                             , level + 1
#endif
                                                            );
                       }
               }
             else
               {
                  if (evas_object_is_opaque(obj) &&
                      evas_object_is_visible(obj))
                    {
                       RDI(level);
                       RD("      opaque + visible\n");
                       eina_array_push(render_objects, obj);
                       obj->rect_del = EINA_TRUE;
                    }
                  else if (evas_object_is_visible(obj))
                    {
                       RDI(level);
                       RD("      visible\n");
                       eina_array_push(render_objects, obj);
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
             eina_array_push(render_objects, obj);
             obj->render_pre = 1;
             EINA_INLIST_FOREACH (evas_object_smart_members_get_direct(obj),
                                  obj2)
               {
                  _evas_render_phase1_object_process(e, obj2,
                                                     active_objects,
                                                     restack_objects,
                                                     delete_objects,
                                                     render_objects,
                                                     restack,
                                                     redraw_all
#ifdef REND_DGB
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
_evas_render_phase1_process(Evas *e,
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
        Evas_Object *obj;

        EINA_INLIST_FOREACH(lay->objects, obj)
          {
             clean_them |= _evas_render_phase1_object_process
                (e, obj, active_objects, restack_objects, delete_objects,
                 render_objects, 0, redraw_all, EINA_FALSE
#ifdef REND_DGB
                 , 1
#endif
                );
          }
     }
   RD("  ---]\n");
   return clean_them;
}

static void
_evas_render_check_pending_objects(Eina_Array *pending_objects, Evas *e)
{
   unsigned int i;

   for (i = 0; i < pending_objects->count; ++i)
     {
        Evas_Object *obj;
        int is_active;
        Eina_Bool ok = EINA_FALSE;
        obj = eina_array_data_get(pending_objects, i);
        if (!obj->layer) goto clean_stuff;

       //If the children are in active objects, They should be cleaned up.
       if (obj->changed_map && _evas_render_has_map(obj))
         goto clean_stuff;

        evas_object_clip_recalc(obj);
        is_active = evas_object_is_active(obj);

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
                  if (obj->smart.smart)
                    {
                       if (obj->render_pre || obj->rect_del) ok = EINA_TRUE;
                    }
                  else
                    if ((is_active) && (obj->restack) && (!obj->clip.clipees) &&
                        (_evas_render_can_render(obj) ||
                         (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
                      {
                         if (!(obj->render_pre || obj->rect_del))
                           ok = EINA_TRUE;
                      }
                    else
                      if (is_active && (!obj->clip.clipees) &&
                          (_evas_render_can_render(obj) ||
                           (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
                        {
                           if (obj->render_pre || obj->rect_del) ok = EINA_TRUE;
                        }
               }
             else
               {
                  if ((!obj->clip.clipees) && (obj->delete_me == 0) &&
                      (!obj->cur.have_clipees || (evas_object_was_visible(obj) && (!obj->prev.have_clipees)))
                      && evas_object_is_opaque(obj) && evas_object_is_visible(obj))
                    {
                       if (obj->rect_del || obj->smart.smart) ok = EINA_TRUE;
                    }
               }
          }

clean_stuff:
        if (!ok)
          {
             eina_array_clean(&e->active_objects);
             eina_array_clean(&e->render_objects);
             eina_array_clean(&e->restack_objects);
             eina_array_clean(&e->delete_objects);
             e->invalidate = EINA_TRUE;
             return ;
          }
     }
}

Eina_Bool
pending_change(void *data, void *gdata __UNUSED__)
{
   Evas_Object *obj;

   obj = data;
   if (obj->delete_me) return EINA_FALSE;
   if (obj->pre_render_done)
     {
        RD("  OBJ [%p] pending change %i -> 0, pre %i\n", obj, obj->changed, obj->pre_render_done);
        obj->pre_render_done = EINA_FALSE;
        evas_object_change_reset(obj);
     }
   return obj->changed ? EINA_TRUE : EINA_FALSE;
}

static Eina_Bool
_evas_render_can_use_overlay(Evas *e, Evas_Object *obj)
{
   Eina_Rectangle *r;
   Evas_Object *tmp;
   Eina_List *alphas = NULL;
   Eina_List *opaques = NULL;
   Evas_Object *video_parent = NULL;
   Eina_Rectangle zone;
   Evas_Coord xc1, yc1, xc2, yc2;
   unsigned int i;
   Eina_Bool nooverlay;

   video_parent = _evas_object_image_video_parent_get(obj);

   /* Check if any one is the stack make this object mapped */
   tmp = obj;
   while (tmp && !_evas_render_has_map(tmp))
     tmp = tmp->smart.parent;

   if (tmp && _evas_render_has_map(tmp)) return EINA_FALSE; /* we are mapped, we can't be an overlay */

   if (!evas_object_is_visible(obj)) return EINA_FALSE; /* no need to update the overlay if it's not visible */

   /* If any recoloring of the surface is needed, n overlay to */
   if ((obj->cur.cache.clip.r != 255) ||
       (obj->cur.cache.clip.g != 255) ||
       (obj->cur.cache.clip.b != 255) ||
       (obj->cur.cache.clip.a != 255))
     return EINA_FALSE;

   /* Check presence of transparent object on top of the video object */
   EINA_RECTANGLE_SET(&zone,
                      obj->cur.cache.clip.x,
                      obj->cur.cache.clip.y,
                      obj->cur.cache.clip.w,
                      obj->cur.cache.clip.h);

   for (i = e->active_objects.count - 1; i > 0; i--)
     {
        Eina_Rectangle self;
        Eina_Rectangle *match;
        Evas_Object *current;
        Eina_List *l;
        int xm1, ym1, xm2, ym2;

        current = eina_array_data_get(&e->active_objects, i);

        /* Did we find the video object in the stack ? */
        if (current == video_parent || current == obj)
          break;

        EINA_RECTANGLE_SET(&self,
                           current->cur.cache.clip.x,
                           current->cur.cache.clip.y,
                           current->cur.cache.clip.w,
                           current->cur.cache.clip.h);

        /* This doesn't cover the area of the video object, so don't bother with that object */
        if (!eina_rectangles_intersect(&zone, &self))
          continue;

        xc1 = current->cur.cache.clip.x;
        yc1 = current->cur.cache.clip.y;
        xc2 = current->cur.cache.clip.x + current->cur.cache.clip.w;
        yc2 = current->cur.cache.clip.y + current->cur.cache.clip.h;

        if (evas_object_is_visible(current) &&
            (!current->clip.clipees) &&
            (current->cur.visible) &&
            (!current->delete_me) &&
            (current->cur.cache.clip.visible) &&
            (!current->smart.smart))
          {
             Eina_Bool included = EINA_FALSE;

             if (evas_object_is_opaque(current) ||
                 ((current->func->has_opaque_rect) &&
                  (current->func->has_opaque_rect(current))))
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

                       r = eina_rectangle_new(current->cur.cache.clip.x, current->cur.cache.clip.y,
                                              current->cur.cache.clip.w, current->cur.cache.clip.h);

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
                       r = eina_rectangle_new(current->cur.cache.clip.x, current->cur.cache.clip.y,
                                              current->cur.cache.clip.w, current->cur.cache.clip.h);

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

   if (nooverlay)
     return EINA_FALSE;

   return EINA_TRUE;
}

Eina_Bool
evas_render_mapped(Evas *e, Evas_Object *obj, void *context, void *surface,
                   int off_x, int off_y, int mapped,
                   int ecx, int ecy, int ecw, int ech
#ifdef REND_DGB
                   , int level
#endif
                  )
{
   void *ctx;
   Evas_Object *obj2;
   Eina_Bool clean_them = EINA_FALSE;

   evas_object_clip_recalc(obj);

   RDI(level);
   RD("      { evas_render_mapped(%p, %p,   %p, %p,   %i, %i,   %i,   %i)\n", e, obj, context, surface, off_x, off_y, mapped, level);
   if (mapped)
     {
        if ((!evas_object_is_visible(obj)) || (obj->clip.clipees) ||
            (obj->cur.have_clipees))
          {
             RDI(level);
             RD("      }\n");
             return clean_them;
          }
     }
   else if (!(((evas_object_is_active(obj) && (!obj->clip.clipees) &&
                (_evas_render_can_render(obj))))
             ))
     {
        RDI(level);
        RD("      }\n");
        return clean_them;
     }

   // set render_pre - for child objs that may not have gotten it.
   obj->pre_render_done = EINA_TRUE;
   RD("          Hasmap: %p (%d) %p %d -> %d\n",obj->func->can_map,
      obj->func->can_map ? obj->func->can_map(obj): -1,
      obj->cur.map, obj->cur.usemap,
      _evas_render_has_map(obj));
   if (_evas_render_has_map(obj))
     {
        int sw, sh;
        Eina_Bool changed = EINA_FALSE, rendered = EINA_FALSE;

        clean_them = EINA_TRUE;

        sw = obj->cur.geometry.w;
        sh = obj->cur.geometry.h;
        RDI(level);
        RD("        mapped obj: %ix%i\n", sw, sh);
        if ((sw <= 0) || (sh <= 0))
          {
             RDI(level);
             RD("      }\n");
             return clean_them;
          }
        evas_object_map_update(obj, off_x, off_y, sw, sh, sw, sh);

        if (obj->cur.map->surface)
          {
             if ((obj->cur.map->surface_w != sw) ||
                 (obj->cur.map->surface_h != sh))
               {
                  RDI(level);
                  RD("        new surf: %ix%i\n", sw, sh);
                  obj->layer->evas->engine.func->image_map_surface_free
                     (e->engine.data.output, obj->cur.map->surface);
                  obj->cur.map->surface = NULL;
               }
          }
        if (!obj->cur.map->surface)
          {
             obj->cur.map->surface_w = sw;
             obj->cur.map->surface_h = sh;

             obj->cur.map->surface =
                obj->layer->evas->engine.func->image_map_surface_new
                (e->engine.data.output, obj->cur.map->surface_w,
                 obj->cur.map->surface_h,
                 obj->cur.map->alpha);
             RDI(level);
             RD("        fisrt surf: %ix%i\n", sw, sh);
             changed = EINA_TRUE;
          }
        if (obj->smart.smart)
          {
             Evas_Object *o2;

             EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj), o2)
               {
                  if (!evas_object_is_visible(o2) &&
                      !evas_object_was_visible(o2))
                    {
                       evas_object_change_reset(o2);
                       continue;
                    }
                  if (o2->changed)
                    {
                       changed = EINA_TRUE;
                       evas_object_change_reset(o2);
                       break;
                    }
               }
             if (obj->changed_color) changed = EINA_TRUE;
             evas_object_change_reset(obj);
          }
        else if (obj->changed)
          {
             if (((obj->changed_pchange) && (obj->changed_map)) ||
                 (obj->changed_color))
               changed = EINA_TRUE;
             evas_object_change_reset(obj);
          }

        // clear surface before re-render
        if ((changed) && (obj->cur.map->surface))
          {
             int off_x2, off_y2;

             RDI(level);
             RD("        children redraw\n");
             // FIXME: calculate "changes" within map surface and only clear
             // and re-render those
             if (obj->cur.map->alpha)
               {
                  ctx = e->engine.func->context_new(e->engine.data.output);
                  e->engine.func->context_color_set
                     (e->engine.data.output, ctx, 0, 0, 0, 0);
                  e->engine.func->context_render_op_set
                     (e->engine.data.output, ctx, EVAS_RENDER_COPY);
                  e->engine.func->rectangle_draw(e->engine.data.output,
                                                 ctx,
                                                 obj->cur.map->surface,
                                                 0, 0,
                                                 obj->cur.map->surface_w,
                                                 obj->cur.map->surface_h);
                  e->engine.func->context_free(e->engine.data.output, ctx);
               }
             ctx = e->engine.func->context_new(e->engine.data.output);
             off_x2 = -obj->cur.geometry.x;
             off_y2 = -obj->cur.geometry.y;
             if (obj->smart.smart)
               {
                  EINA_INLIST_FOREACH
                     (evas_object_smart_members_get_direct(obj), obj2)
                       {
                          clean_them |= evas_render_mapped(e, obj2, ctx,
                                                           obj->cur.map->surface,
                                                           off_x2, off_y2, 1,
                                                           ecx, ecy, ecw, ech
#ifdef REND_DGB
                                                           , level + 1
#endif
                                                          );
                       }
               }
             else
               {
                  int x = 0, y = 0, w = 0, h = 0;

                  w = obj->cur.map->surface_w;
                  h = obj->cur.map->surface_h;
                  RECTS_CLIP_TO_RECT(x, y, w, h,
                                     obj->cur.geometry.x + off_x2,
                                     obj->cur.geometry.y + off_y2,
                                     obj->cur.geometry.w,
                                     obj->cur.geometry.h);

                  e->engine.func->context_clip_set(e->engine.data.output,
                                                   ctx, x, y, w, h);
                  obj->func->render(obj, e->engine.data.output, ctx,
                                    obj->cur.map->surface, off_x2, off_y2);
               }
             e->engine.func->context_free(e->engine.data.output, ctx);
             rendered = EINA_TRUE;
          }

        RDI(level);
        RD("        draw map\n");

        if (rendered)
          {
             obj->cur.map->surface = e->engine.func->image_dirty_region
                (e->engine.data.output, obj->cur.map->surface,
                 0, 0, obj->cur.map->surface_w, obj->cur.map->surface_h);
          }
        e->engine.func->context_clip_unset(e->engine.data.output,
                                           context);
        if (obj->cur.map->surface)
          {
             if (obj->smart.smart)
               {
                  if (obj->cur.clipper)
                    {
                       int x, y, w, h;
                       Evas_Object *tobj;

                       obj->cur.cache.clip.dirty = EINA_TRUE;
                       tobj = obj->cur.map_parent;
                       obj->cur.map_parent = obj->cur.clipper->cur.map_parent;
                       evas_object_clip_recalc(obj);
                       obj->cur.map_parent = tobj;
                       x = obj->cur.cache.clip.x;
                       y = obj->cur.cache.clip.y;
                       w = obj->cur.cache.clip.w;
                       h = obj->cur.cache.clip.h;
                       RECTS_CLIP_TO_RECT(x, y, w, h,
                                          obj->cur.clipper->cur.cache.clip.x,
                                          obj->cur.clipper->cur.cache.clip.y,
                                          obj->cur.clipper->cur.cache.clip.w,
                                          obj->cur.clipper->cur.cache.clip.h);
                       e->engine.func->context_clip_set(e->engine.data.output,
                                                        context,
                                                        x + off_x, y + off_y, w, h);
                    }
               }
             else
               {
                  if (obj->cur.clipper)
                    {
                       int x, y, w, h;

                       evas_object_clip_recalc(obj);
                       x = obj->cur.cache.clip.x;
                       y = obj->cur.cache.clip.y;
                       w = obj->cur.cache.clip.w;
                       h = obj->cur.cache.clip.h;
                       RECTS_CLIP_TO_RECT(x, y, w, h,
                                          obj->cur.clipper->cur.cache.clip.x,
                                          obj->cur.clipper->cur.cache.clip.y,
                                          obj->cur.clipper->cur.cache.clip.w,
                                          obj->cur.clipper->cur.cache.clip.h);
                       e->engine.func->context_clip_set(e->engine.data.output,
                                                        context,
                                                        x + off_x, y + off_y, w, h);
                    }
               }
          }
//        if (surface == e->engine.data.output)
          e->engine.func->context_clip_clip(e->engine.data.output,
                                            context,
                                            ecx, ecy, ecw, ech);
        if (obj->cur.cache.clip.visible)
          {
             obj->layer->evas->engine.func->context_multiplier_unset
               (e->engine.data.output, context);
             obj->layer->evas->engine.func->image_map_draw
               (e->engine.data.output, context, surface,
                   obj->cur.map->surface, obj->spans,
                   obj->cur.map->smooth, 0);
          }
        // FIXME: needs to cache these maps and
        // keep them only rendering updates
        //        obj->layer->evas->engine.func->image_map_surface_free
        //          (e->engine.data.output, obj->cur.map->surface);
        //        obj->cur.map->surface = NULL;
     }
   else
     {
        if (0 && obj->cur.cached_surface)
          fprintf(stderr, "We should cache '%s' [%i, %i, %i, %i]\n",
                  evas_object_type_get(obj),
                  obj->cur.bounding_box.x, obj->cur.bounding_box.x,
                  obj->cur.bounding_box.w, obj->cur.bounding_box.h);
        if (mapped)
          {
             RDI(level);
             RD("        draw child of mapped obj\n");
             ctx = e->engine.func->context_new(e->engine.data.output);
             if (obj->smart.smart)
               {
                  EINA_INLIST_FOREACH
                     (evas_object_smart_members_get_direct(obj), obj2)
                       {
                          clean_them |= evas_render_mapped(e, obj2, ctx,
                                                           surface,
                                                           off_x, off_y, 1,
                                                           ecx, ecy, ecw, ech
#ifdef REND_DGB
                                                           , level + 1
#endif
                                                          );
                       }
               }
             else
               {
                  RDI(level);

                  if (obj->cur.clipper)
                    {
                       RD("        clip: %i %i %ix%i [%i %i %ix%i]\n",
                          obj->cur.cache.clip.x + off_x,
                          obj->cur.cache.clip.y + off_y,
                          obj->cur.cache.clip.w,
                          obj->cur.cache.clip.h,
                          obj->cur.geometry.x + off_x,
                          obj->cur.geometry.y + off_y,
                          obj->cur.geometry.w,
                          obj->cur.geometry.h);

                       RD("        clipper: %i %i %ix%i\n",
                          obj->cur.clipper->cur.cache.clip.x + off_x,
                          obj->cur.clipper->cur.cache.clip.y + off_y,
                          obj->cur.clipper->cur.cache.clip.w,
                          obj->cur.clipper->cur.cache.clip.h);

                       int x, y, w, h;

                       if (_evas_render_has_map(obj))
                         evas_object_clip_recalc(obj);

                       x = obj->cur.cache.clip.x + off_x;
                       y = obj->cur.cache.clip.y + off_y;
                       w = obj->cur.cache.clip.w;
                       h = obj->cur.cache.clip.h;

                       RECTS_CLIP_TO_RECT(x, y, w, h,
                                          obj->cur.clipper->cur.cache.clip.x + off_x,
                                          obj->cur.clipper->cur.cache.clip.y + off_y,
                                          obj->cur.clipper->cur.cache.clip.w,
                                          obj->cur.clipper->cur.cache.clip.h);

                       e->engine.func->context_clip_set(e->engine.data.output,
                                                        ctx, x, y, w, h);
                    }
                  obj->func->render(obj, e->engine.data.output, ctx,
                                    surface, off_x, off_y);
               }
             e->engine.func->context_free(e->engine.data.output, ctx);
          }
        else
          {
             if (obj->cur.clipper)
               {
                  int x, y, w, h;

                  if (_evas_render_has_map(obj))
                    evas_object_clip_recalc(obj);
                  x = obj->cur.cache.clip.x;
                  y = obj->cur.cache.clip.y;
                  w = obj->cur.cache.clip.w;
                  h = obj->cur.cache.clip.h;
                  RECTS_CLIP_TO_RECT(x, y, w, h,
                                     obj->cur.clipper->cur.cache.clip.x,
                                     obj->cur.clipper->cur.cache.clip.y,
                                     obj->cur.clipper->cur.cache.clip.w,
                                     obj->cur.clipper->cur.cache.clip.h);
                  e->engine.func->context_clip_set(e->engine.data.output,
                                                   context,
                                                   x + off_x, y + off_y, w, h);
                  e->engine.func->context_clip_clip(e->engine.data.output,
                                                    context,
                                                    ecx, ecy, ecw, ech);
               }

             RDI(level);
             RD("        draw normal obj\n");
             obj->func->render(obj, e->engine.data.output, context, surface,
                               off_x, off_y);
          }
        if (obj->changed_map) clean_them = EINA_TRUE;
     }
   RDI(level);
   RD("      }\n");

   return clean_them;
}

static void
_evas_render_cutout_add(Evas *e, Evas_Object *obj, int off_x, int off_y)
{
   if (evas_object_is_opaque(obj))
     {
        Evas_Coord cox, coy, cow, coh;

        cox = obj->cur.cache.clip.x;
        coy = obj->cur.cache.clip.y;
        cow = obj->cur.cache.clip.w;
        coh = obj->cur.cache.clip.h;
        if ((obj->cur.map) && (obj->cur.usemap))
          {
             Evas_Object *oo;

             oo = obj;
             while (oo->cur.clipper)
               {
                  if ((oo->cur.clipper->cur.map_parent
                       != oo->cur.map_parent) &&
                      (!((oo->cur.map) && (oo->cur.usemap))))
                    break;
                  RECTS_CLIP_TO_RECT(cox, coy, cow, coh,
                                     oo->cur.geometry.x,
                                     oo->cur.geometry.y,
                                     oo->cur.geometry.w,
                                     oo->cur.geometry.h);
                  oo = oo->cur.clipper;
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

             obj->func->get_opaque_rect(obj, &obx, &oby, &obw, &obh);
             if ((obw > 0) && (obh > 0))
               {
                  obx += off_x;
                  oby += off_y;
                  RECTS_CLIP_TO_RECT(obx, oby, obw, obh,
                                     obj->cur.cache.clip.x + off_x,
                                     obj->cur.cache.clip.y + off_y,
                                     obj->cur.cache.clip.w,
                                     obj->cur.cache.clip.h);
                  e->engine.func->context_cutout_add
                    (e->engine.data.output, e->engine.data.context,
                        obx, oby, obw, obh);
               }
          }
     }
}

static Eina_List *
evas_render_updates_internal(Evas *e,
                             unsigned char make_updates,
                             unsigned char do_draw)
{
   Evas_Object *obj;
   Eina_List *updates = NULL;
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

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   if (!e->changed) return NULL;

#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get())
      evas_cserve2_dispatch();
#endif
   evas_call_smarts_calculate(e);

   RD("[--- RENDER EVAS (size: %ix%i)\n", e->viewport.w, e->viewport.h);

   evas_event_callback_call(e, EVAS_CALLBACK_RENDER_PRE, NULL);

   /* Check if the modified object mean recalculating every thing */
   if (!e->invalidate)
     _evas_render_check_pending_objects(&e->pending_objects, e);

   /* phase 1. add extra updates for changed objects */
   if (e->invalidate || e->render_objects.count <= 0)
     clean_them = _evas_render_phase1_process(e,
                                              &e->active_objects,
                                              &e->restack_objects,
                                              &e->delete_objects,
                                              &e->render_objects,
                                              &redraw_all);

   /* phase 1.5. check if the video should be inlined or stay in their overlay */
   alpha = e->engine.func->canvas_alpha_get(e->engine.data.output,
                                            e->engine.data.context);

   EINA_LIST_FOREACH(e->video_objects, ll, obj)
     {
        /* we need the surface to be transparent to display the underlying overlay */
        if (alpha && _evas_render_can_use_overlay(e, obj))
          _evas_object_image_video_overlay_show(obj);
        else
          _evas_object_image_video_overlay_hide(obj);
     }
   /* phase 1.8. pre render for proxy */
   _evas_render_phase1_direct(e, &e->active_objects, &e->restack_objects,
                              &e->delete_objects, &e->render_objects);

   /* phase 2. force updates for restacks */
   for (i = 0; i < e->restack_objects.count; ++i)
     {
        obj = eina_array_data_get(&e->restack_objects, i);
        obj->func->render_pre(obj);
        _evas_render_prev_cur_clip_cache_add(e, obj);
     }
   eina_array_clean(&e->restack_objects);

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
        int fx, fy, fw, fh;

        fx = e->viewport.x - e->framespace.x;
        fy = e->viewport.y - e->framespace.y;
        fw = e->viewport.w + e->framespace.w;
        fh = e->viewport.h + e->framespace.h;
        if (fx < 0) fx = 0;
        if (fy < 0) fy = 0;
        e->engine.func->output_redraws_rect_add(e->engine.data.output,
                                                fx, fy, fw, fh);
     }

   /* phase 4.5: check if object is not in framespace. if not, we need to clip 
    * it to the 'master' clip.
    * 
    * NB: This is for the wayland engine(s). If we do not do this, then 
    * objects will draw outside the viewport and potentially onto the frame 
    * itself */
   if (!strncmp(e->engine.module->definition->name, "wayland", 7))
     {
        Eina_Rectangle clip_rect;

        /* see if the master clip has been added yet, if not, then create */
        if (!e->framespace.clip)
          {
             e->framespace.clip = evas_object_rectangle_add(e);
             evas_object_color_set(e->framespace.clip, 255, 255, 255, 255);
             evas_object_move(e->framespace.clip,
                              e->framespace.x, e->framespace.y);
             evas_object_resize(e->framespace.clip,
                                e->viewport.w - e->framespace.w, 
                                e->viewport.h - e->framespace.h);
             evas_object_show(e->framespace.clip);
          }
        else
          {
             /* master clip is already present. check for size changes in the 
              * viewport, and update master clip size if needed */
             if ((e->viewport.changed) || (e->output.changed) || 
                 (e->framespace.changed))
               {
                  evas_object_move(e->framespace.clip, 
                                   e->framespace.x, e->framespace.y);
                  evas_object_resize(e->framespace.clip,
                                     e->viewport.w - e->framespace.w,
                                     e->viewport.h - e->framespace.h);
               }
          }

        EINA_RECTANGLE_SET(&clip_rect,
                           e->framespace.clip->cur.geometry.x,
                           e->framespace.clip->cur.geometry.y,
                           e->framespace.clip->cur.geometry.w,
                           e->framespace.clip->cur.geometry.h)

        /* With the master clip all setup, we need to loop the objects on this 
         * canvas and determine if the object is in the viewport space. If it 
         * is in the viewport space (and not in framespace), then we need to 
         * clip the object to the master clip so that it does not draw on top 
         * of the frame (eg: elm 3d test) */
        for (i = 0; i < e->render_objects.count; ++i)
          {
             Eina_Rectangle obj_rect;
             Evas_Object *pclip;

             obj = eina_array_data_get(&e->render_objects, i);
             if (evas_object_is_frame_object_get(obj))
               continue;

             if (obj->delete_me) continue;

             EINA_RECTANGLE_SET(&obj_rect,
                                obj->cur.geometry.x, obj->cur.geometry.y,
                                obj->cur.geometry.w, obj->cur.geometry.h);

             /* if the object does not intersect our clip rect, ignore it */
             if (!eina_rectangles_intersect(&clip_rect, &obj_rect))
               continue;

             if (!(pclip = evas_object_clip_get(obj)))
               {
                  /* skip clipping if the object is itself the 
                   * framespace clip */
                  if (obj == e->framespace.clip) continue;

                  /* clip this object so it does not draw on the window frame */
                  evas_object_clip_set(obj, e->framespace.clip);
               }
          }
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
        if (UNLIKELY((evas_object_is_opaque(obj) ||
                      ((obj->func->has_opaque_rect) &&
                       (obj->func->has_opaque_rect(obj)))) &&
                     evas_object_is_visible(obj) &&
                     (!obj->clip.clipees) &&
                     (obj->cur.visible) &&
                     (!obj->delete_me) &&
                     (obj->cur.cache.clip.visible) &&
                     (!obj->smart.smart)))
          /*	  obscuring_objects = eina_list_append(obscuring_objects, obj); */
          eina_array_push(&e->obscuring_objects, obj);
     }

   /* save this list */
   /*    obscuring_objects_orig = obscuring_objects; */
   /*    obscuring_objects = NULL; */
   /* phase 6. go thru each update rect and render objects in it*/
   if (do_draw)
     {
        unsigned int offset = 0;

        while ((surface =
                e->engine.func->output_redraws_next_update_get
                (e->engine.data.output,
                 &ux, &uy, &uw, &uh,
                 &cx, &cy, &cw, &ch)))
          {
             int off_x, off_y;

             RD("  [--- UPDATE %i %i %ix%i\n", ux, uy, uw, uh);
             if (make_updates)
               {
                  Eina_Rectangle *rect;

                  NEW_RECT(rect, ux, uy, uw, uh);
                  if (rect)
                    updates = eina_list_append(updates, rect);
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
                  obj = (Evas_Object *)eina_array_data_get
                     (&e->obscuring_objects, i);
                  if (evas_object_is_in_output_rect(obj, ux, uy, uw, uh))
                    {
                       eina_array_push(&e->temporary_objects, obj);

                       /* reset the background of the area if needed (using cutout and engine alpha flag to help) */
                       if (alpha)
                         _evas_render_cutout_add(e, obj, off_x, off_y);
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
                                                 cx, cy, cw, ch);
                  e->engine.func->context_cutout_clear(e->engine.data.output,
                                                       e->engine.data.context);
                  e->engine.func->context_clip_unset(e->engine.data.output,
                                                     e->engine.data.context);
               }

             /* render all object that intersect with rect */
             for (i = 0; i < e->active_objects.count; ++i)
               {
                  obj = eina_array_data_get(&e->active_objects, i);

                  /* if it's in our outpout rect and it doesn't clip anything */
                  RD("    OBJ: [%p] '%s' %i %i %ix%i\n", obj, obj->type, obj->cur.geometry.x, obj->cur.geometry.y, obj->cur.geometry.w, obj->cur.geometry.h);
                  if ((evas_object_is_in_output_rect(obj, ux, uy, uw, uh) ||
                       (obj->smart.smart)) &&
                      (!obj->clip.clipees) &&
                      (obj->cur.visible) &&
                      (!obj->delete_me) &&
                      (obj->cur.cache.clip.visible) &&
//		      (!obj->smart.smart) &&
                      ((obj->cur.color.a > 0 || obj->cur.render_op != EVAS_RENDER_BLEND)))
                    {
                       int x, y, w, h;

                       RD("      DRAW (vis: %i, a: %i, clipees: %p\n", obj->cur.visible, obj->cur.color.a, obj->clip.clipees);
                       if ((e->temporary_objects.count > offset) &&
                           (eina_array_data_get(&e->temporary_objects, offset) == obj))
                         offset++;
                       x = cx; y = cy; w = cw; h = ch;
                       if (((w > 0) && (h > 0)) || (obj->smart.smart))
                         {
                            if (!obj->smart.smart)
                              {
                                 RECTS_CLIP_TO_RECT(x, y, w, h,
                                                    obj->cur.cache.clip.x + off_x,
                                                    obj->cur.cache.clip.y + off_y,
                                                    obj->cur.cache.clip.w,
                                                    obj->cur.cache.clip.h);
                              }
                            if (obj->cur.mask)
                              e->engine.func->context_mask_set(e->engine.data.output,
                                                               e->engine.data.context,
                                                               obj->cur.mask->func->engine_data_get(obj->cur.mask),
                                                               obj->cur.mask->cur.geometry.x + off_x,
                                                               obj->cur.mask->cur.geometry.y + off_y,
                                                               obj->cur.mask->cur.geometry.w,
                                                               obj->cur.mask->cur.geometry.h);
                            else
                              e->engine.func->context_mask_unset(e->engine.data.output,
                                                                 e->engine.data.context);
                            e->engine.func->context_clip_set(e->engine.data.output,
                                                             e->engine.data.context,
                                                             x, y, w, h);
#if 1 /* FIXME: this can slow things down... figure out optimum... coverage */
                            for (j = offset; j < e->temporary_objects.count; ++j)
                              {
                                 Evas_Object *obj2;

                                 obj2 = (Evas_Object *)eina_array_data_get
                                   (&e->temporary_objects, j);
                                 _evas_render_cutout_add(e, obj2, off_x, off_y);
                              }
#endif
                            clean_them |= evas_render_mapped(e, obj, e->engine.data.context,
                                                             surface, off_x, off_y, 0,
                                                             cx, cy, cw, ch
#ifdef REND_DGB
                                                             , 1
#endif
                                                            );
                            e->engine.func->context_cutout_clear(e->engine.data.output,
                                                                 e->engine.data.context);
                         }
                    }
               }
             /* punch rect out */
             e->engine.func->output_redraws_next_update_push(e->engine.data.output,
                                                             surface,
                                                             ux, uy, uw, uh);
             /* free obscuring objects list */
             eina_array_clean(&e->temporary_objects);
             RD("  ---]\n");
          }
        /* flush redraws */
        if (haveup)
          {
             evas_event_callback_call(e, EVAS_CALLBACK_RENDER_FLUSH_PRE, NULL);
             e->engine.func->output_flush(e->engine.data.output);
             evas_event_callback_call(e, EVAS_CALLBACK_RENDER_FLUSH_POST, NULL);
          }
     }
   /* clear redraws */
   e->engine.func->output_redraws_clear(e->engine.data.output);
   /* and do a post render pass */
   for (i = 0; i < e->active_objects.count; ++i)
     {
        obj = eina_array_data_get(&e->active_objects, i);
        obj->pre_render_done = EINA_FALSE;
        RD("    OBJ [%p] post... %i %i\n", obj, obj->changed, do_draw);
        if ((obj->changed) && (do_draw))
          {
             RD("    OBJ [%p] post... func1\n", obj);
             obj->func->render_post(obj);
             obj->restack = EINA_FALSE;
             evas_object_change_reset(obj);
          }
        else if (clean_them)
          {
             RD("    OBJ [%p] post... func2\n", obj);
             obj->func->render_post(obj);
             obj->restack = EINA_FALSE;
             evas_object_change_reset(obj);
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
   eina_array_clean(&e->obscuring_objects);

   /* If some object are still marked as changed, do not remove
      them from the pending list. */
   eina_array_remove(&e->pending_objects, pending_change, NULL);

   for (i = 0; i < e->render_objects.count; ++i)
     {
        obj = eina_array_data_get(&e->render_objects, i);
        obj->pre_render_done = EINA_FALSE;
     }

   /* delete all objects flagged for deletion now */
   for (i = 0; i < e->delete_objects.count; ++i)
     {
        obj = eina_array_data_get(&e->delete_objects, i);
        evas_object_free(obj, 1);
     }
   eina_array_clean(&e->delete_objects);

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
        eina_array_clean(&e->active_objects);
        eina_array_clean(&e->render_objects);
        eina_array_clean(&e->restack_objects);
        eina_array_clean(&e->temporary_objects);
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

   evas_module_clean();

   evas_event_callback_call(e, EVAS_CALLBACK_RENDER_POST, NULL);

   RD("---]\n");

   return updates;
}

EAPI void
evas_render_updates_free(Eina_List *updates)
{
   Eina_Rectangle *r;

   EINA_LIST_FREE(updates, r)
      eina_rectangle_free(r);
}

EAPI Eina_List *
evas_render_updates(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

   if (!e->changed) return NULL;
   return evas_render_updates_internal(e, 1, 1);
}

EAPI void
evas_render(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (!e->changed) return;
   evas_render_updates_internal(e, 0, 1);
}

EAPI void
evas_norender(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   //   if (!e->changed) return;
   evas_render_updates_internal(e, 0, 0);
}

EAPI void
evas_render_idle_flush(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   evas_fonts_zero_presure(e);

   if ((e->engine.func) && (e->engine.func->output_idle_flush) &&
       (e->engine.data.output))
     e->engine.func->output_idle_flush(e->engine.data.output);

   eina_array_flush(&e->active_objects);
   eina_array_flush(&e->render_objects);
   eina_array_flush(&e->restack_objects);
   eina_array_flush(&e->delete_objects);
   eina_array_flush(&e->obscuring_objects);
   eina_array_flush(&e->temporary_objects);
   eina_array_flush(&e->clip_changes);

   e->invalidate = EINA_TRUE;
}

EAPI void
evas_sync(Evas *e)
{
  (void) e;
}

static void
_evas_render_dump_map_surfaces(Evas_Object *obj)
{
   if ((obj->cur.map) && obj->cur.map->surface)
     {
        obj->layer->evas->engine.func->image_map_surface_free
           (obj->layer->evas->engine.data.output, obj->cur.map->surface);
        obj->cur.map->surface = NULL;
     }

   if (obj->smart.smart)
     {
        Evas_Object *obj2;

        EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj), obj2)
           _evas_render_dump_map_surfaces(obj2);
     }
}

EAPI void
evas_render_dump(Evas *e)
{
   Evas_Layer *lay;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   EINA_INLIST_FOREACH(e->layers, lay)
     {
        Evas_Object *obj;

        EINA_INLIST_FOREACH(lay->objects, obj)
          {
             if ((obj->type) && (!strcmp(obj->type, "image")))
               evas_object_inform_call_image_unloaded(obj);
             _evas_render_dump_map_surfaces(obj);
          }
     }
   if ((e->engine.func) && (e->engine.func->output_dump) &&
       (e->engine.data.output))
     e->engine.func->output_dump(e->engine.data.output);
}

void
evas_render_invalidate(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   eina_array_clean(&e->active_objects);
   eina_array_clean(&e->render_objects);

   eina_array_flush(&e->restack_objects);
   eina_array_flush(&e->delete_objects);

   e->invalidate = EINA_TRUE;
}

void
evas_render_object_recalc(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if ((!obj->changed) && (obj->delete_me < 2))
     {
       Evas *e;

       e = obj->layer->evas;
       if ((!e) || (e->cleanup)) return;
       eina_array_push(&e->pending_objects, obj);
       obj->changed = EINA_TRUE;
     }
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/

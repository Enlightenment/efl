#include "evas_common.h"
#include "evas_private.h"
#include <math.h>

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
   e->changed = 1;
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
_evas_render_has_map(Evas_Object *obj)
{
   return ((!((obj->func->can_map) && (obj->func->can_map(obj)))) &&
           ((obj->cur.map) && (obj->cur.usemap)));
   //   return ((obj->cur.map) && (obj->cur.usemap));
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
   e->engine.func->output_redraws_rect_del(e->engine.data.output,
                                           obj->cur.cache.clip.x,
                                           obj->cur.cache.clip.y,
                                           obj->cur.cache.clip.w,
                                           obj->cur.cache.clip.h);
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
             if (obj->proxy.proxies)
               {
                  EINA_LIST_FOREACH(obj->proxy.proxies, l, proxy)
                    proxy->proxy.redraw = 1;
               }
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
             if (obj->proxy.proxies)
               {
                  obj->proxy.redraw = 1;
                  EINA_LIST_FOREACH(obj->proxy.proxies, l, proxy)
                    {
                       proxy->func->render_pre(proxy);
                       _evas_render_prev_cur_clip_cache_add(e, proxy);
                    }
               }
             else if (obj->proxy.redraw)
               {
                  _evas_render_prev_cur_clip_cache_add(e, obj);
               }
             if (obj->pre_render_done)
               {
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
             else if (_evas_render_had_map(obj))
               {
                  RD("      no pre-render done\n");
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
                                   int restack, int map,
                                   int *redraw_all
#ifdef REND_DGB
                                   , int level
#endif
                                  )
{
   Eina_Bool clean_them = EINA_FALSE;
   Evas_Object *obj2;
   int is_active;
   Eina_Bool hmap;

   obj->rect_del = 0;
   obj->render_pre = 0;

#ifndef EVAS_FRAME_QUEUING
   /* because of clip objects - delete 2 cycles later */
   if (obj->delete_me == 2)
#else
     if (obj->delete_me == evas_common_frameq_get_frameq_sz() + 2)
#endif
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
   if ((is_active) || (obj->delete_me != 0))
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
             obj->changed = 1;
          }
        obj->restack = 1;
        clean_them = EINA_TRUE;
     }

   if (map)
     {
        RDI(level);
        RD("      obj mapped\n");
        if (obj->changed)
          {
             if (map != hmap)
               {
                  *redraw_all = 1;
               }
             evas_object_clip_recalc(obj);
             if ((obj->restack) &&
                 (is_active) && (!obj->clip.clipees) &&
                 ((evas_object_is_visible(obj) && (!obj->cur.have_clipees)) ||
                  (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
               {
                  eina_array_push(render_objects, obj);
                  _evas_render_prev_cur_clip_cache_add(e, obj);
                  obj->render_pre = 1;
               }
             else if ((is_active) && (!obj->clip.clipees) &&
                      ((evas_object_is_visible(obj) && (!obj->cur.have_clipees)) ||
                       (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
               {
                  eina_array_push(render_objects, obj);
                  _evas_render_prev_cur_clip_cache_add(e, obj);
                  obj->render_pre = 1;
               }
          }
        return clean_them;
     }
   else if (_evas_render_had_map(obj))
     {
        RDI(level);
        RD("      had map - restack objs\n");
        //        eina_array_push(restack_objects, obj);
        _evas_render_prev_cur_clip_cache_add(e, obj);
        if (obj->changed)
          {
             if (hmap)
               {
                  if (!map)
                    {
                       if ((obj->cur.map) && (obj->cur.usemap)) map = 1;
                    }
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
             obj->render_pre = 1;
             EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj), obj2)
               {
                  _evas_render_phase1_object_process(e, obj2,
                                                     active_objects,
                                                     restack_objects,
                                                     delete_objects,
                                                     render_objects,
                                                     obj->restack,
                                                     map,
                                                     redraw_all
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
                       obj->render_pre = 1;
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
           obj->cur.visible, obj->cur.cache.clip.visible, obj->smart.smart, obj->cur.cache.clip.a,
           evas_object_was_visible(obj));
        if ((!obj->clip.clipees) && (obj->delete_me == 0) &&
            (_evas_render_can_render(obj) ||
             (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
          {
             if (obj->smart.smart)
               {
                  RDI(level);
                  RD("      smart + visible/was visible + not clip\n");
                  eina_array_push(render_objects, obj);
                  obj->render_pre = 1;
                  EINA_INLIST_FOREACH
                     (evas_object_smart_members_get_direct(obj), obj2)
                       {
                          _evas_render_phase1_object_process(e, obj2,
                                                             active_objects,
                                                             restack_objects,
                                                             delete_objects,
                                                             render_objects,
                                                             restack, map,
                                                             redraw_all
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
                       obj->rect_del = 1;
                    }
                  else if (evas_object_is_visible(obj))
                    {
                       RDI(level);
                       RD("      visible\n");
                       eina_array_push(render_objects, obj);
                       obj->render_pre = 1;
                    }
                  else
                    {
                       RDI(level);
                       RD("      skip\n");
                    }
               }
          }
        /*
           else if (obj->smart.smart)
           {
           RDI(level);
           RD("      smart + mot visible/was visible\n");
           eina_array_push(render_objects, obj);
           obj->render_pre = 1;
           EINA_INLIST_FOREACH
           (evas_object_smart_members_get_direct(obj), obj2)
           {
           _evas_render_phase1_object_process(e, obj2,
           active_objects,
           restack_objects,
           delete_objects,
           render_objects,
           restack, map,
           redraw_all
#ifdef REND_DGB
, level + 1
#endif
);
}
}
         */
}
if (!is_active) obj->restack = 0;
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
                 render_objects, 0, 0, redraw_all
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
        int is_active, ok = 0;

        obj = eina_array_data_get(pending_objects, i);

        if (!obj->layer) goto clean_stuff;

        evas_object_clip_recalc(obj);
        is_active = evas_object_is_active(obj);

        if ((!is_active) && (!obj->is_active) && (!obj->render_pre) &&
            (!obj->rect_del))
          {
             ok = 1;
             goto clean_stuff;
          }

        if (obj->is_active == is_active)
          {
             if (obj->changed)
               {
                  if (obj->smart.smart)
                    {
                       if (obj->render_pre || obj->rect_del) ok = 1;
                    }
                  else
                    if ((is_active) && (obj->restack) && (!obj->clip.clipees) &&
                        (_evas_render_can_render(obj) ||
                         (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
                      {
                         if (!(obj->render_pre || obj->rect_del)) ok = 1;
                      }
                    else
                      if (is_active && (!obj->clip.clipees) &&
                          (_evas_render_can_render(obj) ||
                           (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
                        {
                           if (obj->render_pre || obj->rect_del) ok = 1;
                        }
               }
             else
               {
                  if ((!obj->clip.clipees) && (obj->delete_me == 0) &&
                      (!obj->cur.have_clipees || (evas_object_was_visible(obj) && (!obj->prev.have_clipees)))
                      && evas_object_is_opaque(obj) && evas_object_is_visible(obj))
                    {
                       if (obj->rect_del || obj->smart.smart) ok = 1;
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
             e->invalidate = 1;
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
        obj->pre_render_done = 0;
        //// FIXME: this wipes out changes
        obj->changed = 0;
        obj->changed_move_only = 0;
        obj->changed_nomove = 0;
        obj->changed_move = 0;
     }
   return obj->changed ? EINA_TRUE : EINA_FALSE;
}
/*
   static void
   unchange(Evas_Object *obj)
   {
   Evas_Object *obj2;

   if (!obj->changed) return;
   obj->changed = 0;
   obj->changed_move_only = 0;
   obj->changed_nomove = 0;
   obj->changed_move = 0;
   EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj), obj2)
   {
   unchange(obj2);
   }
   }

   static int
   chlist(Evas_Object *obj, int i)
   {
   Evas_Object *obj2;
   int j;
   int ret = 0;

   if (!obj->changed) return 0;
   for (j = 0; j < i; j++) printf(" ");
   printf("ch2 %p %s %i [%i %i %ix%i] v %i/%i [r%i] %p\n", obj,
   obj->type,
   obj->changed_move_only,
   obj->cur.geometry.x,
   obj->cur.geometry.y,
   obj->cur.geometry.w,
   obj->cur.geometry.h,
   obj->cur.visible,
   obj->prev.visible,
   obj->restack,
   obj->clip.clipees);
   EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj), obj2)
   {
   if (obj2->changed)
   ret |= chlist(obj2, i + 1);
   }
   }
 */
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
          continue ;

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
   obj->pre_render_done = 1;
   RD("          Hasmap: %p (%d) %p %d -> %d\n",obj->func->can_map,
      obj->func->can_map ? obj->func->can_map(obj): -1,
      obj->cur.map, obj->cur.usemap,
      _evas_render_has_map(obj));
   if (_evas_render_has_map(obj))
     {
        const Evas_Map_Point *p, *p_end;
        RGBA_Map_Point pts[4], *pt;
        int sw, sh;
        int changed = 0, rendered = 0;

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

        pts[0].px = obj->cur.map->persp.px << FP;
        pts[0].py = obj->cur.map->persp.py << FP;
        pts[0].foc = obj->cur.map->persp.foc << FP;
        pts[0].z0 = obj->cur.map->persp.z0 << FP;

        p = obj->cur.map->points;
        p_end = p + obj->cur.map->count;
        pt = pts;
        for (; p < p_end; p++, pt++)
          {
             pt->x = (lround(p->x) + off_x) * FP1;
             pt->y = (lround(p->y) + off_y) * FP1;
             pt->z = (lround(p->z)        ) * FP1;
             pt->fx = p->px;
             pt->fy = p->py;
             pt->fz = p->z;
             pt->u = lround(p->u) * FP1;
             pt->v = lround(p->v) * FP1;
             if      (pt->u < 0) pt->u = 0;
             else if (pt->u > (sw * FP1)) pt->u = (sw * FP1);
             if      (pt->v < 0) pt->v = 0;
             else if (pt->v > (sh * FP1)) pt->v = (sh * FP1);
             pt->col = ARGB_JOIN(p->a, p->r, p->g, p->b);
          }
        /* Copy last for software engine */
        if (obj->cur.map->count & 0x1)
          {
             pts[obj->cur.map->count] = pts[obj->cur.map->count - 1];
          }


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
             changed = 1;
          }
        if (obj->smart.smart)
          {
             Evas_Object *o2;

             EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj), o2)
               {
                  if (!evas_object_is_visible(o2) &&
                      !evas_object_was_visible(o2))
                    {
                       o2->changed = 0;
                       o2->changed_move_only = 0;
                       o2->changed_nomove = 0;
                       o2->changed_move = 0;
                       continue;
                    }
                  if (o2->changed)
                    {
                       //                       chlist(o2, 0);
                       changed = 1;
                       o2->changed = 0;
                       o2->changed_move_only = 0;
                       o2->changed_nomove = 0;
                       o2->changed_move = 0;
                       break;
                    }
               }
             //             unchange(obj);
             obj->changed = 0;
             obj->changed_move_only = 0;
             obj->changed_nomove = 0;
             obj->changed_move = 0;
          }
        else
          {
             if (obj->changed)
               {
                  changed = 1;
                  obj->changed = 0;
                  obj->changed_move_only = 0;
                  obj->changed_nomove = 0;
                  obj->changed_move = 0;
               }
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
             rendered = 1;
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
                                           e->engine.data.context);
        if (obj->cur.map->surface)
          {
             if (obj->smart.smart)
               {
                  if (obj->cur.clipper)
                    {
                       int x, y, w, h;
                       Evas_Object *tobj;

                       obj->cur.cache.clip.dirty = 1;
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
                                                        e->engine.data.context,
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
                                                        e->engine.data.context,
                                                        x + off_x, y + off_y, w, h);
                    }
               }
          }
        if (surface == e->engine.data.output)
          e->engine.func->context_clip_clip(e->engine.data.output,
                                            e->engine.data.context,
                                            ecx, ecy, ecw, ech);
        if (obj->cur.cache.clip.visible)
          obj->layer->evas->engine.func->image_map_draw
             (e->engine.data.output, e->engine.data.context, surface,
              obj->cur.map->surface, obj->cur.map->count, pts,
              obj->cur.map->smooth, 0);
        // FIXME: needs to cache these maps and
        // keep them only rendering updates
        //        obj->layer->evas->engine.func->image_map_surface_free
        //          (e->engine.data.output, obj->cur.map->surface);
        //        obj->cur.map->surface = NULL;
     }
   else
     {
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
                  if (!obj->cur.map)
                    {
                       int x, y, w, h;

                       RDI(level);

                       x = obj->cur.cache.clip.x + off_x;
                       y = obj->cur.cache.clip.y + off_y;
                       w = obj->cur.cache.clip.w;
                       h = obj->cur.cache.clip.h;

                       if (obj->cur.clipper)
                         {
                            if (_evas_render_has_map(obj))
                              evas_object_clip_recalc(obj);

                            RD("        clipper: %i %i %ix%i\n",
                               obj->cur.clipper->cur.cache.clip.x + off_x,
                               obj->cur.clipper->cur.cache.clip.y + off_y,
                               obj->cur.clipper->cur.cache.clip.w,
                               obj->cur.clipper->cur.cache.clip.h);

                            RECTS_CLIP_TO_RECT(x, y, w, h,
                                               obj->cur.clipper->cur.cache.clip.x + off_x,
                                               obj->cur.clipper->cur.cache.clip.y + off_y,
                                               obj->cur.clipper->cur.cache.clip.w,
                                               obj->cur.clipper->cur.cache.clip.h);
                         }

                       RD("        clip: %i %i %ix%i [%i %i %ix%i]\n",
                          obj->cur.cache.clip.x + off_x,
                          obj->cur.cache.clip.y + off_y,
                          obj->cur.cache.clip.w,
                          obj->cur.cache.clip.h,
                          obj->cur.geometry.x + off_x,
                          obj->cur.geometry.y + off_y,
                          obj->cur.geometry.w,
                          obj->cur.geometry.h);
                       e->engine.func->context_clip_set(e->engine.data.output,
                                                        ctx, x, y, w, h);
                    }
                  else
                    {
                       RDI(level);
                       RD("        noclip\n");
                    }
                  obj->func->render(obj, e->engine.data.output, ctx,
                                    surface, off_x, off_y);
                  /*
                                      obj->layer->evas->engine.func->context_color_set(e->engine.data.output,
                                      ctx,
                                      0, 30, 0, 30);
                                      obj->layer->evas->engine.func->rectangle_draw(e->engine.data.output,
                                      ctx,
                                      surface,
                                      0, 0, 9999, 9999);
                   */
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
                                                   e->engine.data.context,
                                                   x + off_x, y + off_y, w, h);
                  e->engine.func->context_clip_clip(e->engine.data.output,
                                                    e->engine.data.context,
                                                    ecx, ecy, ecw, ech);
               }

             RDI(level);
             RD("        draw normal obj\n");
             obj->func->render(obj, e->engine.data.output, context, surface,
                               off_x, off_y);
          }
     }
   RDI(level);
   RD("      }\n");

   return clean_them;
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
   int haveup = 0;
   int redraw_all = 0;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   if (!e->changed) return NULL;

   evas_call_smarts_calculate(e);

   RD("[--- RENDER EVAS (size: %ix%i)\n", e->viewport.w, e->viewport.h);

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
   /* phase 4. output & viewport changes */
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
   if (redraw_all)
     {
        e->engine.func->output_redraws_rect_add(e->engine.data.output,
                                                0, 0,
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
             haveup = 1;
             off_x = cx - ux;
             off_y = cy - uy;
             /* build obscuring objects list (in order from bottom to top) */
             for (i = 0; i < e->obscuring_objects.count; ++i)
               {
                  obj = (Evas_Object *)eina_array_data_get
                     (&e->obscuring_objects, i);
                  if (evas_object_is_in_output_rect(obj, ux, uy, uw, uh))
                    {
                       eina_array_push(&e->temporary_objects, obj);

                       /* reset the background of the area if needed (using cutout and engine alpha flag to help) */
                       if (alpha)
                         {
                            if (evas_object_is_opaque(obj))
                              e->engine.func->context_cutout_add
                                 (e->engine.data.output,
                                  e->engine.data.context,
                                  obj->cur.cache.clip.x + off_x,
                                  obj->cur.cache.clip.y + off_y,
                                  obj->cur.cache.clip.w,
                                  obj->cur.cache.clip.h);
                            else
                              {
                                 if (obj->func->get_opaque_rect)
                                   {
                                      Evas_Coord obx, oby, obw, obh;

                                      obj->func->get_opaque_rect
                                         (obj, &obx, &oby, &obw, &obh);
                                      if ((obw > 0) && (obh > 0))
                                        {
                                           obx += off_x;
                                           oby += off_y;
                                           RECTS_CLIP_TO_RECT
                                              (obx, oby, obw, obh,
                                               obj->cur.cache.clip.x + off_x,
                                               obj->cur.cache.clip.y + off_y,
                                               obj->cur.cache.clip.w,
                                               obj->cur.cache.clip.h);
                                           e->engine.func->context_cutout_add
                                              (e->engine.data.output,
                                               e->engine.data.context,
                                               obx, oby,
                                               obw, obh);
                                        }
                                   }
                              }
                         }
                    }
               }
             if (alpha)
               {
                  e->engine.func->context_clip_set(e->engine.data.output,
                                                   e->engine.data.context,
                                                   ux + off_x, uy + off_y, uw, uh);
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
                            if (obj->cur.clipper)
                              e->engine.func->context_clip_set(e->engine.data.output,
                                                               e->engine.data.context,
                                                               x, y, w, h);
                            else
                              e->engine.func->context_clip_unset(e->engine.data.output,
                                                                 e->engine.data.context);
#if 1 /* FIXME: this can slow things down... figure out optimum... coverage */
                            for (j = offset; j < e->temporary_objects.count; ++j)
                              {
                                 Evas_Object *obj2;

                                 obj2 = (Evas_Object *) eina_array_data_get(&e->temporary_objects, j);
                                 if (evas_object_is_opaque(obj2))
                                   e->engine.func->context_cutout_add(e->engine.data.output,
                                                                      e->engine.data.context,
                                                                      obj2->cur.cache.clip.x + off_x,
                                                                      obj2->cur.cache.clip.y + off_y,
                                                                      obj2->cur.cache.clip.w,
                                                                      obj2->cur.cache.clip.h);
                                 else
                                   {
                                      if (obj2->func->get_opaque_rect)
                                        {
                                           Evas_Coord obx, oby, obw, obh;

                                           obj2->func->get_opaque_rect
                                              (obj2, &obx, &oby, &obw, &obh);
                                           if ((obw > 0) && (obh > 0))
                                             {
                                                obx += off_x;
                                                oby += off_y;
                                                RECTS_CLIP_TO_RECT(obx, oby, obw, obh,
                                                                   obj2->cur.cache.clip.x + off_x,
                                                                   obj2->cur.cache.clip.y + off_y,
                                                                   obj2->cur.cache.clip.w,
                                                                   obj2->cur.cache.clip.h);
                                                e->engine.func->context_cutout_add(e->engine.data.output,
                                                                                   e->engine.data.context,
                                                                                   obx, oby,
                                                                                   obw, obh);
                                             }
                                        }
                                   }
                              }
#endif
                            e->engine.func->context_clip_set(e->engine.data.output,
                                                             e->engine.data.context,
                                                             x, y, w, h);
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
        obj->pre_render_done = 0;
        RD("    OBJ [%p] post... %i %i\n", obj, obj->changed, do_draw);
        if ((obj->changed) && (do_draw))
          {
             RD("    OBJ [%p] post... func1\n", obj);
             obj->func->render_post(obj);
             obj->restack = 0;
             obj->changed = 0;
             obj->changed_move_only = 0;
             obj->changed_nomove = 0;
             obj->changed_move = 0;
          }
        else if ((obj->cur.map != obj->prev.map) ||
                 (obj->cur.usemap != obj->prev.usemap))
          {
             RD("    OBJ [%p] post... func2\n", obj);
             obj->func->render_post(obj);
             obj->restack = 0;
             obj->changed = 0;
             obj->changed_move_only = 0;
             obj->changed_nomove = 0;
             obj->changed_move = 0;
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
        obj->pre_render_done = 0;
     }

   /* delete all objects flagged for deletion now */
   for (i = 0; i < e->delete_objects.count; ++i)
     {
        obj = eina_array_data_get(&e->delete_objects, i);
        evas_object_free(obj, 1);
     }
   eina_array_clean(&e->delete_objects);

   e->changed = 0;
   e->viewport.changed = 0;
   e->output.changed = 0;
   e->invalidate = 0;

   /* If their are some object to restack or some object to delete,
    * it's useless to keep the render object list around. */
   if (clean_them)
     {
        eina_array_clean(&e->active_objects);
        eina_array_clean(&e->render_objects);
        eina_array_clean(&e->restack_objects);
        eina_array_clean(&e->delete_objects);
        eina_array_clean(&e->obscuring_objects);
        e->invalidate = 1;
     }

   evas_module_clean();

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

#ifdef EVAS_FRAME_QUEUING
   evas_common_frameq_flush_ready ();
#endif

   if (!e->changed) return NULL;
   return evas_render_updates_internal(e, 1, 1);
}

EAPI void
evas_render(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

#ifdef EVAS_FRAME_QUEUING
   evas_common_frameq_flush_ready ();
#endif

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

   eina_array_flush(&e->delete_objects);
   eina_array_flush(&e->active_objects);
   eina_array_flush(&e->restack_objects);
   eina_array_flush(&e->render_objects);
   eina_array_flush(&e->clip_changes);

   e->invalidate = 1;
}

EAPI void
evas_sync(Evas *e)
{
#ifdef EVAS_FRAME_QUEUING
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   evas_common_frameq_flush();
#else
   (void) e;
#endif
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

   e->invalidate = 1;
}

void
evas_render_object_recalc(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

#ifndef EVAS_FRAME_QUEUING
   if ((!obj->changed) && (obj->delete_me < 2))
#else
     if ((!obj->changed))
#endif
       {
          Evas *e;

          e = obj->layer->evas;
          if ((!e) || (e->cleanup)) return;
#ifdef EVAS_FRAME_QUEUING
          if (obj->delete_me >= evas_common_frameq_get_frameq_sz() + 2) return;
#endif
          eina_array_push(&e->pending_objects, obj);
          obj->changed = 1;
       }
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/

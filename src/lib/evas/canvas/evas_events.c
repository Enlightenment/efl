#define EFL_INPUT_EVENT_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

int _evas_event_counter = 0;

static Eina_List *
_evas_event_object_list_in_get(Evas *eo_e, Eina_List *in,
                               const Eina_Inlist *list, Evas_Object *stop,
                               int x, int y, int *no_rep, Eina_Bool source);

/* FIXME: use eina_list_clone */
static Eina_List *
evas_event_list_copy(Eina_List *list);

static void
_canvas_event_feed_mouse_move_internal(Evas_Public_Data *e, Efl_Input_Pointer_Data *ev);

static void
_canvas_event_feed_multi_up_internal(Evas_Public_Data *e, Efl_Input_Pointer_Data *ev);

static void
_canvas_event_feed_multi_move_internal(Evas_Public_Data *e, Efl_Input_Pointer_Data *ev);

static void
_canvas_event_feed_mouse_move_legacy(Evas *eo_e, Evas_Public_Data *e, int x, int y,
                                     unsigned int timestamp, const void *data);

static void
_evas_event_havemap_adjust_f(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Eina_Vector2 *point, Eina_Bool mouse_grabbed)
{
   if (obj->smart.parent)
     {
        Evas_Object_Protected_Data *smart_parent_obj = efl_data_scope_get(obj->smart.parent, EFL_CANVAS_OBJECT_CLASS);
        _evas_event_havemap_adjust_f(obj->smart.parent, smart_parent_obj, point, mouse_grabbed);
     }

   if ((!obj->map->cur.usemap) || (!obj->map->cur.map) ||
       (obj->map->cur.map->count != 4))
      return;

   //FIXME: Unless map_coords_get() supports grab mode and extrapolate coords
   //outside map, this should check the return value for outside case.
   if (evas_map_coords_get(obj->map->cur.map, point->x, point->y, &point->x, &point->y, mouse_grabbed))
     {
        point->x += obj->cur->geometry.x;
        point->y += obj->cur->geometry.y;
     }
}

#if 0
# define DDD_DO 1
# define DDD(...) do { for (i = 0; i < spaces; i++) printf(" "); printf(__VA_ARGS__); } while (0)
# define D(...) do { printf(__VA_ARGS__); } while (0)
#else
# define DDD(...) do { } while (0)
# define D(...) do { } while (0)
#endif

#ifdef DDD_DO
static void
walk_clippers_print(int spaces, Evas_Object_Protected_Data *obj)
{
   int i;
   spaces++;
   DDD("<<< CLIP %p c[%6i %6i %6ix%6i] c[%6i %6i %6ix%6i]\n",
       obj->object,
       obj->cur->geometry.x, obj->cur->geometry.y,
       obj->cur->geometry.w, obj->cur->geometry.h,
       obj->cur->cache.clip.x, obj->cur->cache.clip.y,
       obj->cur->cache.clip.w, obj->cur->cache.clip.h
      );
   if (obj->cur->clipper) walk_clippers_print(spaces, obj->cur->clipper);
   spaces--;
}
#endif

static void
clip_calc(Evas_Object_Protected_Data *obj, Evas_Coord_Rectangle *c)
{
   if (!obj) return;
   RECTS_CLIP_TO_RECT(c->x, c->y, c->w, c->h,
                      obj->cur->geometry.x, obj->cur->geometry.y,
                      obj->cur->geometry.w, obj->cur->geometry.h);
   clip_calc(obj->cur->clipper, c);
}

static Eina_List *
_evas_event_object_list_raw_in_get(Evas *eo_e, Eina_List *in,
                                   const Eina_Inlist *list, Evas_Object *stop,
                                   int x, int y, int *no_rep, Eina_Bool source)
{
   Evas_Object *eo_obj;
   Evas_Object_Protected_Data *obj = NULL;
   Evas_Coord_Rectangle c;
   int inside;

#ifdef DDD_DO
   static int spaces = 0;
   int i;
#endif

   if (!list) return in;
#ifdef DDD_DO
   spaces++;
#endif
   for (obj = _EINA_INLIST_CONTAINER(obj, eina_inlist_last(list));
        obj;
        obj = _EINA_INLIST_CONTAINER(obj, EINA_INLIST_GET(obj)->prev))
     {
        eo_obj = obj->object;
        if (eo_obj == stop)
          {
             *no_rep = 1;
#ifdef DDD_DO
             spaces--;
             DDD("***** NO REP - STOP *****\n");
#endif
             return in;
          }
        if (!obj->cur->visible) continue;
        // XXX: this below DYNAMICALLY calculates the current clip rect
        // by walking clippers to each parent clipper until there are
        // no more of them. this is a necessary hack because cache.clip
        // cooreds are broken. somewhere along the way in the past few years
        // someone has forgotten to flag them as dirty and update them
        // so a clicp recalce caqn work... somewhere. maybe a prexy or map fix
        // or an optimization. finding this is really hard, so i'm going
        // for plan b and doing this on the fly. it's only for event or
        // callback handling so its a small percentage of the time, but
        // it's better that we get this right
        if (obj->is_smart)
          {
             Evas_Coord_Rectangle bounding_box = { 0, 0, 0, 0 };

             evas_object_smart_bounding_box_update(eo_obj, obj);
             evas_object_smart_bounding_box_get(obj->object, &bounding_box, NULL);
             c = bounding_box;
          }
        else
          {
             if (obj->clip.clipees) continue;
             c = obj->cur->geometry;
          }
        clip_calc(obj->cur->clipper, &c);
        // only worry about objects that intersect INCLUDING clippint
        if ((!RECTS_INTERSECT(x, y, 1, 1, c.x, c.y, c.w, c.h)))
          {
#ifdef DDD_DO
             if (obj->is_smart)
               {
                  Evas_Coord_Rectangle bounding_box = { 0, 0, 0, 0 };

                  evas_object_smart_bounding_box_get(obj->object, &bounding_box, NULL);
                  DDD("___  %p g[%6i %6i %6ix%6i] c[%6i %6i %6ix%6i] b[%6i %6i %6ix%6i] %s\n",
                      obj->object,
                      obj->cur->geometry.x, obj->cur->geometry.y,
                      obj->cur->geometry.w, obj->cur->geometry.h,
                      obj->cur->cache.clip.x, obj->cur->cache.clip.y,
                      obj->cur->cache.clip.w, obj->cur->cache.clip.h,
                      bounding_box.x, bounding_box.y,
                      bounding_box.w, bounding_box.h,
                      obj->type);
               }
             else
               {
                  DDD("___  %p g[%6i %6i %6ix%6i] c[%6i %6i %6ix%6i] %s\n",
                      obj->object,
                      obj->cur->geometry.x, obj->cur->geometry.y,
                      obj->cur->geometry.w, obj->cur->geometry.h,
                      obj->cur->cache.clip.x, obj->cur->cache.clip.y,
                      obj->cur->cache.clip.w, obj->cur->cache.clip.h,
                      obj->type);
               }
             if (!strcmp(obj->type, "e_layout"))
               {
                  if (obj->cur->clipper)
                    walk_clippers_print(spaces, obj->cur->clipper);
               }
#endif
             continue;
          }
#ifdef DDD_DO
        else
          {
             if (obj->is_smart)
               {
                  Evas_Coord_Rectangle bounding_box = { 0, 0, 0, 0 };

                  evas_object_smart_bounding_box_get(obj->object, &bounding_box, NULL);
                  DDD("OBJ  %p g[%6i %6i %6ix%6i] c[%6i %6i %6ix%6i] b[%6i %6i %6ix%6i] %s\n",
                      obj->object,
                      obj->cur->geometry.x, obj->cur->geometry.y,
                      obj->cur->geometry.w, obj->cur->geometry.h,
                      obj->cur->cache.clip.x, obj->cur->cache.clip.y,
                      obj->cur->cache.clip.w, obj->cur->cache.clip.h,
                      bounding_box.x, bounding_box.y,
                      bounding_box.w, bounding_box.h,
                      obj->type);
               }
             else
               {
                  DDD("OBJ  %p g[%6i %6i %6ix%6i] c[%6i %6i %6ix%6i] %s\n",
                      obj->object,
                      obj->cur->geometry.x, obj->cur->geometry.y,
                      obj->cur->geometry.w, obj->cur->geometry.h,
                      obj->cur->cache.clip.x, obj->cur->cache.clip.y,
                      obj->cur->cache.clip.w, obj->cur->cache.clip.h,
                      obj->type);
               }
//             if (!strcmp(obj->type, "e_layout"))
               {
                  if (obj->cur->clipper)
                    walk_clippers_print(spaces, obj->cur->clipper);
               }
          }
#endif

        if (!source)
          {
             if (evas_event_passes_through(eo_obj, obj)) continue;
             if (evas_object_is_source_invisible(eo_obj, obj)) continue;
          }
        if ((obj->delete_me == 0) &&
            ((source) || ((obj->cur->visible) && (!obj->clip.clipees) &&
             evas_object_clippers_is_visible(eo_obj, obj))))
          {
             if (obj->is_smart)
               {
                  DDD("CHILDREN ->\n");
                  Evas_Object_Protected_Data *clip = obj->cur->clipper;
                  int norep = 0;

                  if (clip && clip->mask->is_mask && clip->precise_is_inside)
                    if (!evas_object_is_inside(clip->object, clip, x, y))
                      continue;

                  if ((obj->map->cur.usemap) && (obj->map->cur.map) &&
                      (obj->map->cur.map->count == 4))
                    {
                       inside = evas_object_is_in_output_rect(eo_obj, obj, x, y, 1, 1);
                       if (inside)
                         {
                            if (!evas_map_coords_get(obj->map->cur.map, x, y,
                                                     &(obj->map->cur.map->mx),
                                                     &(obj->map->cur.map->my), 0))
                              {
                                 inside = 0;
                              }
                            else
                              {
                                 in = _evas_event_object_list_in_get
                                    (eo_e, in,
                                     evas_object_smart_members_get_direct(eo_obj),
                                     stop,
                                     obj->cur->geometry.x + obj->map->cur.map->mx,
                                     obj->cur->geometry.y + obj->map->cur.map->my,
                                     &norep, source);
                              }
                         }
                    }
                  else
                    {
                       Evas_Coord_Rectangle bounding_box = { 0, 0, 0, 0 };

                       if (!obj->child_has_map)
                         evas_object_smart_bounding_box_update(eo_obj, obj);

                       evas_object_smart_bounding_box_get(eo_obj, &bounding_box, NULL);

                       if (obj->child_has_map ||
                           (bounding_box.x <= x &&
                            bounding_box.x + bounding_box.w >= x &&
                            bounding_box.y <= y &&
                            bounding_box.y + bounding_box.h >= y) ||
                           (obj->cur->geometry.x <= x &&
                            obj->cur->geometry.x + obj->cur->geometry.w >= x &&
                            obj->cur->geometry.y <= y &&
                            obj->cur->geometry.y + obj->cur->geometry.h >= y))
                         in = _evas_event_object_list_in_get
                            (eo_e, in, evas_object_smart_members_get_direct(eo_obj),
                            stop, x, y, &norep, source);
                    }
                  if (norep)
                    {
                       if (!obj->repeat_events)
                         {
                            *no_rep = 1;
#ifdef DDD_DO
                            spaces--;
                            DDD("***** NO REP1 *****\n");
#endif
                            return in;
                         }
                    }
               }
             else
               {
                  Evas_Object_Protected_Data *clip = obj->cur->clipper;
                  if (clip && clip->mask->is_mask && clip->precise_is_inside)
                    inside = evas_object_is_inside(clip->object, clip, x, y);
                  else
                    inside = evas_object_is_in_output_rect(eo_obj, obj, x, y, 1, 1);

                  if (inside)
                    {
                       if ((obj->map->cur.usemap) && (obj->map->cur.map) &&
                           (obj->map->cur.map->count == 4))
                         {
                            if (!evas_map_coords_get(obj->map->cur.map, x, y,
                                                     &(obj->map->cur.map->mx),
                                                     &(obj->map->cur.map->my), 0))
                              {
                                 inside = 0;
                              }
                         }
                    }
                  if (inside && ((!obj->precise_is_inside) ||
                                 (evas_object_is_inside(eo_obj, obj, x, y))))
                    {
                       if (!evas_event_freezes_through(eo_obj, obj))
                         {
                            DDD("----------------> ADD obj %p\n", obj->object);
                            in = eina_list_append(in, eo_obj);
                         }
                       if (!obj->repeat_events)
                         {
                            *no_rep = 1;
#ifdef DDD_DO
                            spaces--;
                            DDD("***** NO REP2 *****\n");
#endif
                            return in;
                         }
                    }
               }
          }
     }
   *no_rep = 0;
#ifdef DDD_DO
   spaces--;
#endif
   return in;
}

static void
_transform_to_src_space_f(Evas_Object_Protected_Data *obj, Evas_Object_Protected_Data *src, Eina_Vector2 *point)
{
   double obj_w = obj->cur->geometry.w, obj_h = obj->cur->geometry.h;
   double src_w = src->cur->geometry.w, src_h = src->cur->geometry.h;
   double tmp_x = point->x;
   double tmp_y = point->y;

   tmp_x -= obj->cur->geometry.x;
   tmp_y -= obj->cur->geometry.y;

   if (obj_w != src_w)
     tmp_x = (tmp_x * (src_w / obj_w));
   if (obj_h != src_h)
     tmp_y = (tmp_y * (src_h / obj_h));

   tmp_x += src->cur->geometry.x;
   tmp_y += src->cur->geometry.y;
   point->x = tmp_x;
   point->y = tmp_y;
}

static void
_evas_event_source_mouse_down_events(Evas_Object *eo_obj, Evas *eo_e,
                                     Efl_Input_Pointer *parent_ev, int event_id)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Object_Protected_Data *child;
   Evas_Object *eo_child;
   Eina_List *copy, *l;
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Eina_Vector2 point;
   int addgrab = 0;
   int no_rep = 0;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

   evt = efl_input_dup(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = eo_obj;
   ev->tool = 0;

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_write)
     {
        if (proxy_write->src_event_in)
        proxy_write->src_event_in = eina_list_free(proxy_write->src_event_in);

        if (src->is_smart)
          {
             proxy_write->src_event_in = _evas_event_object_list_raw_in_get
               (eo_e, proxy_write->src_event_in,
                evas_object_smart_members_get_direct(eo_src),
                NULL, ev->cur.x, ev->cur.y, &no_rep, EINA_TRUE);
         }
       else
         proxy_write->src_event_in = eina_list_append(proxy_write->src_event_in, eo_src);
     }
   EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_write);

   if (e->pointer.downs > 1) addgrab = e->pointer.downs - 1;

   EINA_LIST_FOREACH(src->proxy->src_event_in, l, eo_child)
     {
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        if ((child->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) ||
            (child->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN))
          {
             child->mouse_grabbed += (addgrab + 1);
             e->pointer.mouse_grabbed += (addgrab + 1);
             if (child->pointer_mode ==
                 EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
               {
                  e->pointer.nogrep++;
                  break;
               }
          }
     }

   copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
        evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_DOWN, evt,
                                        event_id, EFL_EVENT_POINTER_DOWN);
        if (e->delete_me) break;
        if (obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
          break;
     }
   eina_list_free(copy);

   efl_del(evt);
}

static void
_evas_event_source_mouse_move_events(Evas_Object *eo_obj, Evas *eo_e,
                                     Efl_Input_Pointer *parent_ev, int event_id)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Object_Protected_Data *child;
   Evas_Object *eo_child;
   Eina_List *l;
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Eina_Vector2 curpt, curpt_real, prevpt, prevpt_real;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

   evt = efl_input_dup(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   curpt_real = ev->cur;
   prevpt_real = ev->prev;
   _transform_to_src_space_f(obj, src, &ev->cur);
   _transform_to_src_space_f(obj, src, &ev->prev);
   curpt = ev->cur;
   prevpt = ev->prev;
   ev->source = eo_obj;
   ev->tool = 0;

   if (e->pointer.mouse_grabbed)
     {
        Eina_List *outs = NULL;
        Eina_List *copy = evas_event_list_copy(src->proxy->src_event_in);

        EINA_LIST_FOREACH(copy, l, eo_child)
          {
             child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);

             if ((evas_object_clippers_is_visible(eo_child, child) ||
                 child->mouse_grabbed) &&
               (!evas_event_passes_through(eo_child, child)) &&
               (!evas_event_freezes_through(eo_child, child)) &&
               (!child->clip.clipees))
               {
                  ev->cur = curpt;
                  ev->prev = prevpt;
                  _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
                  _evas_event_havemap_adjust_f(eo_child, child, &ev->prev, child->mouse_grabbed);

                  ev->action = EFL_POINTER_ACTION_MOVE;
                  evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                  event_id, EFL_EVENT_POINTER_MOVE);
               }
             else
               outs = eina_list_append(outs, eo_child);
             if (e->delete_me || e->is_frozen) break;
             //FIXME: take care nograb object 
          }
        eina_list_free(copy);

        EINA_LIST_FREE(outs, eo_child)
          {
             child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
             if ((child->mouse_grabbed == 0) && (!e->delete_me))
               {
                  if (child->mouse_in) continue; /* FIXME: dubious logic! */
                  child->mouse_in = 0;
                  if (child->delete_me || e->is_frozen) continue;
                  ev->cur = curpt_real;
                  _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
                  EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_write)
                    proxy_write->src_event_in = eina_list_remove(proxy_write->src_event_in, eo_child);
                  EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_write);

                  ev->action = EFL_POINTER_ACTION_OUT;
                  evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_OUT, evt,
                                                  event_id, EFL_EVENT_POINTER_OUT);
               }
          }
     }
   else
     {
        Eina_List *ins = NULL;
        Eina_List *copy = evas_event_list_copy(src->proxy->src_event_in);

        if (src->is_smart)
          {
             int no_rep = 0;
             ins = _evas_event_object_list_raw_in_get(eo_e, ins, evas_object_smart_members_get_direct(eo_src),
                                                      NULL, ev->cur.x, ev->cur.y, &no_rep, EINA_TRUE);
          }
        else
          ins = eina_list_append(ins, eo_src);

        EINA_LIST_FOREACH(copy, l, eo_child)
          {
             child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);

             ev->cur = curpt;
             if (evas_object_is_in_output_rect(eo_child, child,
                                               ev->cur.x, ev->cur.y, 1, 1) &&
                (evas_object_clippers_is_visible(eo_child, child) ||
                 child->mouse_grabbed) &&
                eina_list_data_find(ins, eo_child) &&
               (!evas_event_passes_through(eo_child, child)) &&
               (!evas_event_freezes_through(eo_child, child)) &&
               (!child->clip.clipees) &&
               ((!child->precise_is_inside) ||
                evas_object_is_inside(eo_child, child, ev->cur.x, ev->cur.y)))
               {
                  ev->action = EFL_POINTER_ACTION_MOVE;
                  _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
                  evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                  event_id, EFL_EVENT_POINTER_MOVE);
               }
             else if (child->mouse_in)
               {
                  child->mouse_in = 0;
                  if (e->is_frozen) continue;
                  ev->cur = curpt;
                  _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
                  ev->action = EFL_POINTER_ACTION_OUT;
                  evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_OUT, evt,
                                                  event_id, EFL_EVENT_POINTER_OUT);
                  if (e->delete_me) break;

               }
             if (e->delete_me || e->is_frozen) break;
          }
        eina_list_free(copy);

        int event_id2 = _evas_object_event_new();
        EINA_LIST_FOREACH(ins, l, eo_child)
          {
              child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
              if (!eina_list_data_find(src->proxy->src_event_in, eo_child))
                {
                   if (!child->mouse_in)
                     {
                        child->mouse_in = 1;
                        if (e->is_frozen) continue;
                        ev->cur = curpt;
                        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
                        ev->action = EFL_POINTER_ACTION_IN;
                        evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_IN, evt,
                                                        event_id2, EFL_EVENT_POINTER_IN);
                        if (e->delete_me) break;
                     }
                }
          }

        if (e->pointer.mouse_grabbed == 0)
          {
             EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_write)
               {
                  eina_list_free(proxy_write->src_event_in);
                  proxy_write->src_event_in = ins;
               }
             EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_write);
          }
        else
          {
             if (ins) eina_list_free(ins);
          }
     }

   efl_del(evt);
}

static void
_evas_event_source_mouse_up_events(Evas_Object *eo_obj, Evas *eo_e,
                                   Efl_Input_Pointer *parent_ev, int event_id)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Object_Protected_Data *child;
   Evas_Object *eo_child;
   Eina_List *copy, *l;
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Eina_Vector2 point;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

   evt = efl_input_dup(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = eo_obj;
   ev->tool = 0;

   copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        if (src->delete_me) break;
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        if (((child->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) ||
             (child->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)) ||
            (child->mouse_grabbed > 0))
          {
             child->mouse_grabbed--;
             e->pointer.mouse_grabbed--;
          }

        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
        evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_UP, evt,
                                        event_id, EFL_EVENT_POINTER_UP);
        if (e->delete_me) break;
        if (obj->pointer_mode ==
            EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
          {
             if (e->pointer.nogrep > 0) e->pointer.nogrep--;
             break;
          }
     }
   eina_list_free(copy);

   efl_del(evt);
}

static void
_evas_event_source_hold_events(Evas_Object *eo_obj, int event_id, Efl_Input_Hold *evt)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *child;
   Evas_Object *eo_child;
   Eina_List *l;

   if (obj->layer->evas->is_frozen) return;

   EINA_LIST_FOREACH(src->proxy->src_event_in, l, eo_child)
     {
        if (src->delete_me) return;
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_HOLD, evt,
                                        event_id, EFL_EVENT_HOLD);
        if (src->layer->evas->delete_me) break;
     }
}

static void
_evas_event_source_wheel_events(Evas_Object *eo_obj, Evas *eo_e,
                                Efl_Input_Pointer *parent_ev, int event_id)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Object_Protected_Data *child;
   Evas_Object *eo_child;
   Eina_List *copy, *l;
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Eina_Vector2 point;

   if (obj->delete_me || src->delete_me || obj->layer->evas->is_frozen) return;

   evt = efl_input_dup(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = eo_obj;

   copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        if (src->delete_me) return;
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
        evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_WHEEL, evt,
                                        event_id, EFL_EVENT_POINTER_WHEEL);
        if (e->delete_me) break;
     }
   eina_list_free(copy);

   efl_del(evt);
}

static void
_evas_event_source_multi_down_events(Evas_Object_Protected_Data *obj, Evas_Public_Data *e,
                                     Efl_Input_Pointer *parent_ev, int event_id)
{
   Evas_Object *eo_src = _evas_object_image_source_get(obj->object);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *child;
   Evas_Object *eo_child;
   Eina_List *copy, *l;
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Eina_Vector2 point;
   int addgrab = 0;

   if (obj->delete_me || src->delete_me || obj->layer->evas->is_frozen) return;

   evt = efl_input_dup(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = obj->object;
   ev->action = EFL_POINTER_ACTION_DOWN;

   if (e->pointer.downs > 1) addgrab = e->pointer.downs - 1;

   EINA_LIST_FOREACH(src->proxy->src_event_in, l, eo_child)
     {
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        if (child->pointer_mode != EVAS_OBJECT_POINTER_MODE_NOGRAB)
          {
             child->mouse_grabbed += (addgrab + 1);
             e->pointer.mouse_grabbed += (addgrab + 1);
          }
     }

   copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
        evas_object_event_callback_call(obj->object, obj, EVAS_CALLBACK_MULTI_DOWN, evt,
                                        event_id, EFL_EVENT_FINGER_DOWN);
        if (e->delete_me) break;
     }
   eina_list_free(copy);

   efl_del(evt);
}

static void
_evas_event_source_multi_up_events(Evas_Object_Protected_Data *obj, Evas_Public_Data *e,
                                   Efl_Input_Pointer *parent_ev, int event_id)
{
   Evas_Object *eo_src = _evas_object_image_source_get(obj->object);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *child;
   Evas_Object *eo_child;
   Eina_List *copy, *l;
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Eina_Vector2 point;

   if (obj->delete_me || src->delete_me || obj->layer->evas->is_frozen) return;

   evt = efl_input_dup(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = obj->object;
   ev->action = EFL_POINTER_ACTION_UP;

   copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        if (((child->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) ||
             (child->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)) ||
            (child->mouse_grabbed > 0))
          {
             child->mouse_grabbed--;
             e->pointer.mouse_grabbed--;
          }
        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
        evas_object_event_callback_call(obj->object, obj, EVAS_CALLBACK_MULTI_UP, evt,
                                        event_id, EFL_EVENT_FINGER_UP);
        if (e->delete_me || e->is_frozen) break;
     }
    eina_list_free(copy);

    efl_del(evt);
}

static void
_evas_event_source_multi_move_events(Evas_Object_Protected_Data *obj, Evas_Public_Data *e,
                                     Efl_Input_Pointer *parent_ev, int event_id)
{
   Evas_Object *eo_src = _evas_object_image_source_get(obj->object);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *child;
   Evas_Object *eo_child;
   Eina_List *copy, *l;
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Eina_Vector2 point;
   Evas *eo_e = e->evas;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

   evt = efl_input_dup(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = obj->object;
   ev->action = EFL_POINTER_ACTION_UP;

   /* Why a new event id here? Other 'source' events keep the same id. */
   event_id = _evas_object_event_new();

   if (e->pointer.mouse_grabbed > 0)
     {
        copy = evas_event_list_copy(src->proxy->src_event_in);
        EINA_LIST_FOREACH(copy, l, eo_child)
          {
             child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
             if (((evas_object_clippers_is_visible(eo_child, child)) ||
                  ((child->mouse_grabbed) &&
                  (!evas_event_passes_through(eo_child, child)) &&
                  (!evas_event_freezes_through(eo_child, child)) &&
                  (!child->clip.clipees))))
               {
                  child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
                  ev->cur = point;
                  _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
                  evas_object_event_callback_call(obj->object, obj, EVAS_CALLBACK_MULTI_MOVE, evt,
                                                  event_id, EFL_EVENT_FINGER_MOVE);
                  if (e->delete_me || e->is_frozen) break;
               }
          }
        eina_list_free(copy);
     }
   else
     {
        Eina_List *ins = NULL;

        if (src->is_smart)
          {
             int no_rep = 0;
             ins = _evas_event_object_list_raw_in_get
                   (eo_e, ins, evas_object_smart_members_get_direct(eo_src), NULL,
                    ev->cur.x, ev->cur.y, &no_rep, EINA_TRUE);
          }
        else
          ins = eina_list_append(ins, eo_src);

        copy = evas_event_list_copy(src->proxy->src_event_in);
        EINA_LIST_FOREACH(copy, l, eo_child)
          {
             child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
             ev->cur = point;

             if (evas_object_is_in_output_rect(eo_child, child, ev->cur.x, ev->cur.y, 1, 1) &&
                (evas_object_clippers_is_visible(eo_child, child) ||
                 child->mouse_grabbed) &&
                eina_list_data_find(ins, eo_child) &&
               (!evas_event_passes_through(eo_child, child)) &&
               (!evas_event_freezes_through(eo_child, child)) &&
               (!child->clip.clipees) &&
               ((!child->precise_is_inside) ||
                evas_object_is_inside(eo_child, child, ev->cur.x, ev->cur.y)))
               {
                  _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
                  evas_object_event_callback_call(obj->object, obj, EVAS_CALLBACK_MULTI_MOVE, evt,
                                                  event_id, EFL_EVENT_FINGER_MOVE);
                  if (e->delete_me || e->is_frozen) break;
               }
          }
        eina_list_free(copy);
        if (e->pointer.mouse_grabbed == 0)
          {
             EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_write)
               {
                  eina_list_free(proxy_write->src_event_in);
                  proxy_write->src_event_in = ins;
               }
             EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_write);
          }
        else
          eina_list_free(ins);
     }

   efl_del(evt);
}

static void
_evas_event_source_mouse_in_events(Evas_Object *eo_obj, Evas *eo_e,
                                   Efl_Input_Pointer *parent_ev, int event_id)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Object_Protected_Data *child;
   Evas_Object *eo_child;
   Eina_List *ins = NULL, *l;
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Eina_Vector2 point;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

   evt = efl_input_dup(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = eo_obj;
   ev->action = EFL_POINTER_ACTION_IN;

   if (src->is_smart)
     {
        int no_rep = 0;
        ins = _evas_event_object_list_raw_in_get(eo_e, ins, evas_object_smart_members_get_direct(eo_src),
                                                 NULL, ev->cur.x, ev->cur.y, &no_rep, EINA_TRUE);

     }
   else
     ins = eina_list_append(ins, eo_src);

   EINA_LIST_FOREACH(ins, l, eo_child)
     {
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        if (!eina_list_data_find(src->proxy->src_event_in, eo_child))
          {
             if (child->mouse_in) continue;
             child->mouse_in = 1;
             ev->cur = point;
             _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
             evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_IN, evt,
                                             event_id, EFL_EVENT_POINTER_IN);
             if (e->delete_me || e->is_frozen) break;
          }
     }

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_write)
     {
       eina_list_free(proxy_write->src_event_in);
       proxy_write->src_event_in = ins;
     }
   EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_write);

   efl_del(evt);
}

static void
_evas_event_source_mouse_out_events(Evas_Object *eo_obj, Evas *eo_e,
                                    Efl_Input_Pointer *parent_ev, int event_id)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Object_Protected_Data *child;
   Evas_Object *eo_child;
   Eina_List *copy, *l;
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Eina_Vector2 point;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

   evt = efl_input_dup(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = eo_obj;
   ev->action = EFL_POINTER_ACTION_OUT;

   copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        if (!child->mouse_in) continue;
        child->mouse_in = 0;
        if (child->delete_me) continue;

        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, evt,
                                        event_id, EFL_EVENT_POINTER_OUT);
        if (e->is_frozen) continue;
     }
   eina_list_free(copy);

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_write)
     proxy_write->src_event_in = eina_list_free(proxy_write->src_event_in);
   EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_write);

   efl_del(evt);
}

static Eina_List *
_evas_event_object_list_in_get(Evas *eo_e, Eina_List *in,
                               const Eina_Inlist *list, Evas_Object *stop,
                               int x, int y, int *no_rep, Eina_Bool source)
{
   if (!list) return NULL;
   return _evas_event_object_list_raw_in_get(eo_e, in, list->last, stop, x, y,
                                             no_rep, source);
}

static Eina_List *
_evas_event_objects_event_list_no_frozen_check(Evas *eo_e, Evas_Object *stop, int x, int y)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Layer *lay;
   Eina_List *in = NULL;

   if (!e->layers) return NULL;

   D("@@@@@ layer count = %i\n", eina_inlist_count(EINA_INLIST_GET(e->layers)));
   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
        int no_rep = 0;
        D("############################# check layer %i\n", lay->layer);
        in = _evas_event_object_list_in_get(eo_e, in,
                                            EINA_INLIST_GET(lay->objects),
                                            stop, x, y, &no_rep, EINA_FALSE);
        if (no_rep) return in;
     }
   return in;
}

EOLIAN Eina_List*
_evas_canvas_tree_objects_at_xy_get(Eo *eo_e, Evas_Public_Data *e EINA_UNUSED, Evas_Object *stop, int x, int y)
{
   return _evas_event_objects_event_list_no_frozen_check(eo_e, stop, x, y);
}

Eina_List *
evas_event_objects_event_list(Evas *eo_e, Evas_Object *stop, int x, int y)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   if ((!e->layers) || (e->is_frozen)) return NULL;
   D("------------------------------GET EVETNS AT ............... %i %i\n", x, y); 
   return _evas_event_objects_event_list_no_frozen_check(eo_e, stop, x, y);
}

static Eina_List *
evas_event_list_copy(Eina_List *list)
{
   Eina_List *l, *new_l = NULL;
   const void *data;

   EINA_LIST_FOREACH(list, l, data)
     new_l = eina_list_append(new_l, data);
   return new_l;
}
/* public functions */

EOLIAN void
_evas_canvas_event_default_flags_set(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Event_Flags flags)
{
   e->default_event_flags = flags;
}

EOLIAN Evas_Event_Flags
_evas_canvas_event_default_flags_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->default_event_flags;
}

static inline void
_canvas_event_thaw_eval_internal(Eo *eo_e, Evas_Public_Data *e)
{
   _canvas_event_feed_mouse_move_legacy(eo_e, e, e->pointer.x, e->pointer.y,
                                        e->last_timestamp, NULL);
}

EAPI void
evas_event_freeze(Evas *eo_e)
{
   efl_event_freeze(eo_e);
}

EAPI void
evas_event_thaw(Evas *eo_e)
{
   efl_event_thaw(eo_e);
}

EOLIAN void
_evas_canvas_efl_object_event_freeze(Eo *eo_e, Evas_Public_Data *e)
{
   efl_event_freeze(efl_super(eo_e, EVAS_CANVAS_CLASS));
   e->is_frozen = EINA_TRUE;
}

EOLIAN void
_evas_canvas_efl_object_event_thaw(Eo *eo_e, Evas_Public_Data *e)
{
   int fcount = -1;
   efl_event_thaw(efl_super(eo_e, EVAS_CANVAS_CLASS));
   fcount = efl_event_freeze_count_get(efl_super(eo_e, EVAS_CANVAS_CLASS));
   if (0 == fcount)
     {
        Evas_Layer *lay;

        e->is_frozen = EINA_FALSE;
        EINA_INLIST_FOREACH((EINA_INLIST_GET(e->layers)), lay)
          {
             Evas_Object_Protected_Data *obj;

             EINA_INLIST_FOREACH(lay->objects, obj)
               {
                  evas_object_clip_recalc(obj);
                  evas_object_recalc_clippees(obj);
               }
          }

        _canvas_event_thaw_eval_internal(eo_e, e);
     }
}

EAPI int
evas_event_freeze_get(const Evas *eo_e)
{
   return efl_event_freeze_count_get(eo_e);
}

EAPI void
evas_event_thaw_eval(Evas *eo_e)
{
   if (!evas_event_freeze_get(eo_e))
     {
        EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
        Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
        _canvas_event_thaw_eval_internal(eo_e, e);
     }
}

static void
_canvas_event_feed_mouse_down_internal(Evas_Public_Data *e, Efl_Input_Pointer_Data *ev)
{
   Efl_Input_Pointer *evt;
   Eina_List *l, *copy;
   Evas_Object *eo_obj;
   int event_id, b;
   Evas *eo_e;
   int addgrab = 0;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL) |
         _efl_input_value_mask(EFL_INPUT_VALUE_BUTTON);

   if (!e || !ev) return;

   b = ev->button;
   DBG("ButtonEvent:down time=%u x=%d y=%d button=%d downs=%d",
       ev->timestamp, e->pointer.x, e->pointer.y, b, e->pointer.downs);
   if ((b < 1) || (b > 32)) return;

   e->pointer.button |= (1u << (b - 1));
   e->pointer.downs++;

   if (e->is_frozen) return;
   e->last_timestamp = ev->timestamp;
   eo_e = e->evas;
   evt = ev->eo;

   event_id = _evas_object_event_new();

   ev->cur.x = e->pointer.x;
   ev->cur.y = e->pointer.y;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->tool = 0;
   ev->action = EFL_POINTER_ACTION_DOWN;
   ev->value_flags |= value_flags;
   if (ev->device) efl_ref(ev->device);

   _evas_walk(e);
   /* append new touch point to the touch point list */
   _evas_touch_point_append(e->evas, 0, e->pointer.x, e->pointer.y);
   /* If this is the first finger down, i.e no other fingers pressed,
    * get a new event list, otherwise, keep the current grabbed list. */
   if (e->pointer.mouse_grabbed == 0)
     {
        Eina_List *ins = evas_event_objects_event_list(eo_e,
                                                       NULL,
                                                       e->pointer.x,
                                                       e->pointer.y);
        /* free our old list of ins */
        e->pointer.object.in = eina_list_free(e->pointer.object.in);
        /* and set up the new one */
        e->pointer.object.in = ins;
        /* adjust grabbed count by the nuymber of currently held down
         * fingers/buttons */
        if (e->pointer.downs > 1) addgrab = e->pointer.downs - 1;
     }
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if ((obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) ||
            (obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN))
          {
             obj->mouse_grabbed += addgrab + 1;
             e->pointer.mouse_grabbed += addgrab + 1;
             if (obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
               {
                  e->pointer.nogrep++;
                  break;
               }
          }
     }
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if (obj->delete_me) continue;
        ev->cur.x = e->pointer.x;
        ev->cur.y = e->pointer.y;
        _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_DOWN, evt,
                                        event_id, EFL_EVENT_POINTER_DOWN);
        if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
          _evas_event_source_mouse_down_events(eo_obj, eo_e, evt, event_id);
        if (e->is_frozen || e->delete_me)  break;
        if (obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
          break;
     }
   if (copy) eina_list_free(copy);
   e->last_mouse_down_counter++;
   _evas_post_event_callback_call(eo_e, e);
   /* update touch point's state to EVAS_TOUCH_POINT_STILL */
   _evas_touch_point_update(eo_e, 0, e->pointer.x, e->pointer.y, EVAS_TOUCH_POINT_STILL);
   _evas_unwalk(e);

   if (ev->device) efl_unref(ev->device);
}

static int
_post_up_handle(Evas_Public_Data *e, Efl_Input_Pointer *parent_ev)
{
   Eina_List *l, *copy, *ins, *ll;
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Evas_Object *eo_obj;
   Evas *eo_e = e->evas;
   int post_called = 0;
   int event_id;

   event_id = _evas_object_event_new();

   /* Duplicating UP event */
   evt = efl_input_dup(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return 0;

   /* Actually we want an OUT */
   ev->action = EFL_POINTER_ACTION_OUT;

   /* get new list of ins */
   ins = evas_event_objects_event_list(eo_e, NULL, e->pointer.x, e->pointer.y);
   /* go thru old list of in objects */
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, ll, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if ((!eina_list_data_find(ins, eo_obj)) || (!e->pointer.inside))
          {
             if (!obj->mouse_in) continue;
             obj->mouse_in = 0;
             if (!e->is_frozen)
               {
                  ev->cur.x = e->pointer.x;
                  ev->cur.y = e->pointer.y;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, evt,
                                                  event_id, EFL_EVENT_POINTER_OUT);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_out_events(eo_obj, eo_e, evt, event_id);
                  if (e->delete_me) break;
               }
          }
     }
   _evas_post_event_callback_call(eo_e, e);

   eina_list_free(copy);

   if (e->pointer.inside)
     {
        Evas_Object *eo_obj_itr;

        event_id = _evas_object_event_new();
        ev->action = EFL_POINTER_ACTION_IN;

        EINA_LIST_FOREACH(ins, l, eo_obj_itr)
          {
             Evas_Object_Protected_Data *obj_itr = efl_data_scope_get(eo_obj_itr, EFL_CANVAS_OBJECT_CLASS);
             if (!eina_list_data_find(e->pointer.object.in, eo_obj_itr))
               {
                  if (obj_itr->mouse_in) continue;
                  obj_itr->mouse_in = 1;
                  if (e->is_frozen) continue;
                  ev->cur.x = e->pointer.x;
                  ev->cur.y = e->pointer.y;
                  _evas_event_havemap_adjust_f(eo_obj_itr, obj_itr, &ev->cur, obj_itr->mouse_grabbed);
                  evas_object_event_callback_call(eo_obj_itr, obj_itr, EVAS_CALLBACK_MOUSE_IN, evt,
                                                  event_id, EFL_EVENT_POINTER_IN);
                  if ((obj_itr->proxy->is_proxy) &&
                      (obj_itr->proxy->src_events))
                    _evas_event_source_mouse_in_events(eo_obj_itr, eo_e, evt, event_id);
                  if (e->delete_me) break;
               }
          }
        post_called = 1;
        _evas_post_event_callback_call(eo_e, e);
     }
   else
     {
        ins = eina_list_free(ins);
     }

   if (e->pointer.mouse_grabbed == 0)
     {
        /* free our old list of ins */
        eina_list_free(e->pointer.object.in);
        /* and set up the new one */
        e->pointer.object.in = ins;
     }
   else
     {
        /* free our cur ins */
        eina_list_free(ins);
     }
   if (e->pointer.inside)
     _canvas_event_feed_mouse_move_legacy(eo_e, e, e->pointer.x, e->pointer.y, ev->timestamp, ev->data);

   efl_del(evt);

   return post_called;
}

static void
_canvas_event_feed_mouse_up_internal(Evas_Public_Data *e, Efl_Input_Pointer_Data *ev)
{
   Efl_Input_Pointer *evt;
   Eina_List *l, *copy;
   Evas_Object *eo_obj;
   int event_id, b;
   Evas *eo_e;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL) |
         _efl_input_value_mask(EFL_INPUT_VALUE_BUTTON);

   if (!e || !ev) return;

   b = ev->button;
   DBG("ButtonEvent:up time=%u x=%d y=%d button=%d downs=%d",
       ev->timestamp, e->pointer.x, e->pointer.y, b, e->pointer.downs);
   if ((b < 1) || (b > 32)) return;
   if (e->pointer.downs <= 0) return;

   e->pointer.button &= ~(1u << (b - 1));
   e->pointer.downs--;

   if (e->is_frozen) return;
   e->last_timestamp = ev->timestamp;
   eo_e = e->evas;
   evt = ev->eo;

   event_id = _evas_object_event_new();

   ev->cur.x = e->pointer.x;
   ev->cur.y = e->pointer.y;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->tool = 0;
   ev->value_flags |= value_flags;
   if (ev->device) efl_ref(ev->device);

   _evas_walk(e);
   /* update released touch point */
   _evas_touch_point_update(eo_e, 0, e->pointer.x, e->pointer.y, EVAS_TOUCH_POINT_UP);
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if (((obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) ||
             (obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)) &&
            (obj->mouse_grabbed > 0))
          {
             obj->mouse_grabbed--;
             e->pointer.mouse_grabbed--;
          }
        if (!obj->delete_me)
          {
             if ((!e->is_frozen) &&
                 (!evas_event_freezes_through(eo_obj, obj)))
               {
                  ev->cur.x = e->pointer.x;
                  ev->cur.y = e->pointer.y;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_UP, evt,
                                                  event_id, EFL_EVENT_POINTER_UP);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_up_events(eo_obj, eo_e, evt, event_id);
                  if (e->delete_me) break;
               }
          }
        if (obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
          {
             if (e->pointer.nogrep > 0) e->pointer.nogrep--;
             break;
          }
     }
   eina_list_free(copy);
   e->last_mouse_up_counter++;
   _evas_post_event_callback_call(eo_e, e);

   if (e->pointer.mouse_grabbed == 0)
     _post_up_handle(e, evt);

   if (e->pointer.mouse_grabbed < 0)
     {
        ERR("BUG? e->pointer.mouse_grabbed (=%d) < 0!",
            e->pointer.mouse_grabbed);
     }
   /* remove released touch point from the touch point list */
   _evas_touch_point_remove(eo_e, 0);
   _evas_unwalk(e);

   if (ev->device) efl_unref(ev->device);
}

static void
_canvas_event_feed_mouse_updown_legacy(Eo *eo_e, int b, Evas_Button_Flags flags,
                                       unsigned int timestamp, const void *data,
                                       Eina_Bool down)
{
   Efl_Input_Pointer_Data *ev = NULL;
   Efl_Input_Pointer *evt;
   Evas_Public_Data *e;

   e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   if (!e) return;

   evt = efl_input_instance_get(EFL_INPUT_POINTER_CLASS, eo_e, (void **) &ev);
   if (!ev) return;

   ev->data = (void *) data;
   ev->timestamp = timestamp;
   ev->device = _evas_device_top_get(eo_e);
   ev->action = down ? EFL_POINTER_ACTION_DOWN : EFL_POINTER_ACTION_UP;
   ev->button = b;
   ev->button_flags = flags;
   ev->device = _evas_device_top_get(e->evas);
   ev->radius = 1;
   ev->radius_x = 1;
   ev->radius_y = 1;
   ev->pressure = 1;
   ev->angle = 0;
   //ev->window_pos = ?;
   //ev->fake = 1;

   if (down)
     _canvas_event_feed_mouse_down_internal(e, ev);
   else
     _canvas_event_feed_mouse_up_internal(e, ev);

   efl_del(evt);
}

EAPI void
evas_event_feed_mouse_down(Eo *eo_e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   _canvas_event_feed_mouse_updown_legacy(eo_e, b, flags, timestamp, data, 1);
}

EAPI void
evas_event_feed_mouse_up(Eo *eo_e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   _canvas_event_feed_mouse_updown_legacy(eo_e, b, flags, timestamp, data, 0);
}

static void
_canvas_event_feed_mouse_cancel_internal(Evas_Public_Data *e, Efl_Input_Pointer_Data *ev)
{
   Evas_Coord_Touch_Point *point;
   Efl_Input_Pointer_Data save;
   Eina_List *l, *ll;
   Evas_Event_Flags flags;
   Evas *eo_e;
   int i;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL);

   if (!e || !ev) return;
   if (e->is_frozen) return;

   save = *ev;
   eo_e = e->evas;
   _evas_walk(e);

   flags = evas_event_default_flags_get(eo_e);
   evas_event_default_flags_set(eo_e, (flags | EVAS_EVENT_FLAG_ON_HOLD));

   for (i = 0; i < 32; i++)
     {
        if ((e->pointer.button & (1u << i)))
          _canvas_event_feed_mouse_updown_legacy(eo_e, i + 1, 0, ev->timestamp, ev->data, 0);
     }

   ev->action = EFL_POINTER_ACTION_CANCEL;
   ev->value_flags |= value_flags;
   ev->event_flags = flags;
   EINA_LIST_FOREACH_SAFE(e->touch_points, l, ll, point)
     {
        if ((point->state == EVAS_TOUCH_POINT_DOWN) ||
            (point->state == EVAS_TOUCH_POINT_MOVE))
          {
             ev->tool = point->id;
             ev->cur.x = point->x;
             ev->cur.y = point->y;
             _canvas_event_feed_multi_up_internal(e, ev);
          }
     }
   evas_event_default_flags_set(eo_e, flags);
   _evas_unwalk(e);

   *ev = save;
}

EAPI void
evas_event_feed_mouse_cancel(Eo *eo_e, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Efl_Input_Pointer_Data *ev = NULL;
   Efl_Input_Pointer *evt;

   evt = efl_input_instance_get(EFL_INPUT_POINTER_CLASS, eo_e, (void **) &ev);
   if (!ev) return;

   ev->timestamp = timestamp;
   ev->data = (void *) data;
   ev->device = _evas_device_top_get(e->evas);

   _canvas_event_feed_mouse_cancel_internal(e, ev);

   efl_del(evt);
}

static void
_canvas_event_feed_mouse_wheel_internal(Eo *eo_e, Efl_Input_Pointer_Data *pe)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Efl_Input_Pointer_Data *ev = NULL;
   Efl_Input_Pointer *evt;
   Eina_List *l, *copy;
   Evas_Object *eo_obj;
   int event_id = 0;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_WHEEL_DELTA) |
         _efl_input_value_mask(EFL_INPUT_VALUE_WHEEL_DIRECTION);

   if (e->is_frozen) return;
   e->last_timestamp = pe->timestamp;

   event_id = _evas_object_event_new();

   evt = efl_input_dup(pe->eo);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   // adjust missing data based on evas state
   ev->cur.x = e->pointer.x;
   ev->cur.y = e->pointer.y;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->device = efl_ref(_evas_device_top_get(eo_e));
   ev->action = EFL_POINTER_ACTION_WHEEL;
   ev->value_flags |= value_flags;

   _evas_walk(e);
   copy = evas_event_list_copy(e->pointer.object.in);

   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if (!evas_event_freezes_through(eo_obj, obj))
          {
             ev->cur.x = e->pointer.x;
             ev->cur.y = e->pointer.y;
             _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
             evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_WHEEL, evt,
                                             event_id, EFL_EVENT_POINTER_WHEEL);
             if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
               _evas_event_source_wheel_events(eo_obj, eo_e, evt, event_id);
             if (e->delete_me || e->is_frozen) break;
          }
     }
   eina_list_free(copy);
   _evas_post_event_callback_call(eo_e, e);

   efl_unref(ev->device);
   efl_del(evt);
   _evas_unwalk(e);
}

EAPI void
evas_event_feed_mouse_wheel(Eo *eo_e, int direction, int z, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Efl_Input_Pointer_Data *ev = NULL;
   Efl_Input_Pointer *evt = efl_input_instance_get(EFL_INPUT_POINTER_CLASS, eo_e, (void **) &ev);

   if (!ev) return;

   ev->wheel.dir = direction ? EFL_ORIENT_HORIZONTAL : EFL_ORIENT_VERTICAL;
   ev->wheel.z = z;
   ev->timestamp = timestamp;
   ev->data = (void *) data;

   _canvas_event_feed_mouse_wheel_internal(eo_e, ev);
   efl_del(evt);
}

static void
_canvas_event_feed_mouse_move_internal(Evas_Public_Data *e, Efl_Input_Pointer_Data *ev)
{
   Evas_Object *nogrep_obj = NULL;
   Evas_Object_Protected_Data *obj;
   Efl_Input_Pointer *evt;
   Eina_List *l, *copy;
   Evas_Object *eo_obj;
   Eina_Vector2 point;
   Evas *eo_e;
   int event_id;
   int x, y, px, py;

   // inform which values are valid
   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_PREVIOUS_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_PREVIOUS_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_DX) |
         _efl_input_value_mask(EFL_INPUT_VALUE_DY) |
         _efl_input_value_mask(EFL_INPUT_VALUE_BUTTONS_PRESSED) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL);

   if (!e || !ev) return;
   if (e->is_frozen) return;

   eo_e = e->evas;
   e->last_timestamp = ev->timestamp;

   // prev pos
   px = ev->prev.x = e->pointer.x;
   py = ev->prev.y = e->pointer.y;

   // new pos
   x = e->pointer.x = ev->cur.x;
   y = e->pointer.y = ev->cur.y;
   point = ev->cur;

   if ((!e->pointer.inside) && (e->pointer.mouse_grabbed == 0)) return;

   evt = ev->eo;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->pressed_buttons = e->pointer.button;
   ev->tool = 0;
   ev->value_flags |= value_flags;
   if (ev->device) efl_ref(ev->device);

   _evas_walk(e);
   /* update moved touch point */
   if ((px != x) || (py != y))
     _evas_touch_point_update(eo_e, 0, e->pointer.x, e->pointer.y, EVAS_TOUCH_POINT_MOVE);
   /* if our mouse button is grabbed to any objects */
   if (e->pointer.mouse_grabbed > 0)
     {
        Eina_List *outs = NULL;

        /* Send normal mouse move events */
        ev->action = EFL_POINTER_ACTION_MOVE;

        event_id = _evas_object_event_new();

        /* go thru old list of in objects */
        copy = evas_event_list_copy(e->pointer.object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             if ((!e->is_frozen) &&
                 (evas_object_clippers_is_visible(eo_obj, obj) ||
                  obj->mouse_grabbed) &&
                 (!evas_event_passes_through(eo_obj, obj)) &&
                 (!evas_event_freezes_through(eo_obj, obj)) &&
                 (!evas_object_is_source_invisible(eo_obj, obj) ||
                  obj->mouse_grabbed) &&
                 (!obj->clip.clipees))
               {
                  if ((px != x) || (py != y))
                    {
                       ev->cur = point;
                       _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
                       evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                       event_id, EFL_EVENT_POINTER_MOVE);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_move_events(eo_obj, eo_e, evt, event_id);
                    }
               }
             else
                outs = eina_list_append(outs, eo_obj);
             if ((obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN) &&
                 (e->pointer.nogrep > 0))
               {
                  eina_list_free(copy);
                  eina_list_free(outs);
                  nogrep_obj = eo_obj;
                  goto nogrep;
               }
             if (e->delete_me) break;
          }
        eina_list_free(copy);
        _evas_post_event_callback_call(eo_e, e);


        /* Send mouse out events */
        ev->action = EFL_POINTER_ACTION_OUT;

        event_id = _evas_object_event_new();

        EINA_LIST_FREE(outs, eo_obj)
          {
             obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             if ((obj->mouse_grabbed == 0) && (!e->delete_me))
               {
                  if (!obj->mouse_in) continue;
                  obj->mouse_in = 0;
                  if (obj->delete_me || e->is_frozen) continue;
                  e->pointer.object.in = eina_list_remove(e->pointer.object.in, eo_obj);
                  ev->cur = point;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, evt,
                                                  event_id, EFL_EVENT_POINTER_OUT);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_out_events(eo_obj, eo_e, evt, event_id);
               }
          }
        _evas_post_event_callback_call(eo_e, e);
     }
   else
     {
        Eina_List *ins;

        event_id = _evas_object_event_new();

        /* get all new in objects */
        ins = evas_event_objects_event_list(eo_e, NULL, x, y);
        /* go thru old list of in objects */
        copy = evas_event_list_copy(e->pointer.object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             if (!obj) continue;

             /* if its under the pointer and its visible and its in the new */
             /* in list */
             // FIXME: i don't think we need this
             //	     evas_object_clip_recalc(eo_obj);
             if ((!e->is_frozen) &&
                 evas_object_is_in_output_rect(eo_obj, obj, x, y, 1, 1) &&
                 (evas_object_clippers_is_visible(eo_obj, obj) ||
                  obj->mouse_grabbed) &&
                 eina_list_data_find(ins, eo_obj) &&
                 (!evas_event_passes_through(eo_obj, obj)) &&
                 (!evas_event_freezes_through(eo_obj, obj)) &&
                 (!evas_object_is_source_invisible(eo_obj, obj) ||
                  obj->mouse_grabbed) &&
                 (!obj->clip.clipees) &&
                 ((!obj->precise_is_inside) || evas_object_is_inside(eo_obj, obj, x, y))
                )
               {
                  if ((px != x) || (py != y))
                    {
                       ev->cur = point;
                       _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
                       ev->action = EFL_POINTER_ACTION_MOVE;
                       evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                       event_id, EFL_EVENT_POINTER_MOVE);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_move_events(eo_obj, eo_e, evt, event_id);
                    }
               }
             /* otherwise it has left the object */
             else if (obj->mouse_in)
               {
                  obj->mouse_in = 0;
                  if (e->is_frozen) continue;
                  ev->cur = point;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
                  ev->action = EFL_POINTER_ACTION_OUT;
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, evt,
                                                  event_id, EFL_EVENT_POINTER_OUT);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_out_events(eo_obj, eo_e, evt, event_id);
                  if (e->delete_me) break;
               }
          }
        eina_list_free(copy);
        _evas_post_event_callback_call(eo_e, e);

        /* new event id for mouse in */
        event_id = _evas_object_event_new();

        /* go thru our current list of ins */
        EINA_LIST_FOREACH(ins, l, eo_obj)
          {
             obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             /* if its not in the old list of ins send an enter event */
             if (!eina_list_data_find(e->pointer.object.in, eo_obj))
               {
                  if (!obj->mouse_in)
                    {
                       obj->mouse_in = 1;
                       if (e->is_frozen) continue;
                       ev->cur = point;
                       _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
                       ev->action = EFL_POINTER_ACTION_IN;
                       evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_IN, evt,
                                                       event_id, EFL_EVENT_POINTER_IN);
                       if ((obj->proxy->is_proxy) &&
                           (obj->proxy->src_events))
                         _evas_event_source_mouse_in_events(eo_obj, eo_e, evt, event_id);
                       if (e->delete_me) break;
                    }
               }
          }
        if (e->pointer.mouse_grabbed == 0)
          {
             /* free our old list of ins */
             eina_list_free(e->pointer.object.in);
             /* and set up the new one */
             e->pointer.object.in = ins;
          }
        else
          {
             /* free our cur ins */
             eina_list_free(ins);
          }
        _evas_post_event_callback_call(eo_e, e);
     }

nogrep:
   if (nogrep_obj)
     {
        Eina_List *ins = NULL, *newin = NULL, *lst = NULL;
        Evas_Object *eo_below_obj;

        event_id = _evas_object_event_new();

        /* go thru old list of in objects */
        copy = evas_event_list_copy(e->pointer.object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             if (eo_obj == nogrep_obj)
               {
                  lst = l->next;
                  break;
               }
          }

        /* get all new in objects */
        eo_below_obj = evas_object_below_get(nogrep_obj);
        if (eo_below_obj)
          {
             Evas_Object_Protected_Data *below_obj = efl_data_scope_get(eo_below_obj, EFL_CANVAS_OBJECT_CLASS);
             int norep = 0;
             ins = _evas_event_object_list_raw_in_get(eo_e, NULL,
                                                   EINA_INLIST_GET(below_obj), NULL,
                                                   e->pointer.x, e->pointer.y,
                                                   &norep, EINA_FALSE);
          }

        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             newin = eina_list_append(newin, eo_obj);
             if (eo_obj == nogrep_obj) break;
          }

        // NOTE: was foreach + append without free (smelled bad)
        newin = eina_list_merge(newin, ins);

        EINA_LIST_FOREACH(lst, l, eo_obj)
          {
             obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             /* if its under the pointer and its visible and its in the new */
             /* in list */
             // FIXME: i don't think we need this
             //	     evas_object_clip_recalc(eo_obj);
             if ((!e->is_frozen) &&
                 evas_object_is_in_output_rect(eo_obj, obj, x, y, 1, 1) &&
                 (evas_object_clippers_is_visible(eo_obj, obj) ||
                  obj->mouse_grabbed) &&
                 eina_list_data_find(newin, eo_obj) &&
                 (!evas_event_passes_through(eo_obj, obj)) &&
                 (!evas_event_freezes_through(eo_obj, obj)) &&
                 (!evas_object_is_source_invisible(eo_obj, obj) ||
                  obj->mouse_grabbed) &&
                 (!obj->clip.clipees) &&
                 ((!obj->precise_is_inside) || evas_object_is_inside(eo_obj, obj, x, y))
                )
               {
                  if ((px != x) || (py != y))
                    {
                       ev->cur = point;
                       _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
                       ev->action = EFL_POINTER_ACTION_MOVE;
                       evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                       event_id, EFL_EVENT_POINTER_MOVE);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_move_events(eo_obj, eo_e, evt, event_id);
                    }
               }
             else
               {
                  /* otherwise it has left the object */
                  if (!obj->mouse_in) continue;
                  obj->mouse_in = 0;
                  if (e->is_frozen) continue;
                  ev->cur = point;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
                  ev->action = EFL_POINTER_ACTION_OUT;
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, evt,
                                                  event_id, EFL_EVENT_POINTER_OUT);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_out_events(eo_obj, eo_e, evt, event_id);
               }
             if (e->delete_me) break;
          }
        eina_list_free(copy);
        _evas_post_event_callback_call(eo_e, e);

        event_id = _evas_object_event_new();

        /* go thru our current list of ins */
        EINA_LIST_FOREACH(newin, l, eo_obj)
          {
             obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             /* if its not in the old list of ins send an enter event */
             if (!eina_list_data_find(e->pointer.object.in, eo_obj))
               {
                  if (obj->mouse_in) continue;
                  obj->mouse_in = 1;
                  if (e->is_frozen) continue;
                  ev->cur = point;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
                  ev->action = EFL_POINTER_ACTION_IN;
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_IN, evt,
                                                  event_id, EFL_EVENT_POINTER_IN);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_in_events(eo_obj, eo_e, evt, event_id);
                  if (e->delete_me) break;
               }
          }
        /* free our old list of ins */
        eina_list_free(e->pointer.object.in);
        /* and set up the new one */
        e->pointer.object.in = newin;

        _evas_post_event_callback_call(eo_e, e);
     }

   _evas_unwalk(e);
   if (ev->device) efl_unref(ev->device);
}

static void
_canvas_event_feed_mouse_move_legacy(Evas *eo_e, Evas_Public_Data *e, int x, int y,
                                     unsigned int timestamp, const void *data)
{
   Efl_Input_Pointer_Data *ev = NULL;
   Efl_Input_Pointer *evt;

   evt = efl_input_instance_get(EFL_INPUT_POINTER_CLASS, eo_e, (void **) &ev);
   if (!ev) return;

   ev->data = (void *) data;
   ev->timestamp = timestamp;
   ev->device = _evas_device_top_get(eo_e);
   ev->cur.x = x;
   ev->cur.y = y;

   _canvas_event_feed_mouse_move_internal(e, ev);

   efl_del(evt);
}

EAPI void
evas_event_input_mouse_move(Eo *eo_e, int x, int y, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_mouse_move_legacy(eo_e, e, x - e->framespace.x, y - e->framespace.y, timestamp, data);
}

EAPI void
evas_event_feed_mouse_move(Eo *eo_e, int x, int y, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_mouse_move_legacy(eo_e, e, x, y, timestamp, data);
}

static void
_canvas_event_feed_mouse_in_internal(Evas *eo_e, Efl_Input_Pointer_Data *ev)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Efl_Input_Pointer *evt;
   Eina_List *ins;
   Eina_List *l;
   Evas_Object *eo_obj;
   int event_id;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_BUTTONS_PRESSED) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL);

   if (!e || !ev) return;

   e->pointer.inside = 1;
   if (e->is_frozen) return;

   e->last_timestamp = ev->timestamp;
   if (e->pointer.mouse_grabbed != 0) return;

   evt = ev->eo;
   ev->action = EFL_POINTER_ACTION_IN;
   ev->pressed_buttons = e->pointer.button;
   ev->cur.x = e->pointer.x;
   ev->cur.y = e->pointer.y;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->value_flags |= value_flags;
   if (ev->device) efl_ref(ev->device);

   event_id = _evas_object_event_new();

   _evas_walk(e);
   /* get new list of ins */
   ins = evas_event_objects_event_list(eo_e, NULL, e->pointer.x, e->pointer.y);
   EINA_LIST_FOREACH(ins, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if (!eina_list_data_find(e->pointer.object.in, eo_obj))
          {
             if (obj->mouse_in) continue;
             obj->mouse_in = 1;
             ev->cur.x = e->pointer.x;
             ev->cur.y = e->pointer.y;
             _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
             evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_IN, evt,
                                             event_id, EFL_EVENT_POINTER_IN);
             if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
               _evas_event_source_mouse_in_events(eo_obj, eo_e, evt, event_id);
             if (e->delete_me || e->is_frozen) break;
          }
     }
   /* free our old list of ins */
   e->pointer.object.in = eina_list_free(e->pointer.object.in);
   /* and set up the new one */
   e->pointer.object.in = ins;
   _evas_post_event_callback_call(eo_e, e);
   _canvas_event_feed_mouse_move_internal(e, ev);
   _evas_unwalk(e);

   if (ev->device) efl_unref(ev->device);
}

static void
_canvas_event_feed_mouse_out_internal(Evas *eo_e, Efl_Input_Pointer_Data *ev)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Efl_Input_Pointer *evt;
   Eina_List *l, *copy;
   Evas_Object *eo_obj;
   int event_id;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_BUTTONS_PRESSED) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL);

   if (!e || !ev) return;
   e->pointer.inside = 0;

   if (e->is_frozen) return;
   e->last_timestamp = ev->timestamp;

   event_id = _evas_object_event_new();

   evt = ev->eo;
   ev->action = EFL_POINTER_ACTION_OUT;
   ev->pressed_buttons = e->pointer.button;
   ev->cur.x = e->pointer.x;
   ev->cur.y = e->pointer.y;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->value_flags |= value_flags;
   if (ev->device) efl_ref(ev->device);

   _evas_walk(e);
   /* if our mouse button is inside any objects */
   /* go thru old list of in objects */
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if (!obj->mouse_in) continue;
        obj->mouse_in = 0;
        if (!obj->delete_me)
          {
             ev->cur.x = e->pointer.x;
             ev->cur.y = e->pointer.y;
             _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
             evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, evt,
                                             event_id, EFL_EVENT_POINTER_OUT);
             if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
               _evas_event_source_mouse_out_events(eo_obj, eo_e, evt, event_id);
             if (e->delete_me || e->is_frozen) break;
          }
        obj->mouse_grabbed = 0;
     }
   eina_list_free(copy);

   /* free our old list of ins */
   e->pointer.object.in =  eina_list_free(e->pointer.object.in);
   e->pointer.mouse_grabbed = 0;
   _evas_post_event_callback_call(eo_e, e);
   _evas_unwalk(e);

   if (ev->device) efl_unref(ev->device);
}

static void
_canvas_event_feed_mouse_inout_legacy(Eo *eo_e, unsigned int timestamp,
                                      const void *data, Eina_Bool in)
{
   Efl_Input_Pointer_Data *ev = NULL;
   Efl_Input_Pointer *evt;

   evt = efl_input_instance_get(EFL_INPUT_POINTER_CLASS, eo_e, (void **) &ev);
   if (!ev) return;

   ev->timestamp = timestamp;
   ev->data = (void *) data;
   ev->device = _evas_device_top_get(eo_e);

   if (in)
     _canvas_event_feed_mouse_in_internal(eo_e, ev);
   else
     _canvas_event_feed_mouse_out_internal(eo_e, ev);

   efl_del(evt);
}

EAPI void
evas_event_feed_mouse_in(Eo *eo_e, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   _canvas_event_feed_mouse_inout_legacy(eo_e, timestamp, data, EINA_TRUE);
}

EAPI void
evas_event_feed_mouse_out(Eo *eo_e, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   _canvas_event_feed_mouse_inout_legacy(eo_e, timestamp, data, EINA_FALSE);
}

static void
_canvas_event_feed_multi_down_internal(Evas_Public_Data *e, Efl_Input_Pointer_Data *ev)
{
   Efl_Input_Pointer *evt;
   Eina_List *l, *copy;
   Evas_Object *eo_obj;
   Eina_Vector2 point;
   Evas *eo_e;
   int event_id;
   int addgrab = 0;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL) |
         _efl_input_value_mask(EFL_INPUT_VALUE_BUTTON);

   if (!e || !ev) return;

   eo_e = e->evas;
   DBG("ButtonEvent:multi down time=%u x=%.1f y=%.1f button=%d downs=%d",
       ev->timestamp, ev->cur.x, ev->cur.y, ev->tool, e->pointer.downs);
   e->pointer.downs++;
   if (e->is_frozen) return;
   e->last_timestamp = ev->timestamp;

   event_id = _evas_object_event_new();

   point = ev->cur;
   evt = ev->eo;
   ev->action = EFL_POINTER_ACTION_DOWN;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->value_flags |= value_flags;
   if (ev->device) efl_ref(ev->device);

   _evas_walk(e);
   /* append new touch point to the touch point list */
   _evas_touch_point_append(eo_e, ev->tool, ev->cur.x, ev->cur.y);
   if (e->pointer.mouse_grabbed == 0)
     {
        if (e->pointer.downs > 1) addgrab = e->pointer.downs - 1;
     }
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if (obj->pointer_mode != EVAS_OBJECT_POINTER_MODE_NOGRAB)
          {
             obj->mouse_grabbed += addgrab + 1;
             e->pointer.mouse_grabbed += addgrab + 1;
          }
     }
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MULTI_DOWN, evt,
                                        event_id, EFL_EVENT_FINGER_DOWN);
        if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
          _evas_event_source_multi_down_events(obj, e, evt, event_id);
        if (e->delete_me || e->is_frozen) break;
     }
   eina_list_free(copy);

   _evas_post_event_callback_call(eo_e, e);
   /* update touch point's state to EVAS_TOUCH_POINT_STILL */
   _evas_touch_point_update(eo_e, ev->tool, ev->cur.x, ev->cur.y, EVAS_TOUCH_POINT_STILL);
   _evas_unwalk(e);

   if (ev->device) efl_unref(ev->device);
}

static void
_canvas_event_feed_multi_up_internal(Evas_Public_Data *e, Efl_Input_Pointer_Data *ev)
{
   Efl_Input_Pointer *evt;
   Eina_List *l, *copy;
   Evas_Object *eo_obj;
   Eina_Vector2 point;
   Evas *eo_e;
   int event_id;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL);

   if (!e || !ev) return;

   eo_e = e->evas;
   DBG("ButtonEvent:multi up time=%u x=%.1f y=%.1f device=%d downs=%d",
       ev->timestamp, ev->cur.x, ev->cur.y, ev->tool, e->pointer.downs);
   if (e->pointer.downs <= 0) return;
   e->pointer.downs--;
   if (e->is_frozen) return;
   e->last_timestamp = ev->timestamp;

   event_id = _evas_object_event_new();

   point = ev->cur;
   evt = ev->eo;
   ev->action = EFL_POINTER_ACTION_UP;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->value_flags |= value_flags;
   if (ev->device) efl_ref(ev->device);

   _evas_walk(e);
   /* update released touch point */
   _evas_touch_point_update(eo_e, ev->tool, ev->cur.x, ev->cur.y, EVAS_TOUCH_POINT_UP);
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
        if (((obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) ||
            (obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)) &&
            (obj->mouse_grabbed > 0))
          {
             obj->mouse_grabbed--;
             e->pointer.mouse_grabbed--;
          }
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MULTI_UP, evt,
                                        event_id, EFL_EVENT_FINGER_UP);
        if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
          _evas_event_source_multi_up_events(obj, e, evt, event_id);
        if (e->delete_me || e->is_frozen) break;
     }
   eina_list_free(copy);
   if ((e->pointer.mouse_grabbed == 0) && !_post_up_handle(e, evt))
      _evas_post_event_callback_call(eo_e, e);
   /* remove released touch point from the touch point list */
   _evas_touch_point_remove(eo_e, ev->tool);
   _evas_unwalk(e);

   if (ev->device) efl_unref(ev->device);
}

static inline void
_canvas_event_feed_multi_internal(Evas *eo_e, Evas_Public_Data *e,
                                  int d, int x, int y,
                                  double rad, double radx, double rady,
                                  double pres, double ang,
                                  double fx, double fy,
                                  Evas_Button_Flags flags,
                                  unsigned int timestamp,
                                  const void *data, Efl_Pointer_Action action)
{
   Efl_Input_Pointer_Data *ev = NULL;
   Efl_Input_Pointer *evt;

   evt = efl_input_instance_get(EFL_INPUT_POINTER_CLASS, eo_e, (void **) &ev);
   if (!e || !ev) return;

   if (!fx) fx = x;
   if (!fy) fy = y;

   ev->action = action;
   ev->tool = d;
   ev->cur.x = fx;
   ev->cur.y = fy;
   ev->pressure = pres;
   ev->angle = ang;
   ev->radius = rad;
   ev->radius_x = radx;
   ev->radius_y = rady;
   ev->button_flags = flags;
   ev->timestamp = timestamp;
   ev->data = (void *) data;
   ev->device = _evas_device_top_get(eo_e);

   switch (action)
     {
      case EFL_POINTER_ACTION_DOWN:
        _canvas_event_feed_multi_down_internal(e, ev);
        break;

      case EFL_POINTER_ACTION_UP:
        _canvas_event_feed_multi_up_internal(e, ev);
        break;

      case EFL_POINTER_ACTION_MOVE:
        _canvas_event_feed_multi_move_internal(e, ev);
        break;

      default: break;
     }

   efl_del(evt);
}

EAPI void
evas_event_input_multi_down(Eo *eo_e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_multi_internal(eo_e, e, d,
                                     x - e->framespace.x, y - e->framespace.y,
                                     rad, radx, rady, pres, ang,
                                     fx, fy, flags, timestamp, data, EFL_POINTER_ACTION_DOWN);
}

EAPI void
evas_event_feed_multi_down(Eo *eo_e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_multi_internal(eo_e, e, d, x, y, rad, radx, rady, pres, ang,
                                     fx, fy, flags, timestamp, data, EFL_POINTER_ACTION_DOWN);
}

EAPI void
evas_event_input_multi_up(Eo *eo_e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_multi_internal(eo_e, e, d,
                                     x - e->framespace.x, y - e->framespace.y,
                                     rad, radx, rady, pres, ang,
                                     fx, fy, flags, timestamp, data, EFL_POINTER_ACTION_UP);
}

EAPI void
evas_event_feed_multi_up(Eo *eo_e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_multi_internal(eo_e, e, d, x, y, rad, radx, rady, pres, ang,
                                     fx, fy, flags, timestamp, data, EFL_POINTER_ACTION_UP);
}

static void
_canvas_event_feed_multi_move_internal(Evas_Public_Data *e, Efl_Input_Pointer_Data *ev)
{
   Efl_Input_Pointer *evt;
   Eina_List *l, *copy;
   Evas_Object *eo_obj;
   Eina_Vector2 point;
   Evas *eo_e;
   int event_id;

   /* FIXME: Add previous x,y information (from evas touch point list) */
   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL);

   if (!e || !ev) return;

   eo_e = e->evas;
   event_id = _evas_object_event_new();

   if (e->is_frozen) return;
   e->last_timestamp = ev->timestamp;

   if ((!e->pointer.inside) && (e->pointer.mouse_grabbed == 0)) return;

   evt = ev->eo;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->action = EFL_POINTER_ACTION_MOVE;
   ev->value_flags |= value_flags;
   if (ev->device) efl_ref(ev->device);

   point = ev->cur;

   _evas_walk(e);
   /* update moved touch point */
   _evas_touch_point_update(eo_e, ev->tool, ev->cur.x, ev->cur.y, EVAS_TOUCH_POINT_MOVE);
   /* if our mouse button is grabbed to any objects */
   if (e->pointer.mouse_grabbed > 0)
     {
        /* go thru old list of in objects */
        copy = evas_event_list_copy(e->pointer.object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             if ((evas_object_clippers_is_visible(eo_obj, obj) ||
                  obj->mouse_grabbed) &&
                 (!evas_event_passes_through(eo_obj, obj)) &&
                 (!evas_event_freezes_through(eo_obj, obj)) &&
                 (!evas_object_is_source_invisible(eo_obj, obj) ||
                  obj->mouse_grabbed) &&
                 (!obj->clip.clipees))
               {
                  ev->cur = point;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MULTI_MOVE, evt,
                                                  event_id, EFL_EVENT_FINGER_MOVE);
                  if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
                    _evas_event_source_multi_move_events(obj, e, evt, event_id);

                  if (e->delete_me || e->is_frozen) break;
               }
          }
        eina_list_free(copy);
        _evas_post_event_callback_call(eo_e, e);
     }
   else
     {
        Eina_List *ins;

        /* get all new in objects */
        ins = evas_event_objects_event_list(eo_e, NULL, ev->cur.x, ev->cur.y);
        /* go thru old list of in objects */
        copy = evas_event_list_copy(e->pointer.object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             /* if its under the pointer and its visible and its in the new */
             /* in list */
             // FIXME: i don't think we need this
             //	     evas_object_clip_recalc(eo_obj);
             if (evas_object_is_in_output_rect(eo_obj, obj, ev->cur.x, ev->cur.y, 1, 1) &&
                 (evas_object_clippers_is_visible(eo_obj, obj) ||
                  obj->mouse_grabbed) &&
                 eina_list_data_find(ins, eo_obj) &&
                 (!evas_event_passes_through(eo_obj, obj)) &&
                 (!evas_event_freezes_through(eo_obj, obj)) &&
                 (!evas_object_is_source_invisible(eo_obj, obj) ||
                  obj->mouse_grabbed) &&
                 (!obj->clip.clipees) &&
                 ((!obj->precise_is_inside) || evas_object_is_inside(eo_obj, obj, ev->cur.x, ev->cur.y))
                )
               {
                  ev->cur = point;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj->mouse_grabbed);
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MULTI_MOVE, evt,
                                                  event_id, EFL_EVENT_FINGER_MOVE);
                  if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
                    _evas_event_source_multi_move_events(obj, e, evt, event_id);
               }
             if (e->delete_me || e->is_frozen) break;
          }
        eina_list_free(copy);
        if (e->pointer.mouse_grabbed == 0)
          {
             /* free our old list of ins */
             eina_list_free(e->pointer.object.in);
             /* and set up the new one */
             e->pointer.object.in = ins;
          }
        else
          {
             /* free our cur ins */
             eina_list_free(ins);
          }
        _evas_post_event_callback_call(eo_e, e);
     }
   _evas_unwalk(e);

   if (ev->device) efl_unref(ev->device);
}

EAPI void
evas_event_input_multi_move(Eo *eo_e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_multi_internal(eo_e, e, d, x - e->framespace.x, y - e->framespace.y,
                                     rad, radx, rady,
                                     pres, ang, fx, fy, EVAS_BUTTON_NONE, timestamp, data,
                                     EFL_POINTER_ACTION_MOVE);
}

EAPI void
evas_event_feed_multi_move(Eo *eo_e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_multi_internal(eo_e, e, d, x, y, rad, radx, rady,
                                     pres, ang, fx, fy, EVAS_BUTTON_NONE, timestamp, data,
                                     EFL_POINTER_ACTION_MOVE);
}

static void
_canvas_event_feed_key_down_internal(Evas_Public_Data *e, Efl_Input_Key_Data *ev)
{
   Eina_Bool exclusive = EINA_FALSE;
   Efl_Input_Pointer *evt;
   int event_id = 0;

   if (!e || !ev) return;
   if (e->is_frozen) return;
   e->last_timestamp = ev->timestamp;
   _evas_walk(e);

   event_id = _evas_object_event_new();

   evt = ev->eo;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   if (ev->device) efl_ref(ev->device);

   if (e->grabs)
     {
        Eina_List *l;
        Evas_Key_Grab *g;

        e->walking_grabs++;
        EINA_LIST_FOREACH(e->grabs, l, g)
          {
             if (g->just_added)
               {
                  g->just_added = EINA_FALSE;
                  continue;
               }
             if (g->delete_me) continue;
             if (!g->object) continue;
             if (!g->is_active) continue;
             if (((e->modifiers.mask & g->modifiers) ||
                  (g->modifiers == e->modifiers.mask)) &&
                 (!strcmp(ev->keyname, g->keyname)))
               {
                  if (!(e->modifiers.mask & g->not_modifiers))
                    {
                       Evas_Object_Protected_Data *object_obj = efl_data_scope_get(g->object, EFL_CANVAS_OBJECT_CLASS);
                       if (!e->is_frozen &&
                           !evas_event_freezes_through(g->object, object_obj))
                         {
                            evas_object_event_callback_call(g->object, object_obj, EVAS_CALLBACK_KEY_DOWN, evt,
                                                            event_id, EFL_EVENT_KEY_DOWN);
                         }
                       if (g->exclusive) exclusive = EINA_TRUE;
                    }
               }
             if (e->delete_me) break;
          }
        e->walking_grabs--;
        if (e->walking_grabs <= 0)
          {
             while (e->delete_grabs > 0)
               {
                  e->delete_grabs--;
                  for (l = e->grabs; l;)
                    {
                       g = eina_list_data_get(l);
                       l = eina_list_next(l);
                       if (g->delete_me)
                         {
                            Evas_Object_Protected_Data *g_object_obj = efl_data_scope_get(g->object, EFL_CANVAS_OBJECT_CLASS);
                            evas_key_grab_free(g->object, g_object_obj, g->keyname,
                                               g->modifiers, g->not_modifiers);
                         }
                    }
               }
          }
     }
   if ((e->focused) && (!exclusive))
     {
        Evas_Object_Protected_Data *focused_obj = efl_data_scope_get(e->focused, EFL_CANVAS_OBJECT_CLASS);
        if (!e->is_frozen && !evas_event_freezes_through(e->focused, focused_obj))
          {
             evas_object_event_callback_call(e->focused, focused_obj, EVAS_CALLBACK_KEY_DOWN, evt,
                                             event_id, EFL_EVENT_KEY_DOWN);
          }
     }
   _evas_post_event_callback_call(e->evas, e);
   _evas_unwalk(e);

   if (ev->device) efl_unref(ev->device);
}

static void
_canvas_event_feed_key_up_internal(Evas_Public_Data *e, Efl_Input_Key_Data *ev)
{
   Eina_Bool exclusive = EINA_FALSE;
   Efl_Input_Pointer *evt;
   int event_id = 0;

   if (!e || !ev) return;
   if (e->is_frozen) return;
   e->last_timestamp = ev->timestamp;
   _evas_walk(e);

   event_id = _evas_object_event_new();

   evt = ev->eo;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   if (ev->device) efl_ref(ev->device);

   if (e->grabs)
     {
        Eina_List *l;
        Evas_Key_Grab *g;

        e->walking_grabs++;
        EINA_LIST_FOREACH(e->grabs, l, g)
          {
             if (g->just_added)
               {
                  g->just_added = EINA_FALSE;
                  continue;
               }
             if (g->delete_me) continue;
             if (!g->object) continue;
             if (!g->is_active) continue;
             if (((e->modifiers.mask & g->modifiers) ||
                  (g->modifiers == e->modifiers.mask)) &&
                 (!(e->modifiers.mask & g->not_modifiers)) &&
                 (!strcmp(ev->keyname, g->keyname)))
               {
                  Evas_Object_Protected_Data *object_obj = efl_data_scope_get(g->object, EFL_CANVAS_OBJECT_CLASS);
                  if (!e->is_frozen &&
                        !evas_event_freezes_through(g->object, object_obj))
                    {
                       evas_object_event_callback_call
                             (g->object, object_obj, EVAS_CALLBACK_KEY_UP, evt,
                              event_id, EFL_EVENT_KEY_UP);
                    }
                  if (g->exclusive) exclusive = EINA_TRUE;
               }
             if (e->delete_me) break;
          }
        e->walking_grabs--;
        if (e->walking_grabs <= 0)
          {
             while (e->delete_grabs > 0)
               {
                  Eina_List *ll, *l_next;
                  Evas_Key_Grab *gr;

                  e->delete_grabs--;
                  EINA_LIST_FOREACH_SAFE(e->grabs, ll, l_next, gr)
                    {
                       if (gr->delete_me)
                         {
                            Evas_Object_Protected_Data *gr_object_obj =
                               efl_data_scope_get(gr->object, EFL_CANVAS_OBJECT_CLASS);
                            evas_key_grab_free(gr->object, gr_object_obj, gr->keyname,
                                            gr->modifiers, gr->not_modifiers);
                         }
                    }
               }
          }
     }
   if ((e->focused) && (!exclusive))
     {
        Evas_Object_Protected_Data *focused_obj = efl_data_scope_get(e->focused, EFL_CANVAS_OBJECT_CLASS);
        if (!e->is_frozen && !evas_event_freezes_through(e->focused, focused_obj))
          {
             evas_object_event_callback_call
                   (e->focused, focused_obj, EVAS_CALLBACK_KEY_UP, evt,
                    event_id, EFL_EVENT_KEY_UP);
          }
     }
   _evas_post_event_callback_call(e->evas, e);
   _evas_unwalk(e);

   if (ev->device) efl_unref(ev->device);
}

static void
_canvas_event_feed_key_legacy(Eo *eo_e, Evas_Public_Data *e,
                              const char *keyname, const char *key,
                              const char *string, const char *compose,
                              unsigned int timestamp, const void *data,
                              unsigned int keycode, Eina_Bool down)
{
   Efl_Input_Key_Data *ev = NULL;
   Efl_Input_Key *evt;

   if (!keyname) return;

   evt = efl_input_instance_get(EFL_INPUT_KEY_CLASS, eo_e, (void **) &ev);
   if (!ev) return;

   ev->keyname = (char *) keyname;
   ev->data = (void *) data;
   ev->key = key;
   ev->string = string;
   ev->compose = compose;
   ev->timestamp = timestamp;
   ev->keycode = keycode;
   ev->no_stringshare = EINA_TRUE;
   ev->device = _evas_device_top_get(e->evas);

   if (down)
     _canvas_event_feed_key_down_internal(e, ev);
   else
     _canvas_event_feed_key_up_internal(e, ev);

   efl_del(evt);
}

EAPI void
evas_event_feed_key_down(Eo *eo_e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_key_legacy(eo_e, e, keyname, key, string,
                                 compose, timestamp, data, 0, 1);
}

EAPI void
evas_event_feed_key_up(Eo *eo_e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_key_legacy(eo_e, e, keyname, key, string,
                                 compose, timestamp, data, 0, 0);
}

EAPI void
evas_event_feed_key_down_with_keycode(Eo *eo_e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data, unsigned int keycode)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_key_legacy(eo_e, e, keyname, key, string,
                                 compose, timestamp, data, keycode, 1);
}

EAPI void
evas_event_feed_key_up_with_keycode(Eo *eo_e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data, unsigned int keycode)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_key_legacy(eo_e, e, keyname, key, string,
                                 compose, timestamp, data, keycode, 0);
}

EAPI void
evas_event_feed_hold(Eo *eo_e, int hold, unsigned int timestamp, const void *data)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Efl_Input_Hold_Data *ev = NULL;
   Efl_Input_Hold *evt;
   Eina_List *l, *copy;
   Evas_Object *eo_obj;
   int event_id = 0;

   if (e->is_frozen) return;
   e->last_timestamp = timestamp;

   event_id = _evas_object_event_new();

   evt = efl_input_instance_get(EFL_INPUT_HOLD_CLASS, eo_e, (void **) &ev);
   if (!ev) return;

   ev->hold = !!hold;
   ev->data = (void *) data;
   ev->timestamp = timestamp;
   ev->event_flags = e->default_event_flags;
   ev->device = efl_ref(_evas_device_top_get(eo_e));

   _evas_walk(e);
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if ( !evas_event_freezes_through(eo_obj, obj))
          {
             evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_HOLD, evt,
                                             event_id, EFL_EVENT_HOLD);
             if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
               _evas_event_source_hold_events(eo_obj, event_id, evt);
          }
        if (e->delete_me || e->is_frozen) break;
     }
   eina_list_free(copy);
   _evas_post_event_callback_call(eo_e, e);
   _evas_unwalk(e);
   _evas_object_event_new();

   efl_unref(ev->device);
   efl_del(evt);
}

void
_canvas_event_feed_axis_update_internal(Evas_Public_Data *e, Efl_Input_Pointer_Data *ev)
{
   Efl_Input_Pointer *evt;
   Eina_List *l, *copy;
   Evas_Object *eo_obj;
   int event_id = 0;
   Evas *eo_e;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP);

   if (!e || !ev) return;
   if (e->is_frozen) return;

   eo_e = e->evas;
   e->last_timestamp = ev->timestamp;

   ev->action = EFL_POINTER_ACTION_AXIS;
   ev->value_flags |= value_flags;
   event_id = _evas_object_event_new();
   evt = ev->eo;

   if (ev->device) efl_ref(ev->device);

   _evas_walk(e);
   copy = evas_event_list_copy(e->pointer.object.in);

   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if (!evas_event_freezes_through(eo_obj, obj))
          {
             evas_object_event_callback_call(eo_obj, obj,
                                             EVAS_CALLBACK_AXIS_UPDATE, evt,
                                             event_id, EFL_EVENT_POINTER_AXIS);
             if (e->delete_me || e->is_frozen) break;
          }
     }
   eina_list_free(copy);
   _evas_post_event_callback_call(eo_e, e);

   _evas_unwalk(e);
   if (ev->device) efl_unref(ev->device);
}

EAPI void
evas_event_feed_axis_update(Evas *eo_e, unsigned int timestamp, int device, int toolid, int naxis, const Evas_Axis *axes, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Efl_Input_Pointer_Data *ev = NULL;
   Eina_Bool haswinx = 0, haswiny = 0;
   Efl_Input_Pointer *evt;
   double x = 0, y = 0;
   int n;

   evt = efl_input_instance_get(EFL_INPUT_POINTER_CLASS, eo_e, (void **) &ev);
   if (!ev) return;

   ev->data = (void *) data;
   ev->timestamp = timestamp;
   ev->action = EFL_POINTER_ACTION_AXIS;
   ev->tool = toolid;

   // see also ecore_evas.c
   for (n = 0; n < naxis; n++)
     {
        const Evas_Axis *axis = &(axes[n]);
        switch (axis->label)
          {
           case EVAS_AXIS_LABEL_WINDOW_X:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_X);
             x = axis->value;
             haswinx = EINA_TRUE;
             break;

           case EVAS_AXIS_LABEL_WINDOW_Y:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_Y);
             y = axis->value;
             haswiny = EINA_TRUE;
             break;

           case EVAS_AXIS_LABEL_X:
             if (!haswinx)
               {
                  _efl_input_value_mark(ev, EFL_INPUT_VALUE_X);
                  x = axis->value;
               }
             break;

           case EVAS_AXIS_LABEL_Y:
             if (!haswiny)
               {
                  _efl_input_value_mark(ev, EFL_INPUT_VALUE_Y);
                  y = axis->value;
               }
             break;

           case EVAS_AXIS_LABEL_NORMAL_X:
             ev->raw.x = axis->value;
             break;

           case EVAS_AXIS_LABEL_NORMAL_Y:
             ev->raw.y = axis->value;
             break;

           case EVAS_AXIS_LABEL_PRESSURE:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_PRESSURE);
             ev->pressure = axis->value;
             break;

           case EVAS_AXIS_LABEL_DISTANCE:
           case EVAS_AXIS_LABEL_AZIMUTH:
           case EVAS_AXIS_LABEL_TILT:
           case EVAS_AXIS_LABEL_TWIST:
             // TODO

           case EVAS_AXIS_LABEL_UNKNOWN:
           case EVAS_AXIS_LABEL_TOUCH_WIDTH_MAJOR:
           case EVAS_AXIS_LABEL_TOUCH_WIDTH_MINOR:
           case EVAS_AXIS_LABEL_TOOL_WIDTH_MAJOR:
           case EVAS_AXIS_LABEL_TOOL_WIDTH_MINOR:
           default:
             DBG("Unsupported axis label %d, value %f (discarded)",
                 axis->label, axis->value);
             break;
          }
     }

   ev->cur.x = x;
   ev->cur.y = y;

   /* FIXME: set proper device based on the device id (X or WL specific) */
   ev->device = _evas_device_top_get(eo_e); // FIXME
   (void) device;

   _canvas_event_feed_axis_update_internal(e, ev);

   efl_del(evt);
}

static void
_feed_mouse_move_eval_internal(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Public_Data *evas = obj->layer->evas;
   Eina_Bool in_output_rect;
   in_output_rect = evas_object_is_in_output_rect(eo_obj, obj, evas->pointer.x,
                                                  evas->pointer.y, 1, 1);
   if ((in_output_rect) &&
       ((!obj->precise_is_inside) || (evas_object_is_inside(eo_obj, obj,
                                                            evas->pointer.x,
                                                            evas->pointer.y))))
     {
        _canvas_event_feed_mouse_move_legacy(evas->evas, evas,
                                             evas->pointer.x, evas->pointer.y,
                                             evas->last_timestamp, NULL);
     }
}

EOLIAN void
_efl_canvas_object_freeze_events_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool freeze)
{
   freeze = !!freeze;
   if (obj->freeze_events == freeze) return;
   obj->freeze_events = freeze;
   evas_object_smart_member_cache_invalidate(eo_obj, EINA_FALSE, EINA_TRUE,
                                             EINA_FALSE);
   if (obj->freeze_events) return;
   _feed_mouse_move_eval_internal(eo_obj, obj);
}

EOLIAN Eina_Bool
_efl_canvas_object_freeze_events_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->freeze_events;
}

EOLIAN void
_efl_canvas_object_pass_events_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool pass)
{
   pass = !!pass;
   if (obj->pass_events == pass) return;
   obj->pass_events = pass;
   evas_object_smart_member_cache_invalidate(eo_obj, EINA_TRUE, EINA_FALSE, EINA_FALSE);
   _feed_mouse_move_eval_internal(eo_obj, obj);
}

EOLIAN Eina_Bool
_efl_canvas_object_pass_events_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->pass_events;
}

EOLIAN void
_efl_canvas_object_repeat_events_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool repeat)
{
   repeat = !!repeat;
   if (obj->repeat_events == repeat) return;
   obj->repeat_events = repeat;
   _feed_mouse_move_eval_internal(eo_obj, obj);
}

EOLIAN Eina_Bool
_efl_canvas_object_repeat_events_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->repeat_events;
}

EOLIAN void
_efl_canvas_object_propagate_events_set(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Eina_Bool prop)
{
   obj->no_propagate = !prop;
}

EOLIAN Eina_Bool
_efl_canvas_object_propagate_events_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return !(obj->no_propagate);
}

EOLIAN void
_efl_canvas_object_pointer_mode_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Object_Pointer_Mode setting)
{
   int addgrab;
   Evas_Object *cobj;
   const Eina_List *l;

   /* ignore no-ops */
   if (obj->pointer_mode == setting) return;

   /* adjust by number of pointer down events */
   addgrab = obj->layer->evas->pointer.downs;
   switch (obj->pointer_mode)
     {
      /* nothing needed */
      case EVAS_OBJECT_POINTER_MODE_NOGRAB: break;
      /* decrement canvas nogrep (NO Grab/REPeat) counter */
      case EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN:
        if (obj->mouse_grabbed)
          obj->layer->evas->pointer.nogrep--;
        /* fall through */
      /* remove related grabs from canvas and object */
      case EVAS_OBJECT_POINTER_MODE_AUTOGRAB:
        if (obj->mouse_grabbed)
          {
             obj->layer->evas->pointer.mouse_grabbed -= obj->mouse_grabbed;
             obj->mouse_grabbed = 0;
          }
     }
   /* adjustments for new mode */
   switch (setting)
     {
      /* nothing needed */
      case EVAS_OBJECT_POINTER_MODE_NOGRAB: break;
      /* increment canvas nogrep (NO Grab/REPeat) counter */
      case EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN:
        obj->layer->evas->pointer.nogrep++;
        /* having nogrep set indicates that any object following it in
         * the pointer.object.in list will not be receiving events, meaning
         * that they will fail to unset any existing grabs/flags. unset them
         * now to avoid breaking the canvas
         */
        EINA_LIST_FOREACH(obj->layer->evas->pointer.object.in, l, cobj)
          {
             Evas_Object_Protected_Data *cobj_data;

             /* skip to the current object */
             if (cobj != eo_obj) continue;
             /* only change objects past it */
             EINA_LIST_FOREACH(l->next, l, cobj)
               {
                  cobj_data = efl_data_scope_get(cobj, EFL_CANVAS_OBJECT_CLASS);
                  if (!cobj_data->mouse_grabbed) continue;
                  cobj_data->mouse_grabbed -= addgrab;
                  cobj_data->layer->evas->pointer.mouse_grabbed -= addgrab;
                  if (cobj_data->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
                    cobj_data->layer->evas->pointer.nogrep--;
               }
             break;
          }
        /* fall through */
      /* add all button grabs to this object */
      case EVAS_OBJECT_POINTER_MODE_AUTOGRAB:
        obj->mouse_grabbed += addgrab;
        obj->layer->evas->pointer.mouse_grabbed += addgrab;
     }
   obj->pointer_mode = setting;
}

EOLIAN Evas_Object_Pointer_Mode
_efl_canvas_object_pointer_mode_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->pointer_mode;
}

EAPI void
evas_event_refeed_event(Eo *eo_e, void *event_copy, Evas_Callback_Type event_type)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   if (!event_copy) return;

   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_IN:
          {
             Evas_Event_Mouse_In *ev = event_copy;
             evas_event_feed_mouse_in(eo_e, ev->timestamp, ev->data);
             break;
          }
      case EVAS_CALLBACK_MOUSE_OUT:
          {
             Evas_Event_Mouse_Out *ev = event_copy;
             evas_event_feed_mouse_out(eo_e, ev->timestamp, ev->data);
             break;
          }
      case EVAS_CALLBACK_MOUSE_DOWN:
          {
             Evas_Event_Mouse_Down *ev = event_copy;
             evas_event_feed_mouse_down(eo_e, ev->button, ev->flags, ev-> timestamp, ev->data);
             break;
          }
      case EVAS_CALLBACK_MOUSE_UP:
          {
             Evas_Event_Mouse_Up *ev = event_copy;
             evas_event_feed_mouse_up(eo_e, ev->button, ev->flags, ev-> timestamp, ev->data);
             break;
          }
      case EVAS_CALLBACK_MOUSE_MOVE:
          {
             Evas_Event_Mouse_Move *ev = event_copy;
             evas_event_feed_mouse_move(eo_e, ev->cur.canvas.x, ev->cur.canvas.y, ev->timestamp, ev->data);
             break;
          }
      case EVAS_CALLBACK_MOUSE_WHEEL:
          {
             Evas_Event_Mouse_Wheel *ev = event_copy;
             evas_event_feed_mouse_wheel(eo_e, ev->direction, ev-> z, ev->timestamp, ev->data);
             break;
          }
      case EVAS_CALLBACK_MULTI_DOWN:
          {
             Evas_Event_Multi_Down *ev = event_copy;
             evas_event_feed_multi_down(eo_e, ev->device, ev->canvas.x, ev->canvas.y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, ev->canvas.xsub, ev->canvas.ysub, ev->flags, ev->timestamp, ev->data);
             break;
          }
      case EVAS_CALLBACK_MULTI_UP:
          {
             Evas_Event_Multi_Up *ev = event_copy;
             evas_event_feed_multi_up(eo_e, ev->device, ev->canvas.x, ev->canvas.y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, ev->canvas.xsub, ev->canvas.ysub, ev->flags, ev->timestamp, ev->data);
             break;
          }
      case EVAS_CALLBACK_MULTI_MOVE:
          {
             Evas_Event_Multi_Move *ev = event_copy;
             evas_event_feed_multi_move(eo_e, ev->device, ev->cur.canvas.x, ev->cur.canvas.y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, ev->cur.canvas.xsub, ev->cur.canvas.ysub, ev->timestamp, ev->data);
             break;
          }
      case EVAS_CALLBACK_KEY_DOWN:
          {
             Evas_Event_Key_Down *ev = event_copy;
             evas_event_feed_key_down(eo_e, ev->keyname, ev->key, ev->string, ev->compose, ev->timestamp, ev->data);
             break;
          }
      case EVAS_CALLBACK_KEY_UP:
          {
             Evas_Event_Key_Up *ev = event_copy;
             evas_event_feed_key_up(eo_e, ev->keyname, ev->key, ev->string, ev->compose, ev->timestamp, ev->data);
             break;
          }
      case EVAS_CALLBACK_AXIS_UPDATE:
          {
             Evas_Event_Axis_Update *ev = event_copy;
             evas_event_feed_axis_update(eo_e, ev->timestamp, ev->device, ev->toolid, ev->naxis, ev->axis, ev->data);
             break;
          }
      default: /* All non-input events are not handeled */
        break;
     }
}

EOLIAN int
_evas_canvas_event_down_count_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->pointer.downs;
}

static void
_evas_canvas_event_pointer_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Pointer_Data *ev = efl_data_scope_get(event->info, EFL_INPUT_POINTER_CLASS);
   Evas_Public_Data *e = data;
   Evas *eo_e = event->object;
   Eina_Bool nodev = 0;

   if (!ev) return;

   ev->evas_done = EINA_TRUE;
   ev->modifiers = &e->modifiers;
   ev->locks = &e->locks;

   if (!ev->device)
     {
        nodev = 1;
        ev->device = _evas_device_top_get(e->evas);
     }

   switch (ev->action)
     {
      case EFL_POINTER_ACTION_MOVE:
        if (ev->tool == 0)
          _canvas_event_feed_mouse_move_internal(e, ev);
        else
          _canvas_event_feed_multi_move_internal(e, ev);
        break;

      case EFL_POINTER_ACTION_DOWN:
        if (ev->tool == 0)
          _canvas_event_feed_mouse_down_internal(e, ev);
        else
          _canvas_event_feed_multi_down_internal(e, ev);
        break;

      case EFL_POINTER_ACTION_UP:
        if (ev->tool == 0)
          _canvas_event_feed_mouse_up_internal(e, ev);
        else
          _canvas_event_feed_multi_up_internal(e, ev);
        break;

      case EFL_POINTER_ACTION_CANCEL:
        _canvas_event_feed_mouse_cancel_internal(e, ev);
        break;

      case EFL_POINTER_ACTION_IN:
        _canvas_event_feed_mouse_in_internal(eo_e, ev);
        break;

      case EFL_POINTER_ACTION_OUT:
        _canvas_event_feed_mouse_out_internal(eo_e, ev);
        break;

      case EFL_POINTER_ACTION_WHEEL:
        _canvas_event_feed_mouse_wheel_internal(eo_e, ev);
        break;

      case EFL_POINTER_ACTION_AXIS:
        _canvas_event_feed_axis_update_internal(e, ev);
        break;

      default:
        ERR("unsupported event type: %d", ev->action);
        ev->evas_done = EINA_FALSE;
        break;
     }

   if (nodev) ev->device = NULL;
}

static void
_evas_canvas_event_key_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Key *evt = event->info;
   Evas_Public_Data *e = data;
   Efl_Input_Key_Data *ev;
   Eina_Bool nodev = 0;

   ev = efl_data_scope_get(evt, EFL_INPUT_KEY_CLASS);
   if (!ev) return;

   if (!ev->device)
     {
        nodev = 1;
        ev->device = _evas_device_top_get(e->evas);
     }

   if (ev->pressed)
     _canvas_event_feed_key_down_internal(e, ev);
   else
     _canvas_event_feed_key_up_internal(e, ev);

   if (nodev) ev->device = NULL;
   ev->evas_done = EINA_TRUE;
}

// note: "hold" event comes from above (elm), not below (ecore)
EFL_CALLBACKS_ARRAY_DEFINE(_evas_canvas_event_pointer_callbacks,
{ EFL_EVENT_POINTER_MOVE, _evas_canvas_event_pointer_cb },
{ EFL_EVENT_POINTER_DOWN, _evas_canvas_event_pointer_cb },
{ EFL_EVENT_POINTER_UP, _evas_canvas_event_pointer_cb },
{ EFL_EVENT_POINTER_IN, _evas_canvas_event_pointer_cb },
{ EFL_EVENT_POINTER_OUT, _evas_canvas_event_pointer_cb },
{ EFL_EVENT_POINTER_CANCEL, _evas_canvas_event_pointer_cb },
{ EFL_EVENT_POINTER_WHEEL, _evas_canvas_event_pointer_cb },
{ EFL_EVENT_POINTER_AXIS, _evas_canvas_event_pointer_cb },
{ EFL_EVENT_KEY_DOWN, _evas_canvas_event_key_cb },
{ EFL_EVENT_KEY_UP, _evas_canvas_event_key_cb })

void
_evas_canvas_event_init(Evas *eo_e, Evas_Public_Data *e)
{
   efl_event_callback_array_add(eo_e, _evas_canvas_event_pointer_callbacks(), e);
}

void
_evas_canvas_event_shutdown(Evas *eo_e, Evas_Public_Data *e)
{
   efl_event_callback_array_del(eo_e, _evas_canvas_event_pointer_callbacks(), e);
}

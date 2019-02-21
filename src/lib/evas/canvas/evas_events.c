#define EFL_INPUT_EVENT_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

int _evas_event_counter = 0;

static Eina_List *
_evas_event_object_list_in_get(Evas *eo_e, Eina_List *in,
                               const Eina_Inlist *ilist,
                               const Eina_List *list,
                               Evas_Object *stop,
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

static inline void
_evas_event_feed_check(Evas_Public_Data *e)
{
   if (EINA_LIKELY(!e->running_post_events)) return;
   CRI("Feeding new input events from a post-event callback is risky!");
}

static inline Eina_Bool
_evas_event_object_pointer_allow(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Object_Pointer_Data *obj_pdata)
{
   return (obj->is_event_parent || evas_object_clippers_is_visible(eo_obj, obj) || obj_pdata->mouse_grabbed) &&
           (!evas_event_passes_through(eo_obj, obj)) &&
           (!evas_event_freezes_through(eo_obj, obj)) &&
           (!obj->clip.clipees);
}

static inline Eina_Bool
_evas_event_object_pointer_allow_precise(Eo *eo_obj, Evas_Object_Protected_Data *obj, int x, int y, const Eina_List *ins)
{
   return eina_list_data_find(ins, eo_obj) &&
     ((!obj->precise_is_inside) || evas_object_is_inside(eo_obj, obj, x, y));
}

#define EVAS_EVENT_FEED_SAFETY_CHECK(evas) _evas_event_feed_check(evas)

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
# define DDD(...) do { for (int _i = 0; _i < spaces; _i++) printf(" "); printf(__VA_ARGS__); } while (0)
# define D(...) do { printf(__VA_ARGS__); } while (0)
# define DDD_STATIC static
#else
# define DDD(...) do { } while (0)
# define D(...) do { } while (0)
# define DDD_STATIC
#endif

#ifdef DDD_DO
static void
walk_clippers_print(int spaces, Evas_Object_Protected_Data *obj)
{
   DDD("<<< CLIP %p c[%6i %6i %6ix%6i] c[%6i %6i %6ix%6i]\n",
       obj->object,
       obj->cur->geometry.x, obj->cur->geometry.y,
       obj->cur->geometry.w, obj->cur->geometry.h,
       obj->cur->cache.clip.x, obj->cur->cache.clip.y,
       obj->cur->cache.clip.w, obj->cur->cache.clip.h
      );
   if (obj->cur->clipper) walk_clippers_print(spaces + 1, obj->cur->clipper);
}
#endif

static void
clip_calc(Evas_Object_Protected_Data *obj, Eina_Rectangle *c)
{
   if (!obj) return;
   RECTS_CLIP_TO_RECT(c->x, c->y, c->w, c->h,
                      obj->cur->geometry.x, obj->cur->geometry.y,
                      obj->cur->geometry.w, obj->cur->geometry.h);
   clip_calc(obj->cur->clipper, c);
}

static Eina_List *
_evas_event_object_list_raw_in_get_single(Evas *eo_e, Evas_Object_Protected_Data *obj, Eina_List *in, Evas_Object *stop,
                                          int x, int y, int *no_rep, Eina_Bool source, int spaces EINA_UNUSED)
{
   Eina_Rectangle c;
   int inside;
   Evas_Object *eo_obj = obj->object;
   if (eo_obj == stop)
     {
        *no_rep = 1;
        DDD("***** NO REP - STOP *****\n");
        return in;
     }
   if ((!obj->cur->visible) && (!obj->is_event_parent)) return in;
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

   if (EINA_UNLIKELY((!!obj->map) && (obj->map->cur.map)
                     && (obj->map->cur.usemap)))
     c = obj->map->cur.map->normal_geometry;
   else
     {
        if (obj->is_smart)
          {
             Eina_Rectangle bounding_box = { 0, };

             evas_object_smart_bounding_box_update(obj);
             evas_object_smart_bounding_box_get(obj, &bounding_box, NULL);
             c = bounding_box;
          }
        else
          {
             if (obj->clip.clipees) return in;
             c = obj->cur->geometry;
          }
     }
   clip_calc(obj->cur->clipper, &c);
   // only worry about objects that intersect INCLUDING clippint
   if ((!RECTS_INTERSECT(x, y, 1, 1, c.x, c.y, c.w, c.h)) && (!obj->child_has_map))
     {
#ifdef DDD_DO
        if (obj->is_smart)
          {
             Eina_Rectangle bounding_box = { 0, 0, 0, 0 };

             evas_object_smart_bounding_box_get(obj, &bounding_box, NULL);
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
               walk_clippers_print(spaces + 1, obj->cur->clipper);
          }
#endif
        return in;
     }
#ifdef DDD_DO
   else
     {
        if (obj->is_smart)
          {
             Eina_Rectangle bounding_box = { 0, 0, 0, 0 };

             evas_object_smart_bounding_box_get(obj, &bounding_box, NULL);
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
               walk_clippers_print(spaces + 1, obj->cur->clipper);
          }
     }
#endif

   if (!source)
     {
        if (evas_event_passes_through(eo_obj, obj)) return in;
        if (evas_object_is_source_invisible(eo_obj, obj)) return in;
     }
   if ((obj->delete_me == 0) &&
       ((source) || ((obj->cur->visible || obj->is_event_parent) && (!obj->clip.clipees) &&
        (obj->is_event_parent || evas_object_clippers_is_visible(eo_obj, obj)))))
     {
        if (obj->is_smart)
          {
             DDD("CHILDREN ->\n");
             Evas_Object_Protected_Data *clip = obj->cur->clipper;
             int norep = 0;

             if (clip && clip->mask->is_mask && clip->precise_is_inside)
               if (!evas_object_is_inside(clip->object, clip, x, y))
                 return in;

             if ((obj->map->cur.usemap) && (obj->map->cur.map))
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
                                NULL,
                                stop,
                                obj->cur->geometry.x + obj->map->cur.map->mx,
                                obj->cur->geometry.y + obj->map->cur.map->my,
                                &norep, source);
                         }
                    }
               }
             else
               {
                  Eina_Rectangle bounding_box = { 0, };

                  if (!obj->child_has_map)
                    evas_object_smart_bounding_box_update(obj);

                  evas_object_smart_bounding_box_get(obj, &bounding_box, NULL);

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
                      (eo_e, in, evas_object_smart_members_get_direct(eo_obj), NULL,
                       stop, x, y, &norep, source);
               }
             if (norep)
               {
                  if (!obj->repeat_events)
                    {
                       *no_rep = 1;
                       DDD("***** NO REP1 *****\n");
                       return in;
                    }
               }
          }
        else if (obj->is_event_parent)
          {
             int norep = 0;
             in = _evas_event_object_list_in_get(eo_e, in,
               NULL, evas_object_event_grabber_members_list(eo_obj),
               stop, x, y, &norep, source);
             if (norep)
               {
                  if (!obj->repeat_events)
                    {
                       *no_rep = 1;
                       DDD("***** NO REP1 *****\n");
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
                  if ((obj->map->cur.usemap) && (obj->map->cur.map))
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
                       DDD("***** NO REP2 *****\n");
                       return in;
                    }
               }
          }
     }
   return in;
}

static Eina_List *
_evas_event_object_list_raw_in_get(Evas *eo_e, Eina_List *in,
                                   const Eina_Inlist *ilist,
                                   const Eina_List *list,
                                   Evas_Object *stop,
                                   int x, int y, int *no_rep, Eina_Bool source)
{
   Evas_Object_Protected_Data *obj = NULL;
   DDD_STATIC int spaces = 0;

   if ((!ilist) && (!list)) return in;

   spaces++;
   if (ilist)
     {
        for (obj = _EINA_INLIST_CONTAINER(obj, eina_inlist_last(ilist));
             obj;
             obj = _EINA_INLIST_CONTAINER(obj, EINA_INLIST_GET(obj)->prev))
          {
             if (obj->events->parent) continue;
             in = _evas_event_object_list_raw_in_get_single(eo_e, obj, in, stop, x, y, no_rep, source, spaces);
             if (*no_rep) goto end;
          }
     }
   else
     {
        Eina_List *l;

        EINA_LIST_REVERSE_FOREACH(list, l, obj)
          {
             in = _evas_event_object_list_raw_in_get_single(eo_e, obj, in, stop, x, y, no_rep, source, spaces);
             if (*no_rep) goto end;
          }
     }
   *no_rep = 0;

end:
   spaces--;
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

   if (!EINA_DBL_EQ(obj_w, src_w))
     tmp_x = (tmp_x * (src_w / obj_w));
   if (!EINA_DBL_EQ(obj_h, src_h))
     tmp_y = (tmp_y * (src_h / obj_h));

   tmp_x += src->cur->geometry.x;
   tmp_y += src->cur->geometry.y;
   point->x = tmp_x;
   point->y = tmp_y;
}

static Efl_Input_Device *
_evas_event_legacy_device_get(Eo *evas, Eina_Bool mouse)
{
   Efl_Input_Device *dev = _evas_device_top_get(evas);
   //The user did not push a device, use the default mouse/keyboard instead.
   if (!dev)
     {
        Evas_Public_Data *e = efl_data_scope_get(evas, EVAS_CANVAS_CLASS);
        if (mouse)
          return e->default_mouse;
        return e->default_keyboard;
     }
   return dev;
}

static void
_evas_event_source_mouse_down_events(Evas_Object *eo_obj, Evas *eo_e,
                                     Efl_Input_Pointer *parent_ev,
                                     Evas_Pointer_Data *pdata,
                                     int event_id)
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

   evt = efl_duplicate(parent_ev);
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
                evas_object_smart_members_get_direct(eo_src), NULL,
                NULL, ev->cur.x, ev->cur.y, &no_rep, EINA_TRUE);
         }
       else if (src->is_event_parent)
         {
            proxy_write->src_event_in = _evas_event_object_list_raw_in_get
               (eo_e, proxy_write->src_event_in,
                NULL, evas_object_event_grabber_members_list(eo_src),
                NULL, ev->cur.x, ev->cur.y, &no_rep, EINA_TRUE);
         }
       else
         proxy_write->src_event_in = eina_list_append(proxy_write->src_event_in, eo_src);
     }
   EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_write);

   if (pdata->seat->downs > 1) addgrab = pdata->seat->downs - 1;

   EINA_LIST_FOREACH(src->proxy->src_event_in, l, eo_child)
     {
        Evas_Object_Pointer_Data *obj_pdata;

        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        obj_pdata = _evas_object_pointer_data_get(pdata, child);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }
        if ((obj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) ||
            (obj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN))
          {
             obj_pdata->mouse_grabbed += (addgrab + 1);
             pdata->seat->mouse_grabbed += (addgrab + 1);
             if (obj_pdata->pointer_mode ==
                 EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
               {
                  pdata->seat->nogrep++;
                  break;
               }
          }
     }

   copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        Evas_Object_Pointer_Data *obj_pdata;
        Evas_Object_Pointer_Mode pointer_mode;

        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        ev->cur = point;
        obj_pdata = _evas_object_pointer_data_get(pdata, child);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }
        pointer_mode = obj_pdata->pointer_mode;
        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, obj_pdata->mouse_grabbed);
        evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_DOWN, evt,
                                        event_id, EFL_EVENT_POINTER_DOWN);
        if (e->delete_me) break;
        if (pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
          break;
     }
   eina_list_free(copy);

   efl_unref(evt);
}

static void
_evas_event_mouse_in_set(Evas_Pointer_Seat *pseat,
                         Evas_Object_Protected_Data *obj, Eina_Bool mouse_in)
{
   Evas_Pointer_Data *pdata;
   Evas_Object_Pointer_Data *obj_pdata;

   if ((!pseat) || (!obj)) return;

   EINA_INLIST_FOREACH(pseat->pointers, pdata)
     {
        obj_pdata = _evas_object_pointer_data_get(pdata, obj);
        if (obj_pdata)
          obj_pdata->mouse_in = mouse_in;
     }
}

static void
_evas_event_source_mouse_move_events(Evas_Object *eo_obj, Evas *eo_e,
                                     Efl_Input_Pointer *parent_ev,
                                     Evas_Pointer_Data *pdata,
                                     int event_id)
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
   Eina_Vector2 curpt, curpt_real, prevpt;
   Evas_Object_Pointer_Data *obj_pdata;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

   evt = efl_duplicate(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   curpt_real = ev->cur;
   _transform_to_src_space_f(obj, src, &ev->cur);
   _transform_to_src_space_f(obj, src, &ev->prev);
   curpt = ev->cur;
   prevpt = ev->prev;
   ev->source = eo_obj;
   ev->tool = 0;

   if (pdata->seat->mouse_grabbed)
     {
        Eina_List *outs = NULL;
        Eina_List *copy = evas_event_list_copy(src->proxy->src_event_in);

        EINA_LIST_FOREACH(copy, l, eo_child)
          {
             child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
             //FIXME: When object is deleted in the src_event_in list,
             //the src_event_in list should be updated. But now there is no way.
             //So add checking NULL logic, please delete it if you make a better way.
             if (!child) continue;

             obj_pdata = _evas_object_pointer_data_get(pdata, child);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }

             if (_evas_event_object_pointer_allow(eo_child, child, obj_pdata))
               {
                  ev->cur = curpt;
                  ev->prev = prevpt;
                  _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, obj_pdata->mouse_grabbed);
                  _evas_event_havemap_adjust_f(eo_child, child, &ev->prev, obj_pdata->mouse_grabbed);

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
             if (child->delete_me) continue;
             obj_pdata = _evas_object_pointer_data_get(pdata, child);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }
             if ((obj_pdata->mouse_grabbed == 0) && (!e->delete_me))
               {
                  if (obj_pdata->mouse_in) continue; /* FIXME: dubious logic! */
                  _evas_event_mouse_in_set(pdata->seat, child, 0);
                  if (e->is_frozen) continue;
                  ev->cur = curpt_real;
                  _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, obj_pdata->mouse_grabbed);
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
                                                      NULL, NULL, ev->cur.x, ev->cur.y, &no_rep, EINA_TRUE);
          }
        else if (src->is_event_parent)
          {
             int no_rep = 0;
             ins = _evas_event_object_list_raw_in_get(eo_e, ins, NULL,
                                                      evas_object_event_grabber_members_list(eo_src),
                                                      NULL, ev->cur.x, ev->cur.y, &no_rep, EINA_TRUE);
          }
        else
          ins = eina_list_append(ins, eo_src);

        EINA_LIST_FOREACH(copy, l, eo_child)
          {
             child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
             //FIXME: When object is deleted in the src_event_in list,
             //the src_event_in list should be updated. But now there is no way.
             //So add checking NULL logic, please delete it if you make a better way.
             if (!child) continue;

             obj_pdata = _evas_object_pointer_data_get(pdata, child);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }
             ev->cur = curpt;
             if (evas_object_is_in_output_rect(eo_child, child,
                                               ev->cur.x, ev->cur.y, 1, 1) &&
                _evas_event_object_pointer_allow(eo_child, child, obj_pdata) &&
                _evas_event_object_pointer_allow_precise(eo_child, child, ev->cur.x, ev->cur.y, ins))
               {
                  ev->action = EFL_POINTER_ACTION_MOVE;
                  _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, obj_pdata->mouse_grabbed);
                  evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                  event_id, EFL_EVENT_POINTER_MOVE);
               }
             else if (obj_pdata->mouse_in)
               {
                  _evas_event_mouse_in_set(pdata->seat, child, 0);
                  if (e->is_frozen) continue;
                  ev->cur = curpt;
                  _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, obj_pdata->mouse_grabbed);
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
             obj_pdata = _evas_object_pointer_data_get(pdata, child);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }
              if (!eina_list_data_find(src->proxy->src_event_in, eo_child))
                {
                   if (!obj_pdata->mouse_in)
                     {
                        _evas_event_mouse_in_set(pdata->seat, child, 1);
                        if (e->is_frozen) continue;
                        ev->cur = curpt;
                        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, obj_pdata->mouse_grabbed);
                        ev->action = EFL_POINTER_ACTION_IN;
                        evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_IN, evt,
                                                        event_id2, EFL_EVENT_POINTER_IN);
                        if ((curpt.x != ev->prev.x) &&
                            (curpt.y != ev->prev.y))
                          {
                             ev->action = EFL_POINTER_ACTION_MOVE;
                             evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                             event_id2, EFL_EVENT_POINTER_MOVE);
                          }
                        if (e->delete_me) break;
                     }
                }
          }

        if (pdata->seat->mouse_grabbed == 0)
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

   efl_unref(evt);
}

static void
_evas_event_source_mouse_up_events(Evas_Object *eo_obj, Evas *eo_e,
                                   Efl_Input_Pointer *parent_ev,
                                   Evas_Pointer_Data *pdata,
                                   int event_id)
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

   evt = efl_duplicate(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = eo_obj;
   ev->tool = 0;

   copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        Evas_Object_Pointer_Data *obj_pdata;
        Evas_Object_Pointer_Mode pointer_mode;

        if (src->delete_me) break;
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        obj_pdata = _evas_object_pointer_data_get(pdata, child);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }
        if (((obj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) ||
             (obj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)) &&
            (obj_pdata->mouse_grabbed > 0))
          {
             obj_pdata->mouse_grabbed--;
             pdata->seat->mouse_grabbed--;
          }

        ev->cur = point;
        pointer_mode = obj_pdata->pointer_mode;
        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, obj_pdata->mouse_grabbed);
        evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_UP, evt,
                                        event_id, EFL_EVENT_POINTER_UP);
        if (e->delete_me) break;
        if (pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
          {
             if (pdata->seat->nogrep > 0) pdata->seat->nogrep--;
             break;
          }
     }
   eina_list_free(copy);

   efl_unref(evt);
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
        evas_object_event_callback_call(eo_child, child,
                                        EVAS_CALLBACK_HOLD, evt,
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
   Evas_Pointer_Data *pdata;

   if (obj->delete_me || src->delete_me || obj->layer->evas->is_frozen) return;

   evt = efl_duplicate(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   pdata = _evas_pointer_data_by_device_get(e, ev->device);
   if (!pdata) return;

   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = eo_obj;

   copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        Evas_Object_Pointer_Data *obj_pdata;

        if (src->delete_me) return;
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        //FIXME: When object is deleted in the src_event_in list,
        //the src_event_in list should be updated. But now there is no way.
        //So add checking NULL logic, please delete it if you make a better way.
        if (!child) continue;

        obj_pdata = _evas_object_pointer_data_get(pdata, child);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }
        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, obj_pdata->mouse_grabbed);
        evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_WHEEL, evt,
                                        event_id, EFL_EVENT_POINTER_WHEEL);
        if (e->delete_me) break;
     }
   eina_list_free(copy);

   efl_unref(evt);
}

static void
_evas_event_source_multi_down_events(Evas_Object_Protected_Data *obj, Evas_Public_Data *e,
                                     Efl_Input_Pointer *parent_ev, Evas_Pointer_Data *pdata,
                                     int event_id)
{
   Evas_Object *eo_src = _evas_object_image_source_get(obj->object);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *child;
   Evas_Object *eo_child;
   Eina_List *copy, *l;
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Eina_Vector2 point;
   Evas_Object_Pointer_Data *obj_pdata;
   int addgrab = 0;

   if (obj->delete_me || src->delete_me || obj->layer->evas->is_frozen) return;

   evt = efl_duplicate(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = obj->object;
   ev->action = EFL_POINTER_ACTION_DOWN;

   if (pdata->seat->downs > 1) addgrab = pdata->seat->downs - 1;

   EINA_LIST_FOREACH(src->proxy->src_event_in, l, eo_child)
     {
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        obj_pdata = _evas_object_pointer_data_get(pdata, child);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }
        if (obj_pdata->pointer_mode != EVAS_OBJECT_POINTER_MODE_NOGRAB)
          {
             obj_pdata->mouse_grabbed += (addgrab + 1);
             pdata->seat->mouse_grabbed += (addgrab + 1);
          }
     }

   copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        obj_pdata = _evas_object_pointer_data_get(pdata, child);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }
        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, obj_pdata->mouse_grabbed);
        evas_object_event_callback_call(obj->object, obj, EVAS_CALLBACK_MULTI_DOWN, evt,
                                        event_id, EFL_EVENT_FINGER_DOWN);
        if (e->delete_me) break;
     }
   eina_list_free(copy);

   efl_unref(evt);
}

static void
_evas_event_source_multi_up_events(Evas_Object_Protected_Data *obj, Evas_Public_Data *e,
                                   Efl_Input_Pointer *parent_ev, Evas_Pointer_Data *pdata,
                                   int event_id)
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

   evt = efl_duplicate(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = obj->object;
   ev->action = EFL_POINTER_ACTION_UP;

   copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        Evas_Object_Pointer_Data *obj_pdata;

        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);

        obj_pdata = _evas_object_pointer_data_get(pdata, child);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }

        if (((obj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) ||
             (obj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)) &&
            (obj_pdata->mouse_grabbed > 0))
          {
             obj_pdata->mouse_grabbed--;
             pdata->seat->mouse_grabbed--;
          }
        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, obj_pdata->mouse_grabbed);
        evas_object_event_callback_call(obj->object, obj, EVAS_CALLBACK_MULTI_UP, evt,
                                        event_id, EFL_EVENT_FINGER_UP);
        if (e->delete_me || e->is_frozen) break;
     }
    eina_list_free(copy);

    efl_unref(evt);
}

static void
_evas_event_source_multi_move_events(Evas_Object_Protected_Data *obj, Evas_Public_Data *e,
                                     Efl_Input_Pointer *parent_ev, Evas_Pointer_Data *pdata,
                                     int event_id)
{
   Evas_Object *eo_src = _evas_object_image_source_get(obj->object);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *child;
   Evas_Object *eo_child;
   Eina_List *copy, *l;
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Eina_Vector2 point;
   Evas_Object_Pointer_Data *obj_pdata;
   Evas *eo_e = e->evas;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

   evt = efl_duplicate(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = obj->object;
   ev->action = EFL_POINTER_ACTION_UP;

   /* Why a new event id here? Other 'source' events keep the same id. */
   event_id = _evas_object_event_new();

   if (pdata->seat->mouse_grabbed > 0)
     {
        copy = evas_event_list_copy(src->proxy->src_event_in);
        EINA_LIST_FOREACH(copy, l, eo_child)
          {
             child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
             obj_pdata = _evas_object_pointer_data_get(pdata, child);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }
             if (_evas_event_object_pointer_allow(eo_child, child, obj_pdata))
               {
                  ev->cur = point;
                  _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, obj_pdata->mouse_grabbed);
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
                   (eo_e, ins, evas_object_smart_members_get_direct(eo_src), NULL, NULL,
                    ev->cur.x, ev->cur.y, &no_rep, EINA_TRUE);
          }
        if (src->is_event_parent)
          {
             int no_rep = 0;
             ins = _evas_event_object_list_raw_in_get
                   (eo_e, ins, NULL, evas_object_event_grabber_members_list(eo_src), NULL,
                    ev->cur.x, ev->cur.y, &no_rep, EINA_TRUE);
          }
        else
          ins = eina_list_append(ins, eo_src);

        copy = evas_event_list_copy(src->proxy->src_event_in);
        EINA_LIST_FOREACH(copy, l, eo_child)
          {
             child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
             ev->cur = point;
             obj_pdata = _evas_object_pointer_data_get(pdata, child);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }

             if (evas_object_is_in_output_rect(eo_child, child, ev->cur.x, ev->cur.y, 1, 1) &&
                _evas_event_object_pointer_allow(eo_child, child, obj_pdata) &&
                _evas_event_object_pointer_allow_precise(eo_child, child, ev->cur.x, ev->cur.y, ins))
               {
                  _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, obj_pdata->mouse_grabbed);
                  evas_object_event_callback_call(obj->object, obj, EVAS_CALLBACK_MULTI_MOVE, evt,
                                                  event_id, EFL_EVENT_FINGER_MOVE);
                  if (e->delete_me || e->is_frozen) break;
               }
          }
        eina_list_free(copy);
        if (pdata->seat->mouse_grabbed == 0)
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

   efl_unref(evt);
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
   Evas_Pointer_Data *pdata;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;
   evt = efl_duplicate(parent_ev);

   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   pdata = _evas_pointer_data_by_device_get(e, ev->device);
   if (!pdata) return;

   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = eo_obj;
   ev->action = EFL_POINTER_ACTION_IN;

   if (src->is_smart)
     {
        int no_rep = 0;
        ins = _evas_event_object_list_raw_in_get(eo_e, ins, evas_object_smart_members_get_direct(eo_src),
                                                 NULL, NULL, ev->cur.x, ev->cur.y, &no_rep, EINA_TRUE);

     }
   else if (src->is_event_parent)
     {
        int no_rep = 0;
        ins = _evas_event_object_list_raw_in_get(eo_e, ins, NULL, evas_object_event_grabber_members_list(eo_src),
                                                 NULL, ev->cur.x, ev->cur.y, &no_rep, EINA_TRUE);

     }
   else
     ins = eina_list_append(ins, eo_src);

   EINA_LIST_FOREACH(ins, l, eo_child)
     {
        Evas_Object_Pointer_Data *obj_pdata;

        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        obj_pdata = _evas_object_pointer_data_get(pdata, child);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }

        if (!eina_list_data_find(src->proxy->src_event_in, eo_child))
          {
             if (obj_pdata->mouse_in) continue;
             _evas_event_mouse_in_set(pdata->seat, child, 1);
             ev->cur = point;
             _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, obj_pdata->mouse_grabbed);
             ev->action = EFL_POINTER_ACTION_IN;
             evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_IN, evt,
                                             event_id, EFL_EVENT_POINTER_IN);
             if ((ev->cur.x != ev->prev.x) &&
                 (ev->cur.y != ev->prev.y))
               {
                  ev->action = EFL_POINTER_ACTION_MOVE;
                  evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                  event_id, EFL_EVENT_POINTER_MOVE);
               }
             if (e->delete_me || e->is_frozen) break;
          }
     }

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_write)
     {
       eina_list_free(proxy_write->src_event_in);
       proxy_write->src_event_in = ins;
     }
   EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_write);

   efl_unref(evt);
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
   Evas_Pointer_Data *pdata;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

   evt = efl_duplicate(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   pdata = _evas_pointer_data_by_device_get(e, ev->device);
   if (!pdata) return;
   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;
   ev->source = eo_obj;
   ev->action = EFL_POINTER_ACTION_OUT;

   copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        Evas_Object_Pointer_Data *obj_pdata;
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        obj_pdata = _evas_object_pointer_data_get(pdata, child);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }

        if (!obj_pdata->mouse_in) continue;
        _evas_event_mouse_in_set(pdata->seat, child, 0);
        if (child->delete_me) continue;

        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, obj_pdata->mouse_grabbed);
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, evt,
                                        event_id, EFL_EVENT_POINTER_OUT);
        if (e->is_frozen) continue;
     }
   eina_list_free(copy);

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_write)
     proxy_write->src_event_in = eina_list_free(proxy_write->src_event_in);
   EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_write);

   efl_unref(evt);
}

static Eina_List *
_evas_event_object_list_in_get(Evas *eo_e, Eina_List *in,
                               const Eina_Inlist *ilist,
                               const Eina_List *list,
                               Evas_Object *stop,
                               int x, int y, int *no_rep, Eina_Bool source)
{
   return _evas_event_object_list_raw_in_get(eo_e, in, ilist, list, stop, x, y,
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
                                            EINA_INLIST_GET(lay->objects), NULL,
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
_evas_canvas_event_default_flags_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->default_event_flags;
}

static inline void
_canvas_event_thaw_eval_internal(Eo *eo_e, Evas_Public_Data *e)
{
   Evas_Pointer_Data *pdata = _evas_pointer_data_by_device_get(e, NULL);
   if (!pdata) return;
   _canvas_event_feed_mouse_move_legacy(eo_e, e, pdata->seat->x, pdata->seat->y,
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
   Evas_Pointer_Data *pdata;
   Evas_Object_Pointer_Data *obj_pdata;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL) |
         _efl_input_value_mask(EFL_INPUT_VALUE_BUTTON);

   if (!e || !ev) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   pdata = _evas_pointer_data_by_device_get(e, ev->device);
   if (!pdata) return;

   b = ev->button;
   DBG("ButtonEvent:down time=%u x=%d y=%d button=%d downs=%d",
       ev->timestamp, pdata->seat->x, pdata->seat->y, b, pdata->seat->downs);
   if ((b < 1) || (b > 32)) return;

   pdata->button |= (1u << (b - 1));
   pdata->seat->downs++;

   if (e->is_frozen) return;
   e->last_timestamp = ev->timestamp;
   eo_e = e->evas;
   evt = ev->eo;

   event_id = _evas_object_event_new();

   ev->cur.x = pdata->seat->x;
   ev->cur.y = pdata->seat->y;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->tool = 0;
   ev->action = EFL_POINTER_ACTION_DOWN;
   ev->value_flags |= value_flags;
   if (ev->device) efl_ref(ev->device);

   _evas_walk(e);
   /* append new touch point to the touch point list */
   _evas_touch_point_append(e->evas, 0, pdata->seat->x, pdata->seat->y);
   /* If this is the first finger down, i.e no other fingers pressed,
    * get a new event list, otherwise, keep the current grabbed list. */
   if (pdata->seat->mouse_grabbed == 0)
     {
        Eina_List *ins = evas_event_objects_event_list(eo_e,
                                                       NULL,
                                                       pdata->seat->x,
                                                       pdata->seat->y);
        /* free our old list of ins */
        eina_list_free(pdata->seat->object.in);
        /* and set up the new one */
        pdata->seat->object.in = ins;
        /* adjust grabbed count by the nuymber of currently held down
         * fingers/buttons */
        if (pdata->seat->downs > 1) addgrab = pdata->seat->downs - 1;
     }
   copy = evas_event_list_copy(pdata->seat->object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        obj_pdata = _evas_object_pointer_data_get(pdata, obj);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }

        if ((obj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) ||
            (obj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN))
          {
             obj_pdata->mouse_grabbed += addgrab + 1;
             pdata->seat->mouse_grabbed += addgrab + 1;
             if (obj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
               {
                  pdata->seat->nogrep++;
                  break;
               }
          }
     }
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        Evas_Object_Pointer_Mode pointer_mode;
        if (obj->delete_me) continue;
        obj_pdata = _evas_object_pointer_data_get(pdata, obj);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }
        ev->cur.x = pdata->seat->x;
        ev->cur.y = pdata->seat->y;
        pointer_mode = obj_pdata->pointer_mode;
        _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_DOWN, evt,
                                        event_id, EFL_EVENT_POINTER_DOWN);
        if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
          _evas_event_source_mouse_down_events(eo_obj, eo_e, evt, pdata, event_id);
        if (e->is_frozen || e->delete_me)  break;
        if (pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
          break;
     }
   if (copy) eina_list_free(copy);
   e->last_mouse_down_counter++;
   _evas_post_event_callback_call(eo_e, e, event_id);
   /* update touch point's state to EVAS_TOUCH_POINT_STILL */
   _evas_touch_point_update(eo_e, 0, pdata->seat->x, pdata->seat->y, EVAS_TOUCH_POINT_STILL);
   _evas_unwalk(e);

   if (ev->device) efl_unref(ev->device);
}

static void
_post_up_handle(Evas_Public_Data *e, Efl_Input_Pointer *parent_ev,
                Evas_Pointer_Data *pdata)
{
   Eina_List *l, *copy, *ins, *ll;
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Evas_Object *eo_obj;
   Evas *eo_e = e->evas;
   Evas_Object_Pointer_Data *obj_pdata;
   int event_id;

   /* Duplicating UP event */
   evt = efl_duplicate(parent_ev);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   event_id = _evas_object_event_new();

   /* Actually we want an OUT */
   ev->action = EFL_POINTER_ACTION_OUT;

   /* get new list of ins */
   ins = evas_event_objects_event_list(eo_e, NULL, pdata->seat->x, pdata->seat->y);
   /* go thru old list of in objects */
   copy = evas_event_list_copy(pdata->seat->object.in);
   EINA_LIST_FOREACH(copy, ll, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if ((!eina_list_data_find(ins, eo_obj)) || (!pdata->seat->inside))
          {
             obj_pdata = _evas_object_pointer_data_get(pdata, obj);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }
             if (!obj_pdata->mouse_in) continue;
             _evas_event_mouse_in_set(pdata->seat, obj, 0);
             if (!e->is_frozen)
               {
                  ev->cur.x = pdata->seat->x;
                  ev->cur.y = pdata->seat->y;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, evt,
                                                  event_id, EFL_EVENT_POINTER_OUT);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_out_events(eo_obj, eo_e, evt, event_id);
                  if (e->delete_me) break;
               }
          }
     }
   _evas_post_event_callback_call(eo_e, e, event_id);

   eina_list_free(copy);

   if (pdata->seat->inside)
     {
        Evas_Object *eo_obj_itr;

        event_id = _evas_object_event_new();
        ev->action = EFL_POINTER_ACTION_IN;

        EINA_LIST_FOREACH(ins, l, eo_obj_itr)
          {
             Evas_Object_Protected_Data *obj_itr = efl_data_scope_get(eo_obj_itr, EFL_CANVAS_OBJECT_CLASS);
             if (!eina_list_data_find(pdata->seat->object.in, eo_obj_itr))
               {
                  obj_pdata = _evas_object_pointer_data_get(pdata, obj_itr);
                  if (!obj_pdata)
                    {
                       ERR("Could not find the object pointer data for device %p",
                           ev->device);
                       continue;
                    }
                  if (obj_pdata->mouse_in) continue;
                  _evas_event_mouse_in_set(pdata->seat, obj_itr, 1);
                  if (e->is_frozen) continue;
                  ev->cur.x = pdata->seat->x;
                  ev->cur.y = pdata->seat->y;
                  _evas_event_havemap_adjust_f(eo_obj_itr, obj_itr, &ev->cur, obj_pdata->mouse_grabbed);
                  ev->action = EFL_POINTER_ACTION_IN;
                  evas_object_event_callback_call(eo_obj_itr, obj_itr, EVAS_CALLBACK_MOUSE_IN, evt,
                                                  event_id, EFL_EVENT_POINTER_IN);
                  if ((pdata->seat->x != ev->prev.x) &&
                      (pdata->seat->y != ev->prev.y))
                    {
                       ev->action = EFL_POINTER_ACTION_MOVE;
                       evas_object_event_callback_call(eo_obj_itr, obj_itr, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                       event_id, EFL_EVENT_POINTER_MOVE);
                    }
                  if ((obj_itr->proxy->is_proxy) &&
                      (obj_itr->proxy->src_events))
                    _evas_event_source_mouse_in_events(eo_obj_itr, eo_e, evt, event_id);
                  if (e->delete_me) break;
               }
          }
        _evas_post_event_callback_call(eo_e, e, event_id);
     }
   else
     {
        ins = eina_list_free(ins);
     }

   if (pdata->seat->mouse_grabbed == 0)
     {
        /* free our old list of ins */
        eina_list_free(pdata->seat->object.in);
        /* and set up the new one */
        pdata->seat->object.in = ins;
     }
   else
     {
        /* free our cur ins */
        eina_list_free(ins);
     }
   if (pdata->seat->inside)
     _evas_canvas_event_pointer_move_event_dispatch(e, pdata, ev->data);

   efl_unref(evt);
}

static void
_canvas_event_feed_mouse_up_internal(Evas_Public_Data *e, Efl_Input_Pointer_Data *ev)
{
   Efl_Input_Pointer *evt;
   Eina_List *l, *copy;
   Evas_Object *eo_obj;
   int event_id, b;
   Evas *eo_e;
   Evas_Pointer_Data *pdata;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL) |
         _efl_input_value_mask(EFL_INPUT_VALUE_BUTTON);

   if (!e || !ev) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   pdata = _evas_pointer_data_by_device_get(e, ev->device);
   if (!pdata) return;

   b = ev->button;
   DBG("ButtonEvent:up time=%u x=%d y=%d button=%d downs=%d",
       ev->timestamp, pdata->seat->x, pdata->seat->y, b, pdata->seat->downs);
   if ((b < 1) || (b > 32)) return;
   if (pdata->seat->downs <= 0) return;

   pdata->button &= ~(1u << (b - 1));
   pdata->seat->downs--;

   if (e->is_frozen) return;
   e->last_timestamp = ev->timestamp;
   eo_e = e->evas;
   evt = ev->eo;

   event_id = _evas_object_event_new();

   ev->cur.x = pdata->seat->x;
   ev->cur.y = pdata->seat->y;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->tool = 0;
   ev->value_flags |= value_flags;
   if (ev->device) efl_ref(ev->device);

   _evas_walk(e);
   /* update released touch point */
   _evas_touch_point_update(eo_e, 0, pdata->seat->x, pdata->seat->y, EVAS_TOUCH_POINT_UP);
   copy = evas_event_list_copy(pdata->seat->object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Pointer_Mode pointer_mode;
        Evas_Object_Pointer_Data *obj_pdata;
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

        if (obj->delete_me) continue;
        obj_pdata = _evas_object_pointer_data_get(pdata, obj);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }
        if (((obj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) ||
             (obj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)) &&
            (obj_pdata->mouse_grabbed > 0))
          {
             obj_pdata->mouse_grabbed--;
             pdata->seat->mouse_grabbed--;
          }
        pointer_mode = obj_pdata->pointer_mode;
        if ((!e->is_frozen) &&
            (!evas_event_freezes_through(eo_obj, obj)))
          {
             ev->cur.x = pdata->seat->x;
             ev->cur.y = pdata->seat->y;
             _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
             evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_UP, evt,
                                             event_id, EFL_EVENT_POINTER_UP);
             if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
               _evas_event_source_mouse_up_events(eo_obj, eo_e, evt, pdata, event_id);
             if (e->delete_me) break;
          }
        if (pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
          {
             if (pdata->seat->nogrep > 0) pdata->seat->nogrep--;
             break;
          }
     }
   eina_list_free(copy);
   e->last_mouse_up_counter++;
   _evas_post_event_callback_call(eo_e, e, event_id);

   if (pdata->seat->mouse_grabbed == 0)
     _post_up_handle(e, evt, pdata);

   if (pdata->seat->mouse_grabbed < 0)
     {
        ERR("BUG? pdata->seat->mouse_grabbed (=%d) < 0!",
            pdata->seat->mouse_grabbed);
     }
   /* remove released touch point from the touch point list */
   _evas_touch_point_remove(eo_e, 0);
   _evas_unwalk(e);

   if (ev->device) efl_unref(ev->device);
}

static void
_canvas_event_feed_mouse_updown(Eo *eo_e, int b, Evas_Button_Flags flags,
                                unsigned int timestamp, const void *data,
                                Eina_Bool down, Efl_Input_Device *device)
{
   Efl_Input_Pointer_Data *ev = NULL;
   Efl_Input_Pointer *evt;
   Evas_Public_Data *e;

   e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   if (!e) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   evt = efl_input_pointer_instance_get( eo_e, (void **) &ev);
   if (!ev) return;

   ev->data = (void *) data;
   ev->timestamp = timestamp;
   ev->device = efl_ref(device ? device : _evas_event_legacy_device_get(eo_e, EINA_TRUE));
   ev->action = down ? EFL_POINTER_ACTION_DOWN : EFL_POINTER_ACTION_UP;
   ev->button = b;
   ev->button_flags = flags;
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

   efl_unref(evt);
}

static void
_canvas_event_feed_mouse_updown_legacy(Eo *eo_e, int b, Evas_Button_Flags flags,
                                       unsigned int timestamp, const void *data,
                                       Eina_Bool down)
{
   _canvas_event_feed_mouse_updown(eo_e, b, flags, timestamp, data, down, NULL);
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
   Evas_Pointer_Data *pdata;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL);

   if (!e || !ev) return;
   if (e->is_frozen) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   pdata = _evas_pointer_data_by_device_get(e, ev->device);
   if (!pdata) return;

   save = *ev;
   eo_e = e->evas;
   _evas_walk(e);

   flags = evas_event_default_flags_get(eo_e);
   evas_event_default_flags_set(eo_e, (flags | EVAS_EVENT_FLAG_ON_HOLD));

   for (i = 0; i < 32; i++)
     {
        if ((pdata->button & (1u << i)))
          _canvas_event_feed_mouse_updown(eo_e, i + 1, 0, ev->timestamp, ev->data, 0, ev->device);
     }

   ev->action = EFL_POINTER_ACTION_CANCEL;
   ev->value_flags |= value_flags;
   ev->event_flags = flags;
   EINA_LIST_FOREACH_SAFE(e->touch_points, l, ll, point)
     {
        if ((point->state == EVAS_TOUCH_POINT_DOWN) ||
            (point->state == EVAS_TOUCH_POINT_MOVE) ||
            (point->state == EVAS_TOUCH_POINT_STILL))
          {
             ev->tool = point->id;
             ev->cur.x = point->x;
             ev->cur.y = point->y;
             ev->prev = ev->cur;
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

   evt = efl_input_pointer_instance_get( eo_e, (void **) &ev);
   if (!ev) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   ev->timestamp = timestamp;
   ev->data = (void *) data;
   ev->device = efl_ref(_evas_event_legacy_device_get(e->evas, EINA_TRUE));

   _canvas_event_feed_mouse_cancel_internal(e, ev);

   efl_unref(evt);
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
   Evas_Pointer_Data *pdata;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_WHEEL_DELTA) |
         _efl_input_value_mask(EFL_INPUT_VALUE_WHEEL_HORIZONTAL);

   if (e->is_frozen) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   pdata = _evas_pointer_data_by_device_get(e, pe->device);
   if (!pdata) return;
   e->last_timestamp = pe->timestamp;

   event_id = _evas_object_event_new();

   evt = efl_duplicate(pe->eo);
   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   if (!ev) return;

   // adjust missing data based on evas state
   ev->cur.x = pdata->seat->x;
   ev->cur.y = pdata->seat->y;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->action = EFL_POINTER_ACTION_WHEEL;
   ev->value_flags |= value_flags;

   _evas_walk(e);
   copy = evas_event_list_copy(pdata->seat->object.in);

   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Pointer_Data *obj_pdata;
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if (!evas_event_freezes_through(eo_obj, obj))
          {
             obj_pdata = _evas_object_pointer_data_get(pdata, obj);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }
             ev->cur.x = pdata->seat->x;
             ev->cur.y = pdata->seat->y;
             _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
             evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_WHEEL, evt,
                                             event_id, EFL_EVENT_POINTER_WHEEL);
             if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
               _evas_event_source_wheel_events(eo_obj, eo_e, evt, event_id);
             if (e->delete_me || e->is_frozen) break;
          }
     }
   eina_list_free(copy);
   _evas_post_event_callback_call(eo_e, e, event_id);

   efl_unref(evt);
   _evas_unwalk(e);
}

EAPI void
evas_event_feed_mouse_wheel(Eo *eo_e, int direction, int z, unsigned int timestamp, const void *data)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(eo_e, EVAS_CANVAS_CLASS));
   Efl_Input_Pointer_Data *ev = NULL;
   Efl_Input_Pointer *evt = efl_input_pointer_instance_get( eo_e, (void **) &ev);

   if (!ev) return;

   ev->wheel.horizontal = !!direction;
   ev->wheel.z = z;
   ev->timestamp = timestamp;
   ev->data = (void *) data;
   ev->device = efl_ref(_evas_event_legacy_device_get(eo_e, EINA_TRUE));

   _canvas_event_feed_mouse_wheel_internal(eo_e, ev);
   efl_unref(evt);
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
   Evas_Pointer_Data *pdata;
   Evas_Object_Pointer_Data *obj_pdata;

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
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   if (ev->device)
     {
        pdata = _evas_pointer_data_by_device_get(e, ev->device);
        if (!pdata) return;
     }
   else
     {
        Evas_Pointer_Seat *pseat;
        if (!e->seats) return;
        pseat = EINA_INLIST_CONTAINER_GET(e->seats, Evas_Pointer_Seat);
        pseat->inside = 1;
        e->last_timestamp = ev->timestamp;
        pseat->prev.x = pseat->x;
        pseat->prev.y = pseat->y;

        // new pos
        pseat->x = ev->cur.x;
        pseat->y = ev->cur.y;
        return;
     }

   eo_e = e->evas;
   e->last_timestamp = ev->timestamp;

   // prev pos
   pdata->seat->prev.x = pdata->seat->x;
   pdata->seat->prev.y = pdata->seat->y;
   px = ev->prev.x = pdata->seat->x;
   py = ev->prev.y = pdata->seat->y;

   // new pos
   x = pdata->seat->x = ev->cur.x;
   y = pdata->seat->y = ev->cur.y;
   point = ev->cur;

   if ((!pdata->seat->inside) && (pdata->seat->mouse_grabbed == 0)) return;

   evt = ev->eo;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->pressed_buttons = pdata->button;
   ev->tool = 0;
   ev->value_flags |= value_flags;
   if (ev->device) efl_ref(ev->device);

   _evas_walk(e);
   /* update moved touch point */
   if ((px != x) || (py != y))
     _evas_touch_point_update(eo_e, 0, pdata->seat->x, pdata->seat->y, EVAS_TOUCH_POINT_MOVE);
   /* if our mouse button is grabbed to any objects */
   if (pdata->seat->mouse_grabbed > 0)
     {
        Eina_List *outs = NULL;

        /* Send normal mouse move events */
        ev->action = EFL_POINTER_ACTION_MOVE;

        event_id = _evas_object_event_new();

        /* go thru old list of in objects */
        copy = evas_event_list_copy(pdata->seat->object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             obj_pdata = _evas_object_pointer_data_get(pdata, obj);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }
             if ((!e->is_frozen) &&
                 _evas_event_object_pointer_allow(eo_obj, obj, obj_pdata) &&
                 (!evas_object_is_source_invisible(eo_obj, obj) ||
                  obj_pdata->mouse_grabbed))
               {
                  if ((px != x) || (py != y))
                    {
                       ev->cur = point;
                       _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
                       evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                       event_id, EFL_EVENT_POINTER_MOVE);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_move_events(eo_obj, eo_e, evt, pdata, event_id);
                    }
               }
             else
                outs = eina_list_append(outs, eo_obj);
             if ((obj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN) &&
                 (pdata->seat->nogrep > 0))
               {
                  eina_list_free(copy);
                  eina_list_free(outs);
                  nogrep_obj = eo_obj;
                  goto nogrep;
               }
             if (e->delete_me) break;
          }
        eina_list_free(copy);
        _evas_post_event_callback_call(eo_e, e, event_id);


        /* Send mouse out events */
        ev->action = EFL_POINTER_ACTION_OUT;

        event_id = _evas_object_event_new();

        EINA_LIST_FREE(outs, eo_obj)
          {
             obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             obj_pdata = _evas_object_pointer_data_get(pdata, obj);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }
             if ((obj_pdata->mouse_grabbed == 0) && (!e->delete_me))
               {
                  if (!obj_pdata->mouse_in) continue;
                  _evas_event_mouse_in_set(pdata->seat, obj, 0);
                  if (obj->delete_me || e->is_frozen) continue;
                  pdata->seat->object.in = eina_list_remove(pdata->seat->object.in, eo_obj);
                  ev->cur = point;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, evt,
                                                  event_id, EFL_EVENT_POINTER_OUT);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_out_events(eo_obj, eo_e, evt, event_id);
               }
          }
        _evas_post_event_callback_call(eo_e, e, event_id);
     }
   else
     {
        Eina_List *ins;

        event_id = _evas_object_event_new();

        /* get all new in objects */
        ins = evas_event_objects_event_list(eo_e, NULL, x, y);
        /* go thru old list of in objects */
        copy = evas_event_list_copy(pdata->seat->object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             if (!obj) continue;
             obj_pdata = _evas_object_pointer_data_get(pdata, obj);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }

             /* if its under the pointer and its visible and its in the new */
             /* in list */
             // FIXME: i don't think we need this
             //	     evas_object_clip_recalc(eo_obj);
             if ((!e->is_frozen) &&
                 evas_object_is_in_output_rect(eo_obj, obj, x, y, 1, 1) &&
                 _evas_event_object_pointer_allow(eo_obj, obj, obj_pdata) &&
                 _evas_event_object_pointer_allow_precise(eo_obj, obj, x, y, ins) &&
                 (!evas_object_is_source_invisible(eo_obj, obj) ||
                  obj_pdata->mouse_grabbed))
               {
                  if ((px != x) || (py != y))
                    {
                       ev->cur = point;
                       _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
                       ev->action = EFL_POINTER_ACTION_MOVE;
                       evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                       event_id, EFL_EVENT_POINTER_MOVE);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_move_events(eo_obj, eo_e, evt, pdata, event_id);
                    }
               }
             /* otherwise it has left the object */
             else if (obj_pdata->mouse_in)
               {
                  _evas_event_mouse_in_set(pdata->seat, obj, 0);
                  if (e->is_frozen) continue;
                  ev->cur = point;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
                  ev->action = EFL_POINTER_ACTION_OUT;
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, evt,
                                                  event_id, EFL_EVENT_POINTER_OUT);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_out_events(eo_obj, eo_e, evt, event_id);
                  if (e->delete_me) break;
               }
          }
        eina_list_free(copy);
        _evas_post_event_callback_call(eo_e, e, event_id);

        /* new event id for mouse in */
        event_id = _evas_object_event_new();

        /* go thru our current list of ins */
        EINA_LIST_FOREACH(ins, l, eo_obj)
          {
             obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             /* if its not in the old list of ins send an enter event */
             if (!eina_list_data_find(pdata->seat->object.in, eo_obj))
               {
                  obj_pdata = _evas_object_pointer_data_get(pdata, obj);
                  if (!obj_pdata)
                    {
                       ERR("Could not find the object pointer data for device %p",
                           ev->device);
                       continue;
                    }
                  if (!obj_pdata->mouse_in)
                    {
                       _evas_event_mouse_in_set(pdata->seat, obj, 1);
                       if (e->is_frozen) continue;
                       ev->cur = point;
                       _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
                       ev->action = EFL_POINTER_ACTION_IN;
                       evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_IN, evt,
                                                       event_id, EFL_EVENT_POINTER_IN);
                        if ((point.x != ev->prev.x) &&
                            (point.y != ev->prev.y))
                          {
                             ev->action = EFL_POINTER_ACTION_MOVE;
                             evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                             event_id, EFL_EVENT_POINTER_MOVE);
                          }
                       if ((obj->proxy->is_proxy) &&
                           (obj->proxy->src_events))
                         _evas_event_source_mouse_in_events(eo_obj, eo_e, evt, event_id);
                       if (e->delete_me) break;
                    }
               }
          }
        if (pdata->seat->mouse_grabbed == 0)
          {
             /* free our old list of ins */
             eina_list_free(pdata->seat->object.in);
             /* and set up the new one */
             pdata->seat->object.in = ins;
          }
        else
          {
             /* free our cur ins */
             eina_list_free(ins);
          }
        _evas_post_event_callback_call(eo_e, e, event_id);
     }

nogrep:
   if (nogrep_obj)
     {
        Eina_List *ins = NULL, *newin = NULL, *lst = NULL;
        Evas_Object *eo_below_obj;

        event_id = _evas_object_event_new();

        /* go thru old list of in objects */
        copy = evas_event_list_copy(pdata->seat->object.in);
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
                                                      EINA_INLIST_GET(below_obj), NULL, NULL,
                                                      pdata->seat->x, pdata->seat->y,
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
             obj_pdata = _evas_object_pointer_data_get(pdata, obj);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }
             /* if its under the pointer and its visible and its in the new */
             /* in list */
             // FIXME: i don't think we need this
             //	     evas_object_clip_recalc(eo_obj);
             if ((!e->is_frozen) &&
                 evas_object_is_in_output_rect(eo_obj, obj, x, y, 1, 1) &&
                 _evas_event_object_pointer_allow(eo_obj, obj, obj_pdata) &&
                 _evas_event_object_pointer_allow_precise(eo_obj, obj, x, y, newin) &&
                 (!evas_object_is_source_invisible(eo_obj, obj) ||
                  obj_pdata->mouse_grabbed))
               {
                  if ((px != x) || (py != y))
                    {
                       ev->cur = point;
                       _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
                       ev->action = EFL_POINTER_ACTION_MOVE;
                       evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                       event_id, EFL_EVENT_POINTER_MOVE);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_move_events(eo_obj, eo_e, evt, pdata, event_id);
                    }
               }
             else
               {
                  /* otherwise it has left the object */
                  if (!obj_pdata->mouse_in) continue;
                  _evas_event_mouse_in_set(pdata->seat, obj, 0);
                  if (e->is_frozen) continue;
                  ev->cur = point;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
                  ev->action = EFL_POINTER_ACTION_OUT;
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, evt,
                                                  event_id, EFL_EVENT_POINTER_OUT);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_out_events(eo_obj, eo_e, evt, event_id);
               }
             if (e->delete_me) break;
          }
        eina_list_free(copy);
        _evas_post_event_callback_call(eo_e, e, event_id);

        event_id = _evas_object_event_new();

        /* go thru our current list of ins */
        EINA_LIST_FOREACH(newin, l, eo_obj)
          {
             obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             /* if its not in the old list of ins send an enter event */
             if (!eina_list_data_find(pdata->seat->object.in, eo_obj))
               {
                  obj_pdata = _evas_object_pointer_data_get(pdata, obj);
                  if (!obj_pdata)
                    {
                       ERR("Could not find the object pointer data for device %p",
                           ev->device);
                       continue;
                    }
                  if (obj_pdata->mouse_in) continue;
                  _evas_event_mouse_in_set(pdata->seat, obj, 1);
                  if (e->is_frozen) continue;
                  ev->cur = point;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
                  ev->action = EFL_POINTER_ACTION_IN;
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_IN, evt,
                                                  event_id, EFL_EVENT_POINTER_IN);
                  if ((point.x != ev->prev.x) &&
                      (point.y != ev->prev.y))
                    {
                       ev->action = EFL_POINTER_ACTION_MOVE;
                       evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                       event_id, EFL_EVENT_POINTER_MOVE);
                    }
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_in_events(eo_obj, eo_e, evt, event_id);
                  if (e->delete_me) break;
               }
          }
        /* free our old list of ins */
        eina_list_free(pdata->seat->object.in);
        /* and set up the new one */
        pdata->seat->object.in = newin;

        _evas_post_event_callback_call(eo_e, e, event_id);
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

   evt = efl_input_pointer_instance_get( eo_e, (void **) &ev);
   if (!ev) return;

   ev->data = (void *) data;
   ev->timestamp = timestamp;
   ev->device = efl_ref(_evas_event_legacy_device_get(eo_e, EINA_TRUE));
   ev->cur.x = x;
   ev->cur.y = y;

   _canvas_event_feed_mouse_move_internal(e, ev);

   efl_unref(evt);
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
   Evas_Pointer_Data *pdata;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_BUTTONS_PRESSED) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL);

   if (!e || !ev) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   if (ev->device)
     {
        pdata = _evas_pointer_data_by_device_get(e, ev->device);
        if (!pdata) return;
     }
   else
     {
        Evas_Pointer_Seat *pseat;
        if (!e->seats) return;
        pseat = EINA_INLIST_CONTAINER_GET(e->seats, Evas_Pointer_Seat);
        pseat->inside = 1;
        return;
     }

   pdata->seat->inside = 1;
   if (e->is_frozen) return;

   e->last_timestamp = ev->timestamp;
   if (pdata->seat->mouse_grabbed != 0) return;

   evt = ev->eo;
   ev->action = EFL_POINTER_ACTION_IN;
   ev->pressed_buttons = pdata->button;
   ev->cur.x = pdata->seat->x;
   ev->cur.y = pdata->seat->y;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->value_flags |= value_flags;
   if (ev->device) efl_ref(ev->device);

   event_id = _evas_object_event_new();

   _evas_walk(e);
   /* get new list of ins */
   ins = evas_event_objects_event_list(eo_e, NULL, pdata->seat->x, pdata->seat->y);
   EINA_LIST_FOREACH(ins, l, eo_obj)
     {
        Evas_Object_Pointer_Data *obj_pdata;
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if (!eina_list_data_find(pdata->seat->object.in, eo_obj))
          {
             obj_pdata = _evas_object_pointer_data_get(pdata, obj);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }
             if (obj_pdata->mouse_in) continue;
             _evas_event_mouse_in_set(pdata->seat, obj, 1);
             ev->cur.x = pdata->seat->x;
             ev->cur.y = pdata->seat->y;
             _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
             ev->action = EFL_POINTER_ACTION_IN;
             evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_IN, evt,
                                             event_id, EFL_EVENT_POINTER_IN);
             if ((pdata->seat->x != ev->prev.x) &&
                 (pdata->seat->y != ev->prev.y))
               {
                  ev->action = EFL_POINTER_ACTION_MOVE;
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_MOVE, evt,
                                                  event_id, EFL_EVENT_POINTER_MOVE);
               }
             if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
               _evas_event_source_mouse_in_events(eo_obj, eo_e, evt, event_id);
             if (e->delete_me || e->is_frozen) break;
          }
     }
   /* free our old list of ins */
   eina_list_free(pdata->seat->object.in);
   /* and set up the new one */
   pdata->seat->object.in = ins;
   _evas_post_event_callback_call(eo_e, e, event_id);
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
   Evas_Pointer_Data *pdata;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_BUTTONS_PRESSED) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL);

   if (!e || !ev) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);
   if (ev->device)
     {
        pdata = _evas_pointer_data_by_device_get(e, ev->device);
        if (!pdata) return;
     }
   else
     {
        Evas_Pointer_Seat *pseat;
        if (!e->seats) return;
        pseat = EINA_INLIST_CONTAINER_GET(e->seats, Evas_Pointer_Seat);
        pseat->inside = 0;
        return;
     }
   pdata->seat->inside = 0;

   if (e->is_frozen) return;
   e->last_timestamp = ev->timestamp;

   event_id = _evas_object_event_new();

   evt = ev->eo;
   ev->action = EFL_POINTER_ACTION_OUT;
   ev->pressed_buttons = pdata->button;
   ev->cur.x = pdata->seat->x;
   ev->cur.y = pdata->seat->y;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->value_flags |= value_flags;
   if (ev->device) efl_ref(ev->device);

   _evas_walk(e);
   /* if our mouse button is inside any objects */
   /* go thru old list of in objects */
   copy = evas_event_list_copy(pdata->seat->object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Pointer_Data *obj_pdata;
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if (obj->delete_me) continue;
        obj_pdata = _evas_object_pointer_data_get(pdata, obj);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }
        if (!obj_pdata->mouse_in) continue;
        _evas_event_mouse_in_set(pdata->seat, obj, 0);
        ev->cur.x = pdata->seat->x;
        ev->cur.y = pdata->seat->y;
        _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, evt,
                                        event_id, EFL_EVENT_POINTER_OUT);
        if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
          _evas_event_source_mouse_out_events(eo_obj, eo_e, evt, event_id);
        if (e->delete_me || e->is_frozen) break;
        obj_pdata->mouse_grabbed = 0;
     }
   eina_list_free(copy);

   /* free our old list of ins */
   pdata->seat->object.in =  eina_list_free(pdata->seat->object.in);
   pdata->seat->mouse_grabbed = 0;
   _evas_post_event_callback_call(eo_e, e, event_id);
   _evas_unwalk(e);

   if (ev->device) efl_unref(ev->device);
}

static void
_canvas_event_feed_mouse_inout_legacy(Eo *eo_e, unsigned int timestamp,
                                      const void *data, Eina_Bool in)
{
   Efl_Input_Pointer_Data *ev = NULL;
   Efl_Input_Pointer *evt;

   evt = efl_input_pointer_instance_get( eo_e, (void **) &ev);
   if (!ev) return;

   ev->timestamp = timestamp;
   ev->data = (void *) data;
   ev->device = efl_ref(_evas_event_legacy_device_get(eo_e, EINA_TRUE));

   if (in)
     _canvas_event_feed_mouse_in_internal(eo_e, ev);
   else
     _canvas_event_feed_mouse_out_internal(eo_e, ev);

   efl_unref(evt);
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
   Evas_Pointer_Data *pdata;
   Evas_Object_Pointer_Data *obj_pdata;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL) |
         _efl_input_value_mask(EFL_INPUT_VALUE_BUTTON);

   if (!e || !ev) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   pdata = _evas_pointer_data_by_device_get(e, ev->device);
   if (!pdata) return;
   eo_e = e->evas;
   DBG("ButtonEvent:multi down time=%u x=%.1f y=%.1f button=%d downs=%d",
       ev->timestamp, ev->cur.x, ev->cur.y, ev->tool, pdata->seat->downs);
   pdata->seat->downs++;
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
   if (pdata->seat->mouse_grabbed == 0)
     {
        if (pdata->seat->downs > 1) addgrab = pdata->seat->downs - 1;
     }
   copy = evas_event_list_copy(pdata->seat->object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        obj_pdata = _evas_object_pointer_data_get(pdata, obj);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }
        if (obj_pdata->pointer_mode != EVAS_OBJECT_POINTER_MODE_NOGRAB)
          {
             obj_pdata->mouse_grabbed += addgrab + 1;
             pdata->seat->mouse_grabbed += addgrab + 1;
          }
     }
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        obj_pdata = _evas_object_pointer_data_get(pdata, obj);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }
        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MULTI_DOWN, evt,
                                        event_id, EFL_EVENT_FINGER_DOWN);
        if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
          _evas_event_source_multi_down_events(obj, e, evt, pdata, event_id);
        if (e->delete_me || e->is_frozen) break;
     }
   eina_list_free(copy);

   _evas_post_event_callback_call(eo_e, e, event_id);
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
   Evas_Pointer_Data *pdata;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL);

   if (!e || !ev) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   pdata = _evas_pointer_data_by_device_get(e, ev->device);
   if (!pdata) return;
   eo_e = e->evas;
   DBG("ButtonEvent:multi up time=%u x=%.1f y=%.1f device=%d downs=%d",
       ev->timestamp, ev->cur.x, ev->cur.y, ev->tool, pdata->seat->downs);
   if (pdata->seat->downs <= 0) return;
   pdata->seat->downs--;
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
   copy = evas_event_list_copy(pdata->seat->object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Pointer_Data *obj_pdata;
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        obj_pdata = _evas_object_pointer_data_get(pdata, obj);
        if (!obj_pdata)
          {
             ERR("Could not find the object pointer data for device %p",
                 ev->device);
             continue;
          }
        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
        if (((obj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) ||
            (obj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)) &&
            (obj_pdata->mouse_grabbed > 0))
          {
             obj_pdata->mouse_grabbed--;
             pdata->seat->mouse_grabbed--;
          }
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MULTI_UP, evt,
                                        event_id, EFL_EVENT_FINGER_UP);
        if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
          _evas_event_source_multi_up_events(obj, e, evt, pdata, event_id);
        if (e->delete_me || e->is_frozen) break;
     }
   eina_list_free(copy);
   if (pdata->seat->mouse_grabbed == 0)
     {
        _post_up_handle(e, evt, pdata);
        _evas_post_event_callback_call(eo_e, e, event_id);
     }

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

   evt = efl_input_pointer_instance_get( eo_e, (void **) &ev);
   if (!e || !ev) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   if (EINA_DBL_EQ(fx, 0.0)) fx = x;
   if (EINA_DBL_EQ(fy, 0.0)) fy = y;

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
   ev->device = efl_ref(_evas_event_legacy_device_get(eo_e, EINA_TRUE));

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

   efl_unref(evt);
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
   Evas_Pointer_Data *pdata;
   Evas_Object_Pointer_Data *obj_pdata;

   /* FIXME: Add previous x,y information (from evas touch point list) */
   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP) |
         _efl_input_value_mask(EFL_INPUT_VALUE_X) |
         _efl_input_value_mask(EFL_INPUT_VALUE_Y) |
         _efl_input_value_mask(EFL_INPUT_VALUE_TOOL);

   if (!e || !ev) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   pdata = _evas_pointer_data_by_device_get(e, ev->device);
   if (!pdata) return;
   eo_e = e->evas;
   event_id = _evas_object_event_new();

   if (e->is_frozen) return;
   e->last_timestamp = ev->timestamp;

   if ((!pdata->seat->inside) && (pdata->seat->mouse_grabbed == 0)) return;

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
   if (pdata->seat->mouse_grabbed > 0)
     {
        /* go thru old list of in objects */
        copy = evas_event_list_copy(pdata->seat->object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             obj_pdata = _evas_object_pointer_data_get(pdata, obj);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }
             if (_evas_event_object_pointer_allow(eo_obj, obj, obj_pdata) &&
                 (!evas_object_is_source_invisible(eo_obj, obj) ||
                  obj_pdata->mouse_grabbed))
               {
                  ev->cur = point;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MULTI_MOVE, evt,
                                                  event_id, EFL_EVENT_FINGER_MOVE);
                  if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
                    _evas_event_source_multi_move_events(obj, e, evt, pdata, event_id);

                  if (e->delete_me || e->is_frozen) break;
               }
          }
        eina_list_free(copy);
        _evas_post_event_callback_call(eo_e, e, event_id);
     }
   else
     {
        Eina_List *ins;

        /* get all new in objects */
        ins = evas_event_objects_event_list(eo_e, NULL, ev->cur.x, ev->cur.y);
        /* go thru old list of in objects */
        copy = evas_event_list_copy(pdata->seat->object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             obj_pdata = _evas_object_pointer_data_get(pdata, obj);
             if (!obj_pdata)
               {
                  ERR("Could not find the object pointer data for device %p",
                      ev->device);
                  continue;
               }
             /* if its under the pointer and its visible and its in the new */
             /* in list */
             // FIXME: i don't think we need this
             //	     evas_object_clip_recalc(eo_obj);
             if (evas_object_is_in_output_rect(eo_obj, obj, ev->cur.x, ev->cur.y, 1, 1) &&
                 _evas_event_object_pointer_allow(eo_obj, obj, obj_pdata) &&
                 _evas_event_object_pointer_allow_precise(eo_obj, obj, ev->cur.x, ev->cur.y, ins) &&
                 (!evas_object_is_source_invisible(eo_obj, obj) ||
                  obj_pdata->mouse_grabbed))
               {
                  ev->cur = point;
                  _evas_event_havemap_adjust_f(eo_obj, obj, &ev->cur, obj_pdata->mouse_grabbed);
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MULTI_MOVE, evt,
                                                  event_id, EFL_EVENT_FINGER_MOVE);
                  if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
                    _evas_event_source_multi_move_events(obj, e, evt, pdata, event_id);
               }
             if (e->delete_me || e->is_frozen) break;
          }
        eina_list_free(copy);
        if (pdata->seat->mouse_grabbed == 0)
          {
             /* free our old list of ins */
             eina_list_free(pdata->seat->object.in);
             /* and set up the new one */
             pdata->seat->object.in = ins;
          }
        else
          {
             /* free our cur ins */
             eina_list_free(ins);
          }
        _evas_post_event_callback_call(eo_e, e, event_id);
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
_key_event_dispatch(Evas_Public_Data *e, void *event_info,
                    Efl_Input_Device *device,
                    const Efl_Event_Description *efl_event_desc,
                    Evas_Callback_Type evas_event_type, int event_id)
{
   Eo *focused;

   if (!device)
     device = e->default_seat;
   else
     {
        const char *name = efl_name_get(device);

        device = efl_input_device_seat_get(device);
        if (!device)
          {
             ERR("Could not find the parent seat from device name '%s'. Using default seat instead", name);
             device = e->default_seat;
          }
     }

   focused = eina_hash_find(e->focused_objects, &device);

   if (!focused)
     return;

   Evas_Object_Protected_Data *focused_obj =
     efl_data_scope_get(focused, EFL_CANVAS_OBJECT_CLASS);

   if (!focused_obj)
     {
        WRN("No element focused");
        return;
     }

   if (!e->is_frozen && !evas_event_freezes_through(focused, focused_obj))
     {
        evas_object_event_callback_call(focused, focused_obj,
                                        evas_event_type, event_info,
                                        event_id, efl_event_desc);
     }
}

static void
_canvas_event_feed_key_down_internal(Evas_Public_Data *e, Efl_Input_Key_Data *ev)
{
   Eina_Bool exclusive = EINA_FALSE;
   Efl_Input_Pointer *evt;
   int event_id = 0;
   Eo *eo_e;

   if (!e || !ev) return;
   if (e->is_frozen) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   eo_e = e->evas;
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
        Evas_Modifier_Mask *seat_mask, modifier_mask;
        Efl_Input_Device *seat = NULL;

        e->walking_grabs++;
        if (ev->device)
          seat = efl_input_device_seat_get(ev->device);
        seat_mask = eina_hash_find(e->modifiers.masks, &seat);
        modifier_mask = seat_mask ? *seat_mask : 0;
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
             if (((modifier_mask & g->modifiers) ||
                  (g->modifiers == modifier_mask)) &&
                 (!strcmp(ev->keyname, g->keyname)))
               {
                  if (!(modifier_mask & g->not_modifiers))
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
   if (!exclusive)
     _key_event_dispatch(e, evt, ev->device, EFL_EVENT_KEY_DOWN,
                         EVAS_CALLBACK_KEY_DOWN, event_id);
   _evas_post_event_callback_call(eo_e, e, event_id);
   _evas_unwalk(e);

   if (ev->device) efl_unref(ev->device);
}

static void
_canvas_event_feed_key_up_internal(Evas_Public_Data *e, Efl_Input_Key_Data *ev)
{
   Eina_Bool exclusive = EINA_FALSE;
   Efl_Input_Pointer *evt;
   int event_id = 0;
   Eo *eo_e;

   if (!e || !ev) return;
   if (e->is_frozen) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   eo_e = e->evas;
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
        Evas_Modifier_Mask *seat_mask, modifier_mask;
        Efl_Input_Device *seat = NULL;

        e->walking_grabs++;
        if (ev->device)
          seat = efl_input_device_seat_get(ev->device);
        seat_mask = eina_hash_find(e->modifiers.masks, &seat);
        modifier_mask = seat_mask ? *seat_mask : 0;
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
             if (((modifier_mask & g->modifiers) ||
                  (g->modifiers == modifier_mask)) &&
                 (!(modifier_mask & g->not_modifiers)) &&
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
   if (!exclusive)
     _key_event_dispatch(e, evt, ev->device, EFL_EVENT_KEY_UP,
                         EVAS_CALLBACK_KEY_UP, event_id);
   _evas_post_event_callback_call(eo_e, e, event_id);
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

   evt = efl_input_key_instance_get( eo_e, (void **) &ev);
   if (!ev) return;

   ev->keyname = (char *) keyname;
   ev->data = (void *) data;
   ev->key = key;
   ev->string = string;
   ev->compose = compose;
   ev->timestamp = timestamp;
   ev->keycode = keycode;
   ev->no_stringshare = EINA_TRUE;
   ev->device = efl_ref(_evas_event_legacy_device_get(e->evas, EINA_FALSE));

   if (down)
     _canvas_event_feed_key_down_internal(e, ev);
   else
     _canvas_event_feed_key_up_internal(e, ev);

   efl_unref(evt);
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
   Evas_Pointer_Data *pdata;

   if (e->is_frozen) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);
   e->last_timestamp = timestamp;

   event_id = _evas_object_event_new();

   evt = efl_input_hold_instance_get(eo_e, (void **) &ev);
   if (!ev) return;

   ev->hold = !!hold;
   ev->data = (void *) data;
   ev->timestamp = timestamp;
   ev->event_flags = e->default_event_flags;
   ev->device = efl_ref(_evas_event_legacy_device_get(eo_e, EINA_TRUE));

   pdata = _evas_pointer_data_by_device_get(e, ev->device);
   if (!pdata) return;


   _evas_walk(e);
   copy = evas_event_list_copy(pdata->seat->object.in);
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
   _evas_post_event_callback_call(eo_e, e, event_id);
   _evas_unwalk(e);
   _evas_object_event_new();

   efl_unref(evt);
}

void
_canvas_event_feed_axis_update_internal(Evas_Public_Data *e, Efl_Input_Pointer_Data *ev)
{
   Efl_Input_Pointer *evt;
   Eina_List *l, *copy;
   Evas_Object *eo_obj;
   int event_id = 0;
   Evas *eo_e;
   Evas_Pointer_Data *pdata;

   static const int value_flags =
         _efl_input_value_mask(EFL_INPUT_VALUE_TIMESTAMP);

   if (!e || !ev) return;
   if (e->is_frozen) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   pdata = _evas_pointer_data_by_device_get(e, ev->device);
   if (!pdata) return;
   eo_e = e->evas;
   e->last_timestamp = ev->timestamp;

   ev->action = EFL_POINTER_ACTION_AXIS;
   ev->value_flags |= value_flags;
   event_id = _evas_object_event_new();
   evt = ev->eo;

   if (ev->device) efl_ref(ev->device);

   _evas_walk(e);
   copy = evas_event_list_copy(pdata->seat->object.in);

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
   _evas_post_event_callback_call(eo_e, e, event_id);

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

   if (!e) return;
   EVAS_EVENT_FEED_SAFETY_CHECK(e);

   evt = efl_input_pointer_instance_get( eo_e, (void **) &ev);
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
             ev->raw.x = axis->value;
             ev->has_raw = EINA_TRUE;
             break;

           case EVAS_AXIS_LABEL_Y:
             if (!haswiny)
               {
                  _efl_input_value_mark(ev, EFL_INPUT_VALUE_Y);
                  y = axis->value;
               }
             ev->raw.y = axis->value;
             ev->has_raw = EINA_TRUE;
             break;

           case EVAS_AXIS_LABEL_NORMAL_X:
             ev->norm.x = axis->value;
             ev->has_norm = EINA_TRUE;
             break;

           case EVAS_AXIS_LABEL_NORMAL_Y:
             ev->norm.y = axis->value;
             ev->has_norm = EINA_TRUE;
             break;

           case EVAS_AXIS_LABEL_PRESSURE:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_PRESSURE);
             ev->pressure = axis->value;
             break;

           case EVAS_AXIS_LABEL_DISTANCE:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_DISTANCE);
             ev->distance = axis->value;
             break;

           case EVAS_AXIS_LABEL_AZIMUTH:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_AZIMUTH);
             ev->azimuth = axis->value;
             break;

           case EVAS_AXIS_LABEL_TILT:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_TILT);
             ev->tilt = axis->value;
             break;

           case EVAS_AXIS_LABEL_TWIST:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_TWIST);
             ev->twist = axis->value;
             break;

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
   ev->device = efl_ref(_evas_event_legacy_device_get(eo_e, EINA_TRUE)); // FIXME
   (void) device;

   _canvas_event_feed_axis_update_internal(e, ev);

   efl_unref(evt);
}

static void
_feed_mouse_move_eval_internal(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Public_Data *evas = obj->layer->evas;
   Eina_Bool in_output_rect;
   Evas_Pointer_Data *pdata = _evas_pointer_data_by_device_get(evas, NULL);

   if (!pdata) return;
   in_output_rect = evas_object_is_in_output_rect(eo_obj, obj, pdata->seat->x,
                                                  pdata->seat->y, 1, 1);
   if ((in_output_rect) &&
       ((!obj->precise_is_inside) || (evas_object_is_inside(eo_obj, obj,
                                                            pdata->seat->x,
                                                            pdata->seat->y))))
     {
        _canvas_event_feed_mouse_move_legacy(evas->evas, evas,
                                             pdata->seat->x, pdata->seat->y,
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
_efl_canvas_object_freeze_events_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
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
_efl_canvas_object_pass_events_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
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
_efl_canvas_object_repeat_events_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->repeat_events;
}

EOLIAN void
_efl_canvas_object_propagate_events_set(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Eina_Bool prop)
{
   obj->no_propagate = !prop;
}

EOLIAN Eina_Bool
_efl_canvas_object_propagate_events_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return !(obj->no_propagate);
}

EOLIAN Eina_Bool
_efl_canvas_object_pointer_mode_by_device_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Efl_Input_Device *dev, Evas_Object_Pointer_Mode setting)
{
   int addgrab;
   Evas_Object *cobj;
   const Eina_List *l;
   Evas_Pointer_Data *pdata;
   Evas_Object_Pointer_Data *obj_pdata;

   pdata = _evas_pointer_data_by_device_get(obj->layer->evas, dev);
   if (!pdata) return EINA_FALSE;

   obj_pdata = _evas_object_pointer_data_get(pdata, obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj_pdata, EINA_FALSE);

   /* ignore no-ops */
   if (obj_pdata->pointer_mode == setting) return EINA_FALSE;

   /* adjust by number of pointer down events */
   addgrab = pdata->seat->downs;
   switch (obj_pdata->pointer_mode)
     {
      /* nothing needed */
      case EVAS_OBJECT_POINTER_MODE_NOGRAB: break;
      /* decrement canvas nogrep (NO Grab/REPeat) counter */
      case EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN:
        if (obj_pdata->mouse_grabbed)
          pdata->seat->nogrep--;
        /* fall through */
      /* remove related grabs from canvas and object */
      case EVAS_OBJECT_POINTER_MODE_AUTOGRAB:
        if (obj_pdata->mouse_grabbed)
          {
             pdata->seat->mouse_grabbed -= obj_pdata->mouse_grabbed;
             obj_pdata->mouse_grabbed = 0;
          }
     }
   /* adjustments for new mode */
   switch (setting)
     {
      /* nothing needed */
      case EVAS_OBJECT_POINTER_MODE_NOGRAB: break;
      /* increment canvas nogrep (NO Grab/REPeat) counter */
      case EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN:
        pdata->seat->nogrep++;
        /* having nogrep set indicates that any object following it in
         * the pointer.object.in list will not be receiving events, meaning
         * that they will fail to unset any existing grabs/flags. unset them
         * now to avoid breaking the canvas
         */
        EINA_LIST_FOREACH(pdata->seat->object.in, l, cobj)
          {
             Evas_Object_Protected_Data *cobj_data;

             /* skip to the current object */
             if (cobj != eo_obj) continue;
             /* only change objects past it */
             EINA_LIST_FOREACH(l->next, l, cobj)
               {
                  Evas_Object_Pointer_Data *cobj_pdata;
                  cobj_data = efl_data_scope_get(cobj, EFL_CANVAS_OBJECT_CLASS);
                  cobj_pdata = _evas_object_pointer_data_get(pdata, cobj_data);
                  if (!cobj_pdata) continue;
                  if (!cobj_pdata->mouse_grabbed) continue;
                  cobj_pdata->mouse_grabbed -= addgrab;
                  pdata->seat->mouse_grabbed -= addgrab;
                  if (cobj_pdata->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
                    pdata->seat->nogrep--;
               }
             break;
          }
        /* fall through */
      /* add all button grabs to this object */
      case EVAS_OBJECT_POINTER_MODE_AUTOGRAB:
        obj_pdata->mouse_grabbed += addgrab;
        pdata->seat->mouse_grabbed += addgrab;
     }
   obj_pdata->pointer_mode = setting;
   return EINA_TRUE;
}

EOLIAN Evas_Object_Pointer_Mode
_efl_canvas_object_pointer_mode_by_device_get(const Eo *eo_obj EINA_UNUSED,
                                              Evas_Object_Protected_Data *obj,
                                              Efl_Input_Device *dev)
{

   Evas_Pointer_Data *pdata;
   Evas_Object_Pointer_Data *obj_pdata;

   pdata = _evas_pointer_data_by_device_get(obj->layer->evas, dev);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pdata, EVAS_OBJECT_POINTER_MODE_AUTOGRAB);

   obj_pdata = _evas_object_pointer_data_get(pdata, obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj_pdata, EVAS_OBJECT_POINTER_MODE_AUTOGRAB);
   return obj_pdata->pointer_mode;
}

EOLIAN Eina_Bool
_efl_canvas_object_pointer_mode_set(Eo *eo_obj, Evas_Object_Protected_Data *obj,
                                    Evas_Object_Pointer_Mode setting)
{
   return _efl_canvas_object_pointer_mode_by_device_set(eo_obj, obj, NULL, setting);
}

EOLIAN Evas_Object_Pointer_Mode
_efl_canvas_object_pointer_mode_get(const Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   return _efl_canvas_object_pointer_mode_by_device_get(eo_obj, obj, NULL);
}

EOLIAN Eina_Bool
_efl_canvas_object_efl_canvas_pointer_pointer_inside_get(const Eo *eo_obj,
                                                         Evas_Object_Protected_Data *obj,
                                                         Efl_Input_Device *pointer)
{
   Evas_Object_Protected_Data *in, *parent;
   Eo *eo_in, *eo_parent;
   Eina_List *l;
   Evas_Object_Pointer_Data *obj_pdata;
   Evas_Pointer_Data *pdata;

   EVAS_OBJECT_DATA_ALIVE_CHECK(obj, EINA_FALSE);

   if (!pointer)
     pointer = obj->layer->evas->default_mouse;

   if (!pointer) return EINA_FALSE;

   pdata = _evas_pointer_data_by_device_get(obj->layer->evas, pointer);
   if (!pdata) return EINA_FALSE;
   obj_pdata = _evas_object_pointer_data_get(pdata, obj);
   if (!obj_pdata) return EINA_FALSE;
   if (!obj->is_smart)
     return obj_pdata->mouse_in;

   /* This is to keep the legacy APIs evas_object_pointer_inside_by_device_get() & 
    * evas_object_pointer_inside_get() old behaviour. */
   if (obj->is_pointer_inside_legacy) return EINA_FALSE;

   /* For smart objects, this is a bit expensive obj->mouse_in will not be set.
    * Alternatively we could count the number of in and out events propagated
    * to the smart object, assuming they always match. */
   EINA_LIST_FOREACH(pdata->seat->object.in, l, eo_in)
     {
        if (EINA_UNLIKELY(eo_in == eo_obj))
          return EINA_TRUE;

        in = EVAS_OBJECT_DATA_GET(eo_in);
        if (!EVAS_OBJECT_DATA_ALIVE(in)) continue;
        eo_parent = in->smart.parent;
        while (eo_parent)
          {
             if ((eo_parent == eo_obj) && !in->no_propagate)
               return EINA_TRUE;
             parent = EVAS_OBJECT_DATA_GET(eo_parent);
             if (!EVAS_OBJECT_DATA_ALIVE(parent)) break;
             eo_parent = parent->smart.parent;
          }
     }

   return EINA_FALSE;
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
_evas_canvas_event_down_count_by_device_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e,
                                            Efl_Input_Device *dev)
{
   Evas_Pointer_Data *pdata = _evas_pointer_data_by_device_get(e, dev);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pdata, 0);
   return pdata->seat->downs;
}

EOLIAN int
_evas_canvas_event_down_count_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return _evas_canvas_event_down_count_by_device_get(eo_e, e, NULL);
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
        ev->device = _evas_event_legacy_device_get(e->evas, EINA_TRUE);
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
        ev->device = _evas_event_legacy_device_get(e->evas, EINA_FALSE);
     }

   ev->modifiers = &e->modifiers;
   ev->locks = &e->locks;

   if (ev->pressed)
     _canvas_event_feed_key_down_internal(e, ev);
   else
     _canvas_event_feed_key_up_internal(e, ev);

   if (nodev) ev->device = NULL;
   ev->evas_done = EINA_TRUE;
}

static void
_evas_canvas_event_focus_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Device *seat = efl_input_device_get(event->info);
   Evas_Public_Data *e = data;

   if (event->desc == EFL_EVENT_FOCUS_IN)
     {
        if (eina_list_data_find(e->focused_by, seat)) return;
        e->focused_by = eina_list_append(e->focused_by, seat);
        evas_event_callback_call(e->evas, EVAS_CALLBACK_CANVAS_FOCUS_IN,
                                 event->info);
     }
   else
     {
        if (!eina_list_data_find(e->focused_by, seat)) return;
        e->focused_by = eina_list_remove(e->focused_by, seat);
        evas_event_callback_call(e->evas, EVAS_CALLBACK_CANVAS_FOCUS_OUT,
                                 event->info);
     }
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
{ EFL_EVENT_FINGER_MOVE, _evas_canvas_event_pointer_cb },
{ EFL_EVENT_FINGER_DOWN, _evas_canvas_event_pointer_cb },
{ EFL_EVENT_FINGER_UP, _evas_canvas_event_pointer_cb },
{ EFL_EVENT_KEY_DOWN, _evas_canvas_event_key_cb },
{ EFL_EVENT_KEY_UP, _evas_canvas_event_key_cb },
{ EFL_EVENT_FOCUS_IN, _evas_canvas_event_focus_cb },
{ EFL_EVENT_FOCUS_OUT, _evas_canvas_event_focus_cb })

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

void
_evas_canvas_event_pointer_move_event_dispatch(Evas_Public_Data *edata,
                                               Evas_Pointer_Data *pdata,
                                               void *data)
{
   Efl_Input_Pointer_Data *ev = NULL;
   Efl_Input_Pointer *evt;

   evt = efl_input_pointer_instance_get( edata->evas,
                                (void **) &ev);
   if (!evt) return;

   ev->data = (void *) data;
   ev->timestamp = edata->last_timestamp;
   ev->device = efl_ref(pdata->pointer);
   ev->cur.x = pdata->seat->x;
   ev->cur.y = pdata->seat->y;

   _canvas_event_feed_mouse_move_internal(edata, ev);

   efl_unref(evt);
}

void
_evas_canvas_event_pointer_in_rect_mouse_move_feed(Evas_Public_Data *edata,
                                                   Evas_Object *obj,
                                                   Evas_Object_Protected_Data *obj_data,
                                                   int w, int h,
                                                   Eina_Bool in_objects_list,
                                                   void *data)
{
   Evas_Pointer_Seat *pseat;

   EINA_INLIST_FOREACH(edata->seats, pseat)
     {
        if (!pseat->pointers) continue;
        if (!evas_object_is_in_output_rect(obj, obj_data, pseat->x,
                                           pseat->y, w, h))
          continue;
        if ((in_objects_list && eina_list_data_find(pseat->object.in, obj)) || !in_objects_list)
          {
             Evas_Pointer_Data *pdata = EINA_INLIST_CONTAINER_GET(pseat->pointers, Evas_Pointer_Data);
             _evas_canvas_event_pointer_move_event_dispatch(edata, pdata, data);
          }
     }
}

void
_evas_canvas_event_pointer_in_list_mouse_move_feed(Evas_Public_Data *edata,
                                                   Eina_List *was,
                                                   Evas_Object *obj,
                                                   Evas_Object_Protected_Data *obj_data,
                                                   int w, int h,
                                                   Eina_Bool xor_rule,
                                                   void *data)
{
   Evas_Pointer_Seat *pseat;

   EINA_INLIST_FOREACH(edata->seats, pseat)
     {
        Evas_Pointer_Data *pdata, *found = NULL;
        Eina_List *l;
        int in;

        if (!pseat->pointers) continue;
        in = evas_object_is_in_output_rect(obj, obj_data, pseat->x,
                                               pseat->y, w, h);
        EINA_LIST_FOREACH(was, l, pdata)
          if (pdata->seat == pseat)
            {
               found = pdata;
               break;
            }

        if ((xor_rule && ((in && !found) || (!in && found))) ||
            (!xor_rule && (in || found)))
          {
             if (!pdata) pdata = EINA_INLIST_CONTAINER_GET(pseat->pointers, Evas_Pointer_Data);
             _evas_canvas_event_pointer_move_event_dispatch(edata, pdata, data);
          }
     }
}

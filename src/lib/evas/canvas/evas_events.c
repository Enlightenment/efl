#define EFL_EVENT_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

static Eina_List *
_evas_event_object_list_in_get(Evas *eo_e, Eina_List *in,
                               const Eina_Inlist *list, Evas_Object *stop,
                               int x, int y, int *no_rep, Eina_Bool source);

static Eina_List *
evas_event_list_copy(Eina_List *list);

static void
_canvas_event_feed_mouse_move_internal(Eo *eo_e, Evas_Public_Data *e, int x, int y,
                                       unsigned int timestamp, const void *data,
                                       Efl_Event_Pointer *parent_pe);

static void
_canvas_event_feed_multi_up_internal(Evas *eo_e, void *_pd, int d, int x, int y,
                                     double rad, double radx, double rady, double pres, double ang,
                                     double fx, double fy, Evas_Button_Flags flags,
                                     unsigned int timestamp, const void *data,
                                     Efl_Event_Pointer *parent_pe);

/* FIXME: remove this */
static void
_evas_event_havemap_adjust(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Coord *x, Evas_Coord *y, Eina_Bool mouse_grabbed)
{
   double tx = *x, ty = *y;

   if (obj->smart.parent)
     {
        Evas_Object_Protected_Data *smart_parent_obj = efl_data_scope_get(obj->smart.parent, EFL_CANVAS_OBJECT_CLASS);
        _evas_event_havemap_adjust(obj->smart.parent, smart_parent_obj, x, y, mouse_grabbed);
     }

   if ((!obj->map->cur.usemap) || (!obj->map->cur.map) ||
       (obj->map->cur.map->count != 4))
      return;

   //FIXME: Unless map_coords_get() supports grab mode and extrapolate coords
   //outside map, this should check the return value for outside case.
   if (evas_map_coords_get(obj->map->cur.map, tx, ty, &tx, &ty, mouse_grabbed))
     {
        tx += obj->cur->geometry.x;
        ty += obj->cur->geometry.y;
        *x = tx;
        *y = ty;
     }
}

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

static inline Efl_Event *
_efl_event_create(Efl_Event *evt, Evas_Callback_Type type, void *ev,
                  Efl_Event *parentev, Efl_Event_Flags **pflags)
{
   if (!ev) return NULL;

   /* This function converts an existing evas info struct to the efl pointer
    * event. All pointers must be valid.
    *
    * See also evas_callbacks.c: _pointer_event_get()
    *
    * FIXME: evas event logic should not use legacy structs anymore... this
    * should be temporary code. Should be.
    */

#define EV_CASE(TYPE, Type, OBJTYP, objtyp) \
   case EVAS_CALLBACK_ ## TYPE: \
     if (!evt) evt = efl_event_instance_get(EFL_EVENT_ ## OBJTYP ## _CLASS, parentev, NULL); \
     efl_event_ ## objtyp ## _legacy_info_set(evt, ev, type); \
     if (pflags) *pflags = &(((Evas_Event_ ## Type *) ev)->event_flags); \
     break;

   switch (type)
     {
      EV_CASE(MOUSE_MOVE, Mouse_Move, POINTER, pointer);
      EV_CASE(MOUSE_OUT, Mouse_Out, POINTER, pointer);
      EV_CASE(MOUSE_IN, Mouse_In, POINTER, pointer);
      EV_CASE(MOUSE_DOWN, Mouse_Down, POINTER, pointer);
      EV_CASE(MOUSE_UP, Mouse_Up, POINTER, pointer);
      EV_CASE(MULTI_MOVE, Multi_Move, POINTER, pointer);
      EV_CASE(MULTI_DOWN, Multi_Down, POINTER, pointer);
      EV_CASE(MULTI_UP, Multi_Up, POINTER, pointer);
      EV_CASE(MOUSE_WHEEL, Mouse_Wheel, POINTER, pointer);
      EV_CASE(KEY_DOWN, Key_Down, KEY, key);
      EV_CASE(KEY_UP, Key_Up, KEY, key);
      EV_CASE(HOLD, Hold, HOLD, hold);

      default:
        DBG("Support for event type %d not implemented yet.", type);
        break;
     }

#undef EV_CASE

   return evt;
}

static inline const Efl_Event_Description *
_efl_event_desc_get(Evas_Callback_Type type)
{
   switch (type)
     {
      case EVAS_CALLBACK_MOUSE_IN:
        return EFL_EVENT_POINTER_IN;
      case EVAS_CALLBACK_MOUSE_OUT:
        return EFL_EVENT_POINTER_OUT;
      case EVAS_CALLBACK_MOUSE_DOWN:
      case EVAS_CALLBACK_MULTI_DOWN:
        return EFL_EVENT_POINTER_DOWN;
      case EVAS_CALLBACK_MOUSE_UP:
      case EVAS_CALLBACK_MULTI_UP:
        return EFL_EVENT_POINTER_UP;
      case EVAS_CALLBACK_MOUSE_MOVE:
      case EVAS_CALLBACK_MULTI_MOVE:
        return EFL_EVENT_POINTER_MOVE;
      case EVAS_CALLBACK_MOUSE_WHEEL:
        return EFL_EVENT_POINTER_WHEEL;
      case EVAS_CALLBACK_KEY_DOWN:
        return EFL_EVENT_KEY_DOWN;
      case EVAS_CALLBACK_KEY_UP:
        return EFL_EVENT_KEY_UP;
      case EVAS_CALLBACK_HOLD:
        return EFL_EVENT_HOLD;
      default:
        return NULL;
     }
}

#define EV_CALL(_eo_obj, _obj, _typ, _info, _id, _eoev, _parent_ev) do { \
   Efl_Event_Flags *_info_pflags = NULL; \
   _eoev = _efl_event_create(_eoev, _typ, _info, _parent_ev, &_info_pflags); \
   evas_object_event_callback_call(_eo_obj, _obj, _typ, _info, _id, \
                                   _efl_event_desc_get(_typ), _eoev); \
   if (_info_pflags) *_info_pflags = efl_event_flags_get(_eoev); \
   } while (0)
#define EV_RESET(a) do { if (a) efl_event_reset(a); } while (0)
#define EV_DEL(a) do { if (a) { efl_unref(a); } a = NULL; } while (0)

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

/* FIXME: remove this */
static void
_transform_to_src_space(Evas_Object_Protected_Data *obj, Evas_Object_Protected_Data *src, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord obj_w = obj->cur->geometry.w, obj_h = obj->cur->geometry.h;
   Evas_Coord src_w = src->cur->geometry.w, src_h = src->cur->geometry.h;
   Evas_Coord tmp_x = *x;
   Evas_Coord tmp_y = *y;

   tmp_x -= obj->cur->geometry.x;
   tmp_y -= obj->cur->geometry.y;

   if (obj_w != src_w)
     tmp_x = (Evas_Coord) ((float)tmp_x * ((float)src_w / (float)obj_w));
   if (obj_h != src_h)
     tmp_y = (Evas_Coord) ((float)tmp_y * ((float)src_h / (float)obj_h));

   tmp_x += src->cur->geometry.x;
   tmp_y += src->cur->geometry.y;
   *x = tmp_x;
   *y = tmp_y;
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
                                     Evas_Event_Mouse_Down *ev, int event_id,
                                     Efl_Event_Pointer *parent_pe)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Coord_Point canvas = ev->canvas;
   Evas_Object_Protected_Data *child;
   Efl_Event_Pointer *pe = NULL;
   Evas_Object *eo_child;
   Eina_List *l;
   int no_rep = 0;
   int addgrab = 0;
   Evas_Coord_Point point;
   Eina_List *copy;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

   _transform_to_src_space(obj, src, &ev->canvas.x, &ev->canvas.y);

   ev->event_src = eo_obj;

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_write)
     {
        if (proxy_write->src_event_in)
        proxy_write->src_event_in = eina_list_free(proxy_write->src_event_in);

        if (src->is_smart)
          {
             proxy_write->src_event_in = _evas_event_object_list_raw_in_get
               (eo_e, proxy_write->src_event_in,
                evas_object_smart_members_get_direct(eo_src),
                NULL, ev->canvas.x, ev->canvas.y, &no_rep, EINA_TRUE);
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

   point = ev->canvas;
   copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        ev->canvas = point;
        _evas_event_havemap_adjust(eo_child, child, &ev->canvas.x,
                                   &ev->canvas.y,
                                   child->mouse_grabbed);
        EV_CALL(eo_child, child, EVAS_CALLBACK_MOUSE_DOWN, ev, event_id, pe, parent_pe);
        if (e->delete_me) break;
        if (obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
          break;
     }
   eina_list_free(copy);
   ev->canvas = canvas;
   EV_DEL(pe);
}

static void
_evas_event_source_mouse_move_events(Evas_Object *eo_obj, Evas *eo_e,
                                     Evas_Event_Mouse_Move *ev, int event_id,
                                     Efl_Event_Pointer *parent_pe)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Coord_Point canvas = ev->cur.canvas;
   Efl_Event_Pointer *pe = NULL;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

   _transform_to_src_space(obj, src, &ev->cur.canvas.x, &ev->cur.canvas.y);

   ev->event_src = eo_obj;

   //FIXME: transform previous coords also.
   Eina_List *l;
   Evas_Object *eo_child;
   Evas_Object_Protected_Data *child;
   Evas_Coord_Point point = ev->cur.canvas;

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
                  ev->cur.canvas = point;
                  _evas_event_havemap_adjust(eo_child, child, &ev->cur.canvas.x,
                                             &ev->cur.canvas.y,
                                             child->mouse_grabbed);
                  EV_CALL(eo_child, child, EVAS_CALLBACK_MOUSE_MOVE, ev, event_id, pe, parent_pe);
               }
             else
               outs = eina_list_append(outs, eo_child);
             if (e->delete_me || e->is_frozen) break;
             //FIXME: take care nograb object 
          }
        eina_list_free(copy);
        EV_RESET(pe);

        while (outs)
          {
             eo_child = outs->data;
             outs = eina_list_remove(outs, eo_child);
             child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
             if ((child->mouse_grabbed == 0) && (!e->delete_me))
               {
                  if (child->mouse_in) continue;
                  child->mouse_in = 0;
                  if (child->delete_me || e->is_frozen) continue;
                  ev->cur.canvas = canvas;
                  _evas_event_havemap_adjust(eo_child, child, &ev->cur.canvas.x,
                                             &ev->cur.canvas.y,
                                             child->mouse_grabbed);

                  EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_write)
                    proxy_write->src_event_in = eina_list_remove(proxy_write->src_event_in, eo_child);
                  EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_write);

                  EV_CALL(eo_child, child, EVAS_CALLBACK_MOUSE_OUT, ev, event_id, pe, parent_pe);
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
             ins = _evas_event_object_list_raw_in_get(eo_e, ins, evas_object_smart_members_get_direct(eo_src), NULL, ev->cur.canvas.x, ev->cur.canvas.y, &no_rep, EINA_TRUE);
          }
        else
          ins = eina_list_append(ins, eo_src);

        EINA_LIST_FOREACH(copy, l, eo_child)
          {
             child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);

             ev->cur.canvas = point;

             if (evas_object_is_in_output_rect(eo_child, child,
                                               ev->cur.canvas.x,
                                               ev->cur.canvas.y, 1, 1) &&
                (evas_object_clippers_is_visible(eo_child, child) ||
                 child->mouse_grabbed) &&
                eina_list_data_find(ins, eo_child) &&
               (!evas_event_passes_through(eo_child, child)) &&
               (!evas_event_freezes_through(eo_child, child)) &&
               (!child->clip.clipees) &&
               ((!child->precise_is_inside) ||
                evas_object_is_inside(eo_child, child, ev->cur.canvas.x,
                                      ev->cur.canvas.y)))
               {
                  _evas_event_havemap_adjust(eo_child, child, &ev->cur.canvas.x,
                                             &ev->cur.canvas.y,
                                             child->mouse_grabbed);
                  EV_CALL(eo_child, child, EVAS_CALLBACK_MOUSE_MOVE, ev, event_id, pe, parent_pe);
                  EV_RESET(pe);
               }
             else if (child->mouse_in)
               {
                  child->mouse_in = 0;
                  if (e->is_frozen) continue;
                  ev->cur.canvas = point;
                  _evas_event_havemap_adjust(eo_child, child,
                                             &ev->cur.canvas.x,
                                             &ev->cur.canvas.y,
                                             child->mouse_grabbed);
                  // note: Mouse_Move and Mouse_Out are compatible (bad!)
                  EV_CALL(eo_child, child, EVAS_CALLBACK_MOUSE_OUT, ev, event_id, pe, parent_pe);
                  EV_RESET(pe);
                  if (e->delete_me) break;

               }
             if (e->delete_me || e->is_frozen) break;
          }
        eina_list_free(copy);

        _evas_object_event_new();
        int event_id2 = _evas_event_counter;
        EINA_LIST_FOREACH(ins, l, eo_child)
          {
              child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
              if (!eina_list_data_find(src->proxy->src_event_in, eo_child))
                {
                   if (!child->mouse_in)
                     {
                        child->mouse_in = 1;
                        if (e->is_frozen) continue;
                        ev->cur.canvas = point;
                        _evas_event_havemap_adjust(eo_child, child,
                                                   &ev->cur.canvas.x,
                                                   &ev->cur.canvas.y,
                                                   child->mouse_grabbed);
                        // note: Mouse_Move and Mouse_In are compatible (bad!)
                        EV_CALL(eo_child, child, EVAS_CALLBACK_MOUSE_IN, ev, event_id2, pe, parent_pe);
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
   ev->cur.canvas = canvas;
   EV_DEL(pe);
}

static void
_evas_event_source_mouse_up_events(Evas_Object *eo_obj, Evas *eo_e,
                                   Evas_Event_Mouse_Up *ev, int event_id,
                                   Efl_Event_Pointer *parent_pe)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Coord_Point canvas = ev->canvas;
   Efl_Event_Pointer *pe = NULL;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

   _transform_to_src_space(obj, src, &ev->canvas.x, &ev->canvas.y);

   ev->event_src = eo_obj;

   Eina_List *l;
   Evas_Object *eo_child;
   Evas_Object_Protected_Data *child;
   Evas_Coord_Point point = ev->canvas;

   Eina_List *copy = evas_event_list_copy(src->proxy->src_event_in);
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

        ev->canvas = point;
        _evas_event_havemap_adjust(eo_child, child,
                                   &ev->canvas.x,
                                   &ev->canvas.y,
                                   child->mouse_grabbed);
        EV_CALL(eo_child, child, EVAS_CALLBACK_MOUSE_UP, ev, event_id, pe, parent_pe);
        if (e->delete_me) break;
        if (obj->pointer_mode ==
            EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
          {
             if (e->pointer.nogrep > 0) e->pointer.nogrep--;
             break;
          }
     }
   eina_list_free(copy);

   ev->canvas = canvas;
   EV_DEL(pe);
}

static void
_evas_event_source_hold_events(Evas_Object *eo_obj, Evas *eo_e EINA_UNUSED, void *ev,
                               int event_id, Efl_Event_Hold *parent_he)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *child;
   Efl_Event_Hold *he = NULL;
   Evas_Object *eo_child;
   Eina_List *l;

   if (obj->layer->evas->is_frozen) return;

   EINA_LIST_FOREACH(src->proxy->src_event_in, l, eo_child)
     {
        if (src->delete_me) return;
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        EV_CALL(eo_child, child, EVAS_CALLBACK_HOLD, ev, event_id, he, parent_he);
        if (src->layer->evas->delete_me) break;
     }

   EV_DEL(he);
}

static void
_evas_event_source_wheel_events(Evas_Object *eo_obj, Evas *eo_e,
                                Efl_Event_Pointer *parent_ev, int event_id)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Object_Protected_Data *child;
   Evas_Object *eo_child;
   Eina_List *copy, *l;
   Efl_Event_Pointer_Data *ev;
   Efl_Event_Pointer *evt;
   Eina_Vector2 point;

   if (obj->delete_me || src->delete_me || obj->layer->evas->is_frozen) return;

   evt = efl_event_dup(parent_ev);
   ev = efl_data_scope_get(evt, EFL_EVENT_POINTER_CLASS);
   _transform_to_src_space_f(obj, src, &ev->cur);
   point = ev->cur;

   copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        if (src->delete_me) return;
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        ev->cur = point;
        _evas_event_havemap_adjust_f(eo_child, child, &ev->cur, child->mouse_grabbed);
        evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MOUSE_WHEEL, NULL,
                                        event_id, EFL_EVENT_POINTER_WHEEL, evt);
        if (e->delete_me) break;
     }
   eina_list_free(copy);
   EV_DEL(evt);
}

static void
_evas_event_source_multi_down_events(Evas_Object *eo_obj, Evas *eo_e,
                                     Evas_Event_Multi_Down *ev, int event_id,
                                     Efl_Event_Pointer *parent_pe)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Coord_Precision_Point canvas = ev->canvas;
   Efl_Event_Pointer *pe = NULL;

   if (obj->delete_me || src->delete_me || obj->layer->evas->is_frozen) return;

   _transform_to_src_space(obj, src, &ev->canvas.x, &ev->canvas.y);
   //FIXME: transform precision

   Eina_List *l;
   Evas_Object *eo_child;
   Evas_Object_Protected_Data *child = NULL;

   int addgrab = 0;
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

   Evas_Coord_Precision_Point point = ev->canvas;

   Eina_List *copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        ev->canvas = point;
        _evas_event_havemap_adjust(eo_child, child,
                                   &ev->canvas.x,
                                   &ev->canvas.y,
                                   child->mouse_grabbed);
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        EV_CALL(eo_child, child, EVAS_CALLBACK_MULTI_DOWN, ev, event_id, pe, parent_pe);
        if (e->delete_me) break;
     }
    eina_list_free(copy);

   ev->canvas = canvas;
   EV_DEL(pe);
}

static void
_evas_event_source_multi_up_events(Evas_Object *eo_obj, Evas *eo_e,
                                   Evas_Event_Multi_Up *ev, int event_id,
                                   Efl_Event_Pointer *parent_pe)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Coord_Precision_Point canvas = ev->canvas;
   Efl_Event_Pointer *pe = NULL;

   if (obj->delete_me || src->delete_me || obj->layer->evas->is_frozen) return;

   _transform_to_src_space(obj, src, &ev->canvas.x, &ev->canvas.y);
   //FIXME: transform precision

   Evas_Coord_Precision_Point point = ev->canvas;

   Eina_List *copy = evas_event_list_copy(src->proxy->src_event_in);

   Eina_List *l;
   Evas_Object *eo_child;
   Evas_Object_Protected_Data *child = NULL;
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        ev->canvas = point;
        child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
        if (((child->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) ||
             (child->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)) ||
            (child->mouse_grabbed > 0))
          {
             child->mouse_grabbed--;
             e->pointer.mouse_grabbed--;
          }
        _evas_event_havemap_adjust(eo_child, child,
                                   &ev->canvas.x,
                                   &ev->canvas.y,
                                   child->mouse_grabbed);
        EV_CALL(eo_child, child, EVAS_CALLBACK_MULTI_UP, ev, event_id, pe, parent_pe);
        if (e->delete_me || e->is_frozen) break;
     }
    eina_list_free(copy);

   ev->canvas = canvas;
   EV_DEL(pe);
}

static void
_evas_event_source_multi_move_events(Evas_Object *eo_obj, Evas *eo_e,
                                     Evas_Event_Multi_Move *ev, int event_id,
                                     Efl_Event_Pointer *parent_pe)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Coord_Precision_Point canvas = ev->cur.canvas;
   Efl_Event_Pointer *pe = NULL;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

   _transform_to_src_space(obj, src, &ev->cur.canvas.x, &ev->cur.canvas.y);

   //FIXME: transform precision

   Evas_Coord_Precision_Point point = ev->cur.canvas;

   _evas_object_event_new();
   event_id = _evas_event_counter;
   Eina_List *l;
   Evas_Object *eo_child;
   Evas_Object_Protected_Data *child;

   if (e->pointer.mouse_grabbed > 0)
     {
        Eina_List *copy = evas_event_list_copy(src->proxy->src_event_in);
        EINA_LIST_FOREACH(copy, l, eo_child)
          {
             child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
             if (((evas_object_clippers_is_visible(eo_child, child)) ||
                  ((child->mouse_grabbed) &&
                  (!evas_event_passes_through(eo_child, child)) &&
                  (!evas_event_freezes_through(eo_child, child)) &&
                  (!child->clip.clipees))))
               {
                  ev->cur.canvas = point;
                  _evas_event_havemap_adjust(eo_child, child,
                                             &ev->cur.canvas.x,
                                             &ev->cur.canvas.y,
                                             child->mouse_grabbed);
                  child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
                  EV_CALL(eo_child, child, EVAS_CALLBACK_MULTI_MOVE, ev, event_id, pe, parent_pe);
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
             ins = _evas_event_object_list_raw_in_get(eo_e, ins, evas_object_smart_members_get_direct(eo_src), NULL, ev->cur.canvas.x, ev->cur.canvas.y, &no_rep, EINA_TRUE);
          }
        else
          ins = eina_list_append(ins, eo_src);
        Eina_List *copy = evas_event_list_copy(src->proxy->src_event_in);
        EINA_LIST_FOREACH(copy, l, eo_child)
          {
             child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
             ev->cur.canvas = point;

             if (evas_object_is_in_output_rect(eo_child, child,
                                               ev->cur.canvas.x,
                                               ev->cur.canvas.y, 1, 1) &&
                (evas_object_clippers_is_visible(eo_child, child) ||
                 child->mouse_grabbed) &&
                eina_list_data_find(ins, eo_child) &&
               (!evas_event_passes_through(eo_child, child)) &&
               (!evas_event_freezes_through(eo_child, child)) &&
               (!child->clip.clipees) &&
               ((!child->precise_is_inside) ||
                evas_object_is_inside(eo_child, child, ev->cur.canvas.x,
                                      ev->cur.canvas.y)))
               {
                  _evas_event_havemap_adjust(eo_child, child,
                                             &ev->cur.canvas.x,
                                             &ev->cur.canvas.y,
                                             child->mouse_grabbed);
                  child = efl_data_scope_get(eo_child, EFL_CANVAS_OBJECT_CLASS);
                  EV_CALL(eo_child, child, EVAS_CALLBACK_MULTI_MOVE, ev, event_id, pe, parent_pe);
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

   ev->cur.canvas = canvas;
   EV_DEL(pe);
}

static void
_evas_event_source_mouse_in_events(Evas_Object *eo_obj, Evas *eo_e,
                                   Evas_Event_Mouse_In *ev, int event_id,
                                   Efl_Event_Pointer *parent_pe)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Coord_Point canvas = ev->canvas;
   Efl_Event_Pointer *pe = NULL;
   Evas_Object *eo_child;
   Eina_List *ins = NULL;
   Eina_List *l;
   Evas_Coord_Point point;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

  _transform_to_src_space(obj, src, &ev->canvas.x, &ev->canvas.y);

  ev->event_src = eo_obj;

   if (src->is_smart)
     {
        int no_rep = 0;
        ins = _evas_event_object_list_raw_in_get(eo_e, ins, evas_object_smart_members_get_direct(eo_src), NULL, ev->canvas.x, ev->canvas.y, &no_rep, EINA_TRUE);

     }
   else
     ins = eina_list_append(ins, eo_src);

   point = ev->canvas;
   EINA_LIST_FOREACH(ins, l, eo_child)
     {
        Evas_Object_Protected_Data *child = efl_data_scope_get(eo_child,
                                                        EFL_CANVAS_OBJECT_CLASS);
        if (!eina_list_data_find(src->proxy->src_event_in, eo_child))
          {
             if(child->mouse_in) continue;

             child->mouse_in = 1;
             ev->canvas = point;

             _evas_event_havemap_adjust(eo_child, child, &ev->canvas.x,
                                        &ev->canvas.y,
                                        child->mouse_grabbed);
             EV_CALL(eo_child, child, EVAS_CALLBACK_MOUSE_IN, ev, event_id, pe, parent_pe);
             if (e->delete_me || e->is_frozen) break;
          }
     }

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_write)
     {
       eina_list_free(proxy_write->src_event_in);
       proxy_write->src_event_in = ins;
     }
   EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_write);

   ev->canvas = canvas;
   EV_DEL(pe);
}

static void
_evas_event_source_mouse_out_events(Evas_Object *eo_obj, Evas *eo_e,
                                    Evas_Event_Mouse_Out *ev, int event_id,
                                    Efl_Event_Pointer *parent_pe)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = efl_data_scope_get(eo_src, EFL_CANVAS_OBJECT_CLASS);
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Coord_Point canvas = ev->canvas;
   Efl_Event_Pointer *pe = NULL;
   Evas_Object *eo_child;
   Eina_List *l;
   Eina_List *copy;
   Evas_Coord_Point point;

   if (obj->delete_me || src->delete_me || e->is_frozen) return;

  _transform_to_src_space(obj, src, &ev->canvas.x, &ev->canvas.y);

  ev->event_src = eo_obj;

   copy = evas_event_list_copy(src->proxy->src_event_in);
   point = ev->canvas;

   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        Evas_Object_Protected_Data *child = efl_data_scope_get(eo_child,
                                                        EFL_CANVAS_OBJECT_CLASS);
        if (!child->mouse_in) continue;
        child->mouse_in = 0;

        if (child->delete_me) continue;

        ev->canvas = point;

        _evas_event_havemap_adjust(eo_child, child, &ev->canvas.x,
                                   &ev->canvas.y,  child->mouse_grabbed);
        EV_CALL(eo_child, child, EVAS_CALLBACK_MOUSE_OUT, ev, event_id, pe, parent_pe);
        if (e->is_frozen) continue;
     }

   eina_list_free(copy);

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_write)
     proxy_write->src_event_in = eina_list_free(proxy_write->src_event_in);
   EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_write);

   ev->canvas = canvas;
   EV_DEL(pe);
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
   _canvas_event_feed_mouse_move_internal(eo_e, e, e->pointer.x, e->pointer.y,
                                          e->last_timestamp, NULL, NULL);
}

EAPI void
evas_event_freeze(Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   efl_event_freeze(eo_e);
}

EAPI void
evas_event_thaw(Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
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
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   int ret = 0;
   ret = efl_event_freeze_count_get((Eo *)eo_e);
   return ret;
}

EAPI void
evas_event_thaw_eval(Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   if (0 == evas_event_freeze_get(eo_e))
     {
        _canvas_event_thaw_eval_internal(eo_e, e);
     }
}

static void
_canvas_event_feed_mouse_down_internal(Eo *eo_e, int b, Evas_Button_Flags flags,
                                       unsigned int timestamp, const void *data,
                                       Efl_Event_Pointer *parent_pe)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Efl_Event_Pointer *pe = NULL;
   Eina_List *l, *copy;
   Evas_Event_Mouse_Down ev;
   Evas_Object *eo_obj;
   int addgrab = 0;
   int event_id = 0;

   INF("ButtonEvent:down time=%u x=%d y=%d button=%d downs=%d", timestamp, e->pointer.x, e->pointer.y, b, e->pointer.downs);
   if ((b < 1) || (b > 32)) return;

   e->pointer.button |= (1u << (b - 1));
   e->pointer.downs++;

   if (e->is_frozen) return;
   e->last_timestamp = timestamp;

   _evas_object_event_new();

   event_id = _evas_event_counter;
   ev.button = b;
   ev.output.x = e->pointer.x;
   ev.output.y = e->pointer.y;
   ev.canvas.x = e->pointer.x;
   ev.canvas.y = e->pointer.y;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.flags = flags;
   ev.timestamp = timestamp;
   ev.event_flags = e->default_event_flags;
   ev.dev = _evas_device_top_get(eo_e);
   if (ev.dev) efl_ref(ev.dev);

   _evas_walk(e);
   /* append new touch point to the touch point list */
   _evas_touch_point_append(eo_e, 0, e->pointer.x, e->pointer.y);
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
        ev.canvas.x = e->pointer.x;
        ev.canvas.y = e->pointer.y;
        _evas_event_havemap_adjust(eo_obj, obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
        EV_CALL(eo_obj, obj, EVAS_CALLBACK_MOUSE_DOWN, &ev, event_id, pe, parent_pe);
        if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
          _evas_event_source_mouse_down_events(eo_obj, eo_e, &ev, event_id, pe);
        if (e->is_frozen || e->delete_me)  break;
        if (obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
          break;
     }
   if (copy) eina_list_free(copy);
   e->last_mouse_down_counter++;
   _evas_post_event_callback_call(eo_e, e);
   /* update touch point's state to EVAS_TOUCH_POINT_STILL */
   _evas_touch_point_update(eo_e, 0, e->pointer.x, e->pointer.y, EVAS_TOUCH_POINT_STILL);
   if (ev.dev) efl_unref(ev.dev);
   _evas_unwalk(e);
   EV_DEL(pe);
}

EAPI void
evas_event_feed_mouse_down(Eo *eo_e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   _canvas_event_feed_mouse_down_internal(eo_e, b, flags, timestamp, data, NULL);
}

static int
_post_up_handle(Evas *eo_e, unsigned int timestamp, const void *data,
                Efl_Event_Pointer *parent_pe)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Efl_Event_Pointer *pe = NULL;
   Eina_List *l, *copy, *ins, *ll;
   Evas_Event_Mouse_Out ev;
   Evas_Object *eo_obj;
   int post_called = 0;
   int event_id = 0;

   _evas_object_event_new();

   event_id = _evas_event_counter;
   ev.buttons = e->pointer.button;
   ev.output.x = e->pointer.x;
   ev.output.y = e->pointer.y;
   ev.canvas.x = e->pointer.x;
   ev.canvas.y = e->pointer.y;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.timestamp = timestamp;
   ev.event_flags = e->default_event_flags;
   ev.dev = _evas_device_top_get(eo_e);
   if (ev.dev) efl_ref(ev.dev);

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
                  ev.canvas.x = e->pointer.x;
                  ev.canvas.y = e->pointer.y;
                  _evas_event_havemap_adjust(eo_obj, obj, &ev.canvas.x,
                                             &ev.canvas.y, obj->mouse_grabbed);
                  EV_CALL(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, &ev, event_id, pe, parent_pe);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_out_events(eo_obj, eo_e, &ev, event_id, pe);
                  if (e->delete_me) break;
               }
          }
     }
   _evas_post_event_callback_call(eo_e, e);
   EV_DEL(pe);

   eina_list_free(copy);

   if (e->pointer.inside)
     {
        Evas_Event_Mouse_In ev_in;
        Evas_Object *eo_obj_itr;

        _evas_object_event_new();

        event_id = _evas_event_counter;
        ev_in.buttons = e->pointer.button;
        ev_in.output.x = e->pointer.x;
        ev_in.output.y = e->pointer.y;
        ev_in.canvas.x = e->pointer.x;
        ev_in.canvas.y = e->pointer.y;
        ev_in.data = (void *)data;
        ev_in.modifiers = &(e->modifiers);
        ev_in.locks = &(e->locks);
        ev_in.timestamp = timestamp;
        ev_in.event_flags = e->default_event_flags;

        EINA_LIST_FOREACH(ins, l, eo_obj_itr)
          {
             Evas_Object_Protected_Data *obj_itr = efl_data_scope_get(eo_obj_itr, EFL_CANVAS_OBJECT_CLASS);
             if (!eina_list_data_find(e->pointer.object.in, eo_obj_itr))
               {
                  if (obj_itr->mouse_in) continue;
                  obj_itr->mouse_in = 1;
                  if (e->is_frozen) continue;
                  ev_in.canvas.x = e->pointer.x;
                  ev_in.canvas.y = e->pointer.y;
                  _evas_event_havemap_adjust(eo_obj_itr, obj_itr,
                                             &ev_in.canvas.x, &ev_in.canvas.y,
                                             obj_itr->mouse_grabbed);
                  EV_CALL(eo_obj_itr, obj_itr, EVAS_CALLBACK_MOUSE_IN, &ev_in, event_id, pe, parent_pe);
                  if ((obj_itr->proxy->is_proxy) &&
                      (obj_itr->proxy->src_events))
                    _evas_event_source_mouse_in_events(eo_obj_itr, eo_e, &ev_in, event_id, pe);
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
     _canvas_event_feed_mouse_move_internal(eo_e, e, e->pointer.x, e->pointer.y, timestamp, data, pe);
   if (ev.dev) efl_unref(ev.dev);
   EV_DEL(pe);

   return post_called;
}

static void
_canvas_event_feed_mouse_up_internal(Eo *eo_e, int b, Evas_Button_Flags flags,
                                     unsigned int timestamp, const void *data,
                                     Efl_Event_Pointer *parent_pe)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Efl_Event_Pointer *pe = NULL;
   Eina_List *l, *copy;

   INF("ButtonEvent:up time=%u x=%d y=%d button=%d downs=%d", timestamp, e->pointer.x, e->pointer.y, b, e->pointer.downs);
   if ((b < 1) || (b > 32)) return;
   if (e->pointer.downs <= 0) return;

   e->pointer.button &= ~(1u << (b - 1));
   e->pointer.downs--;

   if (e->is_frozen) return;
   e->last_timestamp = timestamp;

     {
        Evas_Event_Mouse_Up ev;
        Evas_Object *eo_obj;
        int event_id = 0;

        _evas_object_event_new();

        event_id = _evas_event_counter;
        ev.button = b;
        ev.output.x = e->pointer.x;
        ev.output.y = e->pointer.y;
        ev.canvas.x = e->pointer.x;
        ev.canvas.y = e->pointer.y;
        ev.data = (void *)data;
        ev.modifiers = &(e->modifiers);
        ev.locks = &(e->locks);
        ev.flags = flags;
        ev.timestamp = timestamp;
        ev.event_flags = e->default_event_flags;
        ev.dev = _evas_device_top_get(eo_e);
        if (ev.dev) efl_ref(ev.dev);

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
                       ev.canvas.x = e->pointer.x;
                       ev.canvas.y = e->pointer.y;
                       _evas_event_havemap_adjust(eo_obj, obj, &ev.canvas.x,
                                                  &ev.canvas.y,
                                                  obj->mouse_grabbed);
                       EV_CALL(eo_obj, obj, EVAS_CALLBACK_MOUSE_UP, &ev, event_id, pe, parent_pe);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_up_events(eo_obj, eo_e, &ev, event_id, pe);
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
        if (ev.dev) efl_unref(ev.dev);
     }

   if (e->pointer.mouse_grabbed == 0)
     {
        _post_up_handle(eo_e, timestamp, data, parent_pe);
     }

   if (e->pointer.mouse_grabbed < 0)
     {
        ERR("BUG? e->pointer.mouse_grabbed (=%d) < 0!",
            e->pointer.mouse_grabbed);
     }
   /* remove released touch point from the touch point list */
   _evas_touch_point_remove(eo_e, 0);

   _evas_unwalk(e);
   EV_DEL(pe);
}

EAPI void
evas_event_feed_mouse_up(Eo *eo_e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   _canvas_event_feed_mouse_up_internal(eo_e, b, flags, timestamp, data, NULL);
}

static void
_canvas_event_feed_mouse_cancel_internal(Eo *eo_e, unsigned int timestamp, const void *data,
                                         Efl_Event_Pointer *parent_pe)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Coord_Touch_Point *point;
   Eina_List *l, *ll;
   Evas_Event_Flags flags;
   int i;

   if (e->is_frozen) return;

   _evas_walk(e);

   flags = evas_event_default_flags_get(eo_e);
   evas_event_default_flags_set(eo_e, (flags | EVAS_EVENT_FLAG_ON_HOLD));

   for (i = 0; i < 32; i++)
     {
        if ((e->pointer.button & (1u << i)))
          _canvas_event_feed_mouse_up_internal(eo_e, i + 1, 0, timestamp, data, parent_pe);
     }
   EINA_LIST_FOREACH_SAFE(e->touch_points, l, ll, point)
     {
        if ((point->state == EVAS_TOUCH_POINT_DOWN) ||
            (point->state == EVAS_TOUCH_POINT_MOVE))
          _canvas_event_feed_multi_up_internal(eo_e, e, point->id, point->x, point->y,
                                               0, 0, 0, 0, 0, 0, 0, 0, timestamp, data, NULL);
     }
   evas_event_default_flags_set(eo_e, flags);
   _evas_unwalk(e);
}

EAPI void
evas_event_feed_mouse_cancel(Eo *eo_e, unsigned int timestamp, const void *data)
{
   _canvas_event_feed_mouse_cancel_internal(eo_e, timestamp, data, NULL);
}

static void
_canvas_event_feed_mouse_wheel_internal(Eo *eo_e, Efl_Event_Pointer_Data *pe)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Efl_Event_Pointer_Data *ev = NULL;
   Efl_Event_Pointer *evt;
   Eina_List *l, *copy;
   Evas_Object *eo_obj;
   int event_id = 0;

   if (e->is_frozen) return;
   e->last_timestamp = pe->timestamp;

   _evas_object_event_new();
   event_id = _evas_event_counter;

   evt = efl_event_dup(pe->eo);
   ev = efl_data_scope_get(evt, EFL_EVENT_POINTER_CLASS);
   if (!ev) return;

   // adjust missing data based on evas state
   ev->cur.x = e->pointer.x;
   ev->cur.y = e->pointer.y;
   ev->modifiers = &(e->modifiers);
   ev->locks = &(e->locks);
   ev->event_flags = e->default_event_flags;
   ev->device = efl_ref(_evas_device_top_get(eo_e));

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
             evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_WHEEL, NULL,
                                             event_id, EFL_EVENT_POINTER_WHEEL, evt);
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
   Efl_Event_Pointer_Data *ev = NULL;
   Efl_Event_Pointer *evt = efl_event_instance_get(EFL_EVENT_POINTER_CLASS, eo_e, (void **) &ev);

   if (!ev) return;

   ev->wheel.dir = direction ? EFL_ORIENT_HORIZONTAL : EFL_ORIENT_VERTICAL;
   ev->wheel.z = z;
   ev->timestamp = timestamp;
   ev->data = (void *) data;

   _canvas_event_feed_mouse_wheel_internal(eo_e, ev);
   efl_del(evt);
}

static void
_canvas_event_feed_mouse_move_internal(Eo *eo_e, Evas_Public_Data *e, int x, int y,
                                       unsigned int timestamp, const void *data,
                                       Efl_Event_Pointer *parent_pe)
{
   Evas_Object *nogrep_obj = NULL;
   Efl_Event_Pointer *pemove = NULL, *peout = NULL, *pein = NULL;
   int px, py;

   px = e->pointer.x;
   py = e->pointer.y;

   if (e->is_frozen) return;
   e->last_timestamp = timestamp;

   e->pointer.x = x;
   e->pointer.y = y;
   if ((!e->pointer.inside) && (e->pointer.mouse_grabbed == 0)) return;
   _evas_walk(e);
   /* update moved touch point */
   if ((px != x) || (py != y))
     _evas_touch_point_update(eo_e, 0, e->pointer.x, e->pointer.y, EVAS_TOUCH_POINT_MOVE);
   /* if our mouse button is grabbed to any objects */
   if (e->pointer.mouse_grabbed > 0)
     {
        /* go thru old list of in objects */
        Eina_List *outs = NULL;
        Eina_List *l, *copy;

          {
             Evas_Event_Mouse_Move ev;
             Evas_Object *eo_obj;
             int event_id = 0;

             _evas_object_event_new();

             event_id = _evas_event_counter;
             ev.buttons = e->pointer.button;
             ev.cur.output.x = e->pointer.x;
             ev.cur.output.y = e->pointer.y;
             ev.cur.canvas.x = e->pointer.x;
             ev.cur.canvas.y = e->pointer.y;
             ev.prev.output.x = px;
             ev.prev.output.y = py;
             ev.prev.canvas.x = px;
             ev.prev.canvas.y = py;
             ev.data = (void *)data;
             ev.modifiers = &(e->modifiers);
             ev.locks = &(e->locks);
             ev.timestamp = timestamp;
             ev.event_flags = e->default_event_flags;
             ev.dev = _evas_device_top_get(eo_e);
             if (ev.dev) efl_ref(ev.dev);

             copy = evas_event_list_copy(e->pointer.object.in);
             EINA_LIST_FOREACH(copy, l, eo_obj)
               {
                  Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
                  if ((!e->is_frozen) &&
                      (evas_object_clippers_is_visible(eo_obj, obj) ||
                       obj->mouse_grabbed) &&
                      (!evas_event_passes_through(eo_obj, obj)) &&
                      (!evas_event_freezes_through(eo_obj, obj)) &&
                      (!evas_object_is_source_invisible(eo_obj, obj) ||
                       obj->mouse_grabbed) &&
                      (!obj->clip.clipees))
                    {
                       ev.cur.canvas.x = e->pointer.x;
                       ev.cur.canvas.y = e->pointer.y;
                       _evas_event_havemap_adjust(eo_obj, obj, &ev.cur.canvas.x,
                                                  &ev.cur.canvas.y,
                                                  obj->mouse_grabbed);

                       if ((px != x) || (py != y))
                         {
                            EV_CALL(eo_obj, obj, EVAS_CALLBACK_MOUSE_MOVE, &ev, event_id, pemove, parent_pe);
                            if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                              _evas_event_source_mouse_move_events(eo_obj, eo_e, &ev, event_id, pemove);
                         }
                    }
                  else
                    outs = eina_list_append(outs, eo_obj);
                  if ((obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN) &&
                      (e->pointer.nogrep > 0))
                    {
                       eina_list_free(copy);
                       nogrep_obj = eo_obj;
                       goto nogrep;
                    }
                  if (e->delete_me) break;
               }
             _evas_post_event_callback_call(eo_e, e);
             if (ev.dev) efl_unref(ev.dev);
             EV_DEL(pemove);
          }
          {
             Evas_Event_Mouse_Out ev;
             int event_id = 0;

             _evas_object_event_new();

             event_id = _evas_event_counter;
             ev.buttons = e->pointer.button;
             ev.output.x = e->pointer.x;
             ev.output.y = e->pointer.y;
             ev.canvas.x = e->pointer.x;
             ev.canvas.y = e->pointer.y;
             ev.data = (void *)data;
             ev.modifiers = &(e->modifiers);
             ev.locks = &(e->locks);
             ev.timestamp = timestamp;
             ev.event_flags = e->default_event_flags;
             ev.dev = _evas_device_top_get(eo_e);
             if (ev.dev) efl_ref(ev.dev);

             eina_list_free(copy);

             while (outs)
               {
                  Evas_Object *eo_obj;
                  eo_obj = outs->data;
                  outs = eina_list_remove(outs, eo_obj);
                  Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
                  if ((obj->mouse_grabbed == 0) && (!e->delete_me))
                    {
                       if (!obj->mouse_in) continue;
                       obj->mouse_in = 0;
                       if (obj->delete_me || e->is_frozen) continue;
                       ev.canvas.x = e->pointer.x;
                       ev.canvas.y = e->pointer.y;
                       _evas_event_havemap_adjust(eo_obj, obj, &ev.canvas.x,
                                                  &ev.canvas.y,
                                                  obj->mouse_grabbed);
                       e->pointer.object.in = eina_list_remove(e->pointer.object.in, eo_obj);
                       EV_CALL(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, &ev, event_id, peout, parent_pe);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_out_events(eo_obj, eo_e, &ev, event_id, peout);
                    }
               }
             _evas_post_event_callback_call(eo_e, e);
             if (ev.dev) efl_unref(ev.dev);
             EV_DEL(peout);
          }
     }
   else
     {
        Eina_List *ins;
        Eina_List *l, *copy;
        Evas_Event_Mouse_Move ev;
        Evas_Event_Mouse_Out ev2;
        Evas_Event_Mouse_In ev3;
        Evas_Object *eo_obj;
        int event_id = 0, event_id2 = 0;

        _evas_object_event_new();

        event_id = _evas_event_counter;
        ev.buttons = e->pointer.button;
        ev.cur.output.x = e->pointer.x;
        ev.cur.output.y = e->pointer.y;
        ev.cur.canvas.x = e->pointer.x;
        ev.cur.canvas.y = e->pointer.y;
        ev.prev.output.x = px;
        ev.prev.output.y = py;
        ev.prev.canvas.x = px;
        ev.prev.canvas.y = py;
        ev.data = (void *)data;
        ev.modifiers = &(e->modifiers);
        ev.locks = &(e->locks);
        ev.timestamp = timestamp;
        ev.event_flags = e->default_event_flags;
        ev.dev = _evas_device_top_get(eo_e);
        if (ev.dev) efl_ref(ev.dev);

        ev2.buttons = e->pointer.button;
        ev2.output.x = e->pointer.x;
        ev2.output.y = e->pointer.y;
        ev2.canvas.x = e->pointer.x;
        ev2.canvas.y = e->pointer.y;
        ev2.data = (void *)data;
        ev2.modifiers = &(e->modifiers);
        ev2.locks = &(e->locks);
        ev2.timestamp = timestamp;
        ev2.event_flags = e->default_event_flags;
        ev2.dev = ev.dev;

        ev3.buttons = e->pointer.button;
        ev3.output.x = e->pointer.x;
        ev3.output.y = e->pointer.y;
        ev3.canvas.x = e->pointer.x;
        ev3.canvas.y = e->pointer.y;
        ev3.data = (void *)data;
        ev3.modifiers = &(e->modifiers);
        ev3.locks = &(e->locks);
        ev3.timestamp = timestamp;
        ev3.event_flags = e->default_event_flags;
        ev3.dev = ev.dev;

        /* get all new in objects */
        ins = evas_event_objects_event_list(eo_e, NULL, x, y);
        /* go thru old list of in objects */
        copy = evas_event_list_copy(e->pointer.object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

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
                       ev.cur.canvas.x = e->pointer.x;
                       ev.cur.canvas.y = e->pointer.y;
                       _evas_event_havemap_adjust(eo_obj, obj, &ev.cur.canvas.x, &ev.cur.canvas.y, obj->mouse_grabbed);
                       EV_CALL(eo_obj, obj, EVAS_CALLBACK_MOUSE_MOVE, &ev, event_id, pemove, parent_pe);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_move_events(eo_obj, eo_e, &ev, event_id, pemove);
                    }
               }
             /* otherwise it has left the object */
             else
               {
                  if (obj->mouse_in)
                    {
                       obj->mouse_in = 0;
                       if (e->is_frozen) continue;
                       ev2.canvas.x = e->pointer.x;
                       ev2.canvas.y = e->pointer.y;
                       _evas_event_havemap_adjust(eo_obj, obj, &ev2.canvas.x,
                                                  &ev2.canvas.y,
                                                  obj->mouse_grabbed);
                       EV_CALL(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, &ev2, event_id, peout, parent_pe);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_out_events(eo_obj, eo_e, &ev2, event_id, peout);
                       if (e->delete_me) break;
                    }
               }
          }
        _evas_post_event_callback_call(eo_e, e);

        _evas_object_event_new();

        event_id2 = _evas_event_counter;
        eina_list_free(copy);

        /* go thru our current list of ins */
        EINA_LIST_FOREACH(ins, l, eo_obj)
          {
             Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             /* if its not in the old list of ins send an enter event */
             if (!eina_list_data_find(e->pointer.object.in, eo_obj))
               {
                  if (!obj->mouse_in)
                    {
                       obj->mouse_in = 1;
                       if (e->is_frozen) continue;
                       ev3.canvas.x = e->pointer.x;
                       ev3.canvas.y = e->pointer.y;
                       _evas_event_havemap_adjust(eo_obj, obj, &ev3.canvas.x,
                                                  &ev3.canvas.y,
                                                  obj->mouse_grabbed);
                       EV_CALL(eo_obj, obj, EVAS_CALLBACK_MOUSE_IN, &ev3, event_id2, pein, parent_pe);
                       if ((obj->proxy->is_proxy) &&
                           (obj->proxy->src_events))
                         _evas_event_source_mouse_in_events(eo_obj, eo_e, &ev3, event_id2, pein);
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
        if (ev.dev) efl_unref(ev.dev);
        EV_DEL(pemove);
        EV_DEL(peout);
        EV_DEL(pein);
     }
   _evas_unwalk(e);
   return;
nogrep:
     {
        Eina_List *ins = NULL;
        Eina_List *newin = NULL;
        Eina_List *l, *copy, *lst = NULL;
        Evas_Event_Mouse_Move ev;
        Evas_Event_Mouse_Out ev2;
        Evas_Event_Mouse_In ev3;
        Evas_Object *eo_obj, *eo_below_obj;
        int event_id = 0, event_id2 = 0;
        int norep = 0, breaknext = 0;

        _evas_object_event_new();

        event_id = _evas_event_counter;
        ev.buttons = e->pointer.button;
        ev.cur.output.x = e->pointer.x;
        ev.cur.output.y = e->pointer.y;
        ev.cur.canvas.x = e->pointer.x;
        ev.cur.canvas.y = e->pointer.y;
        ev.prev.output.x = px;
        ev.prev.output.y = py;
        ev.prev.canvas.x = px;
        ev.prev.canvas.y = py;
        ev.data = (void *)data;
        ev.modifiers = &(e->modifiers);
        ev.locks = &(e->locks);
        ev.timestamp = timestamp;
        ev.event_flags = e->default_event_flags;
        ev.dev = _evas_device_top_get(eo_e);
        if (ev.dev) efl_ref(ev.dev);

        ev2.buttons = e->pointer.button;
        ev2.output.x = e->pointer.x;
        ev2.output.y = e->pointer.y;
        ev2.canvas.x = e->pointer.x;
        ev2.canvas.y = e->pointer.y;
        ev2.data = (void *)data;
        ev2.modifiers = &(e->modifiers);
        ev2.locks = &(e->locks);
        ev2.timestamp = timestamp;
        ev2.event_flags = e->default_event_flags;
        ev2.dev = ev.dev;

        ev3.buttons = e->pointer.button;
        ev3.output.x = e->pointer.x;
        ev3.output.y = e->pointer.y;
        ev3.canvas.x = e->pointer.x;
        ev3.canvas.y = e->pointer.y;
        ev3.data = (void *)data;
        ev3.modifiers = &(e->modifiers);
        ev3.locks = &(e->locks);
        ev3.timestamp = timestamp;
        ev3.event_flags = e->default_event_flags;
        ev3.dev = ev.dev;

        /* go thru old list of in objects */
        copy = evas_event_list_copy(e->pointer.object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             if (breaknext)
               {
                  lst = l;
                  break;
               }
             if (eo_obj == nogrep_obj) breaknext = 1;
          }

        /* get all new in objects */
        eo_below_obj = evas_object_below_get(nogrep_obj);
        if (eo_below_obj)
          {
             Evas_Object_Protected_Data *below_obj = efl_data_scope_get(eo_below_obj, EFL_CANVAS_OBJECT_CLASS);
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
        EINA_LIST_FOREACH(ins, l, eo_obj)
          {
             newin = eina_list_append(newin, eo_obj);
          }

        EINA_LIST_FOREACH(lst, l, eo_obj)
          {
             Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
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
                       ev.cur.canvas.x = e->pointer.x;
                       ev.cur.canvas.y = e->pointer.y;
                       _evas_event_havemap_adjust(eo_obj, obj, &ev.cur.canvas.x, &ev.cur.canvas.y, obj->mouse_grabbed);
                       EV_CALL(eo_obj, obj, EVAS_CALLBACK_MOUSE_MOVE, &ev, event_id, pemove, parent_pe);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_move_events(eo_obj, eo_e, &ev, event_id, pemove);
                    }
               }
             /* otherwise it has left the object */
             else
               {
                  if (!obj->mouse_in) continue;
                  obj->mouse_in = 0;
                  if (e->is_frozen) continue;
                  ev2.canvas.x = e->pointer.x;
                  ev2.canvas.y = e->pointer.y;
                  _evas_event_havemap_adjust(eo_obj, obj, &ev2.canvas.x,
                                             &ev2.canvas.y, obj->mouse_grabbed);
                  EV_CALL(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, &ev2, event_id, peout, parent_pe);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_out_events(eo_obj, eo_e, &ev2, event_id, peout);
               }
             if (e->delete_me) break;
          }
        _evas_post_event_callback_call(eo_e, e);

        _evas_object_event_new();

        event_id2 = _evas_event_counter;
        eina_list_free(copy);

        /* go thru our current list of ins */
        EINA_LIST_FOREACH(newin, l, eo_obj)
          {
             Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             /* if its not in the old list of ins send an enter event */
             if (!eina_list_data_find(e->pointer.object.in, eo_obj))
               {
                  if (obj->mouse_in) continue;
                  obj->mouse_in = 1;
                  if (e->is_frozen) continue;
                  ev3.canvas.x = e->pointer.x;
                  ev3.canvas.y = e->pointer.y;
                  _evas_event_havemap_adjust(eo_obj, obj, &ev3.canvas.x,
                                             &ev3.canvas.y, obj->mouse_grabbed);
                  EV_CALL(eo_obj, obj, EVAS_CALLBACK_MOUSE_IN, &ev3, event_id2, pein, parent_pe);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_in_events(eo_obj, eo_e, &ev3, event_id2, pein);
                  if (e->delete_me) break;
               }
          }
        /* free our old list of ins */
        eina_list_free(e->pointer.object.in);
        /* and set up the new one */
        e->pointer.object.in = newin;

        _evas_post_event_callback_call(eo_e, e);
        if (ev.dev) efl_unref(ev.dev);
        EV_DEL(pemove);
        EV_DEL(peout);
        EV_DEL(pein);
     }
   _evas_unwalk(e);
}

EAPI void
evas_event_input_mouse_move(Eo *eo_e, int x, int y, unsigned int timestamp, const void *data)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_mouse_move_internal(eo_e, e, x - e->framespace.x, y - e->framespace.y, timestamp, data, NULL);
}

EAPI void
evas_event_feed_mouse_move(Eo *eo_e, int x, int y, unsigned int timestamp, const void *data)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_mouse_move_internal(eo_e, e, x, y, timestamp, data, NULL);
}

static void
_canvas_event_feed_mouse_in_internal(Eo *eo_e, unsigned int timestamp, const void *data,
                                     Efl_Event_Pointer *parent_pe)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Efl_Event_Pointer *pe = NULL;
   Eina_List *ins;
   Eina_List *l;
   Evas_Event_Mouse_In ev;
   Evas_Object *eo_obj;
   int event_id = 0;

   e->pointer.inside = 1;

   if (e->is_frozen) return;
   e->last_timestamp = timestamp;

   if (e->pointer.mouse_grabbed != 0) return;

   _evas_object_event_new();

   event_id = _evas_event_counter;
   ev.buttons = e->pointer.button;
   ev.output.x = e->pointer.x;
   ev.output.y = e->pointer.y;
   ev.canvas.x = e->pointer.x;
   ev.canvas.y = e->pointer.y;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.timestamp = timestamp;
   ev.event_flags = e->default_event_flags;
   ev.dev = _evas_device_top_get(eo_e);
   if (ev.dev) efl_ref(ev.dev);
   
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
             ev.canvas.x = e->pointer.x;
             ev.canvas.y = e->pointer.y;
             _evas_event_havemap_adjust(eo_obj, obj, &ev.canvas.x, &ev.canvas.y,
                                        obj->mouse_grabbed);
             EV_CALL(eo_obj, obj, EVAS_CALLBACK_MOUSE_IN, &ev, event_id, pe, parent_pe);
             if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
               _evas_event_source_mouse_in_events(eo_obj, eo_e, &ev, event_id, pe);
             if (e->delete_me || e->is_frozen) break;
          }
     }
   /* free our old list of ins */
   e->pointer.object.in = eina_list_free(e->pointer.object.in);
   /* and set up the new one */
   e->pointer.object.in = ins;
   _evas_post_event_callback_call(eo_e, e);
   _canvas_event_feed_mouse_move_internal(eo_e, e, e->pointer.x, e->pointer.y, timestamp, data, pe);
   if (ev.dev) efl_unref(ev.dev);
   _evas_unwalk(e);
   EV_DEL(pe);
}

EAPI void
evas_event_feed_mouse_in(Eo *eo_e, unsigned int timestamp, const void *data)
{
   _canvas_event_feed_mouse_in_internal(eo_e, timestamp, data, NULL);
}

static void
_canvas_event_feed_mouse_out_internal(Eo *eo_e, unsigned int timestamp, const void *data,
                                      Efl_Event_Pointer *parent_pe)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Efl_Event_Pointer *pe = NULL;
   Evas_Event_Mouse_Out ev;
   int event_id = 0;

   e->pointer.inside = 0;

   if (e->is_frozen) return;
   e->last_timestamp = timestamp;

   _evas_object_event_new();

   event_id = _evas_event_counter;
   ev.buttons = e->pointer.button;
   ev.output.x = e->pointer.x;
   ev.output.y = e->pointer.y;
   ev.canvas.x = e->pointer.x;
   ev.canvas.y = e->pointer.y;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.timestamp = timestamp;
   ev.event_flags = e->default_event_flags;
   ev.dev = _evas_device_top_get(eo_e);
   if (ev.dev) efl_ref(ev.dev);

   _evas_walk(e);
   /* if our mouse button is inside any objects */
   /* go thru old list of in objects */
   Eina_List *l, *copy;
   Evas_Object *eo_obj;

   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if (!obj->mouse_in) continue;
        obj->mouse_in = 0;
        if (!obj->delete_me)
          {
             ev.canvas.x = e->pointer.x;
             ev.canvas.y = e->pointer.y;
             _evas_event_havemap_adjust(eo_obj, obj, &ev.canvas.x, &ev.canvas.y,
                                        obj->mouse_grabbed);
             EV_CALL(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, &ev, event_id, pe, parent_pe);
             if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
               _evas_event_source_mouse_out_events(eo_obj, eo_e, &ev, event_id, pe);
             if (e->delete_me || e->is_frozen) break;
          }
        obj->mouse_grabbed = 0;
     }
   eina_list_free(copy);

   /* free our old list of ins */
   e->pointer.object.in =  eina_list_free(e->pointer.object.in);
   e->pointer.mouse_grabbed = 0;
   _evas_post_event_callback_call(eo_e, e);
   if (ev.dev) efl_unref(ev.dev);
   _evas_unwalk(e);
   EV_DEL(pe);
}

EAPI void
evas_event_feed_mouse_out(Eo *eo_e, unsigned int timestamp, const void *data)
{
   _canvas_event_feed_mouse_out_internal(eo_e, timestamp, data, NULL);
}

static void
_canvas_event_feed_multi_down_internal(Evas *eo_e, void *_pd,
                           int d, int x, int y,
                           double rad, double radx, double rady,
                           double pres, double ang,
                           double fx, double fy,
                           Evas_Button_Flags flags, unsigned int timestamp,
                           const void *data, Efl_Event_Pointer *parent_pe)
{
   Evas_Public_Data *e = _pd;
   Eina_List *l, *copy;
   Evas_Event_Multi_Down ev;
   Efl_Event_Pointer *pe = NULL;
   Evas_Object *eo_obj;
   int addgrab = 0;
   int event_id = 0;

   INF("ButtonEvent:multi down time=%u x=%d y=%d button=%d downs=%d", timestamp, x, y, d, e->pointer.downs);
   e->pointer.downs++;
   if (e->is_frozen) return;
   e->last_timestamp = timestamp;

   _evas_object_event_new();

   event_id = _evas_event_counter;
   ev.device = d;
   ev.output.x = x;
   ev.output.y = y;
   ev.canvas.x = x;
   ev.canvas.y = y;
   ev.radius = rad;
   ev.radius_x = radx;
   ev.radius_y = rady;
   ev.pressure = pres;
   ev.angle = ang;
   ev.canvas.xsub = fx;
   ev.canvas.ysub = fy;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.flags = flags;
   ev.timestamp = timestamp;
   ev.event_flags = e->default_event_flags;
   ev.dev = _evas_device_top_get(eo_e);
   if (ev.dev) efl_ref(ev.dev);

   _evas_walk(e);
   /* append new touch point to the touch point list */
   _evas_touch_point_append(eo_e, d, x, y);
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
        ev.canvas.x = x;
        ev.canvas.y = y;
        ev.canvas.xsub = fx;
        ev.canvas.ysub = fy;
        _evas_event_havemap_adjust(eo_obj, obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
        if (x != ev.canvas.x)
          ev.canvas.xsub = ev.canvas.x; // fixme - lost precision
        if (y != ev.canvas.y)
          ev.canvas.ysub = ev.canvas.y; // fixme - lost precision
        EV_CALL(eo_obj, obj, EVAS_CALLBACK_MULTI_DOWN, &ev, event_id, pe, parent_pe);
        if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
          _evas_event_source_multi_down_events(eo_obj, eo_e, &ev, event_id, pe);
        if (e->delete_me || e->is_frozen) break;
     }
   eina_list_free(copy);

   _evas_post_event_callback_call(eo_e, e);
   /* update touch point's state to EVAS_TOUCH_POINT_STILL */
   _evas_touch_point_update(eo_e, d, x, y, EVAS_TOUCH_POINT_STILL);
   if (ev.dev) efl_unref(ev.dev);
   _evas_unwalk(e);
   EV_DEL(pe);
}

EAPI void
evas_event_input_multi_down(Eo *eo_e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_multi_down_internal(eo_e, e, d,
                                          x - e->framespace.x,
                                          y - e->framespace.y,
                                          rad, radx, rady, pres, ang,
                                          fx, fy, flags, timestamp, data, NULL);
}

EAPI void
evas_event_feed_multi_down(Eo *eo_e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_multi_down_internal(eo_e, e,
                                          d, x, y, rad, radx, rady, pres, ang,
                                          fx, fy, flags, timestamp, data, NULL);
}

static void
_canvas_event_feed_multi_up_internal(Evas *eo_e, void *_pd,
                                     int d, int x, int y,
                                     double rad, double radx, double rady,
                                     double pres, double ang,
                                     double fx, double fy,
                                     Evas_Button_Flags flags,
                                     unsigned int timestamp,
                                     const void *data,
                                     Efl_Event_Pointer *parent_pe)
{
   Efl_Event_Pointer *pe = NULL;
   Evas_Public_Data *e = _pd;
   Eina_List *l, *copy;
   Evas_Event_Multi_Up ev;
   Evas_Object *eo_obj;
   int event_id = 0;

   INF("ButtonEvent:multi up time=%u x=%d y=%d device=%d downs=%d", timestamp, x, y, d, e->pointer.downs);
   if (e->pointer.downs <= 0) return;
   e->pointer.downs--;
   if (e->is_frozen) return;
   e->last_timestamp = timestamp;

   _evas_object_event_new();

   event_id = _evas_event_counter;
   ev.device = d;
   ev.output.x = x;
   ev.output.y = y;
   ev.canvas.x = x;
   ev.canvas.y = y;
   ev.radius = rad;
   ev.radius_x = radx;
   ev.radius_y = rady;
   ev.pressure = pres;
   ev.angle = ang;
   ev.canvas.xsub = fx;
   ev.canvas.ysub = fy;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.flags = flags;
   ev.timestamp = timestamp;
   ev.event_flags = e->default_event_flags;
   ev.dev = _evas_device_top_get(eo_e);
   if (ev.dev) efl_ref(ev.dev);

   _evas_walk(e);
   /* update released touch point */
   _evas_touch_point_update(eo_e, d, x, y, EVAS_TOUCH_POINT_UP);
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        ev.canvas.x = x;
        ev.canvas.y = y;
        ev.canvas.xsub = fx;
        ev.canvas.ysub = fy;
        _evas_event_havemap_adjust(eo_obj, obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
        if (x != ev.canvas.x)
          ev.canvas.xsub = ev.canvas.x; // fixme - lost precision
        if (y != ev.canvas.y)
          ev.canvas.ysub = ev.canvas.y; // fixme - lost precision
        if (((obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) ||
            (obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)) &&
            (obj->mouse_grabbed > 0))
          {
             obj->mouse_grabbed--;
             e->pointer.mouse_grabbed--;
          }
        EV_CALL(eo_obj, obj, EVAS_CALLBACK_MULTI_UP, &ev, event_id, pe, parent_pe);
        if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
          _evas_event_source_multi_up_events(eo_obj, eo_e, &ev, event_id, pe);
        if (e->delete_me || e->is_frozen) break;
     }
   eina_list_free(copy);
   if ((e->pointer.mouse_grabbed == 0) && !_post_up_handle(eo_e, timestamp, data, parent_pe))
      _evas_post_event_callback_call(eo_e, e);
   /* remove released touch point from the touch point list */
   _evas_touch_point_remove(eo_e, d);
   if (ev.dev) efl_unref(ev.dev);
   _evas_unwalk(e);
   EV_DEL(pe);
}

EAPI void
evas_event_input_multi_up(Eo *eo_e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_multi_up_internal(eo_e, e, d,
                                        x - e->framespace.x,
                                        y - e->framespace.y,
                                        rad, radx, rady,
                                        pres, ang, fx, fy, flags, timestamp,
                                        data, NULL);
}

EAPI void
evas_event_feed_multi_up(Eo *eo_e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_multi_up_internal(eo_e, e, d, x, y, rad, radx, rady,
                                        pres, ang, fx, fy, flags, timestamp, data, NULL);
}

static void
_canvas_event_feed_multi_move_internal(Eo *eo_e, void *_pd, int d, int x,
                                       int y, double rad, double radx,
                                       double rady, double pres, double ang,
                                       double fx, double fy,
                                       unsigned int timestamp,
                                       const void *data,
                                       Efl_Event_Pointer *parent_pe)
{
   Evas_Public_Data *e = _pd;
   Efl_Event_Pointer *pe = NULL;

   if (e->is_frozen) return;
   e->last_timestamp = timestamp;

   if ((!e->pointer.inside) && (e->pointer.mouse_grabbed == 0)) return;

   _evas_walk(e);
   /* update moved touch point */
   _evas_touch_point_update(eo_e, d, x, y, EVAS_TOUCH_POINT_MOVE);
   /* if our mouse button is grabbed to any objects */
   if (e->pointer.mouse_grabbed > 0)
     {
        /* go thru old list of in objects */
        Eina_List *l, *copy;
        Evas_Event_Multi_Move ev;
        Evas_Object *eo_obj;
        int event_id = 0;

        _evas_object_event_new();

        event_id = _evas_event_counter;
        ev.device = d;
        ev.cur.output.x = x;
        ev.cur.output.y = y;
        ev.cur.canvas.x = x;
        ev.cur.canvas.y = y;
        ev.radius = rad;
        ev.radius_x = radx;
        ev.radius_y = rady;
        ev.pressure = pres;
        ev.angle = ang;
        ev.cur.canvas.xsub = fx;
        ev.cur.canvas.ysub = fy;
        ev.data = (void *)data;
        ev.modifiers = &(e->modifiers);
        ev.locks = &(e->locks);
        ev.timestamp = timestamp;
        ev.event_flags = e->default_event_flags;
        ev.dev = _evas_device_top_get(eo_e);
        if (ev.dev) efl_ref(ev.dev);

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
                  ev.cur.canvas.x = x;
                  ev.cur.canvas.y = y;
                  ev.cur.canvas.xsub = fx;
                  ev.cur.canvas.ysub = fy;
                  _evas_event_havemap_adjust(eo_obj, obj, &ev.cur.canvas.x, &ev.cur.canvas.y, obj->mouse_grabbed);
                  if (x != ev.cur.canvas.x)
                    ev.cur.canvas.xsub = ev.cur.canvas.x; // fixme - lost precision
                  if (y != ev.cur.canvas.y)
                    ev.cur.canvas.ysub = ev.cur.canvas.y; // fixme - lost precision
                  EV_CALL(eo_obj, obj, EVAS_CALLBACK_MULTI_MOVE, &ev, event_id, pe, parent_pe);
                  if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
                    _evas_event_source_multi_move_events(eo_obj, eo_e, &ev, event_id, pe);

                  if (e->delete_me || e->is_frozen) break;
               }
          }
        eina_list_free(copy);
        _evas_post_event_callback_call(eo_e, e);
        if (ev.dev) efl_unref(ev.dev);
     }
   else
     {
        Eina_List *ins;
        Eina_List *l, *copy;
        Evas_Event_Multi_Move ev;
        Evas_Object *eo_obj;
        int event_id = 0;

        _evas_object_event_new();

        event_id = _evas_event_counter;
        ev.device = d;
        ev.cur.output.x = x;
        ev.cur.output.y = y;
        ev.cur.canvas.x = x;
        ev.cur.canvas.y = y;
        ev.radius = rad;
        ev.radius_x = radx;
        ev.radius_y = rady;
        ev.pressure = pres;
        ev.angle = ang;
        ev.cur.canvas.xsub = fx;
        ev.cur.canvas.ysub = fy;
        ev.data = (void *)data;
        ev.modifiers = &(e->modifiers);
        ev.locks = &(e->locks);
        ev.timestamp = timestamp;
        ev.event_flags = e->default_event_flags;
        ev.dev = _evas_device_top_get(eo_e);
        if (ev.dev) efl_ref(ev.dev);

        /* get all new in objects */
        ins = evas_event_objects_event_list(eo_e, NULL, x, y);
        /* go thru old list of in objects */
        copy = evas_event_list_copy(e->pointer.object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             /* if its under the pointer and its visible and its in the new */
             /* in list */
             // FIXME: i don't think we need this
             //	     evas_object_clip_recalc(eo_obj);
             if (evas_object_is_in_output_rect(eo_obj, obj, x, y, 1, 1) &&
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
                  ev.cur.canvas.x = x;
                  ev.cur.canvas.y = y;
                  ev.cur.canvas.xsub = fx;
                  ev.cur.canvas.ysub = fy;
                  _evas_event_havemap_adjust(eo_obj, obj, &ev.cur.canvas.x, &ev.cur.canvas.y, obj->mouse_grabbed);
                  if (x != ev.cur.canvas.x)
                    ev.cur.canvas.xsub = ev.cur.canvas.x; // fixme - lost precision
                  if (y != ev.cur.canvas.y)
                    ev.cur.canvas.ysub = ev.cur.canvas.y; // fixme - lost precision
                  EV_CALL(eo_obj, obj, EVAS_CALLBACK_MULTI_MOVE, &ev, event_id, pe, parent_pe);
                  if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
                    _evas_event_source_multi_move_events(eo_obj, eo_e, &ev, event_id, pe);
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
        if (ev.dev) efl_unref(ev.dev);
     }
   _evas_unwalk(e);
   EV_DEL(pe);
}

EAPI void
evas_event_input_multi_move(Eo *eo_e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, unsigned int timestamp, const void *data)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_multi_move_internal(eo_e, e, d,
                                          x - e->framespace.x, y - e->framespace.y,
                                          rad, radx, rady,
                                          pres, ang, fx, fy, timestamp, data, NULL);
}

EAPI void
evas_event_feed_multi_move(Eo *eo_e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, unsigned int timestamp, const void *data)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_multi_move_internal(eo_e, e, d, x, y, rad, radx, rady,
                                          pres, ang, fx, fy, timestamp, data, NULL);
}

static void
_canvas_event_feed_key_down_internal(Eo *eo_e,
                                     void *_pd,
                                     const char *keyname,
                                     const char *key,
                                     const char *string,
                                     const char *compose,
                                     unsigned int timestamp,
                                     const void *data,
                                     unsigned int keycode,
                                     Efl_Event_Key_Data *ke)
{
   void *efl_event_info = NULL;
   Evas_Public_Data *e = _pd;
   int event_id = 0;

   if (!keyname) return;
   if (e->is_frozen) return;
   e->last_timestamp = timestamp;
   _evas_walk(e);

   Evas_Event_Key_Down ev;
   Eina_Bool exclusive;

   _evas_object_event_new();

   event_id = _evas_event_counter;
   exclusive = EINA_FALSE;
   ev.keyname = (char *)keyname;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.key = key;
   ev.string = string;
   ev.compose = compose;
   ev.timestamp = timestamp;
   ev.event_flags = e->default_event_flags;
   ev.dev = _evas_device_top_get(eo_e);
   ev.keycode = keycode;
   if (ev.dev) efl_ref(ev.dev);

   if (ke)
     {
        ke->device = ev.dev;
        ke->event_flags = ev.event_flags;
        ke->modifiers = ev.modifiers;
        ke->locks = ev.locks;
        efl_event_info = ke->eo;
     }

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
                 (!strcmp(keyname, g->keyname)))
               {
                  if (!(e->modifiers.mask & g->not_modifiers))
                    {
                       Evas_Object_Protected_Data *object_obj = efl_data_scope_get(g->object, EFL_CANVAS_OBJECT_CLASS);
                       if (!e->is_frozen &&
                           !evas_event_freezes_through(g->object, object_obj))
                         {
                            EV_CALL(g->object, object_obj, EVAS_CALLBACK_KEY_DOWN,
                                    &ev, event_id, efl_event_info, NULL);
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
   if (!ke) EV_DEL(efl_event_info);
   if ((e->focused) && (!exclusive))
     {
        Evas_Object_Protected_Data *focused_obj = efl_data_scope_get(e->focused, EFL_CANVAS_OBJECT_CLASS);
        if (!e->is_frozen && !evas_event_freezes_through(e->focused, focused_obj))
          {
             EV_CALL(e->focused, focused_obj, EVAS_CALLBACK_KEY_DOWN,
                     &ev, event_id, efl_event_info, NULL);
          }
     }
   _evas_post_event_callback_call(eo_e, e);
   if (ev.dev) efl_unref(ev.dev);
   _evas_unwalk(e);

   if (ke)
     {
        ke->device = NULL;
        ke->event_flags = 0;
        ke->modifiers = NULL;
        ke->locks = NULL;
     }
   else
     EV_DEL(efl_event_info);
}

static void
_canvas_event_feed_key_up_internal(Eo *eo_e,
                                   void *_pd,
                                   const char *keyname,
                                   const char *key,
                                   const char *string,
                                   const char *compose,
                                   unsigned int timestamp,
                                   const void *data,
                                   unsigned int keycode,
                                   Efl_Event_Key_Data *ke)
{
   void *efl_event_info = NULL;
   Evas_Public_Data *e = _pd;
   int event_id = 0;
   if (!keyname) return;
   if (e->is_frozen) return;
   e->last_timestamp = timestamp;
   _evas_walk(e);

   Evas_Event_Key_Up ev;
   Eina_Bool exclusive;

   _evas_object_event_new();

   event_id = _evas_event_counter;
   exclusive = EINA_FALSE;
   ev.keyname = (char *)keyname;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.key = key;
   ev.string = string;
   ev.compose = compose;
   ev.timestamp = timestamp;
   ev.event_flags = e->default_event_flags;
   ev.dev = _evas_device_top_get(eo_e);
   ev.keycode = keycode;
   if (ev.dev) efl_ref(ev.dev);

   if (ke)
     {
        ke->device = ev.dev;
        ke->event_flags = ev.event_flags;
        ke->modifiers = ev.modifiers;
        ke->locks = ev.locks;
        efl_event_info = ke->eo;
     }

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
                 (!strcmp(keyname, g->keyname)))
               {
                  Evas_Object_Protected_Data *object_obj = efl_data_scope_get(g->object, EFL_CANVAS_OBJECT_CLASS);
                  if (!e->is_frozen &&
                        !evas_event_freezes_through(g->object, object_obj))
                    {
                       evas_object_event_callback_call
                             (g->object, object_obj, EVAS_CALLBACK_KEY_UP,
                              &ev, event_id, EFL_EVENT_KEY_UP, efl_event_info);
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
                   (e->focused, focused_obj, EVAS_CALLBACK_KEY_UP,
                    &ev, event_id, EFL_EVENT_KEY_UP, efl_event_info);
          }
     }
   _evas_post_event_callback_call(eo_e, e);
   if (ev.dev) efl_unref(ev.dev);
   _evas_unwalk(e);

   if (ke)
     {
        ke->device = NULL;
        ke->event_flags = 0;
        ke->modifiers = NULL;
        ke->locks = NULL;
     }
}

EAPI void
evas_event_feed_key_down(Eo *eo_e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_key_down_internal(eo_e, e, keyname, key, string,
                                        compose, timestamp, data, 0, NULL);
}

EAPI void
evas_event_feed_key_up(Eo *eo_e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_key_up_internal(eo_e, e, keyname, key, string,
                                      compose, timestamp, data, 0, NULL);
}

EAPI void
evas_event_feed_key_down_with_keycode(Eo *eo_e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data, unsigned int keycode)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_key_down_internal(eo_e, e, keyname, key, string,
                                        compose, timestamp, data, keycode, NULL);
}

EAPI void
evas_event_feed_key_up_with_keycode(Eo *eo_e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data, unsigned int keycode)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_key_up_internal(eo_e, e, keyname, key, string,
                                      compose, timestamp, data, keycode, NULL);
}

EAPI void
evas_event_feed_hold(Eo *eo_e, int hold, unsigned int timestamp, const void *data)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Efl_Event_Hold *he = NULL;
   Eina_List *l, *copy;
   Evas_Event_Hold ev;
   Evas_Object *eo_obj;
   int event_id = 0;

   if (e->is_frozen) return;
   e->last_timestamp = timestamp;

   _evas_object_event_new();

   event_id = _evas_event_counter;
   ev.hold = hold;
   ev.data = (void *)data;
   ev.timestamp = timestamp;
   ev.event_flags = e->default_event_flags;
   ev.dev = _evas_device_top_get(eo_e);
   if (ev.dev) efl_ref(ev.dev);

   _evas_walk(e);
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if ( !evas_event_freezes_through(eo_obj, obj))
          {
             EV_CALL(eo_obj, obj, EVAS_CALLBACK_HOLD, &ev, event_id, he, NULL);
             if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
               _evas_event_source_hold_events(eo_obj, eo_e, &ev, event_id, he);
          }
        if (e->delete_me || e->is_frozen) break;
     }
   eina_list_free(copy);
   _evas_post_event_callback_call(eo_e, e);
   if (ev.dev) efl_unref(ev.dev);
   _evas_unwalk(e);
   _evas_object_event_new();

   EV_DEL(he);
}

void
_canvas_event_feed_axis_update_internal(Evas *eo_e, Evas_Public_Data *e, unsigned int timestamp, int device, int toolid, int naxis, const Evas_Axis *axis, const void *data)
{
   Eina_List *l, *copy;
   Evas_Event_Axis_Update ev;
   Evas_Object *eo_obj;
   int event_id = 0;

   if (e->is_frozen) return;
   e->last_timestamp = timestamp;

   _evas_object_event_new();

   event_id = _evas_event_counter;
   ev.data = (void *)data;
   ev.timestamp = timestamp;
   ev.device = device;
   ev.toolid = toolid;
   ev.naxis = naxis;
   ev.axis = (Evas_Axis *)axis;
   ev.dev = _evas_device_top_get(eo_e);
   if (ev.dev) efl_ref(ev.dev);

   _evas_walk(e);
   copy = evas_event_list_copy(e->pointer.object.in);

   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        if (!evas_event_freezes_through(eo_obj, obj))
          {
             evas_object_event_callback_call(eo_obj, obj,
                                             EVAS_CALLBACK_AXIS_UPDATE, &ev,
                                             event_id, NULL, NULL);
             if (e->delete_me || e->is_frozen) break;
          }
     }
   eina_list_free(copy);
   _evas_post_event_callback_call(eo_e, e);

   _evas_unwalk(e);
}

EAPI void
evas_event_feed_axis_update(Evas *eo_e, unsigned int timestamp, int device, int toolid, int naxis, const Evas_Axis *axis, const void *data)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   _canvas_event_feed_axis_update_internal(eo_e, e, timestamp, device, toolid, naxis, axis, data);
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
        _canvas_event_feed_mouse_move_internal(evas->evas, evas,
                                               evas->pointer.x, evas->pointer.y,
                                               evas->last_timestamp,
                                               NULL, NULL);
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
_evas_canvas_event_pointer_cb(void *data, const Eo_Event *event)
{
   Efl_Event_Pointer_Data *ev = efl_data_scope_get(event->info, EFL_EVENT_POINTER_CLASS);
   Evas_Public_Data *e = data;
   Evas *eo_e = event->object;

   if (!ev) return;

   ev->evas_done = EINA_TRUE;
   ev->modifiers = &e->modifiers;
   ev->locks = &e->locks;

   switch (ev->action)
     {
      case EFL_POINTER_ACTION_MOVE:
        if (ev->finger == 0)
          {
             _canvas_event_feed_mouse_move_internal(eo_e, e, ev->cur.x, ev->cur.y,
                                                    ev->timestamp, ev->data, ev->eo);
          }
        else
          {
             _canvas_event_feed_multi_move_internal(eo_e, e, ev->finger, ev->cur.x, ev->cur.y,
                                                    ev->radius, ev->radius_x, ev->radius_y,
                                                    ev->pressure, ev->angle,
                                                    ev->cur.x, ev->cur.y,
                                                    ev->timestamp, ev->data, ev->eo);
          }
        break;

      case EFL_POINTER_ACTION_DOWN:
        if (ev->finger == 0)
          {
             _canvas_event_feed_mouse_down_internal(eo_e, ev->button, ev->button_flags, ev->timestamp, ev->data, ev->eo);
          }
        else
          {
             _canvas_event_feed_multi_down_internal(eo_e, e, ev->finger, ev->cur.x, ev->cur.y,
                                                    ev->radius, ev->radius_x, ev->radius_y,
                                                    ev->pressure, ev->angle,
                                                    ev->cur.x, ev->cur.y, ev->button_flags,
                                                    ev->timestamp, ev->data, ev->eo);
          }
        break;

      case EFL_POINTER_ACTION_UP:
        if (ev->finger == 0)
          {
             _canvas_event_feed_mouse_up_internal(eo_e, ev->button, ev->button_flags, ev->timestamp, ev->data, ev->eo);
          }
        else
          {
             _canvas_event_feed_multi_up_internal(eo_e, e, ev->finger, ev->cur.x, ev->cur.y,
                                                  ev->radius, ev->radius_x, ev->radius_y,
                                                  ev->pressure, ev->angle,
                                                  ev->cur.x, ev->cur.y, ev->button_flags,
                                                  ev->timestamp, ev->data, ev->eo);
          }
        break;

      case EFL_POINTER_ACTION_CANCEL:
        _canvas_event_feed_mouse_cancel_internal(eo_e, ev->timestamp, ev->data, ev->eo);
        break;

      case EFL_POINTER_ACTION_IN:
        _canvas_event_feed_mouse_in_internal(eo_e, ev->timestamp, ev->data, ev->eo);
        break;

      case EFL_POINTER_ACTION_OUT:
        _canvas_event_feed_mouse_out_internal(eo_e, ev->timestamp, ev->data, ev->eo);
        break;

      case EFL_POINTER_ACTION_WHEEL:
        _canvas_event_feed_mouse_wheel_internal(eo_e, ev);
        break;

      default:
        ERR("unsupported event type: %d", ev->action);
        ev->evas_done = EINA_FALSE;
        break;
     }
}

static void
_evas_canvas_event_key_cb(void *data, const Eo_Event *event)
{
   Efl_Event_Key *evt = event->info;
   Evas_Public_Data *e = data;
   Efl_Event_Key_Data *ev;

   ev = efl_data_scope_get(evt, EFL_EVENT_KEY_CLASS);
   if (!ev) return;

   if (ev->pressed)
     {
        _canvas_event_feed_key_down_internal(e->evas, e, ev->keyname, ev->key,
                                             ev->string, ev->compose,
                                             ev->timestamp, ev->data,
                                             ev->keycode, ev);
     }
   else
     {
        _canvas_event_feed_key_up_internal(e->evas, e, ev->keyname, ev->key,
                                           ev->string, ev->compose,
                                           ev->timestamp, ev->data,
                                           ev->keycode, ev);
     }

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

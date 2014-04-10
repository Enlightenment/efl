#include "evas_common_private.h"
#include "evas_private.h"

static Eina_List *
_evas_event_object_list_in_get(Evas *eo_e, Eina_List *in,
                               const Eina_Inlist *list, Evas_Object *stop,
                               int x, int y, int *no_rep, Eina_Bool source);

static Eina_List *
evas_event_list_copy(Eina_List *list);


static void
_evas_event_havemap_adjust(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Coord *x, Evas_Coord *y, Eina_Bool mouse_grabbed)
{
   if (obj->smart.parent)
     {
        Evas_Object_Protected_Data *smart_parent_obj = eo_data_scope_get(obj->smart.parent, EVAS_OBJ_CLASS);
        _evas_event_havemap_adjust(obj->smart.parent, smart_parent_obj, x, y, mouse_grabbed);
     }

   if ((!obj->map->cur.usemap) || (!obj->map->cur.map) || 
       (obj->map->cur.map->count != 4))
      return;

   //FIXME: Unless map_coords_get() supports grab mode and extrapolate coords
   //outside map, this should check the return value for outside case.
   if (evas_map_coords_get(obj->map->cur.map, *x, *y, x, y, mouse_grabbed))
     {
        *x += obj->cur->geometry.x;
        *y += obj->cur->geometry.y;
     }
}

static Eina_List *
_evas_event_object_list_raw_in_get(Evas *eo_e, Eina_List *in,
                                   const Eina_Inlist *list, Evas_Object *stop,
                                   int x, int y, int *no_rep, Eina_Bool source)
{
   Evas_Object *eo_obj;
   Evas_Object_Protected_Data *obj = NULL;
   int inside;

   if (!list) return in;
   for (obj = _EINA_INLIST_CONTAINER(obj, list); 
        obj; 
        obj = _EINA_INLIST_CONTAINER(obj, EINA_INLIST_GET(obj)->prev))
     {
        eo_obj = obj->object;
        if (eo_obj == stop)
          {
             *no_rep = 1;
             return in;
          }
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
                  int norep = 0;

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
                            return in;
                         }
                    }
               }
             else
               {
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
                         in = eina_list_append(in, eo_obj);
                       if (!obj->repeat_events)
                         {
                            *no_rep = 1;
                            return in;
                         }
                    }
               }
          }
     }
   *no_rep = 0;
   return in;
}

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
_evas_event_source_mouse_down_events(Evas_Object *eo_obj, Evas *eo_e, Evas_Event_Mouse_Down *ev, int event_id)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = eo_data_scope_get(eo_src, EVAS_OBJ_CLASS);
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Coord_Point canvas = ev->canvas;
   Evas_Object_Protected_Data *child;
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
	   proxy_write->src_event_in = _evas_event_object_list_raw_in_get(eo_e,
									  proxy_write->src_event_in,
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
        child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);
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
        child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);
        ev->canvas = point;
        _evas_event_havemap_adjust(eo_child, child, &ev->canvas.x,
                                   &ev->canvas.y,
                                   child->mouse_grabbed);
        evas_object_event_callback_call(eo_child, child,
                                        EVAS_CALLBACK_MOUSE_DOWN, ev, event_id);
        if (e->delete_me) break;
        if (obj->pointer_mode ==
            EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
          break;
     }
   eina_list_free(copy);
   ev->canvas = canvas;
}

static void
_evas_event_source_mouse_move_events(Evas_Object *eo_obj, Evas *eo_e, Evas_Event_Mouse_Move *ev, int event_id)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = eo_data_scope_get(eo_src, EVAS_OBJ_CLASS);
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Coord_Point canvas = ev->cur.canvas;

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
             child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);

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
                  if ((e->pointer.x != ev->cur.canvas.x) ||
                      (e->pointer.y != ev->cur.canvas.y))
                    evas_object_event_callback_call(eo_child, child,
                                                    EVAS_CALLBACK_MOUSE_MOVE,
                                                    ev, event_id);
               }
             else
               outs = eina_list_append(outs, eo_child);
             if (e->delete_me || e->is_frozen) break;
             //FIXME: take care nograb object 
          }
        eina_list_free(copy);

        while (outs)
          {
             eo_child = outs->data;
             outs = eina_list_remove(outs, eo_child);
             child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);
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

                  evas_object_event_callback_call(eo_child, child,
                                                  EVAS_CALLBACK_MOUSE_OUT,
                                                  ev, event_id);
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
             child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);

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
                  if ((e->pointer.x != ev->cur.canvas.x) ||
                      (e->pointer.y != ev->cur.canvas.y))
                    evas_object_event_callback_call(eo_child, child,
                                                    EVAS_CALLBACK_MOUSE_MOVE,
                                                    ev, event_id);
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
                  evas_object_event_callback_call(eo_child, child,
                                                  EVAS_CALLBACK_MOUSE_OUT,
                                                  ev, event_id);
                  if (e->delete_me) break;

               }
             if (e->delete_me || e->is_frozen) break;
          }
        eina_list_free(copy);
        _evas_object_event_new();
        int event_id2 = _evas_event_counter;
        EINA_LIST_FOREACH(ins, l, eo_child)
          {
              child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);
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
                        evas_object_event_callback_call(eo_child, child,
                                                        EVAS_CALLBACK_MOUSE_IN,
                                                        ev, event_id2);
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
}

static void
_evas_event_source_mouse_up_events(Evas_Object *eo_obj, Evas *eo_e, Evas_Event_Mouse_Up *ev, int event_id)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = eo_data_scope_get(eo_src, EVAS_OBJ_CLASS);
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Coord_Point canvas = ev->canvas;

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
        if (src->delete_me) return;
        child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);
        if ((child->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) &&
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
        evas_object_event_callback_call(eo_child, child,
                                        EVAS_CALLBACK_MOUSE_UP, ev, event_id);
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
}

static void
_evas_event_source_hold_events(Evas_Object *eo_obj, Evas *eo_e EINA_UNUSED, void *ev, int event_id)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = eo_data_scope_get(eo_src, EVAS_OBJ_CLASS);

   if (obj->layer->evas->is_frozen) return;

   Eina_List *l;
   Evas_Object *child_eo;
   Evas_Object_Protected_Data *child;
   EINA_LIST_FOREACH(src->proxy->src_event_in, l, child_eo)
     {
        if (src->delete_me) return;
        child = eo_data_scope_get(child_eo, EVAS_OBJ_CLASS);
        evas_object_event_callback_call(child_eo, child, EVAS_CALLBACK_HOLD, ev,
                                        event_id);
        if (src->layer->evas->delete_me) break;
     }
}

static void
_evas_event_source_wheel_events(Evas_Object *eo_obj, Evas *eo_e, Evas_Event_Mouse_Wheel *ev, int event_id)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = eo_data_scope_get(eo_src, EVAS_OBJ_CLASS);
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Coord_Point canvas = ev->canvas;

   if (obj->delete_me || src->delete_me || obj->layer->evas->is_frozen) return;

   _transform_to_src_space(obj, src, &ev->canvas.x, &ev->canvas.y);

   Eina_List *l;
   Evas_Object *eo_child;
   Evas_Object_Protected_Data *child;
   Evas_Coord_Point point = ev->canvas;

   Eina_List *copy = evas_event_list_copy(src->proxy->src_event_in);
   EINA_LIST_FOREACH(copy, l, eo_child)
     {
        if (src->delete_me) return;
        child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);
        ev->canvas = point;
        _evas_event_havemap_adjust(eo_child, child,
                                   &ev->canvas.x,
                                   &ev->canvas.y,
                                   child->mouse_grabbed);
        evas_object_event_callback_call(eo_child, child,
                                        EVAS_CALLBACK_MOUSE_WHEEL, ev, event_id);
        if (e->delete_me) break;
     }
   eina_list_free(copy);
   ev->canvas = canvas;
}

static void
_evas_event_source_multi_down_events(Evas_Object *eo_obj, Evas *eo_e, Evas_Event_Multi_Down *ev, int event_id)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = eo_data_scope_get(eo_src, EVAS_OBJ_CLASS);
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Coord_Precision_Point canvas = ev->canvas;

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
        child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);
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
        child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);
        evas_object_event_callback_call(eo_child, child,
                                        EVAS_CALLBACK_MULTI_DOWN, ev, event_id);
        if (e->delete_me) break;
     }
    eina_list_free(copy);

   ev->canvas = canvas;
}

static void
_evas_event_source_multi_up_events(Evas_Object *eo_obj, Evas *eo_e, Evas_Event_Multi_Up *ev, int event_id)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = eo_data_scope_get(eo_src, EVAS_OBJ_CLASS);
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Coord_Precision_Point canvas = ev->canvas;

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
        child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);
        if ((child->pointer_mode != EVAS_OBJECT_POINTER_MODE_NOGRAB) &&
            (child->mouse_grabbed > 0))
          {
             child->mouse_grabbed--;
             e->pointer.mouse_grabbed--;
          }
        _evas_event_havemap_adjust(eo_child, child,
                                   &ev->canvas.x,
                                   &ev->canvas.y,
                                   child->mouse_grabbed);
        evas_object_event_callback_call(eo_child, child, EVAS_CALLBACK_MULTI_UP,
                                        ev, event_id);
        if (e->delete_me || e->is_frozen) break;
     }
    eina_list_free(copy);

   ev->canvas = canvas;
}

static void
_evas_event_source_multi_move_events(Evas_Object *eo_obj, Evas *eo_e, Evas_Event_Multi_Move *ev, int event_id)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = eo_data_scope_get(eo_src, EVAS_OBJ_CLASS);
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Coord_Precision_Point canvas = ev->cur.canvas;

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
             child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);
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
                  child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);
                  evas_object_event_callback_call(eo_child, child,
                                                  EVAS_CALLBACK_MULTI_MOVE, ev,
                                                  event_id);
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
             child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);
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
                  child = eo_data_scope_get(eo_child, EVAS_OBJ_CLASS);
                  evas_object_event_callback_call(eo_child, child,
                                                  EVAS_CALLBACK_MULTI_MOVE, ev,
                                                  event_id);
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
}

static void
_evas_event_source_mouse_in_events(Evas_Object *eo_obj, Evas *eo_e,  Evas_Event_Mouse_In *ev, int event_id)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = eo_data_scope_get(eo_src, EVAS_OBJ_CLASS);
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Coord_Point canvas = ev->canvas;
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
        Evas_Object_Protected_Data *child = eo_data_scope_get(eo_child,
                                                        EVAS_OBJ_CLASS);
        if (!eina_list_data_find(src->proxy->src_event_in, eo_child))
          {
             if(child->mouse_in) continue;

             child->mouse_in = 1;
             ev->canvas = point;

             _evas_event_havemap_adjust(eo_child, child, &ev->canvas.x,
                                        &ev->canvas.y,
                                        child->mouse_grabbed);
             evas_object_event_callback_call(eo_child, child,
                                             EVAS_CALLBACK_MOUSE_IN, ev,
                                             event_id);
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
}

static void
_evas_event_source_mouse_out_events(Evas_Object *eo_obj, Evas *eo_e, Evas_Event_Mouse_Out *ev, int event_id)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object *eo_src = _evas_object_image_source_get(eo_obj);
   Evas_Object_Protected_Data *src = eo_data_scope_get(eo_src, EVAS_OBJ_CLASS);
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Coord_Point canvas = ev->canvas;
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
        Evas_Object_Protected_Data *child = eo_data_scope_get(eo_child,
                                                        EVAS_OBJ_CLASS);
        if (!child->mouse_in) continue;
        child->mouse_in = 0;

        if (child->delete_me) continue;

        ev->canvas = point;

        _evas_event_havemap_adjust(eo_child, child, &ev->canvas.x,
                                   &ev->canvas.y,  child->mouse_grabbed);
        evas_object_event_callback_call(eo_child, child,
                                        EVAS_CALLBACK_MOUSE_OUT, ev, event_id);
        if (e->is_frozen) continue;
     }

   eina_list_free(copy);

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_write)
     proxy_write->src_event_in = eina_list_free(proxy_write->src_event_in);
   EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_write);

   ev->canvas = canvas;
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
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Layer *lay;
   Eina_List *in = NULL;

   if (!e->layers) return NULL;

   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
        int no_rep = 0;
        in = _evas_event_object_list_in_get(eo_e, in,
                                            EINA_INLIST_GET(lay->objects),
                                            stop, x, y, &no_rep, EINA_FALSE);
        if (no_rep) return in;
     }
   return in;
}

EOLIAN Eina_List*
_evas_tree_objects_at_xy_get(Eo *eo_e, Evas_Public_Data *e EINA_UNUSED, Evas_Object *stop, int x, int y)
{
   return _evas_event_objects_event_list_no_frozen_check(eo_e, stop, x, y);
}

Eina_List *
evas_event_objects_event_list(Evas *eo_e, Evas_Object *stop, int x, int y)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);

   if ((!e->layers) || (e->is_frozen)) return NULL;
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
_evas_event_default_flags_set(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Event_Flags flags)
{
   e->default_event_flags = flags;
}

EOLIAN Evas_Event_Flags
_evas_event_default_flags_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->default_event_flags;
}

static inline void
_canvas_event_thaw_eval_internal(Eo *eo_e, Evas_Public_Data *e)
{
   evas_event_feed_mouse_move(eo_e, e->pointer.x, e->pointer.y,
         e->last_timestamp, NULL);
}

EAPI void
evas_event_freeze(Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, eo_event_freeze());
}

EAPI void
evas_event_thaw(Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, eo_event_thaw());
}

void
_canvas_event_freeze(Eo *eo_e, void *_pd, va_list *list EINA_UNUSED)
{
   eo_do_super(eo_e, EVAS_CLASS, eo_event_freeze());
   Evas_Public_Data *e = _pd;
   e->is_frozen = EINA_TRUE;
}

void
_canvas_event_thaw(Eo *eo_e, void *_pd, va_list *list EINA_UNUSED)
{
   int fcount = -1;
   eo_do_super(eo_e, EVAS_CLASS,
         eo_event_thaw());
   eo_do_super(eo_e, EVAS_CLASS,
         fcount = eo_event_freeze_get());
   if (0 == fcount)
     {
        Evas_Public_Data *e = _pd;
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
   eo_do((Eo *)eo_e, ret = eo_event_freeze_get());
   return ret;
}

EAPI void
evas_event_thaw_eval(Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   if (0 == evas_event_freeze_get(eo_e))
     {
        _canvas_event_thaw_eval_internal(eo_e, e);
     }
}

EOLIAN void
_evas_event_feed_mouse_down(Eo *eo_e, Evas_Public_Data *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   Eina_List *l, *copy;
   Evas_Event_Mouse_Down ev;
   Evas_Object *eo_obj;
   int addgrab = 0;
   int event_id = 0;

   INF("ButtonEvent:down time=%u x=%d y=%d button=%d downs=%d", timestamp, e->pointer.x, e->pointer.y, b, e->pointer.downs);
   if ((b < 1) || (b > 32)) return;

   e->pointer.button |= (1 << (b - 1));
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
   if (ev.dev) _evas_device_ref(ev.dev);

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
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
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
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
        if (obj->delete_me) continue;
        ev.canvas.x = e->pointer.x;
        ev.canvas.y = e->pointer.y;
        _evas_event_havemap_adjust(eo_obj, obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);

        evas_object_event_callback_call(eo_obj, obj,
                                        EVAS_CALLBACK_MOUSE_DOWN, &ev,
                                        event_id);
        if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
          _evas_event_source_mouse_down_events(eo_obj, eo_e, &ev,
                                               event_id);
        if (e->is_frozen || e->delete_me)  break;
        if (obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN)
          break;
     }
   if (copy) eina_list_free(copy);
   e->last_mouse_down_counter++;
   _evas_post_event_callback_call(eo_e, e);
   /* update touch point's state to EVAS_TOUCH_POINT_STILL */
   _evas_touch_point_update(eo_e, 0, e->pointer.x, e->pointer.y, EVAS_TOUCH_POINT_STILL);
   if (ev.dev) _evas_device_unref(ev.dev);
   _evas_unwalk(e);
}

static int
_post_up_handle(Evas *eo_e, unsigned int timestamp, const void *data)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
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
   if (ev.dev) _evas_device_ref(ev.dev);
   
   /* get new list of ins */
   ins = evas_event_objects_event_list(eo_e, NULL, e->pointer.x, e->pointer.y);
   /* go thru old list of in objects */
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, ll, eo_obj)
     {
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
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
                  evas_object_event_callback_call(eo_obj, obj,
                                                  EVAS_CALLBACK_MOUSE_OUT,
                                                  &ev, event_id);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_out_events(eo_obj, eo_e, &ev,
                                                        event_id);
                  if (e->delete_me) break;
               }
          }
     }
   _evas_post_event_callback_call(eo_e, e);

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
             Evas_Object_Protected_Data *obj_itr = eo_data_scope_get(eo_obj_itr, EVAS_OBJ_CLASS);
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
                  evas_object_event_callback_call(eo_obj_itr, obj_itr,
                                                  EVAS_CALLBACK_MOUSE_IN,
                                                  &ev_in, event_id);
                  if ((obj_itr->proxy->is_proxy) &&
                      (obj_itr->proxy->src_events))
                    _evas_event_source_mouse_in_events(eo_obj_itr, eo_e, &ev_in,
                                                       event_id);
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
      evas_event_feed_mouse_move(eo_e, e->pointer.x, e->pointer.y, timestamp, data);
   if (ev.dev) _evas_device_unref(ev.dev);
   return post_called;
}

EOLIAN void
_evas_event_feed_mouse_up(Eo *eo_e, Evas_Public_Data *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   Eina_List *l, *copy;

   INF("ButtonEvent:up time=%u x=%d y=%d button=%d downs=%d", timestamp, e->pointer.x, e->pointer.y, b, e->pointer.downs);
   if ((b < 1) || (b > 32)) return;
   if (e->pointer.downs <= 0) return;

   e->pointer.button &= ~(1 << (b - 1));
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
        if (ev.dev) _evas_device_ref(ev.dev);
        
        _evas_walk(e);
        /* update released touch point */
        _evas_touch_point_update(eo_e, 0, e->pointer.x, e->pointer.y, EVAS_TOUCH_POINT_UP);
        copy = evas_event_list_copy(e->pointer.object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
             if ((obj->pointer_mode == EVAS_OBJECT_POINTER_MODE_AUTOGRAB) &&
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
                       evas_object_event_callback_call(eo_obj, obj,
                                                       EVAS_CALLBACK_MOUSE_UP,
                                                       &ev, event_id);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_up_events(eo_obj, eo_e, &ev,
                                                            event_id);
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
        if (ev.dev) _evas_device_unref(ev.dev);
     }

   if (e->pointer.mouse_grabbed == 0)
     {
        _post_up_handle(eo_e, timestamp, data);
     }

   if (e->pointer.mouse_grabbed < 0)
     {
        ERR("BUG? e->pointer.mouse_grabbed (=%d) < 0!",
            e->pointer.mouse_grabbed);
     }
   /* remove released touch point from the touch point list */
   _evas_touch_point_remove(eo_e, 0);

   _evas_unwalk(e);
}

EOLIAN void
_evas_event_feed_mouse_cancel(Eo *eo_e, Evas_Public_Data *e, unsigned int timestamp, const void *data)
{
   Evas_Coord_Touch_Point *point;
   Eina_List *l, *ll;
   int i;

   if (e->is_frozen) return;

   _evas_walk(e);
   for (i = 0; i < 32; i++)
     {
        if ((e->pointer.button & (1 << i)))
          evas_event_feed_mouse_up(eo_e, i + 1, 0, timestamp, data);
     }
   EINA_LIST_FOREACH_SAFE(e->touch_points, l, ll, point)
     {
        if ((point->state == EVAS_TOUCH_POINT_DOWN) ||
            (point->state == EVAS_TOUCH_POINT_MOVE))
          evas_event_feed_multi_up(eo_e, point->id, point->x, point->y,
                                   0, 0, 0, 0, 0, 0, 0, 0, timestamp, data);
     }
   _evas_unwalk(e);
}

EOLIAN void
_evas_event_feed_mouse_wheel(Eo *eo_e, Evas_Public_Data *e, int direction, int z, unsigned int timestamp, const void *data)
{
   Eina_List *l, *copy;
   Evas_Event_Mouse_Wheel ev;
   Evas_Object *eo_obj;
   int event_id = 0;

   if (e->is_frozen) return;
   e->last_timestamp = timestamp;

   _evas_object_event_new();

   event_id = _evas_event_counter;
   ev.direction = direction;
   ev.z = z;
   ev.output.x = e->pointer.x;
   ev.output.y = e->pointer.y;
   ev.canvas.x = e->pointer.x;
   ev.canvas.y = e->pointer.y;
   ev.data = (void *) data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.timestamp = timestamp;
   ev.event_flags = e->default_event_flags;
   ev.dev = _evas_device_top_get(eo_e);
   if (ev.dev) _evas_device_ref(ev.dev);
   
   _evas_walk(e);
   copy = evas_event_list_copy(e->pointer.object.in);

   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
        if (!evas_event_freezes_through(eo_obj, obj))
          {
             ev.canvas.x = e->pointer.x;
             ev.canvas.y = e->pointer.y;
             _evas_event_havemap_adjust(eo_obj, obj, &ev.canvas.x, &ev.canvas.y,
                                        obj->mouse_grabbed);
             evas_object_event_callback_call(eo_obj, obj,
                                             EVAS_CALLBACK_MOUSE_WHEEL, &ev,
                                             event_id);
             if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
               _evas_event_source_wheel_events(eo_obj, eo_e, &ev, event_id);
             if (e->delete_me || e->is_frozen) break;
          }
     }
   eina_list_free(copy);
   _evas_post_event_callback_call(eo_e, e);

   if (ev.dev) _evas_device_unref(ev.dev);
   _evas_unwalk(e);
}

static void
_canvas_event_feed_mouse_move_internal(Eo *eo_e, void *_pd, int x, int y, unsigned int timestamp, const void *data)
{
   Evas_Public_Data *e = _pd;
   Evas_Object *nogrep_obj = NULL;
   int px, py;
////   Evas_Coord pcx, pcy;

   px = e->pointer.x;
   py = e->pointer.y;
////   pcx = e->pointer.canvas_x;
////   pcy = e->pointer.canvas_y;

   if (e->is_frozen) return;
   e->last_timestamp = timestamp;

   e->pointer.x = x;
   e->pointer.y = y;
////   e->pointer.canvas_x = x;
////   e->pointer.canvas_y = y;
////   e->pointer.canvas_x = evas_coord_screen_x_to_world(eo_e, x);
////   e->pointer.canvas_y = evas_coord_screen_y_to_world(eo_e, y);
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
             if (ev.dev) _evas_device_ref(ev.dev);
             copy = evas_event_list_copy(e->pointer.object.in);
             EINA_LIST_FOREACH(copy, l, eo_obj)
               {
                  Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
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
                            evas_object_event_callback_call(eo_obj, obj,
                                                            EVAS_CALLBACK_MOUSE_MOVE, &ev, event_id);
                            if ((obj->proxy->is_proxy) &&
                                (obj->proxy->src_events))
                              _evas_event_source_mouse_move_events(eo_obj, eo_e,
                                                                   &ev,
                                                                   event_id);
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
             if (ev.dev) _evas_device_unref(ev.dev);
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
             if (ev.dev) _evas_device_ref(ev.dev);
             
             eina_list_free(copy);

             while (outs)
               {
                  Evas_Object *eo_obj;
                  eo_obj = outs->data;
                  outs = eina_list_remove(outs, eo_obj);
                  Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
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
                       evas_object_event_callback_call(eo_obj, obj,
                                                       EVAS_CALLBACK_MOUSE_OUT,
                                                       &ev, event_id);
                       if ((obj->proxy->is_proxy) &&
                           (obj->proxy->src_events))
                         _evas_event_source_mouse_out_events(eo_obj, eo_e, &ev,
                                                             event_id);
                    }
               }
             _evas_post_event_callback_call(eo_e, e);
             if (ev.dev) _evas_device_unref(ev.dev);
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
        if (ev.dev) _evas_device_ref(ev.dev);
        
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
             Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
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
                       evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_MOVE, &ev, event_id);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_move_events(eo_obj, eo_e, &ev,
                                                              event_id);
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
                       evas_object_event_callback_call(eo_obj, obj,
                                                       EVAS_CALLBACK_MOUSE_OUT,
                                                       &ev2, event_id);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_out_events(eo_obj, eo_e, &ev2,
                                                             event_id);
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
             Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
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
                       evas_object_event_callback_call(eo_obj, obj,
                                                       EVAS_CALLBACK_MOUSE_IN,
                                                       &ev3, event_id2);
                       if ((obj->proxy->is_proxy) &&
                           (obj->proxy->src_events))
                         _evas_event_source_mouse_in_events(eo_obj, eo_e, &ev3,
                                                            event_id2);
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
        if (ev.dev) _evas_device_unref(ev.dev);
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
        if (ev.dev) _evas_device_ref(ev.dev);
        
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
             Evas_Object_Protected_Data *below_obj = eo_data_scope_get(eo_below_obj, EVAS_OBJ_CLASS);
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
             Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
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
                       evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_MOVE, &ev, event_id);
                       if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                         _evas_event_source_mouse_move_events(eo_obj, eo_e, &ev,
                                                              event_id);
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
                  evas_object_event_callback_call(eo_obj, obj,
                                                  EVAS_CALLBACK_MOUSE_OUT, &ev2,
                                                  event_id);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_out_events(eo_obj, eo_e, &ev2,
                                                        event_id);
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
             Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
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
                  evas_object_event_callback_call(eo_obj, obj,
                                                  EVAS_CALLBACK_MOUSE_IN, &ev3,
                                                  event_id2);
                  if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
                    _evas_event_source_mouse_in_events(eo_obj, eo_e, &ev3,
                                                       event_id2);
                  if (e->delete_me) break;
               }
          }
        /* free our old list of ins */
        eina_list_free(e->pointer.object.in);
        /* and set up the new one */
        e->pointer.object.in = newin;

        _evas_post_event_callback_call(eo_e, e);
        if (ev.dev) _evas_device_unref(ev.dev);
     }
   _evas_unwalk(e);
}

EOLIAN void
_evas_event_input_mouse_move(Eo *eo_e, Evas_Public_Data *e, int x, int y, unsigned int timestamp, const void *data)
{
   _canvas_event_feed_mouse_move_internal(eo_e, e, x - e->framespace.x, y - e->framespace.y, timestamp, data);
}

EOLIAN void
_evas_event_feed_mouse_move(Eo *eo_e, Evas_Public_Data *e, int x, int y, unsigned int timestamp, const void *data)
{
   _canvas_event_feed_mouse_move_internal(eo_e, e, x, y, timestamp, data);
}

EOLIAN void
_evas_event_feed_mouse_in(Eo *eo_e, Evas_Public_Data *e, unsigned int timestamp, const void *data)
{

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
   if (ev.dev) _evas_device_ref(ev.dev);
   
   _evas_walk(e);
   /* get new list of ins */
   ins = evas_event_objects_event_list(eo_e, NULL, e->pointer.x, e->pointer.y);
   EINA_LIST_FOREACH(ins, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
        if (!eina_list_data_find(e->pointer.object.in, eo_obj))
          {
             if (obj->mouse_in) continue;
             obj->mouse_in = 1;
             ev.canvas.x = e->pointer.x;
             ev.canvas.y = e->pointer.y;
             _evas_event_havemap_adjust(eo_obj, obj, &ev.canvas.x, &ev.canvas.y,
                                        obj->mouse_grabbed);
             evas_object_event_callback_call(eo_obj, obj,
                                             EVAS_CALLBACK_MOUSE_IN, &ev,
                                             event_id);
             if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
               _evas_event_source_mouse_in_events(eo_obj, eo_e, &ev, event_id);
             if (e->delete_me || e->is_frozen) break;
          }
     }
   /* free our old list of ins */
   e->pointer.object.in = eina_list_free(e->pointer.object.in);
   /* and set up the new one */
   e->pointer.object.in = ins;
   _evas_post_event_callback_call(eo_e, e);
   evas_event_feed_mouse_move(eo_e, e->pointer.x, e->pointer.y, timestamp, data);
   if (ev.dev) _evas_device_unref(ev.dev);
   _evas_unwalk(e);
}

EOLIAN void
_evas_event_feed_mouse_out(Eo *eo_e, Evas_Public_Data *e, unsigned int timestamp, const void *data)
{

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
   if (ev.dev) _evas_device_ref(ev.dev);

   _evas_walk(e);
   /* if our mouse button is inside any objects */
   /* go thru old list of in objects */
   Eina_List *l, *copy;
   Evas_Object *eo_obj;

   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
        if (!obj->mouse_in) continue;
        obj->mouse_in = 0;
        if (!obj->delete_me)
          {
             ev.canvas.x = e->pointer.x;
             ev.canvas.y = e->pointer.y;
             _evas_event_havemap_adjust(eo_obj, obj, &ev.canvas.x, &ev.canvas.y,
                                        obj->mouse_grabbed);
             evas_object_event_callback_call(eo_obj, obj,
                                             EVAS_CALLBACK_MOUSE_OUT, &ev,
                                             event_id);
             if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
               _evas_event_source_mouse_out_events(eo_obj, eo_e, &ev, event_id);
             if (e->delete_me || e->is_frozen) break;
          }
        obj->mouse_grabbed = 0;
     }
   eina_list_free(copy);

   /* free our old list of ins */
   e->pointer.object.in =  eina_list_free(e->pointer.object.in);
   e->pointer.mouse_grabbed = 0;
   _evas_post_event_callback_call(eo_e, e);
   if (ev.dev) _evas_device_unref(ev.dev);
   _evas_unwalk(e);
}

static void
_canvas_event_feed_multi_down_internal(Evas *eo_e, void *_pd,
                           int d, int x, int y,
                           double rad, double radx, double rady,
                           double pres, double ang,
                           double fx, double fy,
                           Evas_Button_Flags flags, unsigned int timestamp,
                           const void *data)
{
   Evas_Public_Data *e = _pd;
   Eina_List *l, *copy;
   Evas_Event_Multi_Down ev;
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
   if (ev.dev) _evas_device_ref(ev.dev);

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
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
        if (obj->pointer_mode != EVAS_OBJECT_POINTER_MODE_NOGRAB)
          {
             obj->mouse_grabbed += addgrab + 1;
             e->pointer.mouse_grabbed += addgrab + 1;
          }
     }
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
        ev.canvas.x = x;
        ev.canvas.y = y;
        ev.canvas.xsub = fx;
        ev.canvas.ysub = fy;
        _evas_event_havemap_adjust(eo_obj, obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
        if (x != ev.canvas.x)
          ev.canvas.xsub = ev.canvas.x; // fixme - lost precision
        if (y != ev.canvas.y)
          ev.canvas.ysub = ev.canvas.y; // fixme - lost precision
        evas_object_event_callback_call(eo_obj, obj,
                                        EVAS_CALLBACK_MULTI_DOWN, &ev,
                                        event_id);
        if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
          _evas_event_source_multi_down_events(eo_obj, eo_e, &ev, event_id);
        if (e->delete_me || e->is_frozen) break;
     }
   eina_list_free(copy);

   _evas_post_event_callback_call(eo_e, e);
   /* update touch point's state to EVAS_TOUCH_POINT_STILL */
   _evas_touch_point_update(eo_e, d, x, y, EVAS_TOUCH_POINT_STILL);
   if (ev.dev) _evas_device_unref(ev.dev);
   _evas_unwalk(e);
}

EOLIAN void
_evas_event_input_multi_down(Eo *eo_e, Evas_Public_Data *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{

   _canvas_event_feed_multi_down_internal(eo_e, e, d,
                                          x - e->framespace.x,
                                          y - e->framespace.y,
                                          rad, radx, rady, pres, ang,
                                          fx, fy, flags, timestamp, data);
}

EOLIAN void
_evas_event_feed_multi_down(Eo *eo_e, Evas_Public_Data *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{

   _canvas_event_feed_multi_down_internal(eo_e, e,
                                          d, x, y, rad, radx, rady, pres, ang,
                                          fx, fy, flags, timestamp, data);
}

static void
_canvas_event_feed_multi_up_internal(Evas *eo_e, void *_pd,
                                     int d, int x, int y,
                                     double rad, double radx, double rady,
                                     double pres, double ang,
                                     double fx, double fy,
                                     Evas_Button_Flags flags,
                                     unsigned int timestamp,
                                     const void *data)
{
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
   if (ev.dev) _evas_device_ref(ev.dev);
   
   _evas_walk(e);
   /* update released touch point */
   _evas_touch_point_update(eo_e, d, x, y, EVAS_TOUCH_POINT_UP);
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
        ev.canvas.x = x;
        ev.canvas.y = y;
        ev.canvas.xsub = fx;
        ev.canvas.ysub = fy;
        _evas_event_havemap_adjust(eo_obj, obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
        if (x != ev.canvas.x)
          ev.canvas.xsub = ev.canvas.x; // fixme - lost precision
        if (y != ev.canvas.y)
          ev.canvas.ysub = ev.canvas.y; // fixme - lost precision
        if ((obj->pointer_mode != EVAS_OBJECT_POINTER_MODE_NOGRAB) &&
            (obj->mouse_grabbed > 0))
          {
             obj->mouse_grabbed--;
             e->pointer.mouse_grabbed--;
          }
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MULTI_UP,
                                        &ev, event_id);
        if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
          _evas_event_source_multi_up_events(eo_obj, eo_e, &ev, event_id);
        if (e->delete_me || e->is_frozen) break;
     }
   eina_list_free(copy);
   if ((e->pointer.mouse_grabbed == 0) && !_post_up_handle(eo_e, timestamp, data))
      _evas_post_event_callback_call(eo_e, e);
   /* remove released touch point from the touch point list */
   _evas_touch_point_remove(eo_e, d);
   if (ev.dev) _evas_device_unref(ev.dev);
   _evas_unwalk(e);
}

EOLIAN void
_evas_event_input_multi_up(Eo *eo_e, Evas_Public_Data *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{

   _canvas_event_feed_multi_up_internal(eo_e, e, d,
                                        x - e->framespace.x,
                                        y - e->framespace.y,
                                        rad, radx, rady,
                                        pres, ang, fx, fy, flags, timestamp,
                                        data);
}

EOLIAN void
_evas_event_feed_multi_up(Eo *eo_e, Evas_Public_Data *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{

   _canvas_event_feed_multi_up_internal(eo_e, e, d, x, y, rad, radx, rady,
                                        pres, ang, fx, fy, flags, timestamp, data);
}

static void
_canvas_event_feed_multi_move_internal(Eo *eo_e, void *_pd, int d, int x,
                                       int y, double rad, double radx,
                                       double rady, double pres, double ang,
                                       double fx, double fy,
                                       unsigned int timestamp,
                                       const void *data)
{
   Evas_Public_Data *e = _pd;

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
        if (ev.dev) _evas_device_ref(ev.dev);

        copy = evas_event_list_copy(e->pointer.object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
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
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MULTI_MOVE, &ev, event_id);
                  if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
                    _evas_event_source_multi_move_events(eo_obj, eo_e, &ev,
                                                         event_id);

                  if (e->delete_me || e->is_frozen) break;
               }
          }
        eina_list_free(copy);
        _evas_post_event_callback_call(eo_e, e);
        if (ev.dev) _evas_device_unref(ev.dev);
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
        if (ev.dev) _evas_device_ref(ev.dev);
        
        /* get all new in objects */
        ins = evas_event_objects_event_list(eo_e, NULL, x, y);
        /* go thru old list of in objects */
        copy = evas_event_list_copy(e->pointer.object.in);
        EINA_LIST_FOREACH(copy, l, eo_obj)
          {
             Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
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
                  evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MULTI_MOVE, &ev, event_id);
                  if ((obj->proxy->is_proxy) || (obj->proxy->src_events))
                    _evas_event_source_multi_move_events(eo_obj, eo_e, &ev,
                                                         event_id);
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
        if (ev.dev) _evas_device_unref(ev.dev);
     }
   _evas_unwalk(e);
}

EOLIAN void
_evas_event_input_multi_move(Eo *eo_e, Evas_Public_Data *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, unsigned int timestamp, const void *data)
{

   _canvas_event_feed_multi_move_internal(eo_e, e, d,
                                          x - e->framespace.x, y - e->framespace.y,
                                          rad, radx, rady,
                                          pres, ang, fx, fy, timestamp, data);
}

EOLIAN void
_evas_event_feed_multi_move(Eo *eo_e, Evas_Public_Data *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, unsigned int timestamp, const void *data)
{

   _canvas_event_feed_multi_move_internal(eo_e, e, d, x, y, rad, radx, rady,
                                          pres, ang, fx, fy, timestamp, data);
}

EOLIAN void
_evas_event_feed_key_down(Eo *eo_e, Evas_Public_Data *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data)
{

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
   if (ev.dev) _evas_device_ref(ev.dev);
   
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
             if (((e->modifiers.mask & g->modifiers) ||
                  (g->modifiers == e->modifiers.mask)) &&
                 (!strcmp(keyname, g->keyname)))
               {
                  if (!(e->modifiers.mask & g->not_modifiers))
                    {
                       Evas_Object_Protected_Data *object_obj = eo_data_scope_get(g->object, EVAS_OBJ_CLASS);
                       if (!e->is_frozen &&
                           !evas_event_freezes_through(g->object, object_obj))
                         {
                            evas_object_event_callback_call(g->object,
                                                            object_obj,
                                                            EVAS_CALLBACK_KEY_DOWN, &ev, event_id);
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
                            Evas_Object_Protected_Data *g_object_obj = eo_data_scope_get(g->object, EVAS_OBJ_CLASS);
                            evas_key_grab_free(g->object, g_object_obj, g->keyname,
                                               g->modifiers, g->not_modifiers);
                         }
                    }
               }
          }
     }
   if ((e->focused) && (!exclusive))
     {
        Evas_Object_Protected_Data *focused_obj = eo_data_scope_get(e->focused, EVAS_OBJ_CLASS);
        if (!e->is_frozen && !evas_event_freezes_through(e->focused, focused_obj))
             evas_object_event_callback_call(e->focused, focused_obj,
                                             EVAS_CALLBACK_KEY_DOWN,
                                             &ev, event_id);
     }
   _evas_post_event_callback_call(eo_e, e);
   if (ev.dev) _evas_device_unref(ev.dev);
   _evas_unwalk(e);
}

EOLIAN void
_evas_event_feed_key_up(Eo *eo_e, Evas_Public_Data *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data)
{

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
   if (ev.dev) _evas_device_ref(ev.dev);
   
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
             if (((e->modifiers.mask & g->modifiers) ||
                  (g->modifiers == e->modifiers.mask)) &&
                 (!((e->modifiers.mask & g->not_modifiers) ||
                    (g->not_modifiers == ~e->modifiers.mask))) &&
                 (!strcmp(keyname, g->keyname)))
               {
                  Evas_Object_Protected_Data *object_obj = eo_data_scope_get(g->object, EVAS_OBJ_CLASS);
                  if (!e->is_frozen &&
                        !evas_event_freezes_through(g->object, object_obj))
                    {
                       evas_object_event_callback_call(g->object, object_obj,
                                                       EVAS_CALLBACK_KEY_UP,
                                                       &ev, event_id);
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
                               eo_data_scope_get(gr->object, EVAS_OBJ_CLASS);
                            evas_key_grab_free(gr->object, gr_object_obj, gr->keyname,
                                            gr->modifiers, gr->not_modifiers);
                         }
                    }
               }
          }
     }
   if ((e->focused) && (!exclusive))
     {
        Evas_Object_Protected_Data *focused_obj = eo_data_scope_get(e->focused, EVAS_OBJ_CLASS);
        if (!e->is_frozen && !evas_event_freezes_through(e->focused, focused_obj))
             evas_object_event_callback_call(e->focused, focused_obj,
                                             EVAS_CALLBACK_KEY_UP,
                                             &ev, event_id);
     }
   _evas_post_event_callback_call(eo_e, e);
   if (ev.dev) _evas_device_unref(ev.dev);
   _evas_unwalk(e);
}

EOLIAN void
_evas_event_feed_hold(Eo *eo_e, Evas_Public_Data *e, int hold, unsigned int timestamp, const void *data)
{

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
   if (ev.dev) _evas_device_ref(ev.dev);
   
   _evas_walk(e);
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, eo_obj)
     {
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
        if ( !evas_event_freezes_through(eo_obj, obj))
          {
             evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_HOLD,
                                             &ev, event_id);
             if ((obj->proxy->is_proxy) && (obj->proxy->src_events))
               _evas_event_source_hold_events(eo_obj, eo_e, &ev, event_id);
          }
        if (e->delete_me || e->is_frozen) break;
     }
   eina_list_free(copy);
   _evas_post_event_callback_call(eo_e, e);
   if (ev.dev) _evas_device_unref(ev.dev);
   _evas_unwalk(e);
   _evas_object_event_new();
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
     evas_event_feed_mouse_move(evas->evas,
                                evas->pointer.x,
                                evas->pointer.y,
                                evas->last_timestamp,
                                NULL);
}

EOLIAN void
_evas_object_freeze_events_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool freeze)
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
_evas_object_freeze_events_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->freeze_events;
}

EOLIAN void
_evas_object_pass_events_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool pass)
{
   pass = !!pass;
   if (obj->pass_events == pass) return;
   obj->pass_events = pass;
   evas_object_smart_member_cache_invalidate(eo_obj, EINA_TRUE, EINA_FALSE, EINA_FALSE);
   _feed_mouse_move_eval_internal(eo_obj, obj);
}

EOLIAN Eina_Bool
_evas_object_pass_events_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->pass_events;
}

EOLIAN void
_evas_object_repeat_events_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool repeat)
{
   repeat = !!repeat;
   if (obj->repeat_events == repeat) return;
   obj->repeat_events = repeat;
   _feed_mouse_move_eval_internal(eo_obj, obj);
}

EOLIAN Eina_Bool
_evas_object_repeat_events_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->repeat_events;
}

EOLIAN void
_evas_object_propagate_events_set(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Eina_Bool prop)
{
   obj->no_propagate = !prop;
}

EOLIAN Eina_Bool
_evas_object_propagate_events_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return !(obj->no_propagate);
}

EOLIAN void
_evas_object_pointer_mode_set(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Object_Pointer_Mode setting)
{
   obj->pointer_mode = setting;
}

EOLIAN Evas_Object_Pointer_Mode
_evas_object_pointer_mode_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->pointer_mode;
}

EOLIAN void
_evas_event_refeed_event(Eo *eo_e, Evas_Public_Data *e EINA_UNUSED, void *event_copy, Evas_Callback_Type event_type)
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
      default: /* All non-input events are not handeled */
        break;
     }
}

EOLIAN int
_evas_event_down_count_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->pointer.downs;
}


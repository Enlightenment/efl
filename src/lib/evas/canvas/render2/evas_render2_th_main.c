//#define DBG_OBJTREE 1

#define OBJ_ARRAY_PUSH(array, obj)     \
   do                                  \
   {                                   \
      eina_array_push(array, obj);     \
      efl_data_ref(obj->object, EFL_CANVAS_OBJECT_CLASS);  \
   } while (0)

#define OBJS_ARRAY_CLEAN(array)                          \
   do                                                    \
   {                                                     \
      Evas_Object_Protected_Data *item;                  \
      Eina_Array_Iterator iterator;                      \
      unsigned int idx;                                  \
      EINA_ARRAY_ITER_NEXT(array, idx, item, iterator)   \
      efl_data_unref(item->object, item);                 \
      eina_array_clean(array);                           \
   } while (0)

typedef struct _Msg_Main_Render Msg_Main_Render;
typedef struct _Render2_Finish_Data Render2_Finish_Data;

struct _Msg_Main_Render
{
   Eina_Thread_Queue_Msg  head;
   Eo                    *eo_e;
   Evas_Public_Data      *e;
};

struct _Render2_Finish_Data
{
   Eo                    *eo_e;
   Evas_Public_Data      *e;
   Eina_List             *updates;
};

static Eina_Thread_Queue *_th_main_queue = NULL;
static Eina_Thread _th_main;

#ifdef DBG_OBJTREE
static void
indent(int l)
{
   int i; for (i = 0; i < l; i++) printf(" ");
}
#endif

static void
_evas_render2_th_main_delete_objects_clean(Evas_Public_Data *e)
{
   Evas_Object_Protected_Data *obj;
   unsigned int i;
   double t;

   // cleanup deferred object deletion
   t = get_time();
   for (i = 0; i < e->delete_objects.count; ++i)
     {
        obj = eina_array_data_get(&e->delete_objects, i);
        evas_object_free(obj, 1);
     }
//   OBJS_ARRAY_CLEAN(&e->delete_objects);
   eina_array_clean(&e->delete_objects);
   t = get_time() - t;
   printf("T: object deletion: "); out_time(t);
}

static void
_evas_render2_th_main_mainloop_done(void *data, Evas_Callback_Type type EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Render2_Finish_Data *render_finish_data = data;
   Evas_Event_Render_Post post;
   Eo *eo_e;
   Evas_Public_Data *e;
   Eina_Rectangle *rect;

   e = render_finish_data->e;
   eo_e = render_finish_data->eo_e;
   // call the flush post callbacks
   _always_call(render_finish_data->eo_e, EVAS_CALLBACK_RENDER_FLUSH_POST, NULL);

   // and now call the actual render post callbacks with updates list
   post.updated_area = render_finish_data->updates;
   _always_call(eo_e, EVAS_CALLBACK_RENDER_POST, &post);

   EINA_LIST_FREE(render_finish_data->updates, rect) free(rect);
   free(render_finish_data);
   _evas_render2_th_main_delete_objects_clean(e);
}

static Eina_Bool
_evas_render2_th_main_obj_del_handle(Evas_Public_Data *e,
                                     Evas_Object_Protected_Data *obj)
{
   if (obj->delete_me == 2)
     {
        OBJ_ARRAY_PUSH(&e->delete_objects, obj);
        obj->delete_me++;
        return EINA_FALSE;
     }
   else if (obj->delete_me != 0) obj->delete_me++;
   return EINA_TRUE;
}

static void
_evas_render2_th_main_obj_basic_walk_process(Evas_Public_Data *e,
                                             Evas_Object_Protected_Data *obj,
                                             void *updates,
                                             int offx,
                                             int offy,
                                             int l EINA_UNUSED)
{
   Evas_Object *eo_obj = obj->object;

   if (!_evas_render2_th_main_obj_del_handle(e, obj)) return;
   evas_object_clip_recalc(obj);
#ifdef DBG_OBJTREE
   indent(l); printf("BASIC %p %p [%10s]\n", e, eo_obj, obj->type);
#endif
   if (obj->func->render2_walk)
     obj->func->render2_walk(eo_obj, obj, obj->private_data,
                             updates, offx, offy);
   if (obj->changed)
     {
        evas_object_clip_changes_clean(obj);
        evas_object_cur_prev(obj);
        evas_object_change_reset(obj);
     }
}

static void
_evas_render2_th_main_obj_walk_process(Evas_Public_Data *e,
                                       Evas_Object_Protected_Data *obj,
                                       void *updates,
                                       int offx,
                                       int offy,
                                       int l EINA_UNUSED)
{
   // process object OR walk through child objects if smart and process those
   Evas_Object_Protected_Data *obj2;
   Evas_Object *eo_obj = obj->object;
   const Eina_Inlist *il;

   il = obj->is_smart ? evas_object_smart_members_get_direct(eo_obj) : NULL;
   if (il)
     {
        if (!_evas_render2_th_main_obj_del_handle(e, obj)) return;
        evas_object_clip_recalc(obj);
#ifdef DBG_OBJTREE
        indent(l); printf("SMART %p %p [%10s] ch %i\n", e, eo_obj, obj->type, obj->changed);
#endif
        if (obj->func->render2_walk)
          obj->func->render2_walk(eo_obj, obj, obj->private_data,
                                  updates, offx, offy);
        EINA_INLIST_FOREACH(il, obj2)
          _evas_render2_th_main_obj_walk_process(e, obj2, updates,
                                                 offx, offy, l + 1);
        if (obj->changed)
          {
             evas_object_clip_changes_clean(obj);
             evas_object_cur_prev(obj);
             evas_object_change_reset(obj);
          }
     }
   else _evas_render2_th_main_obj_basic_walk_process(e, obj, updates,
                                                     offx, offy, l);
}

static Region *
_evas_render2_regions_merge(Region *region)
{
   Region *region2;
   Box *rects;
   int num, i, w, h;
   int tsize = 16;

   region_size_get(region, &w, &h);
   region2 = region_new(w, h);
   rects = region_rects(region);
   num = region_rects_num(region);
   for (i = 0; i < num; i++)
     {
        int x1, y1, x2, y2;

        x1 = (rects[i].x1 / tsize) * tsize;
        y1 = (rects[i].y1 / tsize) * tsize;
        x2 = ((rects[i].x2 + tsize - 1) / tsize) * tsize;
        y2 = ((rects[i].y2 + tsize - 1) / tsize) * tsize;
        region_rect_add(region2, x1, y1, x2 - x1, y2 - y1);
     }
   region_free(region);
   return region2;
}

static void
_evas_render2_th_main_do(Eo *eo_e, Evas_Public_Data *e)
{
   Render2_Finish_Data *render_finish_data;
   Evas_Layer *lay;
   Evas_Object_Protected_Data *obj;
   Eina_List *updates_list = NULL, *l;
   double t;
   Eina_Rectangle *rect;
   Region *updates;
   Box *rects;
   int rects_num, i;
   static int num = 0;

   updates = region_new(e->output.w, e->output.h);
   printf("........... updates # %i\n", num++);
   t = get_time();
   EINA_INLIST_FOREACH(e->layers, lay)
     {
        EINA_INLIST_FOREACH(lay->objects, obj)
          {
             _evas_render2_th_main_obj_walk_process(e, obj,
                                                    updates, 0, 0,
                                                    0);
          }
     }
   // add explicitly exposed/damaged regions of the canvas
   EINA_LIST_FREE(e->damages, rect)
     {
        region_rect_add(updates, rect->x, rect->y, rect->w, rect->h);
        eina_rectangle_free(rect);
     }
   // build obscure objects list of active objects that obscure
   EINA_LIST_FOREACH(e->obscures, l, rect)
     {
        region_rect_del(updates, rect->x, rect->y, rect->w, rect->h);
     }
   t = get_time() - t;
   printf("T: update generation: "); out_time(t);

   t = get_time();
   updates = _evas_render2_regions_merge(updates);

   rects = region_rects(updates);
   rects_num = region_rects_num(updates);
   for (i = 0; i < rects_num; i++)
     {
        rect = malloc(sizeof(Eina_Rectangle));
        if (rect)
          {
             if (rects[i].x2 > e->output.w) rects[i].x2 = e->output.w;
             if (rects[i].y2 > e->output.h) rects[i].y2 = e->output.h;
             rect->x = rects[i].x1;
             rect->y = rects[i].y1;
             rect->w = rects[i].x2 - rects[i].x1;
             rect->h = rects[i].y2 - rects[i].y1;
             printf(" Render Region [ %4i %4i %4ix%4i ]\n",
                    rect->x, rect->y, rect->w, rect->h);
             updates_list = eina_list_append(updates_list, rect);
          }
     }
   t = get_time() - t;
   printf("T: merge updates: "); out_time(t);

   // ... now render every update region
   EINA_LIST_FOREACH(updates_list, l, rect)
     {
        // XXX: create update buffer
        EINA_INLIST_FOREACH(e->layers, lay)
          {
             EINA_INLIST_FOREACH(lay->objects, obj)
               {
// render to update buffer
//                  _evas_render2_th_main_obj_render_process(e, obj,
//                                                           updates, 0, 0,
//                                                           0);
               }
          }
        // delete update buffer
     }
   // free up updates we don't need anymore
   region_free(updates);

   e->changed = EINA_FALSE;
   // remove from the "i'm rendering" pool - do back in mainloop
   e->rendering = EINA_FALSE;
   _rendering = eina_list_remove(_rendering, eo_e);
   // unblock mainlopp that may be waiting on the render thread
   eina_lock_release(&(e->lock_objects));

   // send back 
   render_finish_data = calloc(1, sizeof(Render2_Finish_Data));
   if (render_finish_data)
     {
        render_finish_data->eo_e = eo_e;
        render_finish_data->e = e;
        render_finish_data->updates = updates_list;
        evas_async_events_put(render_finish_data, 0, NULL,
                              _evas_render2_th_main_mainloop_done);
     }
   else
     {
        EINA_LIST_FREE(updates_list, rect) free(rect);
     }
   efl_unref(eo_e);
}

static void *
_evas_render2_th_main(void *data EINA_UNUSED, Eina_Thread thread EINA_UNUSED)
{
   void *ref = NULL;
   Msg_Main_Render *msg;

   eina_thread_name_set(eina_thread_self(), "Eevas-render2");
   for (;;)
     {
        msg = eina_thread_queue_wait(_th_main_queue, &ref);
        if (msg)
          _evas_render2_th_main_do(msg->eo_e, msg->e);
        eina_thread_queue_wait_done(_th_main_queue, ref);
     }
   return NULL;
}

static void
_evas_render2_th_main_msg_render(Eo *eo_e, Evas_Public_Data *e)
{
   void *ref;
   Msg_Main_Render *msg =
     eina_thread_queue_send(_th_main_queue, sizeof(Msg_Main_Render), &ref);
   msg->eo_e = eo_e;
   msg->e = e;
   eina_thread_queue_send_done(_th_main_queue, ref);
}

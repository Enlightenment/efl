//#define DBG_OBJTREE 1

#define OBJ_ARRAY_PUSH(array, obj)     \
   do                                  \
   {                                   \
      eina_array_push(array, obj);     \
      eo_data_ref(obj->object, NULL);  \
   } while (0)

#define OBJS_ARRAY_CLEAN(array)                          \
   do                                                    \
   {                                                     \
      Evas_Object_Protected_Data *item;                  \
      Eina_Array_Iterator iterator;                      \
      unsigned int idx;                                  \
      EINA_ARRAY_ITER_NEXT(array, idx, item, iterator)   \
      eo_data_unref(item->object, item);                 \
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
        evas_object_free(obj->object, 1);
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
_evas_render2_th_main_obj_basic_process(Evas_Public_Data *e,
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
        evas_object_clip_changes_clean(eo_obj);
        evas_object_cur_prev(eo_obj);
        evas_object_change_reset(eo_obj);
     }
}

static void
_evas_render2_th_main_obj_process(Evas_Public_Data *e,
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

   il = evas_object_smart_members_get_direct(eo_obj);
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
          _evas_render2_th_main_obj_process(e, obj2, updates,
                                            offx, offy, l + 1);
        if (obj->changed)
          {
             evas_object_clip_changes_clean(eo_obj);
             evas_object_cur_prev(eo_obj);
             evas_object_change_reset(eo_obj);
          }
     }
   else _evas_render2_th_main_obj_basic_process(e, obj, updates,
                                                offx, offy, l);
}

static void
_evas_render2_th_main_do(Eo *eo_e, Evas_Public_Data *e)
{
   Render2_Finish_Data *render_finish_data;
   Evas_Layer *lay;
   Evas_Object_Protected_Data *obj;
   double t;
   Tilebuf *updates = NULL;
   Tilebuf_Rect *rects, *r;
   Eina_List *updates_list = NULL, *l;
   Eina_Rectangle *rect;

   updates = evas_common_tilebuf_new(e->output.w, e->output.h);
   evas_common_tilebuf_set_tile_size(updates, TILESIZE, TILESIZE);
//   evas_common_tilebuf_tile_strict_set(updates, EINA_TRUE);
   static int num = 0;
   printf("........... updates # %i\n", num++);
   t = get_time();
   EINA_INLIST_FOREACH(e->layers, lay)
     {
        EINA_INLIST_FOREACH(lay->objects, obj)
          {
             _evas_render2_th_main_obj_process(e, obj,
                                               updates, 0, 0,
                                               0);
          }
     }
   // add explicitly exposed/damaged regions of the canvas
   EINA_LIST_FREE(e->damages, rect)
     {
        evas_common_tilebuf_add_redraw(updates, rect->x, rect->y,
                                       rect->w, rect->h);
        eina_rectangle_free(rect);
     }
   // build obscure objects list of active objects that obscure
   EINA_LIST_FOREACH(e->obscures, l, rect)
     {
        evas_common_tilebuf_del_redraw(updates, rect->x, rect->y,
                                       rect->w, rect->h);
     }
   t = get_time() - t;
   printf("T: update generation: "); out_time(t);

   rects = evas_common_tilebuf_get_render_rects(updates);
   EINA_INLIST_FOREACH(EINA_INLIST_GET(rects), r)
     {
        rect = malloc(sizeof(Eina_Rectangle));
        if (rect)
          {
             printf(" Render Region [ %4i %4i %4ix%4i ]\n", r->x, r->y, r->w, r->h);
             rect->x = r->x; rect->y = r->y;
             rect->w = r->w; rect->h = r->h;
             updates_list = eina_list_append(updates_list, rect);
          }
     }
   evas_common_tilebuf_free_render_rects(rects);

   evas_common_tilebuf_free(updates);

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
   eo_unref(eo_e);
}

static void *
_evas_render2_th_main(void *data EINA_UNUSED, Eina_Thread thread EINA_UNUSED)
{
   void *ref = NULL;
   Msg_Main_Render *msg;

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

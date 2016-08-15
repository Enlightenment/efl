#define EFL_CANVAS_FILTER_INTERNAL_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"
#include "../../lib/efl/interfaces/efl_gfx_filter.eo.h"
#include "efl_canvas_filter_internal.eo.h"
#include "evas_filter.h"

#define MY_CLASS EFL_CANVAS_FILTER_INTERNAL_MIXIN

#define ENFN obj->layer->evas->engine.func
#define ENDT obj->layer->evas->engine.data.output

#define FCOW_BEGIN(_pd) ({ Evas_Object_Filter_Data *_fcow = eina_cow_write(evas_object_filter_cow, (const Eina_Cow_Data**)&(_pd->data)); _state_check(_fcow); _fcow; })
#define FCOW_END(_fcow, _pd) eina_cow_done(evas_object_filter_cow, (const Eina_Cow_Data**)&(_pd->data), _fcow, EINA_TRUE)

typedef struct _Evas_Filter_Data Evas_Filter_Data;
typedef struct _Evas_Filter_Post_Render_Data Evas_Filter_Post_Render_Data;

struct _Evas_Filter_Data
{
   const Evas_Object_Filter_Data *data;
   Eina_Bool has_cb;
   SLK(lck);
   Eina_List *post_data;
};

struct _Evas_Filter_Post_Render_Data
{
   Evas_Filter_Context *ctx;
   Eina_Bool success;
};

static inline void
_state_check(Evas_Object_Filter_Data *fcow)
{
   if (!fcow->state.cur.name)
     fcow->state.cur.name = eina_stringshare_add("default");
   if (!fcow->state.next.name)
     fcow->state.next.name = eina_stringshare_add("default");
}

static void
_filter_end_sync(Evas_Filter_Context *ctx, Evas_Object_Protected_Data *obj,
                 Evas_Filter_Data *pd, Eina_Bool success)
{
   void *previous = pd->data->output;
   Eo *eo_obj = obj->object;

#ifdef DEBUG
   EINA_SAFETY_ON_FALSE_RETURN(eina_main_loop_is());
#endif

   if (!success)
     {
        ERR("Filter failed at runtime!");
        evas_filter_invalid_set(eo_obj, EINA_TRUE);
        evas_filter_dirty(eo_obj);
     }
   else
     {
        Evas_Object_Filter_Data *fcow;
        void *output = evas_filter_buffer_backing_steal(ctx, EVAS_FILTER_BUFFER_OUTPUT_ID);

        fcow = FCOW_BEGIN(pd);
        fcow->output = output;
        FCOW_END(fcow, pd);
     }

   // Destroy context as we won't reuse it.
   evas_filter_buffer_backing_release(ctx, previous);
   evas_filter_context_destroy(ctx);
}

static void
_render_post_cb(void *data, const Eo_Event *event EINA_UNUSED)
{
   Eo *eo_obj = data;
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Filter_Data *pd = efl_data_scope_get(eo_obj, MY_CLASS);
   Eina_List *post_data;
   Evas_Filter_Post_Render_Data *task;

   SLKL(pd->lck);
   post_data = pd->post_data;
   pd->post_data = NULL;
   SLKU(pd->lck);

   EINA_LIST_FREE(post_data, task)
     {
        _filter_end_sync(task->ctx, obj, pd, task->success);
        free(task);
     }
}

static void
_filter_cb(Evas_Filter_Context *ctx, void *data, Eina_Bool success)
{
   Eo *eo_obj = data;
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Filter_Data *pd = efl_data_scope_get(eo_obj, MY_CLASS);
   Evas_Filter_Post_Render_Data *post_data;

   if (!pd->data->async)
     {
        _filter_end_sync(ctx, obj, pd, success);
        return;
     }

   post_data = calloc(1, sizeof(*post_data));
   post_data->success = success;
   post_data->ctx = ctx;
   SLKL(pd->lck);
   pd->post_data = eina_list_append(pd->post_data, post_data);
   SLKU(pd->lck);
}

static void
_filter_source_hash_free_cb(void *data)
{
   Evas_Filter_Proxy_Binding *pb = data;
   Evas_Object_Protected_Data *proxy, *source;
   Evas_Filter_Data *pd;

   proxy = efl_data_scope_get(pb->eo_proxy, EFL_CANVAS_OBJECT_CLASS);
   source = efl_data_scope_get(pb->eo_source, EFL_CANVAS_OBJECT_CLASS);

   if (source)
     {
        EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, source->proxy,
                             Evas_Object_Proxy_Data, source_write)
          source_write->proxies = eina_list_remove(source_write->proxies, pb->eo_proxy);
        EINA_COW_WRITE_END(evas_object_proxy_cow, source->proxy, source_write)
     }

   pd = efl_data_scope_get(pb->eo_proxy, MY_CLASS);

   if (pd && proxy)
     {
        if (!eina_hash_population(pd->data->sources))
          {
             EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, proxy->proxy,
                                  Evas_Object_Proxy_Data, proxy_write)
               proxy_write->is_proxy = EINA_FALSE;
             EINA_COW_WRITE_END(evas_object_proxy_cow, source->proxy, proxy_write)
          }
     }

   eina_stringshare_del(pb->name);
   free(pb);
}

Eina_Bool
evas_filter_object_render(Eo *eo_obj, Evas_Object_Protected_Data *obj,
                          void *output, void *context, void *surface,
                          int x, int y, Eina_Bool do_async, Eina_Bool alpha)
{
   Evas_Filter_Data *pd = efl_data_scope_get(eo_obj, MY_CLASS);

   if (!pd->data->invalid && (pd->data->chain || pd->data->code))
     {
        int X, Y, W, H, l = 0, r = 0, t = 0, b = 0;
        Evas_Filter_Context *filter;
        void *drawctx;
        Eina_Bool ok;
        void *previous = pd->data->output;
        Evas_Object_Filter_Data *fcow;

        /* NOTE: Filter rendering is now done ENTIRELY on CPU.
         * So we rely on cache/cache2 to allocate a real image buffer,
         * that we can draw to. The OpenGL texture will be created only
         * after the rendering has been done, as we simply push the output
         * image to GL.
         */

        W = obj->cur->geometry.w;
        H = obj->cur->geometry.h;
        X = obj->cur->geometry.x;
        Y = obj->cur->geometry.y;

        // Prepare color multiplier
        ENFN->context_color_set(output, context,
                                obj->cur->cache.clip.r,
                                obj->cur->cache.clip.g,
                                obj->cur->cache.clip.b,
                                obj->cur->cache.clip.a);
        if (obj->cur->clipper)
          ENFN->context_multiplier_set(output, context,
                                       obj->cur->clipper->cur->cache.clip.r,
                                       obj->cur->clipper->cur->cache.clip.g,
                                       obj->cur->clipper->cur->cache.clip.b,
                                       obj->cur->clipper->cur->cache.clip.a);
        else
           ENFN->context_multiplier_unset(output, context);

        if (!pd->data->chain)
          {
             Evas_Filter_Program *pgm;
             pgm = evas_filter_program_new(pd->data->name, alpha);
             evas_filter_program_source_set_all(pgm, pd->data->sources);
             evas_filter_program_data_set_all(pgm, pd->data->data);
             evas_filter_program_state_set(pgm, eo_obj, obj,
                                           pd->data->state.cur.name, pd->data->state.cur.value,
                                           pd->data->state.next.name, pd->data->state.next.value,
                                           pd->data->state.pos);
             if (!evas_filter_program_parse(pgm, pd->data->code))
               {
                  ERR("Filter program parsing failed");
                  evas_filter_program_del(pgm);

                  if (!pd->data->invalid)
                    {
                       fcow = FCOW_BEGIN(pd);
                       fcow->invalid = EINA_TRUE;
                       FCOW_END(fcow, pd);
                    }

                  return EINA_FALSE;
               }
             fcow = FCOW_BEGIN(pd);
             fcow->chain = pgm;
             fcow->invalid = EINA_FALSE;
             FCOW_END(fcow, pd);
          }
        else if (previous && !pd->data->changed)
          {
             Eina_Bool redraw;

             redraw = evas_filter_program_state_set(pd->data->chain, eo_obj, obj,
                                                    pd->data->state.cur.name, pd->data->state.cur.value,
                                                    pd->data->state.next.name, pd->data->state.next.value,
                                                    pd->data->state.pos);
             if (redraw)
               DBG("Filter redraw by state change!");
             else if (obj->changed)
               {
                  // FIXME: Check that something else than X,Y changed!
                  redraw = EINA_TRUE;
                  DBG("Filter redraw by object content change!");
               }

             // Scan proxies to find if any changed
             if (!redraw && pd->data->sources)
               {
                  Evas_Filter_Proxy_Binding *pb;
                  Evas_Object_Protected_Data *source;
                  Eina_Iterator *iter;

                  iter = eina_hash_iterator_data_new(pd->data->sources);
                  EINA_ITERATOR_FOREACH(iter, pb)
                    {
                       source = efl_data_scope_get(pb->eo_source, EFL_CANVAS_OBJECT_CLASS);
                       if (source->changed)
                         {
                            redraw = EINA_TRUE;
                            break;
                         }
                    }
                  eina_iterator_free(iter);
               }

             if (!redraw)
               {
                  if (pd->has_cb)
                    {
                       // Post render callback is not required anymore
                       Evas *e = obj->layer->evas->evas;
                       efl_event_callback_del(e, EFL_CANVAS_EVENT_RENDER_POST, _render_post_cb, eo_obj);
                       pd->has_cb = EINA_FALSE;
                    }

                  // Render this image only
                  ENFN->image_draw(ENDT, context,
                                   surface, previous,
                                   0, 0, W, H,         // src
                                   X + x, Y + y, W, H, // dst
                                   EINA_FALSE,         // smooth
                                   do_async);
                  return EINA_TRUE;
               }
          }
        else
           evas_filter_program_state_set(pd->data->chain, eo_obj, obj,
                                         pd->data->state.cur.name, pd->data->state.cur.value,
                                         pd->data->state.next.name, pd->data->state.next.value,
                                         pd->data->state.pos);

        filter = evas_filter_context_new(obj->layer->evas, do_async);

        // Run script
        ok = evas_filter_context_program_use(filter, pd->data->chain);
        if (!filter || !ok)
          {
             ERR("Parsing failed?");
             evas_filter_context_destroy(filter);

             if (!pd->data->invalid)
               {
                  fcow = FCOW_BEGIN(pd);
                  fcow->invalid = EINA_TRUE;
                  FCOW_END(fcow, pd);
               }
             return EINA_FALSE;
          }

        // Proxies
        evas_filter_context_proxy_render_all(filter, eo_obj, EINA_FALSE);

        // Draw Context
        drawctx = ENFN->context_new(ENDT);
        ENFN->context_color_set(ENDT, drawctx, 255, 255, 255, 255);

        // Allocate all buffers now
        evas_filter_context_buffers_allocate_all(filter);
        evas_filter_target_set(filter, context, surface, X + x, Y + y);

        // Request rendering from the object itself (child class)
        evas_filter_program_padding_get(pd->data->chain, &l, &r, &t, &b);
        ok = evas_filter_input_render(eo_obj, filter, drawctx, l, r, t, b, do_async);
        if (!ok) ERR("Filter input render failed.");

        ENFN->context_free(ENDT, drawctx);

        // Add post-run callback and run filter
        if (do_async && !pd->has_cb)
          {
             Evas *e = obj->layer->evas->evas;
             efl_event_callback_add(e, EFL_CANVAS_EVENT_RENDER_POST, _render_post_cb, eo_obj);
             pd->has_cb = EINA_TRUE;
          }
        evas_filter_context_post_run_callback_set(filter, _filter_cb, eo_obj);
        ok = evas_filter_run(filter);

        fcow = FCOW_BEGIN(pd);
        fcow->changed = EINA_FALSE;
        fcow->async = do_async;
        if (!ok) fcow->invalid = EINA_TRUE;
        FCOW_END(fcow, pd);

        if (ok)
          {
             DBG("Effect rendering done.");
             return EINA_TRUE;
          }
        else
          {
             ERR("Rendering failed.");
             return EINA_FALSE;
          }
     }
   return EINA_FALSE;
}

EOLIAN static void
_efl_canvas_filter_internal_efl_gfx_filter_filter_program_set(Eo *eo_obj, Evas_Filter_Data *pd,
                                                              const char *code, const char *name)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Filter_Program *pgm = NULL;
   Evas_Object_Filter_Data *fcow;
   Eina_Bool alpha;

   if (!pd) return;
   if (eina_streq(pd->data->code, code) && eina_streq(pd->data->name, name))
     return;

   evas_object_async_block(obj);
   fcow = FCOW_BEGIN(pd);
   {
      // Parse filter program
      evas_filter_program_del(fcow->chain);
      eina_stringshare_replace(&fcow->name, name);
      if (code)
        {
           alpha = evas_filter_input_alpha(eo_obj);
           pgm = evas_filter_program_new(fcow->name, alpha);
           evas_filter_program_source_set_all(pgm, fcow->sources);
           evas_filter_program_data_set_all(pgm, fcow->data);
           evas_filter_program_state_set(pgm, eo_obj, obj,
                                         fcow->state.cur.name, fcow->state.cur.value,
                                         fcow->state.next.name, fcow->state.next.value,
                                         fcow->state.pos);
           if (!evas_filter_program_parse(pgm, code))
             {
                ERR("Parsing failed!");
                evas_filter_program_del(pgm);
                pgm = NULL;
             }
        }
      fcow->chain = pgm;
      fcow->changed = EINA_TRUE;
      fcow->invalid = (pgm == NULL);
      eina_stringshare_replace(&fcow->code, code);
   }
   FCOW_END(fcow, pd);

   evas_filter_dirty(eo_obj);
}

EOLIAN static void
_efl_canvas_filter_internal_efl_gfx_filter_filter_program_get(Eo *eo_obj EINA_UNUSED, Evas_Filter_Data *pd, const char **code, const char **name)
{
   if (code) *code = pd->data->code;
   if (name) *name = pd->data->name;
}

EOLIAN static void
_efl_canvas_filter_internal_efl_gfx_filter_filter_source_set(Eo *eo_obj, Evas_Filter_Data *pd,
                                                             const char *name, Efl_Gfx *eo_source)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Filter_Proxy_Binding *pb, *pb_old = NULL;
   Evas_Object_Protected_Data *source = NULL;
   Evas_Object_Filter_Data *fcow = NULL;

   if (eo_source)
     source = efl_data_scope_get(eo_source, EFL_CANVAS_OBJECT_CLASS);

   evas_object_async_block(obj);
   if (!name)
     {
        if (!eo_source || !pd->data->sources) return;
        if (eina_hash_del_by_data(pd->data->sources, eo_source))
          goto update;
        return;
     }

   if (!source && !pd->data->sources)
     return;

   if (pd->data->sources)
     {
        pb_old = eina_hash_find(pd->data->sources, name);
        if (pb_old && (pb_old->eo_source == eo_source)) return;
     }

   fcow = FCOW_BEGIN(pd);
   if (!fcow->sources)
     fcow->sources = eina_hash_string_small_new(EINA_FREE_CB(_filter_source_hash_free_cb));
   else if (pb_old)
     eina_hash_del(fcow->sources, name, pb_old);

   if (!source)
     {
        pb_old = eina_hash_find(fcow->sources, name);
        if (!pb_old)
          {
             FCOW_END(fcow, pd);
             return;
          }
        eina_hash_del_by_key(fcow->sources, name);
        goto update;
     }

   pb = calloc(1, sizeof(*pb));
   pb->eo_proxy = eo_obj;
   pb->eo_source = eo_source;
   pb->name = eina_stringshare_add(name);

   if (!eina_list_data_find(source->proxy->proxies, eo_obj))
     {
        EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, source->proxy, Evas_Object_Proxy_Data, source_write)
          source_write->proxies = eina_list_append(source_write->proxies, eo_obj);
        EINA_COW_WRITE_END(evas_object_proxy_cow, source->proxy, source_write)
     }

   if (!obj->proxy->is_proxy)
     {
        EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, obj->proxy, Evas_Object_Proxy_Data, proxy_write)
          proxy_write->is_proxy = EINA_TRUE;
        EINA_COW_WRITE_END(evas_object_proxy_cow, obj->proxy, proxy_write)
     }

   eina_hash_add(fcow->sources, pb->name, pb);
   evas_filter_program_source_set_all(fcow->chain, fcow->sources);
   evas_filter_program_data_set_all(fcow->chain, fcow->data);

   // Update object
update:
   if (fcow)
     {
        fcow->changed = EINA_TRUE;
        fcow->invalid = EINA_FALSE;
        FCOW_END(fcow, pd);
     }

   evas_filter_dirty(eo_obj);
}

EOLIAN static Efl_Gfx *
_efl_canvas_filter_internal_efl_gfx_filter_filter_source_get(Eo *obj EINA_UNUSED, Evas_Filter_Data *pd,
                                                             const char * name)
{
   Evas_Filter_Proxy_Binding *pb = eina_hash_find(pd->data->sources, name);
   if (!pb) return NULL;
   return pb->eo_source;
}

EOLIAN static void
_efl_canvas_filter_internal_efl_gfx_filter_filter_state_set(Eo *eo_obj, Evas_Filter_Data *pd,
                                                            const char *cur_state, double cur_val,
                                                            const char *next_state, double next_val,
                                                            double pos)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   evas_object_async_block(obj);
   if ((cur_state != pd->data->state.cur.name) || (cur_val != pd->data->state.cur.value) ||
       (next_state != pd->data->state.next.name) || (next_val != pd->data->state.next.value) ||
       (pos != pd->data->state.pos))
     {
        Evas_Object_Filter_Data *fcow = FCOW_BEGIN(pd);
        fcow->changed = 1;
        eina_stringshare_replace(&fcow->state.cur.name, cur_state);
        fcow->state.cur.value = cur_val;
        eina_stringshare_replace(&fcow->state.next.name, next_state);
        fcow->state.next.value = next_val;
        fcow->state.pos = pos;
        FCOW_END(fcow, pd);

        if (pd->data->chain)
          {
             evas_filter_program_state_set(pd->data->chain, eo_obj, obj,
                                           pd->data->state.cur.name, pd->data->state.cur.value,
                                           pd->data->state.next.name, pd->data->state.next.value,
                                           pd->data->state.pos);
          }

        evas_filter_dirty(eo_obj);
     }
}

EOLIAN static void
_efl_canvas_filter_internal_efl_gfx_filter_filter_state_get(Eo *obj EINA_UNUSED, Evas_Filter_Data *pd,
                                                            const char **cur_state, double *cur_val,
                                                            const char **next_state, double *next_val,
                                                            double *pos)
{
   if (cur_state) *cur_state = pd->data->state.cur.name;
   if (cur_val) *cur_val = pd->data->state.cur.value;
   if (next_state) *next_state = pd->data->state.next.name;
   if (next_val) *next_val = pd->data->state.next.value;
   if (pos) *pos = pd->data->state.pos;
}

EOLIAN static void
_efl_canvas_filter_internal_efl_gfx_filter_filter_padding_get(Eo *eo_obj EINA_UNUSED, Evas_Filter_Data *pd,
                                                              int *l, int *r, int *t, int *b)
{
   if (!pd->data->chain)
     {
        if (l) *l = 0;
        if (r) *r = 0;
        if (t) *t = 0;
        if (b) *b = 0;
        return;
     }
   evas_filter_program_padding_get(pd->data->chain, l, r, t, b);
}

EOLIAN static void
_efl_canvas_filter_internal_filter_changed_set(Eo *eo_obj EINA_UNUSED, Evas_Filter_Data *pd, Eina_Bool val)
{
   if ((evas_object_filter_cow_default != pd->data) && (pd->data->changed != val))
     {
        Evas_Object_Filter_Data *fcow = FCOW_BEGIN(pd);
        fcow->changed = val;
        FCOW_END(fcow, pd);
     }
}

EOLIAN static void
_efl_canvas_filter_internal_filter_invalid_set(Eo *eo_obj EINA_UNUSED, Evas_Filter_Data *pd, Eina_Bool val)
{
   if (pd->data->invalid != val)
     {
        Evas_Object_Filter_Data *fcow = FCOW_BEGIN(pd);
        fcow->invalid = val;
        FCOW_END(fcow, pd);
     }
}

EOLIAN static Efl_Object *
_efl_canvas_filter_internal_efl_object_constructor(Eo *eo_obj, Evas_Filter_Data *pd)
{
   Eo *obj = NULL;

   obj = efl_constructor(efl_super(eo_obj, MY_CLASS));
   pd->data = eina_cow_alloc(evas_object_filter_cow);
   SLKI(pd->lck);

   return obj;
}

EOLIAN static void
_efl_canvas_filter_internal_efl_object_destructor(Eo *eo_obj, Evas_Filter_Data *pd)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Filter_Data_Binding *db;
   Eina_Inlist *il;

   if (!pd->data || (evas_object_filter_cow_default == pd->data))
     goto finish;

   if (pd->data->output)
     {
        if (!pd->data->async)
          ENFN->image_free(ENDT, pd->data->output);
        else
           evas_unref_queue_image_put(obj->layer->evas, pd->data->output);
     }
   eina_hash_free(pd->data->sources);
   EINA_INLIST_FOREACH_SAFE(pd->data->data, il, db)
     {
        eina_stringshare_del(db->name);
        eina_stringshare_del(db->value);
        free(db);
     }
   evas_filter_program_del(pd->data->chain);
   eina_stringshare_del(pd->data->code);
   eina_stringshare_del(pd->data->state.cur.name);
   eina_stringshare_del(pd->data->state.next.name);

finish:
   eina_cow_free(evas_object_filter_cow, (const Eina_Cow_Data **) &pd->data);
   if (pd->has_cb)
     {
        Evas *e = obj->layer->evas->evas;
        efl_event_callback_del(e, EFL_CANVAS_EVENT_RENDER_POST, _render_post_cb, eo_obj);
     }
   SLKD(pd->lck);

   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

EOLIAN static void
_efl_canvas_filter_internal_efl_gfx_filter_filter_data_set(Eo *eo_obj, Evas_Filter_Data *pd,
                                                           const char *name, const char *value,
                                                           Eina_Bool execute)
{
   Evas_Filter_Data_Binding *db, *found = NULL;
   Evas_Object_Filter_Data *fcow;

   EINA_SAFETY_ON_NULL_RETURN(pd->data);
   EINA_SAFETY_ON_NULL_RETURN(name);

   EINA_INLIST_FOREACH(pd->data->data, db)
     {
        if (!strcmp(name, db->name))
          {
             if (db->execute == execute)
               {
                  if ((value == db->value) || (value && db->value && !strcmp(value, db->value)))
                    return;
               }
             found = db;
             break;
          }
     }

   fcow = FCOW_BEGIN(pd);
   {
      if (found)
        {
           // Note: we are keeping references to NULL values here.
           eina_stringshare_replace(&found->value, value);
           found->execute = execute;
        }
      else if (value)
        {
           db = calloc(1, sizeof(Evas_Filter_Data_Binding));
           db->name = eina_stringshare_add(name);
           db->value = eina_stringshare_add(value);
           db->execute = execute;
           fcow->data = eina_inlist_append(fcow->data, EINA_INLIST_GET(db));
        }
      evas_filter_program_data_set_all(fcow->chain, fcow->data);
      fcow->changed = 1;
   }
   FCOW_END(fcow, pd);

   evas_filter_dirty(eo_obj);
}

EOLIAN static void
_efl_canvas_filter_internal_efl_gfx_filter_filter_data_get(Eo *obj EINA_UNUSED, Evas_Filter_Data *pd,
                                                           const char *name, const char **value,
                                                           Eina_Bool *execute)
{
   Evas_Filter_Data_Binding *db;

   if (!value && !execute) return;
   EINA_SAFETY_ON_NULL_RETURN(pd->data);

   EINA_INLIST_FOREACH(pd->data->data, db)
     {
        if (!strcmp(name, db->name))
          {
             if (value) *value = db->value;
             if (execute) *execute = db->execute;
             return;
          }
     }

   if (value) *value = NULL;
   if (execute) *execute = EINA_FALSE;
}

EOLIAN static void *
_efl_canvas_filter_internal_filter_output_buffer_get(Eo *obj EINA_UNUSED, Evas_Filter_Data *pd)
{
   return pd->data->output;
}

#include "canvas/efl_canvas_filter_internal.eo.c"

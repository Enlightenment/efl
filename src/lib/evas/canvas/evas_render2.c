#include "evas_render2.h"

//////////////////////////////////////////////////////////////////////////////
// this is the start of a rewrite of the evas rendering infra. first port of
// call is to just make it work and still support async rendering with no
// optimizations at all. once it WORKS properly start adding back
// optimizations one at a time very carefully until it is equivalent to where
// evas render was before, THEN... we can consider switching it on by default
// but until then it's off unless you set:
//
//   export EVAS_RENDER2=1
//
// at runtime.
//////////////////////////////////////////////////////////////////////////////

// data types
//////////////////////////////////////////////////////////////////////////////
typedef struct _Update Update;

struct _Update
{
   Eina_Rectangle *area;
   void *surface;
};

// funcs
//////////////////////////////////////////////////////////////////////////////
Eina_Bool _evas_render2_begin(Eo *eo_e, Eina_Bool make_updates, Eina_Bool do_draw, Eina_Bool do_async);
void _evas_render2_idle_flush(Eo *eo_e);
void _evas_render2_dump(Eo *eo_e);
void _evas_render2_wait(Eo *eo_e);

static void _evas_render2_cow_gc(Eina_Cow *cow, int max);
static void _evas_render2_cow_all_gc(int max);
static void _evas_render2_all_sync(void);
static void _evas_render2_wakeup_cb(void *target, Evas_Callback_Type type, void *event_info);
static void _evas_render2_wakeup_send(void *data);
static void _evas_render2_always_call(Eo *eo_e, Evas_Callback_Type type, void *event_info);
static void _evas_render2_updates_clean(Evas_Public_Data *e);
static void _evas_render2_stage_last(Eo *eo_e, Eina_Bool make_updates, Eina_Bool do_async);
static void _evas_render2_stage_explicit_updates(Evas_Public_Data *e);
static void _evas_render2_stage_main_render_prepare(Evas_Public_Data *e);
static void _evas_render2_stage_render_do(Evas_Public_Data *e, Eina_Bool do_async);
static void _evas_render2_stage_reset(Evas_Public_Data *e);
static void _evas_render2_stage_object_cleanup(Evas_Public_Data *e);
static void _evas_render2_th_render(void *data);
static void _evas_render2_end(Eo *eo_e);

// global data (for rendering only)
//////////////////////////////////////////////////////////////////////////////
static Eina_List *_rendering = NULL;

// actual helper/internal functions
//////////////////////////////////////////////////////////////////////////////
static void
_evas_render2_cow_gc(Eina_Cow *cow, int max)
{
   // gc a single cow type up to max iter or if max <= 0, all of them
   int i = 0;

   while (eina_cow_gc(cow))
     {
        if (max < 1) continue;
        i++;
        if (i > max) break;
     }
}

static void
_evas_render2_cow_all_gc(int max)
{
   // gc all known cow types
   _evas_render2_cow_gc(evas_object_proxy_cow, max);
   _evas_render2_cow_gc(evas_object_map_cow, max);
   _evas_render2_cow_gc(evas_object_image_pixels_cow, max);
   _evas_render2_cow_gc(evas_object_image_load_opts_cow, max);
   _evas_render2_cow_gc(evas_object_image_state_cow, max);
}

static void
_evas_render2_all_sync(void)
{
   // wait for ALL canvases to stop rendering
   Eo *eo_e;

   if (!_rendering) return;
   eo_e = eina_list_data_get(eina_list_last(_rendering));
   _evas_render2_wait(eo_e);
}

static void
_evas_render2_wakeup_cb(void *target, Evas_Callback_Type type EINA_UNUSED, void *event_info EINA_UNUSED)
{
   // in mainloop run the rendering end handler
   Eo *eo_e = target;

   _evas_render2_end(eo_e);
}

static void
_evas_render2_wakeup_send(void *data)
{
   // pass an event to the mainloop async event handler in evas so mainloop
   // runs wakeup_cb and not in any thread
   evas_async_events_put(data, 0, NULL, _evas_render2_wakeup_cb);
}

static void
_evas_render2_always_call(Eo *eo_e, Evas_Callback_Type type, void *event_info)
{
   int freeze_num = 0, i;

   eo_do(eo_e, freeze_num = eo_event_freeze_count_get());
   for (i = 0; i < freeze_num; i++) eo_do(eo_e, eo_event_thaw());
   evas_event_callback_call(eo_e, type, event_info);
   for (i = 0; i < freeze_num; i++) eo_do(eo_e, eo_event_freeze());
}

static void
_evas_render2_updates_clean(Evas_Public_Data *e)
{
   void *u;

   // clean out updates and tmp surfaces we were holding/tracking
   EINA_LIST_FREE(e->render.updates, u) eina_rectangle_free(u);
}

static void
_evas_render2_stage_last(Eo *eo_e, Eina_Bool make_updates, Eina_Bool do_async)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Eina_List *ret_updates = NULL;
   Evas_Event_Render_Post post;

   // XXX:
   // XXX: actually update screen from mainloop here if needed - eg software
   // XXX: engine needs to xshmputimage here - engine func does this
   // XXX:

   // if we did do rendering flush output to target and call callbacks
   if (e->render.updates)
     {
        Update *ru;

        _evas_render2_always_call(eo_e, EVAS_CALLBACK_RENDER_FLUSH_PRE, NULL);
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
          }
        if (do_async) post.updated_area = ret_updates;
        else e->render.updates = ret_updates;
        e->engine.func->output_flush(e->engine.data.output,
                                     EVAS_RENDER_MODE_ASYNC_END);
        _evas_render2_always_call(eo_e, EVAS_CALLBACK_RENDER_FLUSH_POST, NULL);
     }
   // clear our previous rendering stuff from the engine
   e->engine.func->output_redraws_clear(e->engine.data.output);
   // stop tracking canvas as being async rendered
   _rendering = eina_list_remove(_rendering, eo_e);
   e->rendering = EINA_FALSE;
   // call the post render callback with info if appropriate
   if (do_async)
     _evas_render2_always_call(eo_e, EVAS_CALLBACK_RENDER_POST, &post);
   else
     _evas_render2_always_call(eo_e, EVAS_CALLBACK_RENDER_POST, NULL);
   // if we don't want to keep updates after this
   if (!make_updates) _evas_render2_updates_clean(e);
   // clean out modules we don't need anymore
   evas_module_clean();
}

static void
_evas_render2_stage_explicit_updates(Evas_Public_Data *e)
{
   Eina_Rectangle *r;
   Eina_List *l;

   // XXX: should time this
   // if the output size changed, add a full redraw
   if ((e->output.changed) || (e->framespace.changed))
     {
        e->engine.func->output_resize(e->engine.data.output,
                                      e->output.w, e->output.h);
        e->engine.func->output_redraws_rect_add(e->engine.data.output, 0, 0,
                                                e->output.w, e->output.h);
     }
   // if there are explicit update regions - add them
   EINA_LIST_FREE(e->damages, r)
     {
        // if we didnt just do a full redraw if output changed
        if ((!e->output.changed) && (!e->framespace.changed))
          e->engine.func->output_redraws_rect_add(e->engine.data.output,
                                                  r->x, r->y, r->w, r->h);
        eina_rectangle_free(r);
     }
   // remove obscures from rendering - we keep them around
   EINA_LIST_FOREACH(e->obscures, l, r)
     e->engine.func->output_redraws_rect_del(e->engine.data.output,
                                             r->x, r->y, r->w, r->h);
}

static void
_evas_render2_stage_main_render_prepare(Evas_Public_Data *e)
{
   // XXX:
   // XXX: do any preparation work here that is needed for the render
   // XXX: threads to do their work, but can't be done in a thread. this
   // XXX: also includes doing the pose render and clear of change flag
   // XXX:
   printf("_evas_render2_stage_main_render_prepare %p\n", e);
}

static void
_evas_render2_stage_render_do(Evas_Public_Data *e, Eina_Bool do_async EINA_UNUSED)
{
   void *surface;
   int ux, uy, uw, uh;
   int cx, cy, cw, ch;

   // XXX: actually render now (either in thread or in mainloop)
   // XXX:
   printf(" _evas_render2_stage_render_do %p\n", e);
   while ((surface =
           e->engine.func->output_redraws_next_update_get
           (e->engine.data.output,
            &ux, &uy, &uw, &uh,
            &cx, &cy, &cw, &ch)))
     {
        Update *ru = NULL;
        void *ctx;

        ctx = e->engine.func->context_new(e->engine.data.output);
        e->engine.func->context_color_set
          (e->engine.data.output, ctx,
              rand() & 0xff, rand() & 0xff, rand() & 0xff, 0xff);
        printf("  %i %i %i %i\n", cx, cy, cw, ch);
        e->engine.func->rectangle_draw(e->engine.data.output,
                                       ctx, surface,
                                       cx, cy, cw, ch,
                                       EINA_FALSE);
        e->engine.func->context_free(e->engine.data.output, ctx);
        ru = malloc(sizeof(*ru));
        ru->surface = surface;
        NEW_RECT(ru->area, ux, uy, uw, uh);
        e->render.updates = eina_list_append(e->render.updates, ru);
        evas_cache_image_ref(surface);
     }
   e->engine.func->output_redraws_clear(e->engine.data.output);
}

static void
_evas_render2_stage_reset(Evas_Public_Data *e)
{
   // cleanup canvas state after a render
   e->changed = EINA_FALSE;
   e->viewport.changed = EINA_FALSE;
   e->output.changed = EINA_FALSE;
   e->framespace.changed = EINA_FALSE;
   e->invalidate = EINA_FALSE;
}

static void
_evas_render2_stage_object_cleanup(Evas_Public_Data *e)
{
   // cleanup objects no longer needed now they have been scanned
   // XXX:
   // XXX: delete objects no longer needed here
   // XXX:
   printf("_evas_render2_stage_object_cleanup %p\n", e);
}

static void
_evas_render2_th_render(void *data)
{
   Evas_Public_Data *e = data;
   printf(".....................................................%p\n", e);
   _evas_render2_stage_render_do(e, EINA_TRUE);
   printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^%p\n", e);
}

// major functions (called from evas_render.c)
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// BEGIN RENDERING (in mainloop)
///////////////////////////////////////////////////////////////////////
Eina_Bool
_evas_render2_begin(Eo *eo_e, Eina_Bool make_updates,
                    Eina_Bool do_draw, Eina_Bool do_async)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   // if nothing changed at all since last render - skip this frame
   if (!e->changed) return EINA_FALSE;
   // we are still rendering while being asked to render - skip this frame
   if (e->rendering && do_async) return EINA_FALSE;
   // check viewport size is same as output - not allowed to differ
   if ((e->output.w != e->viewport.w) || (e->output.h != e->viewport.h))
     ERR("viewport size != output size!");
   // we have to calculate smare objects before render so do that here
   evas_call_smarts_calculate(eo_e);
   // call canvas callbacks saying we are in the pre-render state
   _evas_render2_always_call(eo_e, EVAS_CALLBACK_RENDER_PRE, NULL);
   // begin out actual rendering bits
   _evas_render2_stage_generate_object_updates(e);
   _evas_render2_stage_explicit_updates(e);
   // we are actually asked to draw not just go through the motions for gc
   if (do_draw)
     {
        // now go through any preparation that needs doing in the mainloop
        _evas_render2_stage_main_render_prepare(e);
        // send off rendering to primary thread renderer
        if (do_async)
          {
             // ref the canvas so it stays while threads wortk
             eo_ref(eo_e);
             // track hanvas in list of things going in the background
             e->rendering = EINA_TRUE;
             _rendering = eina_list_append(_rendering, eo_e);
             // queue the render thread command
             evas_thread_cmd_enqueue(_evas_render2_th_render, e);
             // flush the thread queue and call wakeup_send in the thread
             evas_thread_queue_flush(_evas_render2_wakeup_send, eo_e);
          }
        // or if not async, do rendering inline now
        else _evas_render2_stage_render_do(e, EINA_FALSE);
     }
   // reset flags since rendering is processed now
   _evas_render2_stage_reset(e);
   // clean/delete/gc objects here
   _evas_render2_stage_object_cleanup(e);
   // if we are not going to be async then do last render stage here
   if (!do_async) _evas_render2_stage_last(eo_e, make_updates, EINA_FALSE);
   if (!do_draw) _evas_render2_updates_clean(e);
   evas_module_clean();
   return EINA_TRUE;
}

///////////////////////////////////////////////////////////////////////
// END RENDERING (in mainloop)
///////////////////////////////////////////////////////////////////////
static void
_evas_render2_end(Eo *eo_e)
{
   // this is actually called if rendering was async and is done. this is
   // run in the mainloop where rendering began and may handle any cleanup
   // or pixel upload if needed here
   _evas_render2_stage_last(eo_e, EINA_TRUE, EINA_TRUE);
   // release canvas object ref
   eo_unref(eo_e);
}

///////////////////////////////////////////////////////////////////////
// IDLE FLUSH (in mainloop)
///////////////////////////////////////////////////////////////////////
void
_evas_render2_idle_flush(Eo *eo_e)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   // wait for rendering to finish so we don't mess up shared resources
   _evas_render2_wait(eo_e);
   // clean fonts
   evas_fonts_zero_pressure(eo_e);
   // call engine idle flush call
   if ((e->engine.func) && (e->engine.func->output_idle_flush) &&
       (e->engine.data.output))
     e->engine.func->output_idle_flush(e->engine.data.output);
   // mark as invalidated
   e->invalidate = EINA_TRUE;
   // garbage collect up to 500 cow segments from our cow types
   // not e that we should probably expose a call to do this outside of evas
   // so ecore evas can call it in an idler
   _evas_render2_cow_all_gc(500);
}

///////////////////////////////////////////////////////////////////////
// DUMP DATA (in mainloop)
///////////////////////////////////////////////////////////////////////
void
_evas_render2_dump(Eo *eo_e)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   // freeze core cache system async work
   evas_cache_async_freeze();
   // wait for all canvases to render as they may share data we are dumping
   _evas_render2_all_sync();
   // go through idle flush first
   _evas_render2_idle_flush(eo_e);
   // also now tell engine to dump too
   if ((e->engine.func) && (e->engine.func->output_dump) &&
       (e->engine.data.output))
     e->engine.func->output_dump(e->engine.data.output);
   // clean up all cow sections no matter how many
   _evas_render2_cow_all_gc(0);
   // unfreeze core cache system
   evas_cache_async_thaw();
}

///////////////////////////////////////////////////////////////////////
// WAIT ON CANVAS RENDER (if async, in mainloop)
///////////////////////////////////////////////////////////////////////
void
_evas_render2_wait(Eo *eo_e)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   while (e->rendering) evas_async_events_process_blocking();
}

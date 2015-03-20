#include "evas_render2.h"

#ifdef EVAS_RENDER_DEBUG_TIMING
#include <sys/time.h>
#endif

#ifndef _WIN32
static inline double
get_time(void)
{
   struct timeval timev;
   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}
#else
static inline double
get_time(void)
{
   return (double)GetTickCount()/1000.0;
}
#endif









// a list of canvases currently rendering
static Eina_List *_rendering = NULL;

static inline void
out_time(double t)
{
   double b = (t * 100.0) / (1.0 / 60.0);
   printf("%1.8fs (%1.2f%% 60fps budget)\n", t, b);
}

static void
_always_call(Eo *eo_e, Evas_Callback_Type type, void *event_info)
{
   int freeze_num = 0, i;

   eo_do(eo_e, freeze_num = eo_event_freeze_count_get());
   for (i = 0; i < freeze_num; i++) eo_do(eo_e, eo_event_thaw());
   evas_event_callback_call(eo_e, type, event_info);
   for (i = 0; i < freeze_num; i++) eo_do(eo_e, eo_event_freeze());
}


Eina_Bool
_evas_render2(Eo *eo_e, Evas_Public_Data *e)
{
   // if nothing changed at all since last render - skip this frame
   if (!e->changed) return EINA_FALSE;
   // we are still rendering while being asked to render - skip this
   if (e->rendering) return EINA_FALSE;
   // mark this canvas as a render2 canvas - not normal render
   e->render2 = EINA_TRUE;
   // check viewport size is same as output - not allowed to differ
   if ((e->output.w != e->viewport.w) || (e->output.h != e->viewport.h))
     ERR("viewport size != output size!");

   // wait for any previous render pass to do its thing
   evas_canvas_async_block(e);
   // we have to calculate smare objects before render so do that here
   evas_call_smarts_calculate(eo_e);
   // call canvas callbacks saying we are in the pre-render state
   _always_call(eo_e, EVAS_CALLBACK_RENDER_PRE, NULL);
   // bock any susbequent rneders from doing this walk
   eina_lock_take(&(e->lock_objects));
   // XXX: gain a reference
   eo_ref(eo_e);
   // XXX: put into the "i'm rendering" pool
   e->rendering = EINA_TRUE;
   _rendering = eina_list_append(_rendering, eo_e);
   // XXX; should wake up thread here to begin doing it's work



   // XXX: call this from object walk thread
   eina_lock_release(&(e->lock_objects));
   // XXX: remove from the "i'm rendering" pool - do back in mainloop
   e->rendering = EINA_FALSE;
   _rendering = eina_list_remove(_rendering, eo_e);



   // XXX: like below - call from thread messages - figure out if they just
   // should be dumbly called before render post anyway
   _always_call(eo_e, EVAS_CALLBACK_RENDER_FLUSH_PRE, NULL);
   _always_call(eo_e, EVAS_CALLBACK_RENDER_FLUSH_POST, NULL);
   // XXX: call render post - should be a result from thread message called
   // from mainloop - also fill in post struct
   Evas_Event_Render_Post post;
   _always_call(eo_e, EVAS_CALLBACK_RENDER_POST, &post);

   // XXX: release our reference
   eo_unref(eo_e);

   printf("%p %p\n", eo_e, e);
   return EINA_FALSE;
}

void
_evas_norender2(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   evas_canvas_async_block(e);
}

Eina_List *
_evas_render2_updates_wait(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   evas_canvas_async_block(e);
   return NULL;
}

void
_evas_render2_idle_flush(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   evas_canvas_async_block(e);
}

void
_evas_render2_sync(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   evas_canvas_async_block(e);
}

void
_evas_render2_dump(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   evas_canvas_async_block(e);
}

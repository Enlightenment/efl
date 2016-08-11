#include "evas_render2.h"

#include <sys/time.h>

#include "region.c"

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

static inline void
out_time(double t)
{
   double b = (t * 100.0) / (1.0 / 60.0);
   printf("%1.2f%% / 60fps\n", b);
}

static void
_always_call(Eo *eo_e, Evas_Callback_Type type, void *event_info)
{
   int freeze_num = 0, i;

   freeze_num = efl_event_freeze_count_get(eo_e);
   for (i = 0; i < freeze_num; i++) efl_event_thaw(eo_e);
   evas_event_callback_call(eo_e, type, event_info);
   for (i = 0; i < freeze_num; i++) efl_event_freeze(eo_e);
}

// a list of canvases currently rendering
static Eina_List *_rendering = NULL;

// just put the thread code inlined here for now as opposed to separate files
#include "evas_render2_th_main.c"

// init all relevant render threads if needed
static void
_evas_render2_th_init(void)
{
   static Eina_Bool initted = EINA_FALSE;

   if (initted) return;
   initted = EINA_TRUE;
   _th_main_queue = eina_thread_queue_new();
   if (!eina_thread_create(&_th_main, EINA_THREAD_URGENT, 0,
                           _evas_render2_th_main, NULL))
     ERR("Cannot create render2 thread");
}

Eina_Bool
_evas_render2(Eo *eo_e, Evas_Public_Data *e)
{
   double t;

   // if nothing changed at all since last render - skip this frame
   if (!e->changed) return EINA_FALSE;
   // we are still rendering while being asked to render - skip this
   if (e->rendering) return EINA_FALSE;
   // mark this canvas as a render2 canvas - not normal render
   e->render2 = EINA_TRUE;
   // check viewport size is same as output - not allowed to differ
   if ((e->output.w != e->viewport.w) || (e->output.h != e->viewport.h))
     ERR("viewport size != output size!");

   // if render threads not initted - init them - maybe move this later?
   _evas_render2_th_init();

   printf("-------------------------------------------- %p %p\n", eo_e, e);
   // wait for any previous render pass to do its thing
   t = get_time();
   evas_canvas_async_block(e);
   t = get_time() - t;
   printf("T: block wait: "); out_time(t);
   // we have to calculate smare objects before render so do that here
   t = get_time();
   evas_call_smarts_calculate(eo_e);
   t = get_time() - t;
   printf("T: smart calc: "); out_time(t);
   // call canvas callbacks saying we are in the pre-render state
   _always_call(eo_e, EVAS_CALLBACK_RENDER_PRE, NULL);
   // bock any susbequent rneders from doing this walk
   eina_lock_take(&(e->lock_objects));
   // gain a reference
   eo_ref(eo_e);
   // put into the "i'm rendering" pool
   e->rendering = EINA_TRUE;
   _rendering = eina_list_append(_rendering, eo_e);

   // call our flush pre at this point before rendering begins...
   _always_call(eo_e, EVAS_CALLBACK_RENDER_FLUSH_PRE, NULL);

   // tell main render thread to wake up and begin processing this canvas
   _evas_render2_th_main_msg_render(eo_e, e);

   return EINA_FALSE;
}

Eina_List *
_evas_render2_updates(Eo *eo_e, Evas_Public_Data *e)
{
   if (!_evas_render2(eo_e, e)) return NULL;
   return _evas_render2_updates_wait(eo_e, e);
}

Eina_List *
_evas_render2_updates_wait(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   evas_canvas_async_block(e);
   while (e->rendering) evas_async_events_process_blocking();
   return NULL;
}

void
_evas_norender2(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   evas_canvas_async_block(e);
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

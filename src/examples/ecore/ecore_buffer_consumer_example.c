#include <stdio.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Buffer.h>
#include <Ecore_Buffer_Queue.h>
#include <Evas.h>

#ifdef DEBUG
#define LOG(f, x...) printf("[CONSUMER|%30.30s|%04d] " f "\n", __func__, __LINE__, ##x)
#else
#define LOG(f, x...)
#endif

#define WIDTH 720
#define HEIGHT 960

typedef struct _Consumer_Data
{
   Ecore_Buffer_Consumer *consumer;
   Ecore_Buffer *buffer;
   Ecore_Job *render_job;
   struct
   {
      Evas *e;
      Ecore_Evas *ee;
      Evas_Object *bg, *img;
   } win;
} Consumer_Data;

const char *name = "ecore_buffer_queue_test";

static void
shutdown_all(void)
{
   ecore_buffer_queue_shutdown();
   ecore_buffer_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   eina_shutdown();
}

static Eina_Bool
init_all(void)
{
   if (!eina_init()) goto err;
   if (!ecore_init()) goto err;
   if (!ecore_evas_init()) goto err;
   if (!ecore_buffer_init()) goto err;
   if (!ecore_buffer_queue_init()) goto err;

   return EINA_TRUE;
err:
   shutdown_all();
   return EINA_FALSE;
}

static void
_cb_render_post(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Consumer_Data *cd = (Consumer_Data *)data;

   if (cd->buffer)
     {
        ecore_buffer_consumer_buffer_release(cd->consumer, cd->buffer);
        cd->buffer = NULL;
     }
}

static void
_consumer_cb_render_job(void *data)
{
   Consumer_Data *cd = (Consumer_Data *)data;
   void *pixel_data;
   unsigned int w, h;

   LOG("Startup - Render");

   if (!(cd->buffer = ecore_buffer_consumer_buffer_dequeue(cd->consumer)))
     {
        LOG("Failed to dequeue buffer");
        goto end;
     }

   LOG("Success to get Compositable Buffer, "
        "Drawing it to Consumer's Canvas now... - buffer:%p", cd->buffer);
   // Get pixel data and set it to object.
   pixel_data = ecore_buffer_data_get(cd->buffer);
   ecore_buffer_size_get(cd->buffer, &w, &h);
   evas_object_image_data_set(cd->win.img, pixel_data);
   evas_object_image_data_update_add(cd->win.img, 0, 0, w, h);

   ecore_job_del(cd->render_job);
   cd->render_job = NULL;

end:
   LOG("Done - Render");
}

static void
_consumer_render_queue(Consumer_Data *cd)
{
   if (!cd) return;

   LOG("Render Queue");

   if (!cd->render_job)
     cd->render_job = ecore_job_add(_consumer_cb_render_job, cd);
}

static void
_cb_provider_add(Ecore_Buffer_Consumer *consumer EINA_UNUSED, void *data EINA_UNUSED)
{
   LOG("Connected with Provider");
}

static void
_cb_provider_del(Ecore_Buffer_Consumer *consumer EINA_UNUSED, void *data EINA_UNUSED)
{
   LOG("Disconnected with Provider, Shutdown Consumer now.");
   ecore_main_loop_quit();
}

static void
_cb_buffer_enqueued(Ecore_Buffer_Consumer *consumer EINA_UNUSED, void *data)
{
   Consumer_Data *cd = (Consumer_Data *)data;

   LOG("Buffer Enqueued");

   _consumer_render_queue(cd);
}

int
main(void)
{
   Consumer_Data *cd;
   Evas_Object *o;
   const int queue_size = 3;

   if (!init_all())
     {
        LOG("Initializing failed");
        return -1;
     }

   cd = (Consumer_Data *)calloc(1, sizeof(Consumer_Data));

   if (!cd || !(cd->consumer = ecore_buffer_consumer_new(name, queue_size, WIDTH, HEIGHT)))
     {
        LOG("Failed to create consumer");
        goto shutdown;
     }

   ecore_buffer_consumer_provider_add_cb_set(cd->consumer, _cb_provider_add, cd);
   ecore_buffer_consumer_provider_del_cb_set(cd->consumer, _cb_provider_del, cd);
   ecore_buffer_consumer_buffer_enqueued_cb_set(cd->consumer, _cb_buffer_enqueued, cd);

   cd->win.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   cd->win.e = ecore_evas_get(cd->win.ee);

   o = evas_object_rectangle_add(cd->win.e);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, WIDTH, HEIGHT);
   evas_object_color_set(o, 255, 0, 0, 255);
   evas_object_show(o);
   cd->win.bg = o;

   o = evas_object_image_add(cd->win.e);
   evas_object_image_fill_set(o, 0, 0, WIDTH, HEIGHT);
   evas_object_image_size_set(o, WIDTH, HEIGHT);

   evas_object_move(o, 0, 0);
   evas_object_resize(o, WIDTH, HEIGHT);
   evas_object_show(o);
   cd->win.img = o;

   ecore_evas_show(cd->win.ee);

   evas_event_callback_add(cd->win.e, EVAS_CALLBACK_RENDER_POST, _cb_render_post, cd);

   ecore_main_loop_begin();

shutdown:
   if (cd->win.ee) ecore_evas_free(cd->win.ee);
   if (cd->buffer) ecore_buffer_consumer_buffer_release(cd->consumer, cd->buffer);
   if (cd->consumer) ecore_buffer_consumer_free(cd->consumer);
   free(cd);

   shutdown_all();
   return 0;
}

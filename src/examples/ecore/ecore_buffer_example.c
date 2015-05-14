#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Buffer.h>
#include <Ecore_Buffer_Queue.h>

#define EBQ_NAME  "bq-simple-demo-test"

#define DEBUG 1

#if DEBUG
#define LOGC(f, x...) printf("Consumer (" f ")\n", ##x)
#define LOGCR(f, x...) printf("Consumer ---> Request(" f ")\n", ##x)
#define LOGCE(f, x...) printf("Consumer <=== Notify(" f ")\n", ##x)
#define LOGP(f, x...) printf("Provider (" f ")\n", ##x)
#define LOGPR(f, x...) printf("Provider ---> Request(" f ")\n", ##x)
#define LOGPE(f, x...) printf("Provider <=== Notify(" f ")\n", ##x)
#else
#define LOGC(f, x...)
#define LOGCR(f, x...)
#define LOGCE(f, x...)
#define LOGP(f, x...)
#define LOGPR(f, x...)
#define LOGPE(f, x...)
#endif

/******* Common *******/
static int test_count = 0;
int test_count_limit;
static int consumer_pid = -1;
static Ecore_Event_Handler *sig_handler;

static Eina_Bool
init_all(void)
{
   if (!eina_init())
     goto err_eina;

   if (!ecore_init())
     goto err_ecore;

   if (!ecore_buffer_init())
     goto err_buffer;

   if (!ecore_buffer_queue_init())
     goto err_ebq;

   return EINA_TRUE;
err_ebq:
   ecore_buffer_shutdown();
err_buffer:
   ecore_shutdown();
err_ecore:
   eina_shutdown();
err_eina:
   return EINA_FALSE;
}

static void
shutdown_all(void)
{
   ecore_buffer_queue_shutdown();

   ecore_buffer_shutdown();

   ecore_shutdown();
   eina_shutdown();
}

static Eina_Bool
_simple_demo_cb_signal_exit(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   ecore_main_loop_quit();

   return ECORE_CALLBACK_DONE;
}

/******* Provider *******/
typedef struct _Provider_Data
{
   Ecore_Buffer_Provider *provider;
   Ecore_Buffer *buffer;

   unsigned int w, h;

   Ecore_Job *render_job;
   Ecore_Idle_Enterer *post_render;
} Provider_Data;

static Provider_Data *_simple_demo_provider_new(const char *name);
static void           _simple_demo_provider_shutdown(Provider_Data *pd);
static void           _simple_demo_provider_render_queue(Provider_Data *pd);
static void           _simple_demo_provider_run(Provider_Data *pd);
Ecore_Buffer         *_simple_demo_provider_buffer_get(Ecore_Buffer_Provider *provider, unsigned int w, unsigned int h, unsigned int format);

static Eina_Bool      _simple_demo_provider_cb_post_render(void *data);
static void           _simple_demo_provider_cb_render_job(void *data);

static void
_simple_demo_provider_callback_set(Provider_Data *pd,
                                   Ecore_Buffer_Provider_Consumer_Add_Cb consumer_add,
                                   Ecore_Buffer_Provider_Consumer_Del_Cb consumer_del,
                                   Ecore_Buffer_Provider_Enqueue_Cb buffer_released)
{
   ecore_buffer_provider_consumer_add_cb_set(pd->provider, consumer_add, pd);
   ecore_buffer_provider_consumer_del_cb_set(pd->provider, consumer_del, pd);
   ecore_buffer_provider_buffer_released_cb_set(pd->provider, buffer_released, pd);
}

static Provider_Data *
_simple_demo_provider_new(const char *name)
{
   Provider_Data *pd;

   LOGP("Startup");

   if (!init_all())
     goto err;

   pd = (Provider_Data *)calloc(sizeof(Provider_Data), 1);

   if (!(pd->provider = ecore_buffer_provider_new(name)))
     {
        LOGP("ERR");
        goto err_provider;
     }

   return pd;
err_provider:
   shutdown_all();
err:
   return NULL;
}

static void
_simple_demo_provider_shutdown(Provider_Data *pd)
{
   if (pd->buffer) ecore_buffer_free(pd->buffer);
   if (pd->provider) ecore_buffer_provider_free(pd->provider);
   if (pd->render_job) ecore_job_del(pd->render_job);
   if (pd->post_render) ecore_idle_enterer_del(pd->post_render);
   free(pd);
}

static void
_simple_demo_provider_run(Provider_Data *pd)
{
   ecore_main_loop_begin();

   _simple_demo_provider_shutdown(pd);
   shutdown_all();
   LOGP("Shutdown");
   exit(EXIT_SUCCESS);
}

Eina_Bool
_simple_demo_provider_cb_post_render(void *data)
{
   Provider_Data *pd = (Provider_Data *)data;
   Ecore_Buffer *next_buffer = NULL;

   LOGP("Startup - Post Render");

   LOGPR("Submit Buffer - buffer: %p", pd->buffer);
   ecore_buffer_provider_buffer_enqueue(pd->provider, pd->buffer);
   pd->buffer = NULL;

   next_buffer = _simple_demo_provider_buffer_get(pd->provider, pd->w, pd->h, ECORE_BUFFER_FORMAT_XRGB8888);
   if (next_buffer)
     {
        LOGP("Drawable Buffer is Existed, ADD Render job again - buffer:%p", next_buffer);
        pd->buffer = next_buffer;
        _simple_demo_provider_render_queue(pd);
     }

   ecore_idle_enterer_del(pd->post_render);
   pd->post_render = NULL;

   LOGP("Done - Post Render");

   return ECORE_CALLBACK_RENEW;
}

static void
_simple_demo_provider_cb_render_job(void *data)
{
   Provider_Data *pd = (Provider_Data *)data;

   LOGP("Startup - Render");

   if (!pd->buffer)
     {
        pd->buffer = _simple_demo_provider_buffer_get(pd->provider,
                                                      pd->w, pd->h, ECORE_BUFFER_FORMAT_XRGB8888);
     }

   if (pd->buffer)
     {
        LOGP("Success to get Drawable Buffer, Drawing now... - buffer:%p", pd->buffer);
        // Drawing...

        if (!pd->post_render)
          {
             pd->post_render =
                ecore_idle_enterer_before_add(_simple_demo_provider_cb_post_render, pd);
          }
     }

   ecore_job_del(pd->render_job);
   pd->render_job = NULL;

   LOGP("Done - Render");
}

static void
_simple_demo_provider_render_queue(Provider_Data *pd)
{
   if (!pd) return;

   LOGP("Render Queue");

   if (!pd->render_job)
     pd->render_job = ecore_job_add(_simple_demo_provider_cb_render_job, pd);
}

Ecore_Buffer *
_simple_demo_provider_buffer_get(Ecore_Buffer_Provider *provider, unsigned int w, unsigned int h, unsigned int format)
{
   Ecore_Buffer *buffer = NULL;
   Ecore_Buffer_Return ret;
   unsigned int res_w, res_h, res_format;

   LOGPR("Dequeue");
   ret = ecore_buffer_provider_buffer_acquire(provider, &buffer);

   if (ret == ECORE_BUFFER_RETURN_NEED_ALLOC)
     {
        buffer = ecore_buffer_new(NULL, w, h, format, 0);
        LOGP("No buffer in Queue, Create Buffer");
     }
   else if (ret == ECORE_BUFFER_RETURN_SUCCESS)
     {
        ecore_buffer_size_get(buffer, &res_w, &res_h);
        res_format = ecore_buffer_format_get(buffer);

        if ((res_w != w) || (res_h != h) || (res_format != format))
          {
             LOGP("Need to Reallocate Buffer, Free it First: %p", buffer);
             ecore_buffer_free(buffer);

             buffer = ecore_buffer_new(NULL, w, h, format, 0);
             LOGP("Create Buffer: %p", buffer);
          }
     }

   return buffer;
}

static void
_simple_demo_cb_consumer_add(Ecore_Buffer_Provider *provider EINA_UNUSED, int queue_size, int w, int h, void *data)
{
   Provider_Data *pd = (Provider_Data *)data;

   LOGPE("Connected with Consumer, Now We can use Ecore_Buffer_Queue - queue_size:%d, geo(%dx%d)",
       queue_size, w, h);

   pd->w = w;
   pd->h = h;

   _simple_demo_provider_render_queue(pd);
}

static void
_simple_demo_cb_consumer_del(Ecore_Buffer_Provider *provider EINA_UNUSED, void *data)
{
   LOGPE("Disconnected with Consumer, End of Ecore_Buffer_Queue");

   (void)provider;
   (void)data;
   ecore_main_loop_quit();
}

static void
_simple_demo_cb_provider_buffer_released(Ecore_Buffer_Provider *provider EINA_UNUSED, void *data)
{
   Provider_Data *pd = (Provider_Data *)data;

   LOGPE("Buffer Enqueued : Count %d\n", test_count);

   test_count++;
   if (test_count > test_count_limit)
     {
        printf("Done Buffer Queue job - count: %d\n", test_count);
        ecore_main_loop_quit();
        return;
     }

   _simple_demo_provider_render_queue(pd);
}

/******* Consumer *******/
typedef struct _Consumer_Data
{
   Ecore_Buffer_Consumer *consumer;
   Ecore_Buffer *buffer;

   Ecore_Job *render_job;
   Ecore_Idle_Enterer *post_render;
} Consumer_Data;

static Consumer_Data *_simple_demo_consumer_new(const char *name, int32_t queue_size, int32_t w, int32_t h);
static void           _simple_demo_consumer_shutdown(Consumer_Data *cd);
static void           _simple_demo_consumer_run(Consumer_Data *cd);
static void           _simple_demo_consumer_render_queue(Consumer_Data *cd);

static Eina_Bool      _simple_demo_consumer_cb_post_render(void *data);
static void           _simple_demo_consumer_cb_render_job(void *data);
static void           _simple_demo_cb_provider_add(Ecore_Buffer_Consumer *consumer, void *data);
static void           _simple_demo_cb_provider_del(Ecore_Buffer_Consumer *consumer, void *data);
static void           _simple_demo_cb_consumer_enqueued(Ecore_Buffer_Consumer *consumer, void *data);

static void
_simple_demo_consumer_callback_set(Consumer_Data *cd,
                                   Ecore_Buffer_Consumer_Provider_Add_Cb provider_add,
                                   Ecore_Buffer_Consumer_Provider_Del_Cb provider_del,
                                   Ecore_Buffer_Consumer_Enqueue_Cb enqueued)
{
   ecore_buffer_consumer_provider_add_cb_set(cd->consumer, provider_add, cd);
   ecore_buffer_consumer_provider_del_cb_set(cd->consumer, provider_del, cd);
   ecore_buffer_consumer_buffer_enqueued_cb_set(cd->consumer, enqueued, cd);
}

static Consumer_Data *
_simple_demo_consumer_new(const char *name, int32_t queue_size, int32_t w, int32_t h)
{
   Consumer_Data *cd;

   LOGC("Startup");
   if (!init_all())
     {
        LOGC("Failed to init");
        goto end;
     }

   cd = (Consumer_Data *)calloc(sizeof(Consumer_Data), 1);

   if (!(cd->consumer = ecore_buffer_consumer_new(name, queue_size, w, h)))
     {
        goto err_consumer;
        LOGC("Failed to create consumer");
     }

   return cd;
err_consumer:
   shutdown_all();
end:
   return NULL;
}

static void
_simple_demo_consumer_shutdown(Consumer_Data *cd)
{
   if (cd->buffer) ecore_buffer_free(cd->buffer);
   if (cd->consumer) ecore_buffer_consumer_free(cd->consumer);
   if (cd->render_job) ecore_job_del(cd->render_job);
   if (cd->post_render) ecore_idle_enterer_del(cd->post_render);
   free(cd);
}

static void
_simple_demo_consumer_run(Consumer_Data *cd)
{
   ecore_main_loop_begin();

   _simple_demo_consumer_shutdown(cd);
   shutdown_all();
   LOGC("Shutdown");
}

static Eina_Bool
_simple_demo_consumer_cb_post_render(void *data)
{
   Consumer_Data *cd = (Consumer_Data *)data;
   Ecore_Buffer *next_buffer;

   LOGC("Startup - Post Render");

   LOGCR("Release Buffer - buffer: %p", cd->buffer);
   ecore_buffer_consumer_buffer_release(cd->consumer, cd->buffer);
   cd->buffer = NULL;

   if ((next_buffer = ecore_buffer_consumer_buffer_dequeue(cd->consumer)))
     {
        LOGC("Drawable Buffer is Existed, ADD Render job again - buffer:%p", next_buffer);
        cd->buffer = next_buffer;
        _simple_demo_consumer_render_queue(cd);
     }

   ecore_idle_enterer_del(cd->post_render);
   cd->post_render = NULL;

   LOGC("Done - Post Render");

   return ECORE_CALLBACK_RENEW;
}

static void
_simple_demo_consumer_cb_render_job(void *data)
{
   Consumer_Data *cd = (Consumer_Data *)data;
   Ecore_Buffer *buffer;

   LOGC("Startup - Render");

   if (!cd->buffer)
     {
        if ((buffer = ecore_buffer_consumer_buffer_dequeue(cd->consumer)))
            cd->buffer = buffer;
     }

   if (cd->buffer)
     {
        LOGC("Success to get Compositable Buffer, "
             "Drawing it to Consumer's Canvas now... - buffer:%p", cd->buffer);
        // Drawing...

        if (!cd->post_render)
          {
             cd->post_render =
                ecore_idle_enterer_before_add(_simple_demo_consumer_cb_post_render, cd);
          }
     }

   ecore_job_del(cd->render_job);
   cd->render_job = NULL;

   LOGC("Done - Render");
}

static void
_simple_demo_consumer_render_queue(Consumer_Data *cd)
{
   if (!cd) return;

   LOGC("Render Queue");

   if (!cd->render_job)
     cd->render_job = ecore_job_add(_simple_demo_consumer_cb_render_job, cd);
}

static void
_simple_demo_cb_provider_add(Ecore_Buffer_Consumer *consumer, void *data)
{
   LOGCE("Connected with Provider");

   (void)consumer;
   (void)data;
}

static void
_simple_demo_cb_provider_del(Ecore_Buffer_Consumer *consumer, void *data)
{
   LOGCE("Disconnected with Provider");

   (void)consumer;
   (void)data;

   ecore_main_loop_quit();
}

static void
_simple_demo_cb_consumer_enqueued(Ecore_Buffer_Consumer *consumer EINA_UNUSED, void *data)
{
   Consumer_Data *cd = (Consumer_Data *)data;

   LOGCE("Buffer Enqueued");

   _simple_demo_consumer_render_queue(cd);
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   Provider_Data *pd;
   char *str;

   consumer_pid = fork();
   if (consumer_pid == -1)
     return -1;

   if (consumer_pid == 0)
     {
        Consumer_Data *cd;

        if (!(cd = _simple_demo_consumer_new(EBQ_NAME, 3, 123, 456)))
          return -1;

        _simple_demo_consumer_callback_set(cd,
                                           _simple_demo_cb_provider_add,
                                           _simple_demo_cb_provider_del,
                                           _simple_demo_cb_consumer_enqueued);
        _simple_demo_consumer_run(cd);
     }

   if (!(pd = _simple_demo_provider_new(EBQ_NAME)))
     return -1;

   str = getenv("TEST_LIMIT");
   if (!str)
     test_count_limit = 1000;
   else
     test_count_limit = atoi(str);

   sig_handler = ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT,
                                         _simple_demo_cb_signal_exit,
                                         NULL);

   _simple_demo_provider_callback_set(pd,
                                      _simple_demo_cb_consumer_add,
                                      _simple_demo_cb_consumer_del,
                                      _simple_demo_cb_provider_buffer_released);

   _simple_demo_provider_run(pd);

   return 0;
}

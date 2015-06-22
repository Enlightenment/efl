#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include <Eina.h>
#include <Ecore_Getopt.h>
#include <wayland-server.h>

#include "bq_mgr_protocol.h"

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])

#define BQ_LOG(f, x...)       printf("[ES|%30.30s|%04d] " f "\n", __func__, __LINE__, ##x)
#define BQ_DEBUG(f, x...)     if (debug) printf("[ES|%30.30s|%04d] " f "\n", __func__, __LINE__, ##x)
#define BQ_OBJECT_NEW(x, f)   bq_object_new(sizeof(x), ((Bq_Object_Free_Func)(f)))
#define BQ_OBJECT(x)          ((Bq_Object *)(x))
#define BQ_OBJECT_RESOURCE(x) (((Bq_Object *)(x))->resource)

typedef void (*Bq_Object_Free_Func) (void *obj);

typedef struct _Bq_Object Bq_Object;
typedef struct _Bq_Mgr Bq_Mgr;
typedef struct _Bq_Buffer_Queue Bq_Buffer_Queue;
typedef struct _Bq_Buffer_Consumer Bq_Buffer_Consumer;
typedef struct _Bq_Buffer_Provider Bq_Buffer_Provider;
typedef struct _Bq_Buffer Bq_Buffer;
typedef enum _Bq_Buffer_Type Bq_Buffer_Type;

struct _Bq_Object
{
   int ref;
   Bq_Object_Free_Func free_fn;
   Eina_Bool deleted;
   struct wl_resource *resource;
};

struct _Bq_Mgr
{
   Bq_Object bq_obj;

   struct wl_display *wdpy;
   struct wl_event_source *signals[3];

   /*BufferQueue manager*/
   struct wl_global *bq_mgr;
   Eina_Hash *buffer_queues;
};


struct _Bq_Buffer_Queue
{
   Bq_Object bq_obj;
   Eina_Hash *link;

   char *name;
   struct wl_signal connect;

   Bq_Buffer_Consumer *consumer;
   Bq_Buffer_Provider *provider;
   Eina_Inlist *buffers;
};

struct _Bq_Buffer_Consumer
{
   Bq_Object bq_obj;
   Bq_Buffer_Queue *buffer_queue;

   int32_t queue_size;
   int32_t width;
   int32_t height;
};

struct _Bq_Buffer_Provider
{
   Bq_Object bq_obj;
   Bq_Buffer_Queue *buffer_queue;
};

enum _Bq_Buffer_Type
{
   bq_BUFFER_TYPE_ID,
   bq_BUFFER_TYPE_FD,
};

struct _Bq_Buffer
{
   Bq_Object bq_obj; /*Dont use wl_resource in bq_obj*/
   EINA_INLIST;

   struct wl_resource *consumer;
   struct wl_resource *provider;
   uint32_t serial;

   char *engine;
   Bq_Buffer_Type type;
   int32_t width;
   int32_t height;
   int32_t format;
   uint32_t flags;

   int32_t id;
   int32_t offset0;
   int32_t stride0;
   int32_t offset1;
   int32_t stride1;
   int32_t offset2;
   int32_t stride2;
};

static Eina_Bool debug;

static void *
bq_object_new(size_t size, Bq_Object_Free_Func fn)
{
   Bq_Object *o = calloc(1, size);

   if ((!o)) return NULL;

   o->ref = 1;
   o->free_fn = fn;

   return o;
}

static int
bq_object_ref(Bq_Object *o)
{
   o->ref++;

   return o->ref;
}

static int
bq_object_unref(Bq_Object *o)
{
   o->ref--;

   if (o->ref <= 0 || o->deleted)
     {
        if (o->free_fn)
          o->free_fn(o);

        free(o);
     }

   return o->ref;
}

static int
bq_object_free(Bq_Object *o)
{
   if (!o) return 0;
   if (o->deleted) return 0;

   o->deleted = EINA_TRUE;

   return bq_object_unref(o);
}

static void
bq_mgr_buffer_queue_free(Bq_Buffer_Queue *bq)
{
   Bq_Buffer *buf;

   if (!bq) return;

   BQ_DEBUG("destroy buffer queue : %s\n", bq->name);

   if (bq->consumer)
     {
        wl_resource_destroy(BQ_OBJECT_RESOURCE(bq->consumer));
        bq->consumer = NULL;
     }

   if (bq->provider)
     {
        wl_resource_destroy(BQ_OBJECT_RESOURCE(bq->provider));
        bq->provider = NULL;
     }

   while (bq->buffers)
     {
        buf = EINA_INLIST_CONTAINER_GET(bq->buffers,Bq_Buffer);
        bq->buffers = eina_inlist_remove(bq->buffers, bq->buffers);
        if (buf->consumer)
          {
             wl_resource_destroy(buf->consumer);
             buf->consumer = NULL;
          }

        if (buf->provider)
          {
             wl_resource_destroy(buf->provider);
             buf->provider = NULL;
          }
     }

   if (bq->link)
     {
        eina_hash_del(bq->link, bq->name, bq);
        bq->link = NULL;
     }
   if (bq->name)
     {
        free(bq->name);
        bq->name = NULL;
     }
}

static Bq_Buffer_Queue*
bq_mgr_buffer_queue_new(Bq_Mgr *bq_mgr, const char *name)
{
   Bq_Buffer_Queue *bq;

   bq = eina_hash_find(bq_mgr->buffer_queues, name);
   if (bq)
     {
        bq_object_ref(BQ_OBJECT(bq));
        return bq;
     }

   bq = BQ_OBJECT_NEW(Bq_Buffer_Queue, bq_mgr_buffer_queue_free);
   EINA_SAFETY_ON_NULL_RETURN_VAL(bq, NULL);

   bq->link = bq_mgr->buffer_queues;
   bq->name = strdup(name);
   if (!eina_hash_add(bq->link,bq->name,bq))
     {
        bq_object_free(BQ_OBJECT(bq));
        return NULL;
     }

   wl_signal_init(&bq->connect);
   bq->buffers = NULL;
   return bq;
}

static void
bq_mgr_buffer_consumer_release_buffer(struct wl_client *client EINA_UNUSED,
                                      struct wl_resource *resource,
                                      struct wl_resource *buffer)
{
   Bq_Buffer_Queue *bq;
   Bq_Buffer_Consumer *bq_consumer;
   Bq_Buffer_Provider *bq_provider;
   Bq_Buffer *bq_buffer;

   bq_consumer = (Bq_Buffer_Consumer*)wl_resource_get_user_data(resource);
   bq_buffer = (Bq_Buffer*)wl_resource_get_user_data(buffer);
   bq = bq_consumer->buffer_queue;
   bq_provider = bq->provider;

   if (bq_provider && bq_buffer->provider)
     {
        bq_provider_send_add_buffer(BQ_OBJECT_RESOURCE(bq_provider),
                                    bq_buffer->provider, bq_buffer->serial);
     }
}

static const struct bq_consumer_interface _bq_consumer_interface = {
     bq_mgr_buffer_consumer_release_buffer
};

static void
bq_mgr_buffer_consumer_destroy(struct wl_resource *resource)
{
   Bq_Buffer_Consumer *bq_consumer = wl_resource_get_user_data(resource);
   Bq_Buffer_Provider *bq_provider;
   Bq_Buffer_Queue *bq;
   Bq_Buffer *buf;

   if (!bq_consumer) return;

   BQ_DEBUG("destroy buffer consumer : %s\n", bq_consumer->buffer_queue->name);

   bq = bq_consumer->buffer_queue;
   bq_provider = bq->provider;

   bq->consumer = NULL;
   BQ_OBJECT_RESOURCE(bq_consumer) = NULL;

   if (bq_provider)
     {
        bq_provider_send_disconnected(BQ_OBJECT_RESOURCE(bq_provider));
     }

   while (bq->buffers)
     {
        buf = EINA_INLIST_CONTAINER_GET(bq->buffers,Bq_Buffer);
        bq->buffers = eina_inlist_remove(bq->buffers,bq->buffers);

        BQ_DEBUG("destroy BUFFER : %d\n", buf->type);
        if (buf->consumer)
          {
             wl_resource_destroy(buf->consumer);
             buf->consumer = NULL;
             bq_object_unref(BQ_OBJECT(buf));
          }

        if (buf->provider)
          {
             wl_resource_destroy(buf->provider);
             buf->provider = NULL;
             bq_object_unref(BQ_OBJECT(buf));
          }
     }

   bq_object_unref(BQ_OBJECT(bq_consumer));
   bq_object_unref(BQ_OBJECT(bq));
}

static void
bq_mgr_buffer_destroy(struct wl_resource *resource)
{
   Bq_Buffer *buf = wl_resource_get_user_data(resource);

   if (resource == buf->consumer)
     {
        BQ_DEBUG("destroy buffer : consumer\n");
     }
   else if (resource == buf->provider)
     {
        BQ_DEBUG("destroy buffer : provider\n");
     }
}

static void
bq_buffer_create_consumer_side(Bq_Buffer_Consumer *bq_consumer, Bq_Buffer *bq_buffer)
{
   if (!bq_consumer) return;

   bq_buffer->consumer = wl_resource_create(wl_resource_get_client(BQ_OBJECT_RESOURCE(bq_consumer)),
                                            &bq_buffer_interface, 1, 0);
   wl_resource_set_implementation(bq_buffer->consumer, NULL, bq_buffer, bq_mgr_buffer_destroy);
   bq_object_ref(BQ_OBJECT(bq_buffer));

   bq_consumer_send_buffer_attached(BQ_OBJECT_RESOURCE(bq_consumer),
                                    bq_buffer->consumer,
                                    bq_buffer->engine,
                                    bq_buffer->width,
                                    bq_buffer->height,
                                    bq_buffer->format,
                                    bq_buffer->flags);
}

static void
bq_buffer_set_consumer_side(Bq_Buffer_Consumer *bq_consumer, Bq_Buffer *bq_buffer)
{
   if (!bq_consumer) return;
   EINA_SAFETY_ON_NULL_RETURN(bq_buffer);
   EINA_SAFETY_ON_NULL_RETURN(bq_buffer->consumer);

   if (bq_buffer->type == bq_BUFFER_TYPE_ID)
     bq_consumer_send_set_buffer_id(BQ_OBJECT_RESOURCE(bq_consumer),
                                    bq_buffer->consumer,
                                    bq_buffer->id,
                                    bq_buffer->offset0,
                                    bq_buffer->stride0,
                                    bq_buffer->offset1,
                                    bq_buffer->stride1,
                                    bq_buffer->offset2,
                                    bq_buffer->stride2);
   else
     {
        bq_consumer_send_set_buffer_fd(BQ_OBJECT_RESOURCE(bq_consumer),
                                       bq_buffer->consumer,
                                       bq_buffer->id,
                                       bq_buffer->offset0,
                                       bq_buffer->stride0,
                                       bq_buffer->offset1,
                                       bq_buffer->stride1,
                                       bq_buffer->offset2,
                                       bq_buffer->stride2);
        close(bq_buffer->id);
     }
}

static void
bq_mgr_buffer_queue_create_consumer(struct wl_client *client,
                                    struct wl_resource *resource,
                                    uint32_t id,
                                    const char *name,
                                    int32_t queue_size,
                                    int32_t width,
                                    int32_t height)
{
   Bq_Mgr *bq_mgr = (Bq_Mgr*)wl_resource_get_user_data(resource);
   Bq_Buffer_Queue *bq;
   Bq_Buffer_Consumer *bq_consumer;
   Bq_Buffer_Provider *bq_provider;
   Bq_Buffer *buf;

   EINA_SAFETY_ON_NULL_RETURN(bq_mgr);

   bq = bq_mgr_buffer_queue_new(bq_mgr,name);
   EINA_SAFETY_ON_NULL_RETURN(bq);

   if (bq->consumer)
     {
        bq_object_unref(BQ_OBJECT(bq));
        wl_resource_post_error(resource,
                               BQ_MGR_ERROR_ALREADY_USED,
                               "%s consumer already used",name);
        return;
     }

   bq_consumer = BQ_OBJECT_NEW(Bq_Buffer_Consumer, NULL);
   EINA_SAFETY_ON_NULL_RETURN(bq_consumer);
   BQ_OBJECT_RESOURCE(bq_consumer) = wl_resource_create(client,
                                                        &bq_consumer_interface,
                                                        1, id);
   if (!BQ_OBJECT_RESOURCE(bq_consumer))
     {
        bq_object_unref(BQ_OBJECT(bq_consumer));
        wl_client_post_no_memory(client);
        return;
     }

   wl_resource_set_implementation(BQ_OBJECT_RESOURCE(bq_consumer),
                                  &_bq_consumer_interface,
                                  bq_consumer,
                                  bq_mgr_buffer_consumer_destroy);

   bq_consumer->buffer_queue = bq;
   bq_consumer->queue_size = queue_size;
   bq_consumer->width = width;
   bq_consumer->height = height;

   bq_provider = bq->provider;
   bq->consumer = bq_consumer;
   if (bq_provider)
     {
        bq_provider_send_connected(BQ_OBJECT_RESOURCE(bq_provider),
                                   queue_size, width, height);
        bq_consumer_send_connected(BQ_OBJECT_RESOURCE(bq_consumer));
     }

   EINA_INLIST_FOREACH(bq->buffers,buf)
     {
        bq_buffer_create_consumer_side(bq_consumer, buf);
        bq_buffer_set_consumer_side(bq_consumer, buf);
     }
}

static void
bq_buffer_free(Bq_Buffer *buf)
{
   if (buf->engine)
     free(buf->engine);
}

static void
bq_mgr_buffer_provider_attatch_buffer(struct wl_client *client,
                                      struct wl_resource *resource,
                                      uint32_t buffer,
                                      const char *engine,
                                      int32_t width,
                                      int32_t height,
                                      int32_t format,
                                      uint32_t flags)
{
   Bq_Buffer_Provider *bq_provider = wl_resource_get_user_data(resource);
   Bq_Buffer_Consumer *bq_consumer;
   Bq_Buffer_Queue *bq;
   Bq_Buffer *bq_buffer;

   EINA_SAFETY_ON_NULL_RETURN(bq_provider);
   bq = bq_provider->buffer_queue;
   bq_consumer = bq->consumer;

   bq_buffer = BQ_OBJECT_NEW(Bq_Buffer, bq_buffer_free);
   bq_buffer->provider = wl_resource_create(client, &bq_buffer_interface, 1, buffer);
   wl_resource_set_implementation(bq_buffer->provider, NULL, bq_buffer, bq_mgr_buffer_destroy);

   if (!bq_buffer->provider)
     {
        wl_client_post_no_memory(client);
        bq_object_unref(BQ_OBJECT(bq_buffer));
        return;
     }

   bq_buffer->engine = strdup(engine);
   bq_buffer->width = width;
   bq_buffer->height = height;
   bq_buffer->format = format;
   bq_buffer->flags = flags;

   bq->buffers = eina_inlist_append(bq->buffers,EINA_INLIST_GET(bq_buffer));
   BQ_DEBUG("add BUFFER : %d\n", bq_buffer->type);

   bq_buffer_create_consumer_side(bq_consumer, bq_buffer);

}

static void
bq_mgr_buffer_provider_set_buffer_id(struct wl_client *client EINA_UNUSED,
                                     struct wl_resource *resource,
                                     struct wl_resource *buffer,
                                     int32_t id,
                                     int32_t offset0,
                                     int32_t stride0,
                                     int32_t offset1,
                                     int32_t stride1,
                                     int32_t offset2,
                                     int32_t stride2)
{
   Bq_Buffer_Provider *bq_provider = wl_resource_get_user_data(resource);
   Bq_Buffer_Consumer *bq_consumer;
   Bq_Buffer_Queue *bq;
   Bq_Buffer *bq_buffer;

   EINA_SAFETY_ON_NULL_RETURN(bq_provider);
   bq = bq_provider->buffer_queue;
   bq_consumer = bq->consumer;
   bq_buffer = wl_resource_get_user_data(buffer);
   EINA_SAFETY_ON_NULL_RETURN(bq_buffer);

   bq_buffer->type = bq_BUFFER_TYPE_ID;
   bq_buffer->id = id;
   bq_buffer->offset0 = offset0;
   bq_buffer->stride0 = stride0;
   bq_buffer->offset1 = offset1;
   bq_buffer->stride1 = stride1;
   bq_buffer->offset2 = offset2;
   bq_buffer->stride2 = stride2;

   bq_buffer_set_consumer_side(bq_consumer, bq_buffer);
}

static void
bq_mgr_buffer_provider_set_buffer_fd(struct wl_client *client EINA_UNUSED,
                                     struct wl_resource *resource,
                                     struct wl_resource *buffer,
                                     int32_t fd,
                                     int32_t offset0,
                                     int32_t stride0,
                                     int32_t offset1,
                                     int32_t stride1,
                                     int32_t offset2,
                                     int32_t stride2)
{
   Bq_Buffer_Provider *bq_provider = wl_resource_get_user_data(resource);
   Bq_Buffer_Consumer *bq_consumer;
   Bq_Buffer_Queue *bq;
   Bq_Buffer *bq_buffer;

   EINA_SAFETY_ON_NULL_RETURN(bq_provider);
   bq = bq_provider->buffer_queue;
   bq_consumer = bq->consumer;
   bq_buffer = wl_resource_get_user_data(buffer);
   EINA_SAFETY_ON_NULL_RETURN(bq_buffer);

   bq_buffer->type = bq_BUFFER_TYPE_FD;
   bq_buffer->id = fd;
   bq_buffer->offset0 = offset0;
   bq_buffer->stride0 = stride0;
   bq_buffer->offset1 = offset1;
   bq_buffer->stride1 = stride1;
   bq_buffer->offset2 = offset2;
   bq_buffer->stride2 = stride2;

   bq_buffer_set_consumer_side(bq_consumer, bq_buffer);
}


static void
bq_mgr_buffer_provider_detach_buffer(struct wl_client *client EINA_UNUSED,
                                     struct wl_resource *resource,
                                     struct wl_resource *buffer)
{
   Bq_Buffer_Provider *bq_provider = wl_resource_get_user_data(resource);
   Bq_Buffer_Consumer *bq_consumer;
   Bq_Buffer_Queue *bq;
   Bq_Buffer *bq_buffer;

   EINA_SAFETY_ON_NULL_RETURN(bq_provider);
   bq = bq_provider->buffer_queue;
   bq_consumer = bq->consumer;
   bq_buffer = wl_resource_get_user_data(buffer);

   if (bq_consumer)
     {
        bq_consumer_send_buffer_detached(BQ_OBJECT_RESOURCE(bq_consumer),
                                         bq_buffer->consumer);
        wl_resource_destroy(bq_buffer->consumer);
        bq_object_unref(BQ_OBJECT(bq_buffer));
     }

   wl_resource_destroy(bq_buffer->provider);
   bq->buffers = eina_inlist_remove(bq->buffers,EINA_INLIST_GET(bq_buffer));
   bq_object_unref(BQ_OBJECT(bq_buffer));
}

static void
bq_mgr_buffer_provider_enqueue_buffer(struct wl_client *client EINA_UNUSED,
                                      struct wl_resource *resource,
                                      struct wl_resource *buffer,
                                      uint32_t serial)
{
   Bq_Buffer_Provider *bq_provider = wl_resource_get_user_data(resource);
   Bq_Buffer_Consumer *bq_consumer;
   Bq_Buffer_Queue *bq;
   Bq_Buffer *bq_buffer;

   EINA_SAFETY_ON_NULL_RETURN(bq_provider);
   bq = bq_provider->buffer_queue;
   bq_consumer = bq->consumer;
   if (!bq_consumer)
     {
        wl_resource_post_error(BQ_OBJECT_RESOURCE(bq_consumer),
                               BQ_PROVIDER_ERROR_CONNECTION,
                               "Not connected:%s", bq->name);
        return;
     }

   bq_buffer = wl_resource_get_user_data(buffer);
   EINA_SAFETY_ON_NULL_RETURN(bq_buffer);
   bq_buffer->serial = serial;

   bq_consumer_send_add_buffer(BQ_OBJECT_RESOURCE(bq_consumer),
                               bq_buffer->consumer,
                               bq_buffer->serial);
}

static const struct bq_provider_interface _bq_provider_interface = {
     bq_mgr_buffer_provider_attatch_buffer,
     bq_mgr_buffer_provider_set_buffer_id,
     bq_mgr_buffer_provider_set_buffer_fd,
     bq_mgr_buffer_provider_detach_buffer,
     bq_mgr_buffer_provider_enqueue_buffer
};

static void
bq_mgr_buffer_provider_destroy(struct wl_resource *resource)
{
   Bq_Buffer_Queue *bq;
   Bq_Buffer_Provider *bq_provider = wl_resource_get_user_data(resource);
   Bq_Buffer_Consumer *bq_consumer;
   Bq_Buffer *buf;

   EINA_SAFETY_ON_NULL_RETURN(bq_provider);
   BQ_DEBUG("destroy buffer provider : %s\n", bq_provider->buffer_queue->name);
   bq = bq_provider->buffer_queue;
   bq_consumer = bq->consumer;

   BQ_OBJECT_RESOURCE(bq_provider) = NULL;
   bq->provider = NULL;

   while (bq->buffers)
     {
        buf = EINA_INLIST_CONTAINER_GET(bq->buffers, Bq_Buffer);
        bq->buffers = eina_inlist_remove(bq->buffers, bq->buffers);

        if (buf->consumer)
          {
             bq_consumer_send_buffer_detached(BQ_OBJECT_RESOURCE(bq_consumer), buf->consumer);
             wl_resource_destroy(buf->consumer);
             buf->consumer = NULL;
             bq_object_unref(BQ_OBJECT(buf));
          }

        if (buf->provider)
          {
             wl_resource_destroy(buf->provider);
             buf->provider = NULL;
             bq_object_unref(BQ_OBJECT(buf));
          }
     }

   if (bq_consumer)
     {
        if (BQ_OBJECT_RESOURCE(bq_consumer))
          bq_consumer_send_disconnected(BQ_OBJECT_RESOURCE(bq_consumer));
     }

   bq_object_unref(BQ_OBJECT(bq_provider));
   bq_object_unref(BQ_OBJECT(bq));
}

static void
bq_mgr_buffer_queue_create_provider(struct wl_client *client,
                                    struct wl_resource *resource,
                                    uint32_t id,
                                    const char *name)
{
   Bq_Mgr *bq_mgr = (Bq_Mgr*)wl_resource_get_user_data(resource);
   Bq_Buffer_Queue *bq;
   Bq_Buffer_Provider *bq_provider;
   Bq_Buffer_Consumer *bq_consumer;

   EINA_SAFETY_ON_NULL_RETURN(bq_mgr);

   bq = bq_mgr_buffer_queue_new(bq_mgr,name);
   EINA_SAFETY_ON_NULL_RETURN(bq);

   if (bq->provider)
     {
        bq_object_unref(BQ_OBJECT(bq));
        wl_resource_post_error(resource,
                               BQ_MGR_ERROR_ALREADY_USED,
                               "%s rpovider already used",name);
        return;
     }

   bq_provider = BQ_OBJECT_NEW(Bq_Buffer_Provider, NULL);
   EINA_SAFETY_ON_NULL_GOTO(bq_provider, on_error);

   BQ_OBJECT_RESOURCE(bq_provider)= wl_resource_create(client,
                                                       &bq_provider_interface,
                                                       1, id);
   EINA_SAFETY_ON_NULL_GOTO(BQ_OBJECT_RESOURCE(bq_provider), on_error);

   wl_resource_set_implementation(BQ_OBJECT_RESOURCE(bq_provider),
                                  &_bq_provider_interface,
                                  bq_provider,
                                  bq_mgr_buffer_provider_destroy);

   bq_provider->buffer_queue = bq;
   bq->provider = bq_provider;
   bq_consumer = bq->consumer;
   if (bq_consumer)
     {
        /*Send connect*/
        bq_consumer_send_connected(BQ_OBJECT_RESOURCE(bq_consumer));
        bq_provider_send_connected(BQ_OBJECT_RESOURCE(bq_provider),
                                   bq_consumer->queue_size,
                                   bq_consumer->width, bq_consumer->height);
     }

   return;

on_error:
   if (bq) bq_object_unref(BQ_OBJECT(bq));
   if (bq_provider) bq_object_unref(BQ_OBJECT(bq_provider));
   wl_client_post_no_memory(client);
}

static const struct bq_mgr_interface _bq_mgr_interface =
{
   bq_mgr_buffer_queue_create_consumer,
   bq_mgr_buffer_queue_create_provider
};

static void
bq_mgr_buffer_queue_bind(struct wl_client *client, void *data,
                         uint32_t version EINA_UNUSED, uint32_t id)
{
   Bq_Mgr *bq_mgr = (Bq_Mgr*)data;
   struct wl_resource *resource;

   resource = wl_resource_create(client, &bq_mgr_interface,
                                 1, id);
   if (resource == NULL) {
        wl_client_post_no_memory(client);
        return;
   }

   wl_resource_set_implementation(resource,
                                  &_bq_mgr_interface,
                                  bq_mgr, NULL);
}

Eina_Bool
bq_mgr_buffer_queue_manager_init(Bq_Mgr *bq_mgr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(bq_mgr,EINA_FALSE);
   bq_mgr->bq_mgr = wl_global_create(bq_mgr->wdpy
                                     ,&bq_mgr_interface,1
                                     ,bq_mgr
                                     ,bq_mgr_buffer_queue_bind);

   EINA_SAFETY_ON_NULL_RETURN_VAL(bq_mgr->bq_mgr,EINA_FALSE);

   bq_mgr->buffer_queues = eina_hash_string_superfast_new(NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(bq_mgr->buffer_queues,EINA_FALSE);

   return EINA_TRUE;
}

static const Ecore_Getopt optdesc =
{
   "tbm_daemon",
   "%prog [options]",
   "0.1.0",
   "(C) Samsung",
   "BSD 2-Clause",
   "Tizen Buffer Manager Daemon",
   EINA_FALSE,
   {
      ECORE_GETOPT_STORE_STR('s', "socket_name",
                             "socket name"),
      ECORE_GETOPT_STORE_BOOL('d',"debug","enable debug log"),
      ECORE_GETOPT_VERSION('v', "version"),
      ECORE_GETOPT_HELP('h', "help"),
      ECORE_GETOPT_SENTINEL
   }
};

static int
bq_mgr_on_term_signal(int signal_number, void *data)
{
   Bq_Mgr *bq_mgr = data;

   BQ_LOG("caught signal %d\n", signal_number);
   wl_display_terminate(bq_mgr->wdpy);

   return 1;
}

static void
bq_mgr_free(Bq_Mgr *bq_mgr)
{
   int i;

   for (i = ARRAY_LENGTH(bq_mgr->signals) - 1; i >= 0; i--)
     {
        if (bq_mgr->signals[i])
          wl_event_source_remove(bq_mgr->signals[i]);
     }

   if (bq_mgr->wdpy)
     wl_display_destroy(bq_mgr->wdpy);
}

static Bq_Mgr*
bq_mgr_new(char *sock_name)
{
   static char *default_sock_name = "bq_mgr_daemon";
   Bq_Mgr *bq_mgr = BQ_OBJECT_NEW(Bq_Mgr, bq_mgr_free);
   struct wl_event_loop *loop;

   if (!bq_mgr) return NULL;

   bq_mgr->wdpy = wl_display_create();
   loop = wl_display_get_event_loop(bq_mgr->wdpy);
   EINA_SAFETY_ON_NULL_GOTO(loop, on_err);

   bq_mgr->signals[0] = wl_event_loop_add_signal(loop, SIGTERM, bq_mgr_on_term_signal, bq_mgr);
   bq_mgr->signals[1] = wl_event_loop_add_signal(loop, SIGINT, bq_mgr_on_term_signal, bq_mgr);
   bq_mgr->signals[2] = wl_event_loop_add_signal(loop, SIGQUIT, bq_mgr_on_term_signal, bq_mgr);

   if (!sock_name)
     sock_name = default_sock_name;
   wl_display_add_socket(bq_mgr->wdpy, sock_name);

   return bq_mgr;

on_err:
   bq_object_free(BQ_OBJECT(bq_mgr));
   return NULL;
}

int
main(int argc, char **argv)
{
   Bq_Mgr *bq_mgr = NULL;
   int res, ret = EXIT_FAILURE;
   char *opt_path = NULL;
   Eina_Bool quit = EINA_FALSE;
   Ecore_Getopt_Value values[] =
     {
        ECORE_GETOPT_VALUE_STR(opt_path),
        ECORE_GETOPT_VALUE_BOOL(debug),
        ECORE_GETOPT_VALUE_BOOL(quit),
        ECORE_GETOPT_VALUE_BOOL(quit),
        ECORE_GETOPT_VALUE_NONE
     };

   eina_init();

   res = ecore_getopt_parse(&optdesc,
                            values,
                            argc, argv);

   if ((res < 0) || (quit)) goto finish;

   if (opt_path)
     BQ_LOG("socket_name : %s\n", opt_path);

   bq_mgr = bq_mgr_new(opt_path);
   if (!bq_mgr) goto finish;

   if (!bq_mgr_buffer_queue_manager_init(bq_mgr))
     {
        bq_mgr_free(bq_mgr);
        goto finish;
     }

   wl_display_run(bq_mgr->wdpy);

   ret = EXIT_SUCCESS;
finish:
   eina_shutdown();
   bq_object_free(BQ_OBJECT(bq_mgr));

   return ret;
}

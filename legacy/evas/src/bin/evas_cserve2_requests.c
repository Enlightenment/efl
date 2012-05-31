#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "evas_cserve2.h"

#define MAX_SLAVES 1

typedef enum
{
   SLAVE_IMAGE,
   SLAVE_FONT,
   SLAVE_NONE
} Slave_Type;

struct _Slave_Worker
{
   Slave_Type type;
   void *data;
   Slave *slave;
   Eina_Binbuf *ret;
   int ret_size;
   Eina_Bool done;
   Eina_Bool delete_me;
};

typedef struct _Slave_Worker Slave_Worker;

/* This struct is used to match font request types to the respective slave
 * type, and the message type that will be used for that request. The order
 * of the request types on it is the order in which these requests will
 * be processed.
 */
static struct _Request_Match
{
   Font_Request_Type rtype;
   Slave_Type stype;
   Slave_Command ctype;
} _request_match[] =
{
   { CSERVE2_REQ_FONT_LOAD, SLAVE_FONT, FONT_LOAD },
   { CSERVE2_REQ_FONT_GLYPHS_LOAD, SLAVE_FONT, FONT_GLYPHS_LOAD },
   { CSERVE2_REQ_LAST, 0 }
};

static Slave *_create_image_slave(void *data);
static Slave *_create_font_slave(void *data);

static struct _Worker
{
   Slave_Type type;
   unsigned int max;
   Eina_List *idle;
   Eina_List *working;
   Slave *(*create_slave)(void *data);
} _workers[] =
{
   { SLAVE_IMAGE, 3, NULL, NULL, _create_image_slave },
   { SLAVE_FONT, 1, NULL, NULL, _create_font_slave },
};

struct _Font_Request
{
   EINA_INLIST;
   Font_Request_Type type;
   void *data;
   void *msg;
   Eina_List *waiters;
   Eina_Bool processing;
   Font_Request_Funcs *funcs;
};

struct _Waiter
{
   unsigned int rid;
   Client *client;
};

typedef struct _Waiter Waiter;

struct _Request_Queue
{
   Eina_Inlist *waiting;
   Eina_Inlist *processing;
};

typedef struct _Request_Queue Request_Queue;

static Request_Queue *requests = NULL;
// static Eina_List *processing = NULL;

static void
_request_waiter_add(Font_Request *req, Client *client, unsigned int rid)
{
   Waiter *w = malloc(sizeof(*w));

   DBG("Add waiter to request. Client: %d, rid: %d", client->id, rid);

   w->client = client;
   w->rid = rid;

   req->waiters = eina_list_append(req->waiters, w);
}

Font_Request *
cserve2_request_add(Font_Request_Type type, unsigned int rid, Client *client, Font_Request_Funcs *funcs, void *data)
{
   Font_Request *req, *r;

   req = NULL;

   EINA_INLIST_FOREACH(requests[type].processing, r)
     {
        if (r->data == data)
          continue;

        req = r;
        break;
     }

   if (!req)
     {
        EINA_INLIST_FOREACH(requests[type].waiting, r)
          {
             if (r->data != data)
               continue;

             req = r;
             break;
          }
     }

   if (!req)
     {
        DBG("Add request for rid: %d", rid);
        req = malloc(sizeof(*req));
        req->data = data;
        req->waiters = NULL;
        req->processing = EINA_FALSE;
        requests[type].waiting = eina_inlist_append(requests[type].waiting,
                                                    EINA_INLIST_GET(req));
     }

   _request_waiter_add(req, client, rid);

   return req;
}

void
cserve2_request_cancel(Font_Request *req, Client *client, Error_Type err)
{
   Eina_List *l, *l_next;
   Waiter *w;

   EINA_LIST_FOREACH_SAFE(req->waiters, l, l_next, w)
     {
        if (w->client->id == client->id)
          {
             DBG("Removing answer from waiter client: %d, rid: %d",
                 client->id, w->rid);
             if (req->funcs && req->funcs->error)
               req->funcs->error(client, req->data, err, w->rid);
             req->waiters = eina_list_remove_list(req->waiters, l);
             free(w);
          }
     }

   // TODO: When we have speculative preload, there may be no waiters,
   // so we need a flag or something else to make things still load.
   if (!req->waiters)
     {
        Eina_Inlist **reqlist = &requests[req->type].waiting;
        *reqlist = eina_inlist_remove(*reqlist, EINA_INLIST_GET(req));
        // TODO: If the request is being processed, it can't be deleted. Must
        // be marked as delete_me instead.
        req->funcs->msg_free(req->msg);
        free(req);
     }

}

void
cserve2_request_cancel_all(Font_Request *req, Error_Type err)
{
   Waiter *w;

   DBG("Removing all answers.");

   EINA_LIST_FREE(req->waiters, w)
     {
        DBG("Removing answer from waiter client: %d, rid: %d",
            w->client->id, w->rid);
        if (req->funcs && req->funcs->error)
          req->funcs->error(w->client, req->data, err, w->rid);
        free(w);
     }

   requests[req->type].waiting = eina_inlist_remove(
      requests[req->type].waiting, EINA_INLIST_GET(req));
   req->funcs->msg_free(req->msg);
   free(req);
}

void
cserve2_requests_init(void)
{
   DBG("Initializing requests.");
   requests = calloc(CSERVE2_REQ_LAST, sizeof(*requests));
}

void
cserve2_requests_shutdown(void)
{
   DBG("Shutting down requests.");
   free(requests);
}

static void
_cserve2_request_failed(Font_Request *req, Error_Type type)
{
   Waiter *w;

   EINA_LIST_FREE(req->waiters, w)
     {
        req->funcs->error(w->client, req->data, type, w->rid);
        free(w);
     }

   req->funcs->msg_free(req->msg);
   requests[req->type].processing = eina_inlist_remove(
      requests[req->type].processing, EINA_INLIST_GET(req));
   free(req);
}

static void
_slave_read_cb(Slave *s __UNUSED__, Slave_Command cmd, void *msg, void *data)
{
   Slave_Worker *sw = data;
   Font_Request *req = sw->data;
   Eina_List **working, **idle;
   Waiter *w;

   EINA_LIST_FREE(req->waiters, w)
     {
        if (cmd == ERROR)
          {
             Error_Type *err = msg;
             req->funcs->error(w->client, req->data, *err, w->rid);
          }
        else
          req->funcs->response(w->client, req->data, msg, w->rid);
        free(w);
     }

   req->funcs->msg_free(req->msg);
   // FIXME: We shouldn't free this message directly, it must be freed by a
   // callback.
   free(msg);
   requests[req->type].processing = eina_inlist_remove(
      requests[req->type].processing, EINA_INLIST_GET(req));
   free(req);
   sw->data = NULL;

   working = &_workers[sw->type].working;
   idle = &_workers[sw->type].idle;
   *working = eina_list_remove(*working, sw);
   *idle = eina_list_append(*idle, sw);
}

static void
_slave_dead_cb(Slave *s __UNUSED__, void *data)
{
   Slave_Worker *sw = data;
   Font_Request *req = sw->data;
   Eina_List **working = &_workers[sw->type].working;

   if (req)
     _cserve2_request_failed(req, CSERVE2_LOADER_DIED);

   *working = eina_list_remove(*working, sw);
   free(sw);
}

static Slave *
_create_image_slave(void *data)
{
   char *exe;
   Slave *slave;

   exe = getenv("EVAS_CSERVE2_SLAVE");
   if (!exe) exe = "evas_cserve2_slave";

   slave = cserve2_slave_run(exe, _slave_read_cb,
                             _slave_dead_cb, data);

   return slave;
}

static Slave *
_create_font_slave(void *data)
{
   Slave *slave;

   slave = cserve2_slave_thread_run(cserve2_font_slave_cb, NULL,
                                    _slave_read_cb, _slave_dead_cb,
                                    data);

   return slave;
}

static Slave_Worker *
_slave_for_request_create(Slave_Type type)
{
   Slave_Worker *sw;
   Slave *slave;

   sw = calloc(1, sizeof(Slave_Worker));
   if (!sw) return NULL;

   slave = _workers[type].create_slave(sw);
   if (!slave)
     {
        ERR("Could not launch slave process");
        free(sw);
        return NULL;
     }

   sw->slave = slave;
   sw->type = type;
   _workers[type].idle = eina_list_append(_workers[type].idle, sw);

   return sw;
}

static Eina_Bool
_cserve2_request_dispatch(Slave_Worker *sw, Slave_Command ctype, Font_Request *req)
{
   int size;
   char *slave_msg = req->funcs->msg_create(req->data, &size);

   if (!slave_msg)
     {
        ERR("Could not create slave message for request type %d.", req->type);
        return EINA_FALSE;
     }

   req->msg = slave_msg;
   sw->data = req;
   cserve2_slave_send(sw->slave, ctype, slave_msg, size);
   req->processing = EINA_TRUE;

   return EINA_TRUE;
}

void
cserve2_requests_process(void)
{
    int rtype, j;

    for (rtype = 0; rtype < CSERVE2_REQ_LAST; rtype++)
      {
         Slave_Type type = SLAVE_NONE;
         Slave_Command ctype;
         unsigned int max_workers;
         Eina_List **idle, **working;

         for (j = 0; _request_match[j].rtype != CSERVE2_REQ_LAST; j++)
           {
              if (_request_match[j].rtype == j)
                {
                   type = _request_match[j].stype;
                   ctype = _request_match[j].ctype;
                   break;
                }
           }

         if (type == SLAVE_NONE)
           continue;

         if (!requests[rtype].waiting)
           continue;

         /* Now we have the worker type to use (image or font), and the list
          * of requests to process. Just process as many requests as we can.
          */
         max_workers = _workers[type].max;
         idle = &_workers[type].idle;
         working = &_workers[type].working;

         while (requests[j].waiting &&
                (eina_list_count(*working) < max_workers))
           {
              Slave_Worker *sw;
              Font_Request *req = EINA_INLIST_CONTAINER_GET(
                 requests[rtype].waiting, Font_Request);

              requests[rtype].waiting = eina_inlist_remove(
                 requests[rtype].waiting, requests[rtype].waiting);
              requests[rtype].processing = eina_inlist_append(
                 requests[rtype].processing, EINA_INLIST_GET(req));

              if (!(*idle))
                sw = _slave_for_request_create(type);

              if (!(*idle))
                {
                   ERR("No idle slave available to process request type %d.",
                       rtype);
                   _cserve2_request_failed(req, CSERVE2_GENERIC);
                   continue;
                }

              sw = eina_list_data_get(*idle);
              if (!_cserve2_request_dispatch(sw, ctype, req))
                {
                   ERR("Could not dispatch request.");
                   _cserve2_request_failed(req, CSERVE2_GENERIC);
                   continue;
                }

              *idle = eina_list_remove_list(*idle, *idle);
              *working = eina_list_append(*working, sw);

           }
      }
}

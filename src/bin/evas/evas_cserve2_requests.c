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
static const struct _Request_Match
{
   Slave_Request_Type rtype;
   Slave_Type stype;
   Slave_Command ctype;
   int require_spares; /* for speculative operations, will require to leave at
                          least this number of slaves always available */
} _request_match[] =
{
   { CSERVE2_REQ_IMAGE_OPEN, SLAVE_IMAGE, IMAGE_OPEN, 0 },
   { CSERVE2_REQ_IMAGE_LOAD, SLAVE_IMAGE, IMAGE_LOAD, 0 },
   { CSERVE2_REQ_IMAGE_SPEC_LOAD, SLAVE_IMAGE, IMAGE_LOAD, 1 },
   { CSERVE2_REQ_FONT_LOAD, SLAVE_FONT, FONT_LOAD, 0 },
   { CSERVE2_REQ_FONT_GLYPHS_LOAD, SLAVE_FONT, FONT_GLYPHS_LOAD, 0 },
   { CSERVE2_REQ_LAST, SLAVE_NONE, ERROR, 0 }
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

struct _Slave_Request
{
   EINA_INLIST;
   Slave_Request_Type type;
   void *data;
   void *msg;
   Eina_List *waiters;
   Eina_Bool processing;
   Slave_Request_Funcs *funcs;
   Slave_Request *dependency;
   Eina_List *dependents; /* list of requests that depend on this one finishing */
   Eina_Bool locked : 1; /* locked waiting for a dependency request to finish */
   Eina_Bool cancelled : 1;
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
   Eina_Inlist *processing; // TODO: Check if is there any use for this list.
};

typedef struct _Request_Queue Request_Queue;

static Request_Queue *requests = NULL;
// static Eina_List *processing = NULL;

static void _cserve2_requests_process(void);

static void
_request_waiter_add(Slave_Request *req, Client *client, unsigned int rid)
{
   Waiter *w = malloc(sizeof(*w));

   DBG("Add waiter to request. Client: %d, rid: %d", client->id, rid);

   w->client = client;
   w->rid = rid;

   req->waiters = eina_list_append(req->waiters, w);
}

Slave_Request *
cserve2_request_add(Slave_Request_Type type, unsigned int rid, Client *client, Slave_Request *dep, Slave_Request_Funcs *funcs, void *data)
{
   Slave_Request *req, *r;

   req = NULL;

   /* Check if this request was already being processed. */
   EINA_INLIST_FOREACH(requests[type].processing, r)
     {
        if (r->data != data)
          continue;

        req = r;
        break;
     }

   /* Check if this request was already waiting to be processed. */
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

   /* create new request */
   if (!req)
     {
        DBG("Add request for rid: %d", rid);
        req = calloc(1, sizeof(*req));
        req->type = type;
        req->data = data;
        req->waiters = NULL;
        req->processing = EINA_FALSE;
        req->funcs = funcs;
        requests[type].waiting = eina_inlist_append(requests[type].waiting,
                                                    EINA_INLIST_GET(req));
     }

   if (dep && !req->dependency)
     {
        req->locked = EINA_TRUE;
        dep->dependents = eina_list_append(dep->dependents, req);
        req->dependency = dep;
     }

   if (client && rid)
     _request_waiter_add(req, client, rid);

   _cserve2_requests_process();

   return req;
}

void
cserve2_request_waiter_add(Slave_Request *req, unsigned int rid, Client *client)
{
   _request_waiter_add(req, client, rid);
}

void
cserve2_request_type_set(Slave_Request *req, Slave_Request_Type type)
{
   Eina_Inlist **from, **to;

   if (req->processing || (type == req->type))
     return;

   from = &requests[req->type].waiting;
   to = &requests[type].waiting;

   req->type = type;
   *from = eina_inlist_remove(*from, EINA_INLIST_GET(req));
   *to = eina_inlist_append(*to, EINA_INLIST_GET(req));
}

static void
_request_dependents_cancel(Slave_Request *req, Error_Type err)
{
   Slave_Request *dep;

   EINA_LIST_FREE(req->dependents, dep)
     {
        dep->locked = EINA_FALSE;
        dep->dependency = NULL;
        /* Maybe we need a better way to inform the creator of the request
         * that it was cancelled because its dependency failed? */
        cserve2_request_cancel_all(dep, err);
     }
}

void
cserve2_request_cancel(Slave_Request *req, Client *client, Error_Type err)
{
   Eina_List *l, *l_next;
   Waiter *w;

   if (req->funcs && req->funcs->error)
     req->funcs->error(req->data, err);

   EINA_LIST_FOREACH_SAFE(req->waiters, l, l_next, w)
     {
        if (w->client->id == client->id)
          {
             DBG("Removing answer from waiter client: %d, rid: %d",
                 client->id, w->rid);
             cserve2_client_error_send(w->client, w->rid, err);
             req->waiters = eina_list_remove_list(req->waiters, l);
             free(w);
          }
     }

   // TODO: When we have speculative preload, there may be no waiters,
   // so we need a flag or something else to make things still load.
   if ((!req->waiters) && (!req->processing))
     {
        Eina_Inlist **reqlist = &requests[req->type].waiting;
        *reqlist = eina_inlist_remove(*reqlist, EINA_INLIST_GET(req));
        // TODO: If the request is being processed, it can't be deleted. Must
        // be marked as delete_me instead.
        req->funcs->msg_free(req->msg, req->data);

        if (req->dependency)
          req->dependency->dependents = eina_list_remove(
             req->dependency->dependents, req);

        _request_dependents_cancel(req, err);

        free(req);
     }
   else if (!req->waiters)
     req->cancelled = EINA_TRUE;
}

void
cserve2_request_cancel_all(Slave_Request *req, Error_Type err)
{
   Waiter *w;

   DBG("Removing all answers.");

   if (req->funcs && req->funcs->error)
     req->funcs->error(req->data, err);

   EINA_LIST_FREE(req->waiters, w)
     {
        DBG("Removing answer from waiter client: %d, rid: %d",
            w->client->id, w->rid);
        cserve2_client_error_send(w->client, w->rid, err);
        free(w);
     }

   _request_dependents_cancel(req, err);

   if (req->processing)
     {
        req->cancelled = EINA_TRUE;
        return;
     }

   if (req->dependency)
     req->dependency->dependents = eina_list_remove(
        req->dependency->dependents, req);

   requests[req->type].waiting = eina_inlist_remove(
      requests[req->type].waiting, EINA_INLIST_GET(req));
   req->funcs->msg_free(req->msg, req->data);
   free(req);
}

void
cserve2_request_dependents_drop(Slave_Request *req, Slave_Request_Type type)
{
   Slave_Request *dep;
   Eina_List *l, *l_next;

   if (type != CSERVE2_REQ_IMAGE_SPEC_LOAD)
     {
        CRI("Only CSERVE2_REQ_IMAGE_SPEC_LOAD is supported.");
        return;
     }

   EINA_LIST_FOREACH_SAFE(req->dependents, l, l_next, dep)
     {
        if (dep->type == type)
          {
             req->dependents = eina_list_remove_list(req->dependents, l);

             if (dep->processing)
               dep->cancelled = EINA_TRUE;
             else
               {
                  cserve2_entry_request_drop(dep->data, type);
                  requests[type].waiting = eina_inlist_remove(
                           requests[type].waiting, EINA_INLIST_GET(dep));
                  dep->funcs->msg_free(dep->msg, dep->data);
                  free(dep);
               }
          }
     }
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
_cserve2_request_failed(Slave_Request *req, Error_Type type)
{
   Waiter *w;

   req->funcs->error(req->data, type);

   EINA_LIST_FREE(req->waiters, w)
     {
        cserve2_client_error_send(w->client, w->rid, type);
        free(w);
     }

   req->funcs->msg_free(req->msg, req->data);
   requests[req->type].processing = eina_inlist_remove(
      requests[req->type].processing, EINA_INLIST_GET(req));

   _request_dependents_cancel(req, type);

   free(req);
}

static void
_slave_read_cb(Slave *s EINA_UNUSED, Slave_Command cmd, void *msg, void *data)
{
   Slave_Worker *sw = data;
   Slave_Request *dep, *req = sw->data;
   Eina_List **working, **idle;
   Waiter *w;
   Msg_Base *resp = NULL;
   int resp_size;

   if (req->cancelled)
     goto free_it;

   if (cmd == ERROR)
     {
        Error_Type *err = msg;
        WRN("Received error %d from slave, for request type %d.",
            *err, req->type);
        req->funcs->error(req->data, *err);
     }
   else
     {
        DBG("Received response from slave for message type %d.", req->type);
        resp = req->funcs->response(req->data, msg, &resp_size);
     }

   EINA_LIST_FREE(req->waiters, w)
     {
        if (cmd == ERROR)
          {
             Error_Type *err = msg;
             cserve2_client_error_send(w->client, w->rid, *err);
          }
        else
          {
             resp->rid = w->rid;
             cserve2_client_send(w->client, &resp_size, sizeof(resp_size));
             cserve2_client_send(w->client, resp, resp_size);
          }
        free(w);
     }

   free(resp);

free_it:
   req->funcs->msg_free(req->msg, req->data);

   // FIXME: We shouldn't free this message directly, it must be freed by a
   // callback.
   free(msg);
   requests[req->type].processing = eina_inlist_remove(
      requests[req->type].processing, EINA_INLIST_GET(req));

   EINA_LIST_FREE(req->dependents, dep)
     {
        dep->locked = EINA_FALSE;
        dep->dependency = NULL;
     }

   free(req);
   sw->data = NULL;

   working = &_workers[sw->type].working;
   idle = &_workers[sw->type].idle;
   *working = eina_list_remove(*working, sw);
   *idle = eina_list_append(*idle, sw);

   _cserve2_requests_process();
}

static void
_slave_dead_cb(Slave *s EINA_UNUSED, void *data)
{
   Slave_Worker *sw = data;
   Slave_Request *req = sw->data;
   Eina_List **working = &_workers[sw->type].working;
   Eina_List **idle = &_workers[sw->type].idle;

   if (req)
     _cserve2_request_failed(req, CSERVE2_LOADER_DIED);

   *working = eina_list_remove(*working, sw);
   *idle = eina_list_remove(*idle, sw);
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
_cserve2_request_dispatch(Slave_Worker *sw, Slave_Command ctype, Slave_Request *req)
{
   int size;
   char *slave_msg = req->funcs->msg_create(req->data, &size);


   DBG("dispatching message of type %d to slave.", req->type);
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

static void
_cserve2_requests_process(void)
{
    unsigned int rtype, j;

    for (rtype = 0; rtype < CSERVE2_REQ_LAST; rtype++)
      {
         Slave_Type type = SLAVE_NONE;
         Slave_Command ctype;
         unsigned int max_workers;
         Eina_List **idle, **working;
         Eina_Inlist *itr;
         Slave_Request *req;

         for (j = 0; _request_match[j].rtype != CSERVE2_REQ_LAST; j++)
           {
              if (_request_match[j].rtype == rtype)
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

         EINA_INLIST_FOREACH_SAFE(requests[rtype].waiting, itr, req)
           {
              Slave_Worker *sw;

              if (eina_list_count(*working) >=
                  (max_workers - _request_match[j].require_spares))
                break;

              if (req->locked)
                continue;

              requests[rtype].waiting = eina_inlist_remove(
                 requests[rtype].waiting, EINA_INLIST_GET(req));
              requests[rtype].processing = eina_inlist_append(
                 requests[rtype].processing, EINA_INLIST_GET(req));

              if (!(*idle))
                _slave_for_request_create(type);

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

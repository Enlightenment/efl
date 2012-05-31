#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "evas_cserve2.h"

struct _Font_Request {
   Font_Request_Type type;
   void *data;
   Eina_List *waiters;
   Eina_Bool processing;
   Font_Request_Funcs *funcs;
};

struct _Waiter {
   unsigned int rid;
   Client *client;
};

typedef struct _Waiter Waiter;

static Eina_List **requests = NULL;

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
   Eina_List *l;

   req = NULL;
   EINA_LIST_FOREACH(requests[type], l, r)
     {
        if (r->data != data)
          continue;

        req = r;
        break;
     }

   if (!req)
     {
        DBG("Add request for rid: %d", rid);
        req = malloc(sizeof(*req));
        req->data = data;
        req->waiters = NULL;
        req->processing = EINA_FALSE;
        requests[type] = eina_list_append(requests[type], req);
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
               req->funcs->error(client, req->data, err);
             req->waiters = eina_list_remove_list(req->waiters, l);
             free(w);
          }
     }

   // TODO: When we have speculative preload, there may be no waiters,
   // so we need a flag or something else to make things still load.
   if (!req->waiters)
     {
        Eina_List **reqlist = &requests[req->type];
        *reqlist = eina_list_remove(*reqlist, req);
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
          req->funcs->error(w->client, req->data, err);
        free(w);
     }

   requests[req->type] = eina_list_remove(requests[req->type], req);
   free(req);
}

void
cserve2_requests_init(void)
{
   DBG("Initializing requests.");
   requests = calloc(CSERVE2_REQ_LAST, sizeof(Eina_List *));
}

void
cserve2_requests_shutdown(void)
{
   DBG("Shutting down requests.");
   free(requests);
}

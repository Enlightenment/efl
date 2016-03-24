#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

struct _Elm_Url
{
   const char *url;

   struct {
      Elm_Url_Done done;
      Elm_Url_Cancel cancel;
      Elm_Url_Progress progress;
   } cb;

   const void *data;

   Ecore_Con_Url *target;
   Eina_Binbuf *download;

   struct {
      Ecore_Event_Handler *progress;
      Ecore_Event_Handler *done;
      Ecore_Event_Handler *data;
   } handler;
};

static void
_elm_url_free(Elm_Url *r)
{
   ecore_con_url_free(r->target);
   eina_binbuf_free(r->download);
   ecore_event_handler_del(r->handler.progress);
   ecore_event_handler_del(r->handler.done);
   ecore_event_handler_del(r->handler.data);
   eina_stringshare_del(r->url);
   free(r);
}

static Eina_Bool
_elm_url_progress(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Url_Progress *url_progress = event;
   Elm_Url *r = data;

   if (url_progress->url_con != r->target) return EINA_TRUE;

   r->cb.progress((void*) r->data, r, url_progress->down.now, url_progress->down.total);

   return EINA_TRUE;
}

static Eina_Bool
_elm_url_done(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Url_Complete *url_complete = event;
   Elm_Url *r = data;

   if (url_complete->url_con != r->target) return EINA_TRUE;

   if (url_complete->status == 200)
     {
        r->cb.done((void*) r->data, r, r->download);
     }
   else
     {
        r->cb.cancel((void*) r->data, r, url_complete->status);
     }

   _elm_url_free(r);

   return EINA_TRUE;
}

static Eina_Bool
_elm_url_data(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Url_Data *url_data = event;
   Elm_Url *r = data;

   if (url_data->url_con != r->target) return EINA_TRUE;

   eina_binbuf_append_length(r->download, url_data->data, url_data->size);

   return EINA_TRUE;
}

Elm_Url *
_elm_url_download(const char *url, Elm_Url_Done done_cb, Elm_Url_Cancel cancel_cb, Elm_Url_Progress progress_cb, const void *data)
{
   Ecore_Con_Url *target;
   Elm_Url *r;

   ecore_con_url_init();

   target = ecore_con_url_new(url);
   if (!target) goto on_error;

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        if (getenv("http_proxy")) ecore_con_url_proxy_set(target, getenv("http_proxy"));
        if (getenv("https_proxy")) ecore_con_url_proxy_set(target, getenv("https_proxy"));
        if (getenv("ftp_proxy")) ecore_con_url_proxy_set(target, getenv("ftp_proxy"));
     }

   r = malloc(sizeof (Elm_Url));
   if (!r) goto on_error;

   r->url = eina_stringshare_add(url);
   r->cb.done = done_cb;
   r->cb.cancel = cancel_cb;
   r->cb.progress = progress_cb;
   r->data = data;

   r->download = eina_binbuf_new();
   r->target = target;
   r->handler.progress = ecore_event_handler_add(ECORE_CON_EVENT_URL_PROGRESS, _elm_url_progress, r);
   r->handler.done = ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _elm_url_done, r);
   r->handler.data = ecore_event_handler_add(ECORE_CON_EVENT_URL_DATA, _elm_url_data, r);

   if (!ecore_con_url_get(r->target))
     {
       _elm_url_free(r);
       cancel_cb((void*) data, NULL, -1);
       return NULL;
     }

   return r;

 on_error:
   ecore_con_url_shutdown();

   cancel_cb((void*) data, NULL, -1);
   return NULL;
}

void
_elm_url_cancel(Elm_Url *r)
{
   r->cb.cancel((void*) r->data, r, 0);
   _elm_url_free(r);
}

const char *
_elm_url_get(Elm_Url *r)
{
   return r->url;
}

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef _WIN32
# include <evil_private.h> /* setenv */
#endif

#include <Ecore.h>
#include <Ecore_Ipc.h>

#include "efreetd.h"
#include "efreetd_cache.h"

static int init = 0;
static Ecore_Ipc_Server *ipc = NULL;
static Ecore_Event_Handler *hnd_add = NULL;
static Ecore_Event_Handler *hnd_del = NULL;
static Ecore_Event_Handler *hnd_data = NULL;
static int clients = 0;
static Ecore_Timer *quit_timer = NULL;

static Eina_Bool
_cb_quit_timer(void *data EINA_UNUSED)
{
   quit_timer = NULL;
   quit();
   return EINA_FALSE;
}

static void
_broadcast(Ecore_Ipc_Server *svr, int major, int minor, void *data, int size)
{
   Eina_List *ipc_clients = ecore_ipc_server_clients_get(svr);
   Eina_List *l;
   Ecore_Ipc_Client *cl;

   EINA_LIST_FOREACH(ipc_clients, l, cl)
     {
        ecore_ipc_client_send(cl, major, minor, 0, 0, 0, data, size);
     }
}

static char *
_parse_str(void *data, int size)
{
   char *str = malloc(size + 1);
   if (!str) return NULL;
   memcpy(str, data, size);
   str[size] = 0;
   return str;
}

static Eina_List *
_parse_strs(void *data, int size)
{
   Eina_List *list = NULL;
   char *p, *p0 = NULL, *p1 = NULL, *e = (char *)data + size;

   for (p = data; p < e; p++)
     {
        if (!p0)
          {
             if (*p)
               {
                  p0 = p;
                  p1 = e;
               }
          }
        if ((!*p) && (p0))
          {
             p1 = strdup(p0);
             if (p1) list = eina_list_append(list, p1);
             p0 = NULL;
          }
     }
   if (p0)
     {
        p = malloc(p1 - p0 + 1);
        if (p)
          {
             memcpy(p, p0, p1 - p0);
             p[p1 - p0] = 0;
             list = eina_list_append(list, p);
          }
     }
   return list;
}

#define IPC_HEAD(_type) \
   Ecore_Ipc_Event_Client_##_type *e = event; \
   if (ecore_ipc_client_server_get(e->client) != ipc) \
     return ECORE_CALLBACK_PASS_ON

static Eina_Bool
_cb_client_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   IPC_HEAD(Add);
   if (quit_timer)
     {
        ecore_timer_del(quit_timer);
        quit_timer = NULL;
     }
   clients++;
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_cb_client_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   IPC_HEAD(Del);
   clients--;
   if (clients == 0)
     {
        if (quit_timer) ecore_timer_del(quit_timer);
        quit_timer = ecore_timer_add(10.0, _cb_quit_timer, NULL);
     }
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_cb_client_data(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eina_List *strs;
   char *s;
   IPC_HEAD(Data);
   if (e->major == 1) // register lang
     { // input: str -> lang
        if ((s = _parse_str(e->data, e->size)))
          {
             setenv("LANG", s, 1);
             free(s);
          }
        // return if desktop cache exists (bool as minor)
        ecore_ipc_client_send(e->client, 1 /* register reply */,
                              cache_desktop_exists(), 0, 0, 0, NULL, 0);
     }
   else if (e->major == 2) // add desktop dirs
     { // input: array of str -> dirs
        strs = _parse_strs(e->data, e->size);
        EINA_LIST_FREE(strs, s)
          {
             cache_desktop_dir_add(s);
             free(s);
          }
     }
   else if (e->major == 3) // build desktop cache
     { // input: str -> lang
        if ((s = _parse_str(e->data, e->size)))
          {
             setenv("LANG", s, 1);
             free(s);
          }
        cache_desktop_update();
     }
   else if (e->major == 4) // add icon dirs
     { // input: array of str -> dirs
        strs = _parse_strs(e->data, e->size);
        EINA_LIST_FREE(strs, s)
          {
             cache_icon_dir_add(s);
             free(s);
          }
     }
   else if (e->major == 5) // add icon exts
     { // input: array of str -> exts
        strs = _parse_strs(e->data, e->size);
        EINA_LIST_FREE(strs, s)
          {
             cache_icon_ext_add(s);
             free(s);
          }
     }
   return ECORE_CALLBACK_DONE;
}

///////////////////////////////////////////////////////////////////////////

void
send_signal_icon_cache_update(Eina_Bool update)
{
   _broadcast(ipc, 2 /* icon cache update */, update, NULL, 0);
}

void
send_signal_desktop_cache_update(Eina_Bool update)
{
   _broadcast(ipc, 3 /* desktop cache update */, update, NULL, 0);
}

void
send_signal_desktop_cache_build(void)
{
   _broadcast(ipc, 1 /* desktop cache build */, 1, NULL, 0);
}

void
send_signal_mime_cache_build(void)
{
   _broadcast(ipc, 4 /* mime cache build */, 1, NULL, 0);
}

Eina_Bool
ipc_init(void)
{
   if (init > 0) return EINA_TRUE;
   if (!ecore_ipc_init()) return EINA_FALSE;
   ipc = ecore_ipc_server_add(ECORE_IPC_LOCAL_USER, "efreetd", 0, NULL);
   if (!ipc)
     {
        ecore_ipc_shutdown();
        return EINA_FALSE;
     }
   quit_timer = ecore_timer_add(10.0, _cb_quit_timer, NULL);
   hnd_add = ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_ADD,
                                     _cb_client_add, NULL);
   hnd_del = ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DEL,
                                     _cb_client_del, NULL);
   hnd_data = ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DATA,
                                      _cb_client_data, NULL);
   init++;
   return EINA_TRUE;
}

Eina_Bool
ipc_shutdown(void)
{
   if (init <= 0) return EINA_TRUE;
   init--;
   if (init > 0) return EINA_TRUE;
   ecore_ipc_server_del(ipc);
   ecore_event_handler_del(hnd_add);
   ecore_event_handler_del(hnd_del);
   ecore_event_handler_del(hnd_data);
   ipc = NULL;
   hnd_add = NULL;
   hnd_del = NULL;
   hnd_data = NULL;
   ecore_ipc_shutdown();
   return EINA_TRUE;
}


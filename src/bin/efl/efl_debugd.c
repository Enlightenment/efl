#include "efl_debug_common.h"

typedef struct _Client Client;

struct _Client
{
   Ecore_Con_Client *client;
   unsigned char    *buf;
   unsigned int      buf_size;

   Ecore_Timer      *evlog_fetch_timer;
   int               evlog_on;
   FILE             *evlog_file;
   int               evlog_inset;

   int               version;
   pid_t             pid;
};

static Ecore_Con_Server *svr = NULL;
static Eina_List *clients = NULL;

static Client *
_client_pid_find(int pid)
{
   Client *c;
   Eina_List *l;

   if (pid <= 0) return NULL;
   EINA_LIST_FOREACH(clients, l, c)
     {
        if (c->pid == pid) return c;
     }
   return NULL;
}

static Eina_Bool
_cb_evlog(void *data)
{
   Client *c = data;
   send_cli(c->client, "EVLG", NULL, 0);
   return EINA_TRUE;
}

static void
_do(Client *c, char *op, unsigned char *d, int size)
{
   Client *c2;
   Eina_List *l;

   if ((!strcmp(op, "HELO")) && (size >= 8))
     {
        int version;
        int pid;

        fetch_val(version, d, 0);
        fetch_val(pid, d, 4);
        c->version = version;
        c->pid = pid;
     }
   else if (!strcmp(op, "LIST"))
     {
        int n = eina_list_count(clients);
        unsigned int *pids = malloc(n * sizeof(int));
        if (pids)
          {
             int i = 0;

             EINA_LIST_FOREACH(clients, l, c2)
               {
                  pids[i] = c2->pid;
                  i++;
               }
             send_cli(c->client, "CLST", pids, n * sizeof(int));
             free(pids);
          }
     }
   else if ((!strcmp(op, "PLON")) && (size >= 8))
     {
        int pid;
        unsigned int freq = 1000;
        fetch_val(pid, d, 0);
        fetch_val(freq, d, 4);
        if ((c2 = _client_pid_find(pid)))
          {
             unsigned char buf[4];
             store_val(buf, 0, freq);
             send_cli(c2->client, "PLON", buf, sizeof(buf));
          }
     }
   else if (!strcmp(op, "PLOF"))
     {
        int pid;
        fetch_val(pid, d, 0);
        if ((c2 = _client_pid_find(pid)))
          {
             send_cli(c2->client, "PLOF", NULL, 0);
          }
     }
   else if (!strcmp(op, "EVON"))
     {
        int pid;
        fetch_val(pid, d, 0);
        if ((c2 = _client_pid_find(pid)))
          {
             c2->evlog_on++;
             if (c2->evlog_on == 1)
               {
                  char buf[4096];

                  send_cli(c2->client, "EVON", NULL, 0);
                  c2->evlog_fetch_timer = ecore_timer_add(0.2, _cb_evlog, c2);
                  snprintf(buf, sizeof(buf), "%s/efl_debug_evlog-%i.txt",
                           getenv("HOME"), c->pid);
                  c2->evlog_file = fopen(buf, "w");
                  c->evlog_inset = 0;
               }
          }
     }
   else if (!strcmp(op, "EVOF"))
     {
        int pid;
        fetch_val(pid, d, 0);
        if ((c2 = _client_pid_find(pid)))
          {
             c2->evlog_on--;
             if (c2->evlog_on == 0)
               {
                  send_cli(c2->client, "EVOF", NULL, 0);
                  if (c2->evlog_fetch_timer)
                    {
                       ecore_timer_del(c2->evlog_fetch_timer);
                       c2->evlog_fetch_timer = NULL;
                    }
                  if (c2->evlog_file)
                    {
                       fclose(c2->evlog_file);
                       c2->evlog_file = NULL;
                    }
               }
             else if (c2->evlog_on < 0)
               c2->evlog_on = 0;
          }
     }
   else if (!strcmp(op, "EVLG"))
     {
//        unsigned int *overflow = (unsigned int *)(d + 0);
        unsigned char *end = d + size;
        unsigned char *p = d + 8;
        char *event_str, *detail_str;
        Eina_Evlog_Item *item;
        int i, inset;

        printf("EVLG!!!! %i\n", size);
        inset = c->evlog_inset;
        if (c->evlog_file)
          {
             printf("  have out file\n");
             while (p < end)
               {
                  item = (Eina_Evlog_Item *)p;
                  printf("    have item %p\n", p);
                  if ((item->event_next > item->detail_offset) &&
                      ((p + item->event_next) < end))
                    {
                       detail_str = "";
                       event_str = (char *)(p + item->event_offset);
                       if (event_str[0] == '+') inset++;
                       if (item->detail_offset)
                         detail_str = (char *)(p + item->detail_offset);
                       for (i = 0; i < inset; i++) fprintf(c->evlog_file, " ");
                       fprintf(c->evlog_file,
                               "%1.10f [%s] %1.10f 0x%llx 0x%llx %s\n",
                               item->tim, event_str, item->srctim,
                               item->thread, item->obj,
                               detail_str);
                       if (event_str[0] == '-') inset--;
                    }
                  p += item->event_next;
               }
          }
        c->evlog_inset = inset;
     }
}

static Eina_Bool
_client_add(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Client_Add *ev)
{
   Client *c = calloc(1, sizeof(Client));
   if (c)
     {
        c->client = ev->client;
        clients = eina_list_append(clients, c);
        ecore_con_client_data_set(c->client, c);
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_client_del(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Client_Del *ev)
{
   Client *c = ecore_con_client_data_get(ev->client);
   if (c)
     {
        clients = eina_list_remove(clients, c);
        if (c->evlog_fetch_timer)
          {
             ecore_timer_del(c->evlog_fetch_timer);
             c->evlog_fetch_timer = NULL;
          }
        if (c->evlog_file)
          {
             fclose(c->evlog_file);
             c->evlog_file = NULL;
          }
        free(c);
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_client_data(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Client_Data *ev)
{
   Client *c = ecore_con_client_data_get(ev->client);
   if (c)
     {
        char op[5];
        unsigned char *d = NULL;
        int size;

        _protocol_collect(&(c->buf), &(c->buf_size), ev->data, ev->size);
        while ((size = _proto_read(&(c->buf), &(c->buf_size), op, &d)) >= 0)
          {
             _do(c, op, d, size);
             free(d);
             d = NULL;
          }
     }
   return ECORE_CALLBACK_RENEW;
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   eina_init();
   ecore_init();
   ecore_con_init();

   svr = ecore_con_server_add(ECORE_CON_LOCAL_USER, "efl_debug", 0, NULL);
   if (!svr)
     {
        fprintf(stderr, "ERROR: Cannot create debug daemon.\n");
        return -1;
     }

   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, (Ecore_Event_Handler_Cb)_client_add, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, (Ecore_Event_Handler_Cb)_client_del, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, (Ecore_Event_Handler_Cb)_client_data, NULL);

   ecore_main_loop_begin();

   ecore_con_server_del(svr);

   ecore_con_shutdown();
   ecore_shutdown();
   eina_shutdown();
}

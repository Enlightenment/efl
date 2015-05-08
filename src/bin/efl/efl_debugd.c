#include "efl_debug_common.h"

typedef struct _Client Client;

struct _Client
{
   Ecore_Con_Client *client;
   int version;
   pid_t pid;
   unsigned char *buf;
   unsigned int   buf_size;
};

static Ecore_Con_Server *svr = NULL;
static Eina_List *clients = NULL;

static void
_proto(Client *c, unsigned char *d, unsigned int size)
{
   if (size >= 4)
     {
        char *cmd = (char *)d;

        if (!strncmp(cmd, "HELO", 4))
          {
             int version;
             int pid;

             memcpy(&version, d + 4, 4);
             memcpy(&pid, d + 8, 4);
             c->version = version;
             c->pid = pid;
          }
        else if (!strncmp(cmd, "LIST", 4))
          {
             int n = eina_list_count(clients), i;
             unsigned int *pids, size2;
             Client *c2;
             Eina_List *l;
             char *head = "CLST";

             pids = malloc(n * sizeof(int));
             i = 0;
             size2 = 4 + (n * sizeof(int));
             EINA_LIST_FOREACH(clients, l, c2)
               {
                  pids[i] = c2->pid;
                  i++;
               }
             ecore_con_client_send(c->client, &size2, 4);
             ecore_con_client_send(c->client, head, 4);
             ecore_con_client_send(c->client, pids, n * sizeof(int));
             free(pids);
          }
        else if (!strncmp(cmd, "PLON", 4))
          {
             int pid;
             unsigned int freq = 1000;
             Client *c2;
             Eina_List *l;

             memcpy(&pid, d + 4, 4);
             memcpy(&freq, d + 8, 4);
             if (pid > 0)
               {
                  EINA_LIST_FOREACH(clients, l, c2)
                    {
                       if (c2->pid == pid)
                         {
                            unsigned int size2 = 8;

                            ecore_con_client_send(c2->client, &size2, 4);
                            ecore_con_client_send(c2->client, d, 4);
                            ecore_con_client_send(c2->client, &freq, 4);
                            break;
                         }
                    }
               }
          }
        else if (!strncmp(cmd, "PLOF", 4))
          {
             int pid;
             Client *c2;
             Eina_List *l;

             memcpy(&pid, d + 4, 4);
             if (pid > 0)
               {
                  EINA_LIST_FOREACH(clients, l, c2)
                    {
                       if (c2->pid == pid)
                         {
                            unsigned int size2 = 4;

                            ecore_con_client_send(c2->client, &size2, 4);
                            ecore_con_client_send(c2->client, d, 4);
                            break;
                         }
                    }
               }
          }
     }
}

static Eina_Bool
_client_proto(Client *c)
{
   unsigned int size, newsize;
   unsigned char *b;
   if (!c->buf) return EINA_FALSE;
   if (c->buf_size < 4) return EINA_FALSE;
   memcpy(&size, c->buf, 4);
   if (c->buf_size < (size + 4)) return EINA_FALSE;
   _proto(c, c->buf + 4, size);
   newsize = c->buf_size - (size + 4);
   if (c->buf_size == newsize)
     {
        free(c->buf);
        c->buf = NULL;
        c->buf_size = 0;
     }
   else
     {
        b = malloc(newsize);
        memcpy(b, c->buf + size + 4, newsize);
        free(c->buf);
        c->buf = b;
        c->buf_size = newsize;
     }
   return EINA_TRUE;
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
        if (!c->buf)
          {
             c->buf = malloc(ev->size);
             if (c->buf)
               {
                  c->buf_size = ev->size;
                  memcpy(c->buf, ev->data, ev->size);
               }
          }
        else
          {
             unsigned char *b = realloc(c->buf, c->buf_size + ev->size);
             if (b)
               {
                  c->buf = b;
                  memcpy(c->buf + c->buf_size, ev->data, ev->size);
                  c->buf_size += ev->size;
               }
          }
        while (_client_proto(c));
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

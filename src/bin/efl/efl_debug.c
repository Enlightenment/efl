#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static unsigned char *buf;
static unsigned int   buf_size;

static int my_argc;
static char **my_argv;
static const char *expect = NULL;

static Ecore_Con_Server *svr;

static void
_proto(unsigned char *d, unsigned int size)
{
   if (size >= 4)
     {
        char *cmd = (char *)d;

        if (!strncmp(cmd, "CLST", 4))
          {
             int i, n;

             n = (size - 4) / sizeof(int);
             if (n < 10000)
               {
                  int *pids = malloc(n * sizeof(int));
                  memcpy(pids, d + 4, n * sizeof(int));
                  for (i = 0; i < n; i++)
                    {
                       if (pids[i] > 0) printf("%i\n", pids[i]);
                    }
                  free(pids);
               }
          }
        if ((expect) && (!strncmp(cmd, expect, 4)))
          ecore_main_loop_quit();
     }
}


static Eina_Bool
_server_proto(void)
{
   unsigned int size, newsize;
   unsigned char *b;
   if (!buf) return EINA_FALSE;
   if (buf_size < 4) return EINA_FALSE;
   memcpy(&size, buf, 4);
   if (buf_size < (size + 4)) return EINA_FALSE;
   _proto(buf + 4, size);
   newsize = buf_size - (size + 4);
   if (buf_size == newsize)
     {
        free(buf);
        buf = NULL;
        buf_size = 0;
     }
   else
     {
        b = malloc(newsize);
        memcpy(b, buf + size + 4, newsize);
        free(buf);
        buf = b;
        buf_size = newsize;
     }
   return EINA_TRUE;
}

Eina_Bool
_server_add(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Server_Add *ev)
{
   int i;
   for (i = 1; i < my_argc; i++)
     {
        if (!strcmp(my_argv[i], "list"))
          {
             unsigned int size = 4;
             char *head = "LIST";
             expect = "CLST";
             ecore_con_server_send(svr, &size, 4);
             ecore_con_server_send(svr, head, 4);
          }
        else if ((!strcmp(my_argv[i], "pon")) &&
                 (i < (my_argc - 2)))
          {
             unsigned int size = 12;
             char *head = "PLON";
             int pid = atoi(my_argv[i + 1]);
             unsigned int freq = atoi(my_argv[i + 2]);
             i++;
             ecore_con_server_send(svr, &size, 4);
             ecore_con_server_send(svr, head, 4);
             ecore_con_server_send(svr, &pid, 4);
             ecore_con_server_send(svr, &freq, 4);
             ecore_main_loop_quit();
          }
        else if ((!strcmp(my_argv[i], "poff")) &&
                 (i < (my_argc - 1)))
          {
             unsigned int size = 8;
             char *head = "PLOFF";
             int pid = atoi(my_argv[i + 1]);
             i++;
             ecore_con_server_send(svr, &size, 4);
             ecore_con_server_send(svr, head, 4);
             ecore_con_server_send(svr, &pid, 4);
             ecore_main_loop_quit();
          }
     }
   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_server_del(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Server_Del *ev)
{
   ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_server_data(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Server_Data *ev)
{
   if (!buf)
     {
        buf = malloc(ev->size);
        if (buf)
          {
             buf_size = ev->size;
             memcpy(buf, ev->data, ev->size);
          }
     }
   else
     {
        unsigned char *b = realloc(buf, buf_size + ev->size);
        if (b)
          {
             buf = b;
             memcpy(buf + buf_size, ev->data, ev->size);
             buf_size += ev->size;
          }
     }
   while (_server_proto());
   return ECORE_CALLBACK_RENEW;
}

int
main(int argc, char **argv)
{
   eina_init();
   ecore_init();
   ecore_con_init();

   my_argc = argc;
   my_argv = argv;

   svr = ecore_con_server_connect(ECORE_CON_LOCAL_USER, "efl_debug", 0, NULL);
   if (!svr)
     {
        fprintf(stderr, "ERROR: Cannot connetc to debug daemon.\n");
        return -1;
     }

   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD, (Ecore_Event_Handler_Cb)_server_add, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL, (Ecore_Event_Handler_Cb)_server_del, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA, (Ecore_Event_Handler_Cb)_server_data, NULL);

   ecore_main_loop_begin();
   ecore_con_server_flush(svr);

   ecore_con_shutdown();
   ecore_shutdown();
   eina_shutdown();
}

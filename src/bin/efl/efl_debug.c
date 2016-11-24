/* EINA - EFL data type library
 * Copyright (C) 2015 Carsten Haitzler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#include "efl_debug_common.h"

static unsigned char *buf;
static unsigned int   buf_size;

static int my_argc;
static char **my_argv;
static const char *expect = NULL;

static Ecore_Con_Server *svr;

static void
_do(char *op, unsigned char *d, int size)
{
   if (!strcmp(op, "CLST"))
     {
        int i, n;

        n = (size) / sizeof(int);
        if (n < 10000)
          {
             int *pids = malloc(n * sizeof(int));
             if (pids)
               {
                  int mypid = getpid();
                  memcpy(pids, d, n * sizeof(int));
                  for (i = 0; i < n; i++)
                    {
                       if (pids[i] == mypid) continue;
                       if (pids[i] > 0) printf("%i\n", pids[i]);
                    }
                  free(pids);
               }
          }
     }
   if ((expect) && (!strcmp(op, expect))) ecore_main_loop_quit();
}

Eina_Bool
_server_add(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Server_Add *ev EINA_UNUSED)
{
   int i;
   for (i = 1; i < my_argc; i++)
     {
        if (!strcmp(my_argv[i], "list"))
          {
             send_svr(svr, "LIST", NULL, 0);
             expect = "CLST";
          }
        else if ((!strcmp(my_argv[i], "pon")) &&
                 (i < (my_argc - 2)))
          {
             unsigned char tmp[8];
             int pid = atoi(my_argv[i + 1]);
             unsigned int freq = atoi(my_argv[i + 2]);
             i += 2;
             store_val(tmp, 0, pid);
             store_val(tmp, 4, freq);
             send_svr(svr, "PLON", tmp, sizeof(tmp));
             ecore_main_loop_quit();
          }
        else if ((!strcmp(my_argv[i], "poff")) &&
                 (i < (my_argc - 1)))
          {
             unsigned char tmp[4];
             int pid = atoi(my_argv[i + 1]);
             i++;
             store_val(tmp, 0, pid);
             send_svr(svr, "PLOF", tmp, sizeof(tmp));
             ecore_main_loop_quit();
          }
        else if ((!strcmp(my_argv[i], "evlogon")) &&
                 (i < (my_argc - 1)))
          {
             unsigned char tmp[4];
             int pid = atoi(my_argv[i + 1]);
             i++;
             store_val(tmp, 0, pid);
             send_svr(svr, "EVON", tmp, sizeof(tmp));
             ecore_main_loop_quit();
          }
        else if ((!strcmp(my_argv[i], "evlogoff")) &&
                 (i < (my_argc - 1)))
          {
             unsigned char tmp[4];
             int pid = atoi(my_argv[i + 1]);
             i++;
             store_val(tmp, 0, pid);
             send_svr(svr, "EVOF", tmp, sizeof(tmp));
             ecore_main_loop_quit();
          }
     }
   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_server_del(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Server_Del *ev EINA_UNUSED)
{
   ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_server_data(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Server_Data *ev)
{
   char op[5];
   unsigned char *d = NULL;
   int size;

   _protocol_collect(&(buf), &(buf_size), ev->data, ev->size);
   while ((size = _proto_read(&(buf), &(buf_size), op, &d)) >= 0)
     {
        _do(op, d, size);
        free(d);
        d = NULL;
     }
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
        fprintf(stderr, "ERROR: Cannot connect to debug daemon.\n");
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

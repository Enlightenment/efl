#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_suite.h"

#include <stdio.h>
#include <Ecore.h>
#include <Ecore_Con.h>

Eina_Bool
_add(void *data, int type EINA_UNUSED, void *ev)
{
   fail_if (type != ECORE_CON_EVENT_CLIENT_ADD &&
       type != ECORE_CON_EVENT_SERVER_ADD);

   /* Server */
   if (type == ECORE_CON_EVENT_CLIENT_ADD)
     {
       Ecore_Con_Event_Client_Add *event = ev;

       fail_if (data != (void *) 1);
       fail_if (!event->client);

       printf("Client with ip %s, port %d, connected = %d!\n",
           ecore_con_client_ip_get(event->client),
           ecore_con_client_port_get(event->client),
           ecore_con_client_connected_get(event->client));

       ecore_con_client_timeout_set(event->client, 10);

     }
   else if (type == ECORE_CON_EVENT_SERVER_ADD)
     {
       Ecore_Con_Event_Server_Add *event = ev;
       const char ping[] = "PING";
       int ret;

       fail_if (data != (void *) 2);
       fail_if (!event->server);
       printf("Server with ip %s, name %s, port %d, connected = %d!\n",
           ecore_con_server_ip_get(event->server),
           ecore_con_server_name_get(event->server),
           ecore_con_server_port_get(event->server),
           ecore_con_server_connected_get(event->server));
       ret = ecore_con_server_send(event->server, ping, sizeof(ping));
       fail_if (ret != sizeof(ping));
       ecore_con_server_flush(event->server);
     }

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_del(void *data , int type EINA_UNUSED, void *ev)
{
   fail_if (type != ECORE_CON_EVENT_CLIENT_DEL &&
       type != ECORE_CON_EVENT_SERVER_DEL);

   /* Server */
   if (type == ECORE_CON_EVENT_CLIENT_DEL)
     {
       Ecore_Con_Event_Client_Del *event = ev;

       fail_if (data != (void *) 1);
       fail_if (!event->client);

       printf("Lost client with ip %s!\n", ecore_con_client_ip_get(event->client));
       printf("Client was connected for %0.3f seconds.\n",
           ecore_con_client_uptime_get(event->client));

       ecore_con_client_del(event->client);
     }
   else if (type == ECORE_CON_EVENT_SERVER_DEL)
     {
       Ecore_Con_Event_Server_Del *event = ev;

       fail_if (!event->server);

       fail_if (data != (void *) 2);

       printf("Lost server with ip %s!\n", ecore_con_server_ip_get(event->server));

       ecore_con_server_del(event->server);
     }
   fail ();

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_data(void *data, int type EINA_UNUSED, void *ev)
{

   fail_if (type != ECORE_CON_EVENT_CLIENT_DATA &&
       type != ECORE_CON_EVENT_SERVER_DATA);

   /* Server */
   if (type == ECORE_CON_EVENT_CLIENT_DATA)
     {
       Ecore_Con_Event_Client_Data *event = ev;
       const char pong[] = "PONG";
       int ret;

       char fmt[128];
       fail_if (data != (void *) 1);

       snprintf(fmt, sizeof(fmt),
           "Received %i bytes from client %s port %d:\n"
           ">>>>>\n"
           "%%.%is\n"
           ">>>>>\n",
           event->size, ecore_con_client_ip_get(event->client),
           ecore_con_client_port_get(event->client), event->size);

       printf(fmt, event->data);
       fail_if (event->size != sizeof("PING"));
       fail_if (memcmp (event->data, "PING", sizeof("PING")) != 0);

       ret = ecore_con_client_send(event->client, pong, sizeof(pong));
       fail_if (ret != sizeof(pong));
       ecore_con_client_flush(event->client);
     }
   else if (type == ECORE_CON_EVENT_SERVER_DATA)
     {
       Ecore_Con_Event_Server_Data *event = ev;
       char fmt[128];

       fail_if (data != (void *) 2);

       snprintf(fmt, sizeof(fmt),
           "Received %i bytes from server:\n"
           ">>>>>\n"
           "%%.%is\n"
           ">>>>>\n",
           event->size, event->size);

       printf(fmt, event->data);
       fail_if (event->size != sizeof("PONG"));
       fail_if (memcmp (event->data, "PONG", sizeof("PONG")) != 0);
       ecore_main_loop_quit();
     }

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_dns_add_del(void *data, int type EINA_UNUSED, void *ev EINA_UNUSED)
{
   Eina_Bool *err_check = data;
   *err_check = EINA_FALSE;
   ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_dns_err(void *data, int type EINA_UNUSED, void *ev EINA_UNUSED)
{
   Eina_Bool *err_check = data;
   *err_check = EINA_TRUE;
   ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

START_TEST(ecore_test_ecore_con_server)
{
   Ecore_Con_Server *server;
   Ecore_Con_Server *client;
   Ecore_Con_Client *cl;
   const Eina_List *clients, *l;
   Ecore_Event_Handler *handlers[6];
   void *server_data = malloc (1);
   void *client_data = malloc (1);
   int ret;
   void *del_ret;

   ret = eina_init();
   fail_if(ret != 1);
   ret = ecore_init();
   fail_if(ret < 1);
   ret = ecore_con_init();
   fail_if(ret != 1);

   handlers[0] = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,
       _add, (void *) 1);
   fail_if(handlers[0] == NULL);
   handlers[1] = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,
       _del, (void *) 1);
   fail_if(handlers[1] == NULL);
   handlers[2] = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA,
       _data, (void *) 1);
   fail_if(handlers[2] == NULL);

   handlers[3] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,
       _add, (void *) 2);
   fail_if(handlers[3] == NULL);
   handlers[4] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL,
       _del, (void *) 2);
   fail_if(handlers[4] == NULL);
   handlers[5] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA,
       _data, (void *) 2);
   fail_if(handlers[5] == NULL);

   server = ecore_con_server_add(ECORE_CON_REMOTE_TCP, "127.0.0.1", 1234,
       server_data);
   fail_if (server == NULL);

   ecore_con_server_timeout_set(server, 10);
   ecore_con_server_client_limit_set(server, 1, 0);

   client = ecore_con_server_connect(ECORE_CON_REMOTE_TCP, "127.0.0.1", 1234,
       client_data);
   fail_if (client == NULL);

   ecore_main_loop_begin();

   clients = ecore_con_server_clients_get(server);
   printf("Clients connected to this server when exiting: %d\n",
          eina_list_count(clients));
   EINA_LIST_FOREACH(clients, l, cl)
     {
       printf("%s\n", ecore_con_client_ip_get(cl));
     }

   printf("Server was up for %0.3f seconds\n",
          ecore_con_server_uptime_get(server));

   del_ret = ecore_con_server_del(server);
   fail_if (del_ret != server_data);
   free (server_data);
   del_ret = ecore_con_server_del(client);
   fail_if (del_ret != client_data);
   free (client_data);

   del_ret = ecore_event_handler_del (handlers[0]);
   fail_if (del_ret != (void *) 1);
   del_ret = ecore_event_handler_del (handlers[1]);
   fail_if (del_ret != (void *) 1);
   del_ret = ecore_event_handler_del (handlers[2]);
   fail_if (del_ret != (void *) 1);

   del_ret = ecore_event_handler_del (handlers[3]);
   fail_if (del_ret != (void *) 2);
   del_ret = ecore_event_handler_del (handlers[4]);
   fail_if (del_ret != (void *) 2);
   del_ret = ecore_event_handler_del (handlers[5]);
   fail_if (del_ret != (void *) 2);

   ret = ecore_con_shutdown();
   fail_if(ret != 0);
   ret = ecore_shutdown();
   ret = eina_shutdown();
}
END_TEST

START_TEST(ecore_test_ecore_con_init)
{
   int ret;

   ret = ecore_con_init();
   fail_if(ret != 1);

   ret = ecore_con_shutdown();
   fail_if(ret != 0);
}
END_TEST

START_TEST(ecore_test_ecore_con_dns)
{
   Ecore_Con_Server *client;
   Ecore_Event_Handler *e_err;
   Ecore_Event_Handler *e_add;
   Eina_Bool err_check = EINA_FALSE;
   int ret;

   ret = eina_init();
   fail_if(ret != 1);
   ret = ecore_init();
   fail_if(ret < 1);
   ret = ecore_con_init();
   fail_if(ret != 1);

   e_add = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD, _dns_add_del, (void *) &err_check);
   e_err = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ERROR, _dns_err, (void *) &err_check);
   /* For timeout */
   e_err = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL, _dns_add_del, (void *) &err_check);

   client = ecore_con_server_connect(ECORE_CON_REMOTE_TCP,
                                     "wongsub.wrongdns.lan", 1234, NULL);
   fail_if (client == NULL);
   ecore_con_server_timeout_set(client, 5.0);

   ecore_main_loop_begin();
   fail_if (err_check == EINA_FALSE);
   fail_if (ecore_event_handler_del(e_err) != (void *) &err_check);
   fail_if (ecore_event_handler_del(e_add) != (void *) &err_check);

   ret = ecore_con_shutdown();
   fail_if(ret != 0);
   ret = ecore_shutdown();
   ret = eina_shutdown();
}
END_TEST

void ecore_test_ecore_con(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_con_init);
   tcase_add_test(tc, ecore_test_ecore_con_server);
   tcase_add_test(tc, ecore_test_ecore_con_dns);
}

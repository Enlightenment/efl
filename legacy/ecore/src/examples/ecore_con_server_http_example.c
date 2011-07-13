#include <stdio.h>
#include <sys/time.h>
#include <Ecore.h>
#include <Ecore_Con.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define __UNUSED__
#endif

static const char response_template[] =
"HTTP/1.0 200 OK\r\n"
"Server: Ecore_Con custom server\r\n"
"Content-Length: %zd\r\n"
"Content-Type: text/html; charset=UTF-8\r\n"
"Set-Cookie: MYCOOKIE=1; path=/; expires=%s"
"Set-Cookie: SESSIONCOOKIE=1; path=/\r\n"
"\r\n"
"%s"
"\r\n\r\n";

struct _Client {
    int sdata;
};

Eina_Bool
_add(void *data __UNUSED__, int type __UNUSED__, Ecore_Con_Event_Client_Add *ev)
{
   struct _Client *client = malloc(sizeof(*client));
   client->sdata = 0;
   static char buf[4096];
   char welcome[] = "Welcome to Ecore_Con server!";
   int nbytes;
   time_t t;

   printf("Client with ip %s, port %d, connected = %d!\n",
	  ecore_con_client_ip_get(ev->client),
	  ecore_con_client_port_get(ev->client),
	  ecore_con_client_connected_get(ev->client));

   ecore_con_client_data_set(ev->client, client);

   t = time(NULL);
   t += 60 * 60 * 24;
   nbytes = snprintf(buf, sizeof(buf), response_template, sizeof(welcome), ctime(&t), welcome);

   ecore_con_client_send(ev->client, buf, nbytes);
   ecore_con_client_flush(ev->client);


   return ECORE_CALLBACK_RENEW;
}


Eina_Bool
_del(void *data __UNUSED__, int type __UNUSED__, Ecore_Con_Event_Client_Del *ev)
{
   struct _Client *client;

   if (!ev->client)
     return ECORE_CALLBACK_RENEW;

   client = ecore_con_client_data_get(ev->client);

   printf("Lost client with ip %s!\n", ecore_con_client_ip_get(ev->client));
   printf("Total data received from this client: %d\n", client->sdata);
   printf("Client was connected for %0.3f seconds.\n",
	  ecore_con_client_uptime_get(ev->client));

   if (client)
     free(client);

   ecore_con_client_del(ev->client);

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_data(void *data __UNUSED__, int type __UNUSED__, Ecore_Con_Event_Client_Data *ev)
{
   char fmt[128];
   struct _Client *client = ecore_con_client_data_get(ev->client);

   snprintf(fmt, sizeof(fmt),
            "\nReceived %i bytes from client %s port %d:\n"
            ">>>>>\n"
            "%%.%is\n"
            ">>>>>\n\n",
            ev->size, ecore_con_client_ip_get(ev->client),
	    ecore_con_client_port_get(ev->client), ev->size);

   printf(fmt, ev->data);

   client->sdata += ev->size;

   return ECORE_CALLBACK_RENEW;
}

int main(void)
{
   Ecore_Con_Server *svr;
   Ecore_Con_Client *cl;
   const Eina_List *clients, *l;

   eina_init();
   ecore_init();
   ecore_con_init();

   if (!(svr = ecore_con_server_add(ECORE_CON_REMOTE_TCP, "127.0.0.1", 8080, NULL)))
     exit(1);

   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, (Ecore_Event_Handler_Cb)_add, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, (Ecore_Event_Handler_Cb)_del, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, (Ecore_Event_Handler_Cb)_data, NULL);
   ecore_con_server_client_limit_set(svr, 3, 0);

   ecore_main_loop_begin();

   clients = ecore_con_server_clients_get(svr);
   printf("Clients still connected to this server when exiting: %d\n",
          eina_list_count(clients));
   EINA_LIST_FOREACH(clients, l, cl)
     {
	printf("%s\n", ecore_con_client_ip_get(cl));
	free(ecore_con_client_data_get(cl));
     }

   printf("Server was up for %0.3f seconds\n",
          ecore_con_server_uptime_get(svr));

   ecore_con_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return 0;
}

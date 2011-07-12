#include <stdio.h>
#include <Ecore.h>
#include <Ecore_Con.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define __UNUSED__
#endif

struct _Server {
    int sdata;
};

Eina_Bool
_add(void *data __UNUSED__, int type __UNUSED__, Ecore_Con_Event_Server_Add *ev)
{
   char welcome[] = "hello! - sent from the client";
   struct _Server *server = malloc(sizeof(*server));
   server->sdata = 0;

   ecore_con_server_data_set(ev->server, server);
   printf("Server with ip %s, name %s, port %d, connected = %d!\n",
	  ecore_con_server_ip_get(ev->server),
	  ecore_con_server_name_get(ev->server),
	  ecore_con_server_port_get(ev->server),
	  ecore_con_server_connected_get(ev->server));
   ecore_con_server_send(ev->server, welcome, sizeof(welcome));
   ecore_con_server_flush(ev->server);

   return ECORE_CALLBACK_RENEW;
}


Eina_Bool
_del(void *data __UNUSED__, int type __UNUSED__, Ecore_Con_Event_Server_Del *ev)
{
   if (!ev->server)
     {
	printf("Failed to establish connection to the server.\nExiting.\n");
	ecore_main_loop_quit();
	return ECORE_CALLBACK_RENEW;
     }

   struct _Server *server = ecore_con_server_data_get(ev->server);

   printf("Lost server with ip %s!\n", ecore_con_server_ip_get(ev->server));

   if (server)
     {
	printf("Total data received from this server: %d\n", server->sdata);
	free(server);
     }

   ecore_con_server_del(ev->server);

   ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_data(void *data __UNUSED__, int type __UNUSED__, Ecore_Con_Event_Server_Data *ev)
{
   char fmt[128];
   struct _Server *server = ecore_con_server_data_get(ev->server);

   snprintf(fmt, sizeof(fmt),
            "Received %i bytes from server:\n"
            ">>>>>\n"
            "%%.%is\n"
            ">>>>>\n",
            ev->size, ev->size);

   printf(fmt, ev->data);

   server->sdata += ev->size;
   return ECORE_CALLBACK_RENEW;
}

int main(int argc, const char *argv[])
{
   Ecore_Con_Server *svr;
   const char *address;
   int port = 8080;

   if (argc < 2)
     {
	printf("wrong usage. Command syntax is:\n");
	printf("\tecore_con_client_simple_example <address> [port]\n");
	exit (1);
     }

   address = argv[1];

   if (argc > 2)
     port = atoi(argv[2]);

   eina_init();
   ecore_init();
   ecore_con_init();

   if (!(svr = ecore_con_server_connect(ECORE_CON_REMOTE_TCP, address, port, NULL)))
     {
	printf("could not connect to the server: %s, port %d.\n",
	       address, port);
	exit(2);
     }

   /* set event handler for server connect */
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD, (Ecore_Event_Handler_Cb)_add, NULL);
   /* set event handler for server disconnect */
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL, (Ecore_Event_Handler_Cb)_del, NULL);
   /* set event handler for receiving server data */
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA, (Ecore_Event_Handler_Cb)_data, NULL);

   /* start client */
   ecore_main_loop_begin();

   ecore_con_init();
   ecore_init();
   eina_init();

   return 0;
}

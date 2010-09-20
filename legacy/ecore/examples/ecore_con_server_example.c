#include <stdio.h>
#include <Ecore.h>
#include <Ecore_Con.h>

/* Ecore_Con server example
 * 2010 Mike Blumenkrantz
 */

/* uncomment if using gnutls
static void
tls_log_func(int level, const char *str)
{
   fprintf(stderr, "|<%d>| %s", level, str);
}
*/

Eina_Bool
_add(void *data, int type, Ecore_Con_Event_Client_Add *ev)
{
   printf("Client with ip %s connected!\n", ecore_con_client_ip_get(ev->client));
   ecore_con_client_send(ev->client, "hello!", 6);
   ecore_con_client_flush(ev->client);
   ecore_con_client_del(ev->client);

   return ECORE_CALLBACK_RENEW;
}


Eina_Bool
_del(void *data, int type, Ecore_Con_Event_Client_Del *ev)
{
   printf("Lost client with ip %s!\n", ecore_con_client_ip_get(ev->client));
   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_data(void *data, int type, Ecore_Con_Event_Client_Data *ev)
{
   char fmt[128];

   snprintf(fmt, sizeof(fmt),
            "Received %i bytes from client:\n"
            ">>>>>\n"
            "%%.%is\n"
            ">>>>>\n",
            ev->size, ev->size);

   printf(fmt, ev->data);
   return ECORE_CALLBACK_RENEW;
}
int main()
{
   eina_init();
   ecore_init();
   ecore_con_init();

/* uncomment if using gnutls
   gnutls_global_set_log_level(9);
   gnutls_global_set_log_function(tls_log_func);
*/


/* to use a PEM certificate with TLS and SSL3, uncomment the lines below */
/*
   if ((!ecore_con_client_ssl_cert_add("server.pem", NULL, "server.pem")))
     printf("Error loading certificate!\n");

   ecore_con_server_add(ECORE_CON_REMOTE_TCP | ECORE_CON_USE_TLS | ECORE_CON_USE_SSL3 | ECORE_CON_LOAD_CERT, "127.0.0.1", 8080, NULL);
*/

/* to use simple tcp without ssl/tls, use this line */
   ecore_con_server_add(ECORE_CON_REMOTE_TCP, "127.0.0.1", 8080, NULL);

/* set event handler for client connect */
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, (Ecore_Event_Handler_Cb)_add, NULL);
/* set event handler for client disconnect */
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, (Ecore_Event_Handler_Cb)_del, NULL);
/* set event handler for receiving client data */
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, (Ecore_Event_Handler_Cb)_data, NULL);

/* start server */
   ecore_main_loop_begin();
}

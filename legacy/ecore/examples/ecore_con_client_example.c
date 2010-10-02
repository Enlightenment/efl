#include <stdio.h>
#include <Ecore.h>
#include <Ecore_Con.h>

/* Ecore_Con client example
 * 2010 Mike Blumenkrantz
 */

/* comment if not using gnutls */
static void
tls_log_func(int level, const char *str)
{
   fprintf(stderr, "|<%d>| %s", level, str);
}

Eina_Bool
_add(void *data, int type, Ecore_Con_Event_Server_Add *ev)
{
   printf("Server with ip %s connected!\n", ecore_con_server_ip_get(ev->server));
   ecore_con_server_send(ev->server, "hello!", 6);
   ecore_con_server_flush(ev->server);

   return ECORE_CALLBACK_RENEW;
}


Eina_Bool
_del(void *data, int type, Ecore_Con_Event_Server_Del *ev)
{
   printf("Lost server with ip %s!\n", ecore_con_server_ip_get(ev->server));
   ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_data(void *data, int type, Ecore_Con_Event_Server_Data *ev)
{
   char fmt[128];

   snprintf(fmt, sizeof(fmt),
            "Received %i bytes from server:\n"
            ">>>>>\n"
            "%%.%is\n"
            ">>>>>\n",
            ev->size, ev->size);

   printf(fmt, ev->data);
   return ECORE_CALLBACK_RENEW;
}
int main()
{
   Ecore_Con_Server *svr;
   Eina_Iterator *it;
   const char *ca;

   eina_init();
   ecore_init();
   ecore_con_init();

/* comment if not using gnutls */
   gnutls_global_set_log_level(9);
   gnutls_global_set_log_function(tls_log_func);

   if (!(it = eina_file_ls("/etc/ssl/certs")))
     exit(1);

   if (!(svr = ecore_con_server_connect(ECORE_CON_REMOTE_TCP | ECORE_CON_USE_MIXED, "www.verisign.com", 443, NULL)))
     exit(1);

   /* add all the CAs */
   EINA_ITERATOR_FOREACH(it, ca)
     {
        if (!ecore_con_ssl_server_cafile_add(svr, ca))
          printf("Could not load CA: %s!\n", ca);
        eina_stringshare_del(ca);   
     }

   eina_iterator_free(it);
   ecore_con_ssl_server_verify(svr);

/* set event handler for server connect */
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD, (Ecore_Event_Handler_Cb)_add, NULL);
/* set event handler for server disconnect */
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL, (Ecore_Event_Handler_Cb)_del, NULL);
/* set event handler for receiving server data */
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA, (Ecore_Event_Handler_Cb)_data, NULL);

/* start client */
   ecore_main_loop_begin();
}

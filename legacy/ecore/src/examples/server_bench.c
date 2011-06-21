#include <stdio.h>
#include <Ecore.h>
#include <Ecore_Con.h>

/* Ecore_Con server example
 * 2010 Mike Blumenkrantz
 */

static Ecore_Con_Server *svr;
static int add;
static int del;

Eina_Bool
_add(void *data, int type, Ecore_Con_Event_Client_Add *ev)
{
   ++add;
//   printf ("%s ", ecore_con_client_ip_get(ev->client));
   printf("Client #%i!\n", add);
   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_del(void *data, int type, Ecore_Con_Event_Client_Del *ev)
{
   ++del;
   printf("Disconnected #%i!\n", del);
   if (add == del)
     ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

int main(int argc, const char *argv[])
{
   ecore_init();
   ecore_con_init();
   ecore_app_args_set(argc, argv);
   eina_log_domain_level_set("ecore_con", EINA_LOG_LEVEL_ERR);
   eina_log_domain_level_set("eina", EINA_LOG_LEVEL_ERR);


/* to use a PEM certificate with TLS and SSL3, uncomment the lines below */
//   if (!(svr = ecore_con_server_add(ECORE_CON_REMOTE_NODELAY | ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT, "127.0.0.1", 8080, NULL)))

/* to use simple tcp with ssl/tls, use this line */
   svr = ecore_con_server_add(ECORE_CON_REMOTE_NODELAY , "127.0.0.1", 8080, NULL);
   if (!svr)
     exit(1);
     
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, (Ecore_Event_Handler_Cb)_add, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, (Ecore_Event_Handler_Cb)_del, NULL);

/* start server */
   ecore_main_loop_begin();
   if (add && del)
     {
        printf("Restarting server after %i connections\n", add);
        add = del = 0;
        ecore_con_server_del(svr);
        ecore_app_restart();
     }
   return 0;
}

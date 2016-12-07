//Compile with:
// gcc -o ecore_con_server_example ecore_con_server_example.c `pkg-config --libs --cflags ecore ecore-con eina`

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include <Ecore_Getopt.h>

/* Ecore_Con server example
 * 2010 Mike Blumenkrantz
 */

static int retval = EXIT_SUCCESS;
static Eina_Bool echo = EINA_FALSE;
static Eina_Bool do_flush = EINA_FALSE;
static Eina_Bool single_message = EINA_FALSE;

Eina_Bool
_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Client_Add *ev = event;

   printf("INFO: client added %p: %s\n", ev->client, ecore_con_client_ip_get(ev->client));

   if (!echo)
     {
        ecore_con_client_send(ev->client, "Hello World!", strlen("Hello World!"));
        if (do_flush) ecore_con_client_flush(ev->client);
        if (single_message) ecore_con_client_del(ev->client);
     }

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Client_Del *ev = event;

   printf("INFO: client deleted %p: %s!\n", ev->client, ecore_con_client_ip_get(ev->client));
   ecore_con_client_del(ev->client);
   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_data(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Client_Data *ev = event;

   printf("INFO: client data %p: %s\n"
          "INFO:  - size: %d\n"
          "-- BEGIN DATA --\n",
          ev->client, ecore_con_client_ip_get(ev->client),
          ev->size);

   fwrite(ev->data, ev->size, 1, stdout);
   puts("-- END DATA --");

   if (echo)
     {
        ecore_con_client_send(ev->client, ev->data, ev->size);
        if (do_flush) ecore_con_client_flush(ev->client);
        if (single_message) ecore_con_client_del(ev->client);
     }

   return ECORE_CALLBACK_RENEW;
}


Eina_Bool
_write(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Client_Write *ev = event;
   printf("Sent %d bytes to client %s\n", ev->size, ecore_con_client_ip_get(ev->client));
   return ECORE_CALLBACK_RENEW;
}

static const char *types_strs[] = {
  "tcp",
  "udp",
  "ssl",
  "mcast",
  "local-user",
  "local-system",
  NULL
};

static const Ecore_Getopt options = {
  "ecore_con_server_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project; 2010 Mike Blumenkrantz", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of ecore_con_server_add() usage.\n",
  EINA_FALSE,
  {
    ECORE_GETOPT_CHOICE('t', "type", "Server type to use, defaults to 'tcp'", types_strs),

    ECORE_GETOPT_STORE_INT('l', "clients-limit",
                            "If set will limit number of clients to accept"),
    ECORE_GETOPT_STORE_TRUE('r', "clients-reject-excess",
                            "Immediately reject excess clients (over limit)"),

    ECORE_GETOPT_STORE_TRUE('e', "echo",
                            "Behave as 'echo' server, send back to client all the data receive"),
    ECORE_GETOPT_STORE_TRUE('f', "flush", "Force a flush after every send call."),
    ECORE_GETOPT_STORE_TRUE('m', "single-message", "Send a single message and delete the client."),

    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),

    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL, "The server name.", "name"),
    ECORE_GETOPT_STORE_METAVAR_INT(0, NULL, "The server port.", "port"),

    ECORE_GETOPT_SENTINEL
  }
};

int
main(int argc, char **argv)
{
   Ecore_Con_Type type;
   char *name = NULL;
   char *type_choice = NULL;
   int clients_limit = -1;
   int port = -1;
   Eina_Bool clients_reject_excess = EINA_FALSE;
   Eina_Bool quit_option = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(type_choice),

     ECORE_GETOPT_VALUE_INT(clients_limit),
     ECORE_GETOPT_VALUE_BOOL(clients_reject_excess),

     ECORE_GETOPT_VALUE_BOOL(echo),
     ECORE_GETOPT_VALUE_BOOL(do_flush),
     ECORE_GETOPT_VALUE_BOOL(single_message),

     /* standard block to provide version, copyright, license and help */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -V/--version quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -C/--copyright quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -L/--license quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -h/--help quits */

     /* positional argument */
     ECORE_GETOPT_VALUE_STR(name),
     ECORE_GETOPT_VALUE_INT(port),

     ECORE_GETOPT_VALUE_NONE /* sentinel */
   };
   int args;
   Ecore_Con_Server *svr;

   eina_init();
   ecore_init();
   ecore_con_init();

   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        fputs("ERROR: Could not parse command line options.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   if (quit_option) goto end;

   args = ecore_getopt_parse_positional(&options, values, argc, argv, args);
   if (args < 0)
     {
        fputs("ERROR: Could not parse positional arguments.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   if (!type_choice) type_choice = "tcp";

   if (strcmp(type_choice, "tcp") == 0)
     type = ECORE_CON_REMOTE_TCP;
   else if (strcmp(type_choice, "udp") == 0)
     type = ECORE_CON_REMOTE_UDP;
   else if (strcmp(type_choice, "ssl") == 0)
     type = ECORE_CON_REMOTE_TCP | ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT;
   else if (strcmp(type_choice, "local-user") == 0)
     type = ECORE_CON_LOCAL_USER;
   else if (strcmp(type_choice, "system") == 0)
     type = ECORE_CON_LOCAL_SYSTEM;
   else if (strcmp(type_choice, "mcast") == 0)
     type = ECORE_CON_REMOTE_MCAST;
   else
     {
        fprintf(stderr, "ERROR: unsupported --type/-t '%s'\n", type_choice);
        retval = EXIT_FAILURE;
        goto end;
     }

   svr = ecore_con_server_add(type, name, port, NULL);
   if (!svr) goto end;

   if (strcmp(type_choice, "ssl") == 0)
     {
        if (!ecore_con_ssl_server_cert_add(svr, "server.pem"))
          {
             fprintf(stderr, "ERROR: could not add cert: server.pem\n");
             goto no_mainloop;
          }
        if (!ecore_con_ssl_server_privkey_add(svr, "server.pem"))
          {
             fprintf(stderr, "ERROR: could not add privkey: server.pem\n");
             goto no_mainloop;
          }
     }

/* set event handler for client connect */
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, (Ecore_Event_Handler_Cb)_add, NULL);
/* set event handler for client disconnect */
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, (Ecore_Event_Handler_Cb)_del, NULL);
/* set event handler for receiving client data */
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, (Ecore_Event_Handler_Cb)_data, NULL);
/* set event handler that notifies of sent data */
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_WRITE, (Ecore_Event_Handler_Cb)_write, NULL);

/* start server */
   ecore_main_loop_begin();

 no_mainloop:
   ecore_con_server_del(svr);
   svr = NULL;

 end:
   ecore_con_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return retval;
}
